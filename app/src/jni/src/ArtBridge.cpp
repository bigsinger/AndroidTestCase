//
// Created by chaopei on 2016/5/22.
//

#include "ArtBridge.h"

#include "art/art.h"

ArtBridge *ArtBridge::sInstance = new ArtBridge;

bool ArtBridge::setup(JNIEnv *env, int apilevel) {
    mApiLevel = apilevel;
    int res = art_setup(env, apilevel);
    setStatus(res);
    return 0 == res;
}

void ArtBridge::setFieldFlag(JNIEnv *env, jobject field) {
    if (mApiLevel > 22) {
        setFieldFlag_6_0(env, field);
    } else if (mApiLevel > 21) {
        setFieldFlag_5_1(env, field);
    } else {
        setFieldFlag_5_0(env, field);
    }
}

void ArtBridge::applyPatch(JNIEnv *env, jobject src, jobject dest, Mode mode) {
    switch (mode) {
        case MODE_METHOD_REPLACE:
            if (mApiLevel > 22) {
                replace_6_0(env, src, dest);
            } else if (mApiLevel > 21) {
                replace_5_1(env, src, dest);
            } else {
                replace_5_0(env, src, dest);
            }
            break;
        case MODE_METHOD_DISPATCH_CPP:
            art_dispatch_6_0(env, src, dest, false);
            break;
        case MODE_METHOD_DISPATCH_JAVA:
            art_dispatch_6_0(env, src, dest, true);
            break;
        default:
            break;
    }
}
