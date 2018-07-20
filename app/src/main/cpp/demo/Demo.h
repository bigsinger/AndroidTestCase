#pragma once

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string>
#include <android/log.h>
#include "debug.h"
#include "TimeLog.h"

using namespace std;


void testDemo(JNIEnv *jni);
int regNativeMethods(JNIEnv *jni);


extern "C" {
    JNIEXPORT jstring   JNICALL getStr(JNIEnv *, jclass, jobject, jint, jstring);
    JNIEXPORT jint      JNICALL getInt(JNIEnv *, jclass, jobject, jint, jstring);
    JNIEXPORT jstring   JNICALL
    Java_com_bigsing_NativeHandler_getString(JNIEnv *, jclass, jobject, jint, jstring);
    JNIEXPORT jobject    JNICALL Jump(JNIEnv *, jclass, jint nMethodId, jint nRetType, jint nArgCount, jobjectArray arrArgs);
}

void dalvik_replace(Method *pSrc, Method *pDst);
void dalvik_replace(JNIEnv *jni, jobject src, jobject dest);
void *thread_fun(void *arg);