#include "dalvik_core.h"

void dalvik_replace(JNIEnv *env, jobject src, jobject dest) {
    Method *meth, *target;
    meth = (Method *) env->FromReflectedMethod(src);
    target = (Method *) env->FromReflectedMethod(dest);

    meth->accessFlags = target->accessFlags;
    meth->clazz = target->clazz;
    meth->fastJni = target->fastJni;
    meth->insns = target->insns;
    meth->insSize = target->insSize;
    meth->jniArgInfo = target->jniArgInfo;
    meth->methodIndex = target->methodIndex;
    meth->name = target->name;
    meth->nativeFunc = target->nativeFunc;
    meth->noRef = target->noRef;
    meth->outsSize = target->outsSize;
    meth->prototype = target->prototype;
    meth->registersSize = target->registersSize;
    meth->shorty = target->shorty;
}