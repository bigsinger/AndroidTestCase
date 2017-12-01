//
// Created by chaopei on 2016/5/22.
//

#ifndef HOTPATCH_DALVIKBRIDGE_H
#define HOTPATCH_DALVIKBRIDGE_H

#include "Bridge.h"
#include "dalvik/dalvik.h"

class DalvikBridge : public Bridge {
private:
    static DalvikBridge *sInstance;

    DalvikBridge() {}

public:
    static DalvikBridge *getInstance() {
        return sInstance;
    }

    virtual bool setup(JNIEnv *env, int apilevel);

    virtual void applyPatch(JNIEnv *env, jobject src, jobject dest, Mode mode);

    virtual void setFieldFlag(JNIEnv *env, jobject field);

};


#endif //HOTPATCH_DALVIKBRIDGE_H
