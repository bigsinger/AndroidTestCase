#include "art_5_0.h"
#include "art_5_1.h"
#include "art_6_0.h"

#include "art.h"

#include "../common.h"

artDeliverPendingExceptionFromCode_func artDeliverPendingExceptionFromCode_fnPtr;

QuickArgumentVisitor_constr QuickArgumentVisitor_constrPtr;

static void *art_dlsym(void *hand, const char *name) {
    void *ret = dlsym(hand, name);
    char msg[1024] = {0};
    snprintf(msg, sizeof(msg) - 1, "%p", ret);
#ifdef DEBUG
    LOGD("%s = %s\n", name, msg);
#endif
    return ret;
}

void throwNPE(JNIEnv *env, const char *msg) {
    LOGE("setup error: %s", msg);
//	env->ThrowNew(NPEClazz, msg);
}

jint __attribute__ ((visibility ("hidden"))) art_setup(
        JNIEnv *env, int apilevel) {
    int res = 0;

    void *art_hand = dlopen("libart.so", RTLD_NOW);
    if (art_hand) {
        artDeliverPendingExceptionFromCode_fnPtr = (artDeliverPendingExceptionFromCode_func) art_dlsym(
                art_hand,
                "artDeliverPendingExceptionFromCode");
        if (!artDeliverPendingExceptionFromCode_fnPtr) {
            throwNPE(env, "artDeliverPendingExceptionFromCode_fnPtr");
            res |= 0x40000;
        }
//        QuickArgumentVisitor_constrPtr = (QuickArgumentVisitor_constr) art_dlsym(art_hand,
//                                                                                 "_ZN3art20QuickArgumentVisitorC2EPNS_14StackReferenceINS_6mirror9ArtMethodEEEbPKcj");
//        if (!QuickArgumentVisitor_constrPtr) {
//            throwNPE(env, "QuickArgumentVisitor_constrPtr");
//            return JNI_FALSE;
//        }
    } else {
        res |= 0x20000;
    }
    return res;
}

extern "C" void artDeliverPendingExceptionFromCode(void *self) {
    LOGD("artDeliverPendingExceptionFromCode");
    artDeliverPendingExceptionFromCode_fnPtr(self);
}