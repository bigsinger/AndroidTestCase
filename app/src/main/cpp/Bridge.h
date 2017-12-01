//
// Created by chaopei on 2016/5/22.
//

#ifndef HOTPATCH_BRIDGE_H
#define HOTPATCH_BRIDGE_H

#include "common.h"

enum Mode {
    MODE_METHOD_DISPATCH_JAVA = 0, MODE_METHOD_DISPATCH_CPP, MODE_METHOD_REPLACE, MODE_CLASS
};

class Bridge {

private:
    int status;

public:

    void setStatus(int status) {
        this->status = status;
    }

    int getStatus() {
        return status;
    }

    virtual bool setup(JNIEnv *env, int apilevel) = 0;

    virtual void applyPatch(JNIEnv *env, jobject src, jobject dest, Mode mode) = 0;

    virtual void setFieldFlag(JNIEnv *env, jobject field) = 0;
};


#endif //HOTPATCH_BRIDGE_H
