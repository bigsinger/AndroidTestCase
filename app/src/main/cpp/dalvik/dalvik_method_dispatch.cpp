#include <cstdlib>
#include "dalvik_core.h"
#include "assert.h"

void dalvik_dispatch(JNIEnv *env, jobject srcMethod, jobject dstMethod, bool javaBridge,
                     const char *lpszMethodDesc) {
    Method *dest = NULL;
    Method *src = (Method *) env->FromReflectedMethod(srcMethod);

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
        HotFixInfo *info = new HotFixInfo(srcCopy, dest);

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
        HotFixInfo *info = new HotFixInfo(srcCopy, srcCopy);
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
            if (src->nativeFunc != (DalvikBridgeFunc) nativeFunc_dispatcher_only_log_call) {
                src->nativeFunc = (DalvikBridgeFunc) nativeFunc_dispatcher_only_log_call;
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

    HotFixInfo *info = (HotFixInfo *) method->insns;
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
        argArray = boxMethodArgs(dstMeth, args + 1);
        if (dvmCheckException_fnPtr(self))
            goto bail;
        LOGD("[dispatcher_cpp] : before dvmCallMethod_fnPtr");
        dvmCallMethod_fnPtr(self, (Method *) jInvokeMethod,
                            dvmCreateReflectMethodObject_fnPtr(dstMeth), &result, thisObj,
                            argArray);
        LOGD("[dispatcher_cpp] : after dvmCallMethod_fnPtr");
        thisObj->clazz = tmp;
    } else {
        argArray = boxMethodArgs(dstMeth, args);
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

static void nativeFunc_dispatcher_only_log_call(const u4 *args, jvalue *pResult, const Method *method, void *self) {
    ArrayObject *argArray = NULL;
    ClassObject *returnType = NULL;
    jvalue result = {0};

    HotFixInfo *info = (HotFixInfo *) method->insns;
    Method *pNowMethod = (Method *) method;
    Method *pOrignMethod = info->dest;
//    LOGD("[%s]: method on call: %s \nflag: %08X, nativeFunc: %p orign: %p flag: %08X, nativeFunc: %p |%s %s",
//         __FUNCTION__,
//         info->sMethodDesc.c_str(), method->accessFlags, method->nativeFunc,
//         pOrignMethod, pOrignMethod->accessFlags, pOrignMethod->nativeFunc, method->name,
//         method->shorty);

    //调用前恢复，否则会进入递归
    pNowMethod->jniArgInfo = pOrignMethod->jniArgInfo;
    pNowMethod->accessFlags = pOrignMethod->accessFlags;
    pNowMethod->registersSize = pOrignMethod->registersSize;
    pNowMethod->insns = pOrignMethod->insns;
    pNowMethod->nativeFunc = pOrignMethod->nativeFunc;

    returnType = dvmGetBoxedReturnType_fnPtr(method);
    if (returnType == NULL) {
        LOGE("returnType == NULL");
        assert(dvmCheckException_fnPtr(self));
        goto bail;
    }
    LOGD("	start call->");
    if (dvmIsStaticMethod(pOrignMethod) == false) {
        //非静态函数
        Object *thisObj = (Object *) args[0];
        thisObj->clazz = pOrignMethod->clazz;
        argArray = boxMethodArgs(pOrignMethod, args + 1);
        if (dvmCheckException_fnPtr(self)){
            LOGE("dvmCheckException_fnPtr failed");
            goto bail;
        }
        Object *pObj = dvmCreateReflectMethodObject_fnPtr(pOrignMethod);
        LOGD("[dispatcher_cpp]3 : before dvmCallMethod_fnPtr %s %p", method->name, pObj);
        dvmCallMethod_fnPtr(self, (Method *) jInvokeMethod, pObj, &result, thisObj, argArray);
        LOGD("[dispatcher_cpp]2 : after dvmCallMethod_fnPtr %s", method->name);
    } else {
        //静态函数
        argArray = boxMethodArgs(pOrignMethod, args);
        if (dvmCheckException_fnPtr(self)){
            LOGE("static dvmCheckException_fnPtr failed");
            goto bail;
        }

        LOGD("[dispatcher_cpp]4 : static before dvmCallMethod_fnPtr");
        dvmCallMethod_fnPtr(self, (Method *) jInvokeMethod,
                            dvmCreateReflectMethodObject_fnPtr(pOrignMethod), &result, NULL,
                            argArray);
        LOGD("[dispatcher_cpp]4 : static after dvmCallMethod_fnPtr result.l: %p", result.l);
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
        if (dvmUnboxPrimitive_fnPtr(result.l, returnType, pResult) == 0) {
            LOGE("dvmUnboxPrimitive_fnPtr failed");
            char msg[1024] = {0};
            snprintf(msg, sizeof(msg) - 1, "return error %s!=%s",
                     ((Object *) result.l)->clazz->descriptor, returnType->descriptor);
            jni_env->ThrowNew(CastEClazz, msg);
            goto bail;
        }
    }
    LOGD("[%s] : success %s", __FUNCTION__, info->sMethodDesc.c_str());
    bail:
    dvmReleaseTrackedAlloc_fnPtr((Object *) argArray, self);
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
        dvmCallMethod_fnPtr(self, bridgeHandleMethod, NULL, &result, NULL, (int) info->srcCopy,
                            dstMeth, thisObj, argArray);
        thisObj->clazz = tmp;
    } else {
        argArray = boxMethodArgs(dstMeth, args);
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