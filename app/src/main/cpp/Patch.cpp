#include <jni.h>
#include <stdio.h>

#include "ArtBridge.h"
#include "DalvikBridge.h"

#define JNIREG_CLASS "com/zero/bandaid/patch/Patch"

static Bridge *sBridgeImpl;

static bool setup(JNIEnv *env, jclass clazz, jboolean isart, jint apilevel) {
#ifdef DEBUG
    LOGD("[setup] : vm is: %s , apilevel is: %i", (isart ? "art" : "dalvik"), (int) apilevel);
#endif
    if (isart) {
        sBridgeImpl = ArtBridge::getInstance();
    } else {
        sBridgeImpl = DalvikBridge::getInstance();
    }
    return sBridgeImpl->setup(env, apilevel);
}

/**
 * JNI方法，替换某方法，src -> dest
 */
static void applyMethodPatch(JNIEnv *env, jclass clazz, jobject src, jobject dest, jint mode) {
#ifdef DEBUG
    LOGD("replaceMethod");
#endif
    if (sBridgeImpl) {
        sBridgeImpl->applyPatch(env, src, dest, (Mode) mode);
    }
}

static void setFieldFlag(JNIEnv *env, jclass clazz, jobject field) {
#ifdef DEBUG
    LOGD("setFieldFlag");
#endif
    if (sBridgeImpl) {
        sBridgeImpl->setFieldFlag(env, field);
    }
}


/*
 * JNI registration.
 */
static JNINativeMethod gMethods[] = {
/* name, signature, funcPtr */
        {"setupNative", "(ZI)Z",                        (bool *) setup},
        {"applyMethodPatchNative",
                        "(Ljava/lang/reflect/Method;Ljava/lang/reflect/Method;I)V",
                                                        (void *) applyMethodPatch},
        {"setFieldFlagNative",
                        "(Ljava/lang/reflect/Field;)V", (void *) setFieldFlag},};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv *env, const char *className,
                                 JNINativeMethod *gMethods, int numMethods) {
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Register native methods for all classes we know about.
 */
static int registerNatives(JNIEnv *env) {
    if (!registerNativeMethods(env, JNIREG_CLASS, gMethods,
                               sizeof(gMethods) / sizeof(gMethods[0])))
        return JNI_FALSE;

    return JNI_TRUE;
}

///*
// * Set some test stuff up.
// *
// * Returns the JNI version on success, -1 on failure.
// */
//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
//    JNIEnv *env = NULL;
//    jint result = -1;
//
//    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
//        return -1;
//    }
//    assert(env != NULL);
//
//    if (!registerNatives(env)) { //注册
//        return -1;
//    }
//    /* success -- return valid version number */
//    result = JNI_VERSION_1_4;
//
//    return result;
//}