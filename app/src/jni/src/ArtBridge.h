//
// Created by chaopei on 2016/5/22.
//

#ifndef HOTPATCH_ARTBRIDGE_H
#define HOTPATCH_ARTBRIDGE_H

#include "Bridge.h"

class ArtBridge : public Bridge{
private:
    static ArtBridge *sInstance;
    int mApiLevel;
public:
    static ArtBridge *getInstance() {
        return sInstance;
    }

    virtual bool setup(JNIEnv *env, int apilevel);

    virtual void setFieldFlag(JNIEnv *env, jobject field);

    virtual void applyPatch(JNIEnv *env, jobject src, jobject dest, Mode mode);
};


#endif //HOTPATCH_ARTBRIDGE_H
