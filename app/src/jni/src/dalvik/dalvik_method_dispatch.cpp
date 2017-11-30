#include "dalvik_core.h"
#include "assert.h"

void dalvik_dispatch(JNIEnv *env, jobject srcMethod, jobject dstMethod, bool javaBridge) {
	Method *src, *dest;
	jobject clazz = env->CallObjectMethod(dstMethod, jClassMethod);
	ClassObject *clz = (ClassObject *)dvmDecodeIndirectRef_fnPtr(dvmThreadSelf_fnPtr(), clazz);
	clz->status = CLASS_INITIALIZED;

	src = (Method *)env->FromReflectedMethod(srcMethod);
	dest = (Method *)env->FromReflectedMethod(dstMethod);

	LOGD("dalvik_dispatch: %s", src->name);

	src->jniArgInfo = 0x80000000;
	src->accessFlags |= ACC_NATIVE;

	int argsSize = dvmComputeMethodArgsSize_fnPtr(src);
	if (!dvmIsStaticMethod(src)) {
		argsSize++;
	}
	src->registersSize = src->insSize = argsSize;

	// 保存原方法
	Method *srcCopy = (Method *)malloc(sizeof(Method));
	memcpy(srcCopy, src, sizeof(Method));
	HotFixInfo *info = new HotFixInfo(srcCopy, dest);

	src->insns = (u2 *)info;

	if (javaBridge) {
		src->nativeFunc = (DalvikBridgeFunc)dispatcher_java;
	} else {
		src->nativeFunc = (DalvikBridgeFunc)dispatcher_cpp;
	}
}

bool dalvik_is_dispatched(JNIEnv *env, jobject src) {
    Method *meth = (Method *) env->FromReflectedMethod(src);
    bool isHookedToJava = meth->nativeFunc == (DalvikBridgeFunc)dispatcher_java;
    bool isHookedToCpp = meth->nativeFunc == (DalvikBridgeFunc)dispatcher_cpp;
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

	HotFixInfo *info = (HotFixInfo *)method->insns;
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
		Object *thisObj = (Object *)args[0];
		ClassObject *tmp = thisObj->clazz;
		thisObj->clazz = dstMeth->clazz;
		argArray = boxMethodArgs(dstMeth, args + 1);
		if (dvmCheckException_fnPtr(self))
			goto bail;
		LOGD("[dispatcher_cpp] : before dvmCallMethod_fnPtr");
		dvmCallMethod_fnPtr(self, (Method *)jInvokeMethod,
			dvmCreateReflectMethodObject_fnPtr(dstMeth), &result, thisObj,
			argArray);
		LOGD("[dispatcher_cpp] : after dvmCallMethod_fnPtr");
		thisObj->clazz = tmp;
	} else {
		argArray = boxMethodArgs(dstMeth, args);
		if (dvmCheckException_fnPtr(self))
			goto bail;

		dvmCallMethod_fnPtr(self, (Method *)jInvokeMethod,
			dvmCreateReflectMethodObject_fnPtr(dstMeth), &result, NULL,
			argArray);
	}
	if (dvmCheckException_fnPtr(self)) {
		Object *excep = dvmGetException_fnPtr(self);
		jni_env->Throw((jthrowable)excep);
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
			char msg[1024] = { 0 };
			snprintf(msg, sizeof(msg) - 1, "%s!=%s", ((Object *)result.l)->clazz->descriptor, returnType->descriptor);
			jni_env->ThrowNew(CastEClazz, msg);
			goto bail;
		}
	}
	LOGD("[dispatcher_cpp] : success");
bail:
	dvmReleaseTrackedAlloc_fnPtr((Object *)argArray, self);
}

static void dispatcher_java(const u4 *args, jvalue *pResult, const Method *method, void *self) {
    ClassObject *returnType;
    jvalue result;
    ArrayObject *argArray;
    HotFixInfo *info = (HotFixInfo *) method->insns;
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
        argArray = boxMethodArgs(dstMeth, args + 1);
        if (dvmCheckException_fnPtr(self))
            goto bail;
        dvmCallMethod_fnPtr(self, bridgeHandleMethod, NULL, &result, NULL, (int) info->srcCopy, dstMeth, thisObj, argArray);
        thisObj->clazz = tmp;
    } else {
        argArray = boxMethodArgs(dstMeth, args);
        if (dvmCheckException_fnPtr(self))
            goto bail;

        dvmCallMethod_fnPtr(self, bridgeHandleMethod, NULL, &result, NULL, (int) info->srcCopy, dstMeth, NULL, argArray);
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