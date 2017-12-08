#include <cstdlib>
#include "../Utils.h"
#include <algorithm>
#include "dalvik_core.h"
#include "assert.h"

void dalvik_dispatch(JNIEnv *env, jobject srcMethod, jobject dstMethod, bool javaBridge,
                     const char *lpszMethodDesc) {
    Method *dest = NULL;
    Method *src = (Method *) env->FromReflectedMethod(srcMethod);
    if(dalvik_is_dispatched(env, srcMethod)){
        LOGD("[*] method had been hooked");
        return;
    }

    if (src->accessFlags & ACC_ABSTRACT || src->accessFlags & ACC_CONSTRUCTOR) {
        //抽象方法不处理
        return;
    }
    if (src->accessFlags != ACC_PUBLIC/* && src->accessFlags != ACC_PRIVATE*/) {
        return;
    }

    if (dstMethod != NULL) {
        jobject clazz = env->CallObjectMethod(dstMethod, jClassMethod);
        ClassObject *clz = (ClassObject *) dvmDecodeIndirectRef_fnPtr(dvmThreadSelf_fnPtr(), clazz);
        clz->status = CLASS_INITIALIZED;
        dest = (Method *) env->FromReflectedMethod(dstMethod);

        src->jniArgInfo = 0x80000000;
        src->accessFlags |= ACC_NATIVE;

        int argsSize = dvmComputeMethodArgsSize_fnPtr(src);
        if (!dvmIsStaticMethod(src)) {
            argsSize++;
        }
        src->registersSize = src->insSize = argsSize;

        // 保存原方法
        Method *srcCopy = (Method *) malloc(sizeof(Method));
        memcpy(srcCopy, src, sizeof(Method));
        HookInfo *info = new HookInfo(srcCopy, dest);

        src->insns = (u2 *) info;

        if (javaBridge) {
            src->nativeFunc = (DalvikBridgeFunc) dispatcher_java;
        } else {
            src->nativeFunc = (DalvikBridgeFunc) dispatcher_cpp;
        }
    }
}

bool dalvik_is_dispatched(JNIEnv *env, jobject src) {
    Method *meth = (Method *) env->FromReflectedMethod(src);
    bool isHookedToJava = meth->nativeFunc == (DalvikBridgeFunc) dispatcher_java;
    bool isHookedToCpp = meth->nativeFunc == (DalvikBridgeFunc) dispatcher_cpp;
#ifdef DEBUG
    LOGD("dalvik_is_hooked: isHookedToJava=%b", isHookedToJava);
    LOGD("dalvik_is_hooked: isHookedToCpp=%b", isHookedToCpp);
#endif
    return isHookedToCpp || isHookedToJava;
}

static void dispatcher_cpp(const u4 *args, jvalue *pResult, const Method *method, void *self) {
    ClassObject *returnType;
    jvalue result;
    ArrayObject *argArray;

    LOGD("[dispatcher_cpp] : source method: %s %s", method->name, method->shorty);

    HookInfo *info = (HookInfo *) method->insns;
    Method *dstMeth = info->dest;
    dstMeth->accessFlags = dstMeth->accessFlags | ACC_PUBLIC;

    LOGD("[dispatcher_cpp] : target method: %s %s", dstMeth->name, dstMeth->shorty);

    returnType = dvmGetBoxedReturnType_fnPtr(method);
    if (returnType == NULL) {
        assert(dvmCheckException_fnPtr(self));
        goto bail;
    }
    LOGD("[dispatcher_cpp] : start call->");
    if (!dvmIsStaticMethod(dstMeth)) {
        Object *thisObj = (Object *) args[0];
        ClassObject *tmp = thisObj->clazz;
        thisObj->clazz = dstMeth->clazz;
        argArray = dvmBoxMethodArgs(dstMeth, args + 1);
        if (dvmCheckException_fnPtr(self))
            goto bail;
        LOGD("[dispatcher_cpp] : before dvmCallMethod_fnPtr");
        dvmCallMethod_fnPtr(self, (Method *) jInvokeMethod,
                            dvmCreateReflectMethodObject_fnPtr(dstMeth), &result, thisObj,
                            argArray);
        LOGD("[dispatcher_cpp] : after dvmCallMethod_fnPtr");
        thisObj->clazz = tmp;
    } else {
        argArray = dvmBoxMethodArgs(dstMeth, args);
        if (dvmCheckException_fnPtr(self))
            goto bail;

        dvmCallMethod_fnPtr(self, (Method *) jInvokeMethod,
                            dvmCreateReflectMethodObject_fnPtr(dstMeth), &result, NULL,
                            argArray);
    }
    if (dvmCheckException_fnPtr(self)) {
        Object *excep = dvmGetException_fnPtr(self);
        jni_env->Throw((jthrowable) excep);
        goto bail;
    }

    if (returnType->primitiveType == PRIM_VOID) {
        LOGD("[dispatcher_cpp] : +++ ignoring return to void");
    } else if (result.l == NULL) {
        if (dvmIsPrimitiveClass(returnType)) {
            jni_env->ThrowNew(NPEClazz, "null result when primitive expected");
            goto bail;
        }
        pResult->l = NULL;
    } else {
        if (!dvmUnboxPrimitive_fnPtr(result.l, returnType, pResult)) {
            char msg[1024] = {0};
            snprintf(msg, sizeof(msg) - 1, "%s!=%s", ((Object *) result.l)->clazz->descriptor,
                     returnType->descriptor);
            jni_env->ThrowNew(CastEClazz, msg);
            goto bail;
        }
    }
    LOGD("[dispatcher_cpp] : success");
    bail:
    dvmReleaseTrackedAlloc_fnPtr((Object *) argArray, self);
}

jclass dvmFindJNIClass(JNIEnv *env, const char *classDesc) {
	jclass classObj = env->FindClass(classDesc);
    if(env->ExceptionCheck() == JNI_TRUE){
        env->ExceptionClear();
    }

	if (classObj == NULL) {
		jclass clazzApplicationLoaders = env->FindClass("android/app/ApplicationLoaders");
		ASSERT(clazzApplicationLoaders);

		jfieldID fieldApplicationLoaders = env->GetStaticFieldID(
			clazzApplicationLoaders, "gApplicationLoaders",
			"Landroid/app/ApplicationLoaders;");
		ASSERT(fieldApplicationLoaders);

		jobject objApplicationLoaders = env->GetStaticObjectField(
			clazzApplicationLoaders, fieldApplicationLoaders);
		ASSERT(objApplicationLoaders);

		jfieldID fieldLoaders = env->GetFieldID(clazzApplicationLoaders, "mLoaders", "Ljava/util/Map;");
        if (fieldLoaders == NULL) {
            if(env->ExceptionCheck() == JNI_TRUE){
                env->ExceptionClear();
            }
            return NULL;
        }
		//ASSERT(fieldLoaders);

		jobject objLoaders = env->GetObjectField(objApplicationLoaders,
			fieldLoaders);
		ASSERT(objLoaders);

		jclass clazzHashMap = env->GetObjectClass(objLoaders);
		static jmethodID methodValues = env->GetMethodID(clazzHashMap, "values",
			"()Ljava/util/Collection;");
		jobject values = env->CallObjectMethod(objLoaders, methodValues);
		jclass clazzValues = env->GetObjectClass(values);
		static jmethodID methodToArray = env->GetMethodID(clazzValues,
			"toArray", "()[Ljava/lang/Object;");
		jobjectArray classLoaders = (jobjectArray)env->CallObjectMethod(values,
			methodToArray);

		int size = env->GetArrayLength(classLoaders);
		jstring param = env->NewStringUTF(classDesc);

		for (int i = 0; i < size; i++) {
			jobject classLoader = env->GetObjectArrayElement(classLoaders, i);
			jclass clazzCL = env->GetObjectClass(classLoader);
			static jmethodID loadClass = env->GetMethodID(clazzCL, "loadClass",
				"(Ljava/lang/String;)Ljava/lang/Class;");
			classObj = (jclass)env->CallObjectMethod(classLoader, loadClass,
				param);

			if (classObj != NULL) {
				break;
			}
		}
	}
	//局部变量返回，直接返回值使用会报错JNI ERROR (app bug): attempt to use stale local reference 0xfbc00025
	return (jclass)env->NewGlobalRef(classObj);
}

static ClassObject* dvmFindClass(const char *classDesc){
    //JNIEnv *env = AndroidRuntime_getJNIEnv_fnPtr();//AndroidRuntime::getJNIEnv();
    JNIEnv *env = NULL;
    Utils::getenv(&env);
    assert(env != NULL);

    char *newclassDesc = dvmDescriptorToName_fnPtr(classDesc);

    jclass jnicls = dvmFindJNIClass(env, newclassDesc);
    if (jnicls) {
        ClassObject *res = jnicls ? static_cast<ClassObject*>(dvmDecodeIndirectRef_fnPtr(dvmThreadSelf_fnPtr(), jnicls)) : NULL;
        env->DeleteGlobalRef(jnicls);
        free(newclassDesc);
        return res;
    }

    return NULL;
}

ArrayObject* dvmGetMethodParamTypes(const Method* method, const char* methodsig){
    /* count args */
    size_t argCount = dexProtoGetParameterCount_fnPtr(&method->prototype);

    /* allocate storage */
    ArrayObject* argTypes = dvmAllocArrayByClass_fnPtr(classJavaLangObjectArray, argCount, ALLOC_DEFAULT);
    if(argTypes == NULL){
        return NULL;
    }

    Object** argObjects = (Object**) argTypes->contents;
    const char *pPosTemp = strchr(methodsig, '(');
    const char *desc = (const char *) (pPosTemp + 1);
    if (pPosTemp == NULL) {
        LOGE("[%s] error sig is wrong: %s", __FUNCTION__, methodsig);
        return NULL;
    }

    /*
     * Fill in the array.
     */
    size_t desc_index = 0;
    size_t arg_index = 0;
    bool isArray = false;
    char descChar = desc[desc_index];

    while (descChar != ')') {

        switch (descChar) {
            case 'Z':
            case 'C':
            case 'F':
            case 'B':
            case 'S':
            case 'I':
            case 'D':
            case 'J':
                if(!isArray){
                    argObjects[arg_index++] = (Object*)dvmFindPrimitiveClass_fnPtr(descChar);
                }else{
                    char buf[3] = {0};
                    memcpy(buf, desc + desc_index - 1, 2);
                    argObjects[arg_index++] = dvmFindSystemClass_fnPtr(buf);
                }

                isArray = false;
                desc_index++;
                break;

            case '[':
                isArray = true;
                desc_index++;
                break;

            case 'L':
                int s_pos = desc_index, e_pos = desc_index;
                while(desc[++e_pos] != ';');
                s_pos = isArray ? s_pos - 1 : s_pos;
                isArray = false;

                size_t len = e_pos - s_pos + 1;
                char buf[128] = { 0 };
                memcpy((void *)buf, (const void *)(desc + s_pos), len);
                argObjects[arg_index++] = dvmFindClass(buf);
                desc_index = e_pos + 1;
                break;
        }

        descChar = desc[desc_index];
    }

    return argTypes;
}

//JavaVM *g
//Method* dvmGetMethodFromReflectObj(Object* obj)
//{
//    ClassObject *clazz = (ClassObject*)dvmGetFieldObject(obj, gDvm.offJavaLangReflectMethod_declClass);
//
//    int slot = dvmGetFieldInt(obj, gDvm.offJavaLangReflectMethod_slot);
//
//    return dvmSlotToMethod(clazz, slot);
//}


static void dispatcher_cppex(const u4 *args, jvalue *pResult, const Method *method, void *self) {
    ClassObject *returnType;
    jvalue result;
    ArrayObject *argArray = NULL;

    HookInfo *info = (HookInfo *) method->insns;
    Method* originalMethod = (Method*)(info->originalMethod);

    LOGD("[dispatcher_cpp] : originalMethod: %s %s", originalMethod->name, originalMethod->shorty);

    returnType = dvmGetBoxedReturnType_fnPtr(originalMethod);
    if (returnType == NULL) {
        assert(dvmCheckException_fnPtr(self));
        goto bail;
    }
    LOGD("[dispatcher_cpp] : start call->");
    if (!dvmIsStaticMethod(originalMethod)) {
        Object *thisObj = (Object *) args[0];
        argArray = dvmBoxMethodArgs(originalMethod, args + 1);
        if (dvmCheckException_fnPtr(self))
            goto bail;
        Object *pMethodObject = dvmCreateReflectMethodObject_fnPtr(originalMethod);
        LOGD("[dispatcher_cpp] : before dvmCallMethod_fnPtr2 %p %p %p %p", dvmCallMethod_fnPtr, pMethodObject,
             thisObj, argArray );
        dvmCallMethod_fnPtr(self, (Method *) jInvokeMethod,
                            pMethodObject, &result, thisObj,
                            argArray);
        LOGD("[dispatcher_cpp] : after dvmCallMethod_fnPtr");
    } else {
        argArray = dvmBoxMethodArgs(originalMethod, args);
        if (dvmCheckException_fnPtr(self))
            goto bail;

        dvmCallMethod_fnPtr(self, (Method *) jInvokeMethod,
                            dvmCreateReflectMethodObject_fnPtr(originalMethod), &result, NULL,
                            argArray);
    }
    if (dvmCheckException_fnPtr(self)) {
        Object *excep = dvmGetException_fnPtr(self);
        jni_env->Throw((jthrowable) excep);
        goto bail;
    }

    if (returnType->primitiveType == PRIM_VOID) {
        LOGD("[dispatcher_cpp] : +++ ignoring return to void");
    } else if (result.l == NULL) {
        if (dvmIsPrimitiveClass(returnType)) {
            jni_env->ThrowNew(NPEClazz, "null result when primitive expected");
            goto bail;
        }
        pResult->l = NULL;
    } else {
        if (!dvmUnboxPrimitive_fnPtr(result.l, returnType, pResult)) {
            char msg[1024] = {0};
            snprintf(msg, sizeof(msg) - 1, "%s!=%s", ((Object *) result.l)->clazz->descriptor,
                     returnType->descriptor);
            jni_env->ThrowNew(CastEClazz, msg);
            goto bail;
        }
    }
    LOGD("[dispatcher_cpp] : success");
    bail:
    dvmReleaseTrackedAlloc_fnPtr((Object *) argArray, self);
}

static void dispatcher_java(const u4 *args, jvalue *pResult, const Method *method, void *self) {
    ClassObject *returnType;
    jvalue result;
    ArrayObject *argArray;
    HookInfo *info = (HookInfo *) method->insns;
    Method *dstMeth = info->dest;
    dstMeth->accessFlags = dstMeth->accessFlags | ACC_PUBLIC;
    returnType = dvmGetBoxedReturnType_fnPtr(method);
    if (returnType == NULL) {
        assert(dvmCheckException_fnPtr(self));
        goto bail;
    }
    if (!dvmIsStaticMethod(dstMeth)) {
        Object *thisObj = (Object *) args[0];
        ClassObject *tmp = thisObj->clazz;
        thisObj->clazz = dstMeth->clazz;
        argArray = dvmBoxMethodArgs(dstMeth, args + 1);
        if (dvmCheckException_fnPtr(self))
            goto bail;
        dvmCallMethod_fnPtr(self, bridgeHandleMethod, NULL, &result, NULL, (int) info->srcCopy,
                            dstMeth, thisObj, argArray);
        thisObj->clazz = tmp;
    } else {
        argArray = dvmBoxMethodArgs(dstMeth, args);
        if (dvmCheckException_fnPtr(self))
            goto bail;

        dvmCallMethod_fnPtr(self, bridgeHandleMethod, NULL, &result, NULL, (int) info->srcCopy,
                            dstMeth, NULL, argArray);
    }


    if (dvmCheckException_fnPtr(self)) {
        Object *excep = dvmGetException_fnPtr(self);
        jni_env->Throw((jthrowable) excep);
        goto bail;
    }

    if (returnType->primitiveType == PRIM_VOID) {
        LOGD("[dispatcher_java] : +++ ignoring return to void");
    } else if (result.l == NULL) {
        if (dvmIsPrimitiveClass(returnType)) {
            jni_env->ThrowNew(NPEClazz, "null result when primitive expected");
            goto bail;
        }
        pResult->l = NULL;
    } else {
        if (!dvmUnboxPrimitive_fnPtr(result.l, returnType, pResult)) {
            char msg[1024] = {0};
            snprintf(msg, sizeof(msg) - 1, "%s!=%s",
                     ((Object *) result.l)->clazz->descriptor,
                     returnType->descriptor);
            jni_env->ThrowNew(CastEClazz, msg);
            goto bail;
        }
    }
    bail:
    dvmReleaseTrackedAlloc_fnPtr((Object *) argArray, self);
}