#include <cstdlib>
#include "..\Utils.h"
#include <algorithm>
#include "dalvik_core.h"
#include "assert.h"

void dalvik_dispatch(JNIEnv *env, jobject srcMethod, jobject dstMethod, bool javaBridge,
                     const char *lpszMethodDesc) {
    Method *dest = NULL;
    Method *src = (Method *) env->FromReflectedMethod(srcMethod);
    if(src->nativeFunc == (DalvikBridgeFunc) nativeFunc_logMethodCall){
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
    } else {
        //没有设置替换的目标，仅仅显示一个调用日志。保存原始方法的副本后再做修改。
        Method *srcCopy = (Method *) malloc(sizeof(Method));
        memcpy(srcCopy, src, sizeof(Method));
        HookInfo *info = new HookInfo(srcCopy, srcCopy);
        info->sMethodDesc = lpszMethodDesc;

        src->jniArgInfo = 0x80000000;
        src->accessFlags |= ACC_NATIVE;
        int argsSize = dvmComputeMethodArgsSize_fnPtr(src);
        if (!dvmIsStaticMethod(src)) {
            argsSize++;
        }
        src->registersSize = src->insSize = argsSize;
        src->insns = (u2 *) info;

        if (javaBridge) {
            src->nativeFunc = (DalvikBridgeFunc) dispatcher_java;
        } else {
            if (src->nativeFunc != (DalvikBridgeFunc) nativeFunc_logMethodCall) {
                src->nativeFunc = (DalvikBridgeFunc) nativeFunc_logMethodCall;
            }
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

static jclass dvmFindJNIClass(JNIEnv *env, const char *classDesc) {
	jclass classObj = env->FindClass(classDesc);
    if(env->ExceptionCheck() == JNI_TRUE){
        env->ExceptionClear();
    }

	if (classObj == NULL) {
		jclass clazzApplicationLoaders = env->FindClass(
			"android/app/ApplicationLoaders");
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

static ArrayObject* dvmGetMethodParamTypes(const Method* method, const char* methodsig){
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

 bool dalvik_hook_java_method(JNIEnv *jni, jclass cls, jobject srcMethod, const char*szClassName, const char*szMethodName,
                             const char*szSig, const char*szDesc)
{
    LOGD("[%s] begin1", __FUNCTION__);
    Method *method = NULL;
    method = (Method *) jni->FromReflectedMethod(srcMethod);
    return true;
    if (method == NULL) {
        LOGE("[dalvik_hook_java_method] method == NULL %s::%s", szClassName, szMethodName);
        if(jni->ExceptionCheck() == JNI_TRUE){
            LOGE("[dalvik_hook_java_method] method == NULL exception %s::%s", szClassName, szMethodName);
            jni->ExceptionClear();
        }
        return false;
//		jmethodID mid = jni->GetMethodID(cls, szMethodName, szSig);
//        method = (Method *) mid;
//        if (method == NULL) {
//            LOGE("[dalvik_hook_java_method] method == NULL again %s::%s", szClassName, szMethodName);
//            return false;
//        }
    }
    if(method->nativeFunc == (DalvikBridgeFunc) nativeFunc_logMethodCall){
        LOGD("[dalvik_hook_java_method] method had been hooked");
        return true;
    }else{
        return true;
    }

    if (method->accessFlags & ACC_ABSTRACT || method->accessFlags & ACC_CONSTRUCTOR||
            method->accessFlags & ACC_NATIVE||
            method->accessFlags & ACC_INTERFACE||
            method->accessFlags & ACC_ANNOTATION) {
        //抽象方法不处理
        return true;
    }
//    if (method->accessFlags != ACC_PUBLIC && method->accessFlags != ACC_PRIVATE) {
//        return;
//    }

    //没有设置替换的目标，仅仅显示一个调用日志。保存原始方法的副本后再做修改。
    Method* bakMethod = (Method*) malloc(sizeof(Method));
    memcpy(bakMethod, method, sizeof(Method));

    // init info
    HookInfo *info = new HookInfo;
    info->env = jni;
    info->originalMethod = bakMethod;
    info->sMethodSig = szSig;
    info->sClassName = szClassName;
    info->sMethodName = szMethodName;
    info->sMethodDesc = szDesc;

    int argsSize = dvmComputeMethodArgsSize_fnPtr(method);
    if (dvmIsStaticMethod(method)== false) {
        argsSize++;
    }
    method->accessFlags |= ACC_NATIVE;
    method->registersSize = method->insSize = argsSize;
    method->outsSize = 0;
    //method->jniArgInfo = dvmComputeJniArgInfo(method->shorty);
    method->insns = (u2 *) info;
    method->nativeFunc = (DalvikBridgeFunc) nativeFunc_logMethodCall;

     LOGD("[%s] end", __FUNCTION__);
     return true;
}

//ref : Hook Java中返回值问题的解决和修改https://github.com/Harold1994/program_total/blob/7659177e1d562a8396d0ee9c9eda9f36a12727e5/program_total/3%E6%B3%A8%E5%85%A5%E7%9A%84so%E6%96%87%E4%BB%B6/documents/HookJava%E4%B8%AD%E8%BF%94%E5%9B%9E%E5%80%BC%E9%97%AE%E9%A2%98%E7%9A%84%E8%A7%A3%E5%86%B3%E5%92%8C%E4%BF%AE%E6%94%B9.md
static void nativeFunc_logMethodCall(const u4 *args, JValue *pResult, const Method *method,
                                     void *self) {
    Object *result = NULL;
    HookInfo* info = (HookInfo*)method->insns; //get hookinfo pointer from method-insns
    LOGD("nativeFunc_logMethodCall: %s::%s %s", info->sClassName.c_str(), info->sMethodName.c_str(), info->sMethodSig.c_str());

    Method* originalMethod = (Method*)(info->originalMethod);
    if (info->returnType == NULL) {
        info->returnType = dvmGetBoxedReturnType_fnPtr(originalMethod);
        if (info->returnType == NULL) {
            LOGE("nativeFunc_logMethodCall error 1");
            if (dvmCheckException_fnPtr(self)) {
                LOGE("nativeFunc_logMethodCall error 1 Exception");
            }
        }
    }
    //在替换方法的时候可能有些参数类型并没有被加载呢，所以要在调用的时候获取。
    if (info->paramTypes == NULL) {
        info->paramTypes = dvmGetMethodParamTypes(originalMethod, info->sMethodSig.c_str());
        if (info->paramTypes == NULL) {
            LOGE("nativeFunc_logMethodCall error 4");
            if (dvmCheckException_fnPtr(self)) {
                LOGE("nativeFunc_logMethodCall error 4 Exception");
                Object *excep = dvmGetException_fnPtr(self);
                jni_env->Throw((jthrowable) excep);
            }
        }
    }
    ArrayObject *argArr = NULL;
    if (dvmIsStaticMethod(originalMethod)== false) {
        Object* thisObject = (Object*)args[0];
        argArr = dvmBoxMethodArgs(originalMethod, args + 1);
        result = dvmInvokeMethod_fnPtr(thisObject, originalMethod, argArr, info->paramTypes, info->returnType, true);
    }else{
        argArr = dvmBoxMethodArgs(originalMethod, args);
        result = dvmInvokeMethod_fnPtr(NULL, originalMethod, argArr, info->paramTypes, info->returnType, true);
    }

    if (dvmCheckException_fnPtr(self)) {
        LOGE("nativeFunc_logMethodCall error 3 on invoke");
        //Object *excep = dvmGetException_fnPtr(self);
        //jni_env->Throw((jthrowable) excep);
        jni_env->ExceptionClear();
    }
    dvmReleaseTrackedAlloc_fnPtr((Object *)argArr, self);

    if (result != NULL) {

        switch (info->returnType->primitiveType) {
            case PRIM_NOT: {
                pResult->l = result;
                LOGD("nativeFunc_logMethodCall: %s::%s ret object %p", info->sClassName.c_str(),
                     info->sMethodName.c_str(), result);

//                JNIEnv *env = NULL;
//                Utils::getenv(&env);
//
//                jobject* i = reinterpret_cast<jobject*>(&result[1]);
//                jobject jobj = (jobject)*i;
//
//
//                char *newclassDesc = dvmDescriptorToName_fnPtr(info->returnType->descriptor);
//                jclass retClass = env->FindClass(newclassDesc);
//                info->toStringMethod = env->GetMethodID(retClass, "toString", "()Ljava/lang/String;");
//                free(newclassDesc);
//
//                jstring msg = (jstring) env->CallObjectMethod(jobj, info->toStringMethod);
//                env->DeleteLocalRef(retClass);
//                const char *szMsg = env->GetStringUTFChars(msg, 0);
//                LOGD("nativeFunc_logMethodCall: ret object %s", szMsg);
//                env->ReleaseStringUTFChars(msg, szMsg);
//                env->DeleteLocalRef(msg);
            }
                break;
            case PRIM_VOID: {
                pResult->l = NULL;
                LOGD("nativeFunc_logMethodCall: %s::%s ret void", info->sClassName.c_str(),
                     info->sMethodName.c_str());
            }
                break;
            case PRIM_BOOLEAN: {
                unsigned char *i = reinterpret_cast<unsigned char *>(&result[1]);
                pResult->z = *i;
                LOGD("nativeFunc_logMethodCall: %s::%s ret bool %d", info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->z);
            }
                break;
            case PRIM_BYTE: {
                signed char *i = reinterpret_cast<signed char *>(&result[1]);
                pResult->b = *i;
                LOGD("nativeFunc_logMethodCall: %s::%s ret byte %c", info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->b);
            }
                break;
            case PRIM_CHAR: {
                unsigned short *i = reinterpret_cast<unsigned short *>(&result[1]);
                pResult->c = *i;
                LOGD("nativeFunc_logMethodCall: %s::%s ret char %c", info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->c);
            }
                break;
            case PRIM_SHORT: {
                signed short *i = reinterpret_cast<signed short *>(&result[1]);
                pResult->s = *i;
                LOGD("nativeFunc_logMethodCall: %s::%s ret byte %c", info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->s);
            }
                break;
            case PRIM_INT: {
                if (result != NULL) {
                    int *i = reinterpret_cast<int *>(&result[1]);
                    pResult->i = *i;
                    LOGD("nativeFunc_logMethodCall: %s::%s ret int %d", info->sClassName.c_str(),
                         info->sMethodName.c_str(), pResult->i);
                } else {
                    pResult->l = result;
                    LOGE("nativeFunc_logMethodCall: %s::%s ret int , but NULL returned",
                         info->sClassName.c_str(), info->sMethodName.c_str());
                }
            }
                break;
            case PRIM_LONG: {
                long *i = reinterpret_cast<long *>(&result[1]);
                pResult->j = *i;
                LOGD("nativeFunc_logMethodCall: %s::%s ret long %d", info->sClassName.c_str(),
                     info->sMethodName.c_str(), (int) pResult->j);
            }
                break;
            case PRIM_FLOAT: {
                float *i = reinterpret_cast<float *>(&result[1]);
                pResult->f = *i;
                LOGD("nativeFunc_logMethodCall: %s::%s ret fload %f", info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->f);
            }
                break;
            case PRIM_DOUBLE: {
                double *i = reinterpret_cast<double *>(&result[1]);
                pResult->d = *i;
                LOGD("nativeFunc_logMethodCall: %s::%s ret double %f", info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->d);
            }
                break;
            default: {
                pResult->l = (void *) result;
                LOGD("nativeFunc_logMethodCall: %s::%s ret unkonw type", info->sClassName.c_str(),
                     info->sMethodName.c_str());
            }
                break;
        }
    }else {
        pResult->l = (void *) result;
        LOGD("nativeFunc_logMethodCall: %s::%s ret NULL", info->sClassName.c_str(),
             info->sMethodName.c_str());
    }
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