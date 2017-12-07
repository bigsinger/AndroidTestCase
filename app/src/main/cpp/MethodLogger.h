#pragma once

#include <jni.h>

class CMethodLogger {
public:
    CMethodLogger();

    ~CMethodLogger();

public:
    static bool start(JNIEnv *jni);
};

