/*

*/

#include <jni.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string>
#include <android/log.h>
#include <algorithm>

using namespace std;

#include "debug.h"
#include "Constant.h"
#include "Utils.h"
#include "substrate/substrate.h"
#include "dalvik/object.h"
#include "dalvik/dalvik_core.h"

void *thread_fun(void *arg);

extern "C" {
JNIEXPORT jstring    JNICALL getStr(JNIEnv *, jclass, jobject, jint, jstring);
JNIEXPORT jint        JNICALL getInt(JNIEnv *, jclass, jobject, jint, jstring);
JNIEXPORT jstring    JNICALL
Java_com_bigsing_NativeHandler_getString(JNIEnv *, jclass, jobject, jint, jstring);
JNIEXPORT jobject    JNICALL Jump(JNIEnv *, jclass, jint nMethodId, jobject objArgs...);
}


//该函数不需要动态注册
JNIEXPORT jstring JNICALL
Java_com_bigsing_NativeHandler_getString(JNIEnv *env, jclass arg, jobject jCtxObj, jint paramInt,
                                         jstring paramStr) {
    return getStr(env, arg, jCtxObj, paramInt, paramStr);
}


/*
**jobject	jCtxObj：	传入的Context 
**jint		paramInt：	作为命令ID，不同的命令ID执行不同的功能
**string	paramStr：	参数字符串

返回值：Java层返回string
*/
JNIEXPORT jstring JNICALL
getStr(JNIEnv *env, jclass clsJavaJNI, jobject jCtxObj, jint paramInt, jstring paramStr) {
    jstring jstrResult = NULL;

    switch (paramInt) {
        case CMD_INIT: {
            //保存JVM
            JavaVM *vm = NULL;
            env->GetJavaVM(&vm);
            Utils::setJavaVM(vm);
            Utils::setContext(env->NewGlobalRef(jCtxObj));
            Utils::setJavaJNIClass((jclass) env->NewGlobalRef(clsJavaJNI));

            LOGD("newThread begin");
            pthread_t pt;
            pthread_create(&pt, NULL, &thread_fun, (void *) paramStr);
            std::string s = Utils::fmt("env: %p, jCtxObj: %p, g_jvm: %p, g_obj: %p", env, jCtxObj,
                                       Utils::getJavaVM(), Utils::getContext());
            jstrResult = env->NewStringUTF(s.c_str());
        }
            break;
        case CMD_GET_TEST_STR: {
            LOGD("[%s] CMD_GET_TEST_STR\n", __FUNCTION__);
            std::string s = Utils::fmt("%s Hello Android Native!",
                                       Utils::GetCurrentTimeStr().c_str());
            jstrResult = env->NewStringUTF(s.c_str());
        }
            break;
        case CMD_GET_MAC: {
            string strMacs = Utils::getMacs();
            jstrResult = Utils::str2jstr(env, strMacs);
        }
            break;
        case CMD_GET_FILE_TEXT: {
            string strText;
            string sFileName = Utils::jstr2str(env, paramStr);
            LOGD("[%s] CMD_GET_FILE_TEXT readTextFile: %s", __FUNCTION__, sFileName.c_str());
            if (Utils::readTextFile(sFileName.c_str(), strText) == true) {
                jstrResult = Utils::str2jstr(env, strText);
            }
        }
            break;
        case CMD_GET_PPID:
            break;
        default:
            break;
    }

    return jstrResult;
}

/*
**jobject	jCtxObj：	传入的Context 
**jint		paramInt：	作为命令ID，不同的命令ID执行不同的功能
**string	paramStr：	参数字符串

返回值：Java层返回int
*/
JNIEXPORT jint JNICALL
getInt(JNIEnv *env, jclass arg, jobject jCtxObj, jint paramInt, jstring paramStr) {
    int result = -1;

    switch (paramInt) {
        case CMD_GET_PPID:
            result = getppid();
            break;
        default:
            break;
    }

    return result;
}

JNIEXPORT jobject    JNICALL Jump(JNIEnv *env, jclass, jint nMethodId, jobject objArgs...) {
    LOGD("[%s] MethodId: %d", __FUNCTION__, nMethodId);
    jobject result = NULL;
    va_list args;
    va_start(args, objArgs);

    switch (int(nMethodId)) {
        case 100: {
            jint a = va_arg(args, jint);
            LOGD("[%s] a: %d", __FUNCTION__, a);
            jstring b = va_arg(args, jstring);
            LOGD("[%s] 2", __FUNCTION__);
            jobject c = va_arg(args, jobject);
            LOGD("[%s] 3", __FUNCTION__);
            jdouble d = va_arg(args, jdouble);
            LOGD("[%s] 4", __FUNCTION__);
            jarray arr = va_arg(args, jarray);
            LOGD("[%s] 5", __FUNCTION__);
            jdouble f = va_arg(args, jdouble);
            LOGD("[%s] 6", __FUNCTION__);
            //call origin

            //新建一个长度为len的jintArray数组
            jintArray array = env->NewIntArray(1);
            //给需要返回的数组赋值
            jint num[1] = {100};
            env->SetIntArrayRegion(array, 0, 1, num);
            result = array;
        }
            break;
        case 101: {
            LOGD("[%s] 11", __FUNCTION__);
            jobject a = va_arg(args, jobject);
            LOGD("[%s] 22", __FUNCTION__);
            //std::string s = Utils::jstr2str(env, a);
            //LOGD("param a is %s", s.c_str());
            //s = Utils::fmt("from jni: %s", s.c_str());
            result = env->NewStringUTF("101 is testB ");
        }
            break;
        case 102: {
            //对应testC函数，没有返回值
            jobject a = va_arg(args, jobject);
            LOGD("[%s] 44", __FUNCTION__);
            //std::string s = Utils::jstr2str(env, (jstring)a);
            LOGD("[%s] 33", __FUNCTION__);
            //LOGD("param a is %s", s.c_str());
            result = NULL;
        }
            break;
        default:
            break;
    }

    va_end(args);
    LOGD("[%s] 8", __FUNCTION__);
    return result;
}

/**
* Register native methods for all classes we know about.
* Table of methods associated with a single class.
*/
static int regNativeMethods(JNIEnv *env) {
    int nResult = JNI_FALSE;
    int nError = 0;
    jclass clazz = NULL;

    //查找Java层对应的接口类
    clazz = env->FindClass(Java_Interface_Class_Name);
    if (clazz != NULL) {
        JNINativeMethod methods[] = {
                {Native_Method_1_Name, Native_Method_1_Signature, (void *) getStr},
                {Native_Method_2_Name, Native_Method_2_Signature, (void *) getInt},
                {Native_Method_3_Name, Native_Method_3_Signature, (void *) Jump},
        };

        nError = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
        if (nError >= 0) {
            nResult = JNI_TRUE;
        } else {
            LOGE("[%s] RegisterNatives error: %d", __FUNCTION__, nError);
        }

        env->DeleteLocalRef(clazz);
    } else {
        LOGE("[%s] FindClass error, not found class: %s", __FUNCTION__, Java_Interface_Class_Name);
    }


    return nResult;
}

//////////////////////////////////////////////////////////////////////////
JNIEXPORT jstring    JNICALL incNum(JNIEnv *env, jclass, jobject, jint n) {
    std::string s = Utils::fmt("%s", n + 1);
    return Utils::str2jstr(env, s);
}

//当类被加载时触发的回调函数
static void OnCallback_JavaClassLoad(JNIEnv *jni, jclass _class, void *arg) {
    LOGTIME;
    string sClassName = Utils::getClassName(jni, _class);
    std::replace(sClassName.begin(), sClassName.end(), '.', '/');
    LOGD("[%s] begin class name: %s", __FUNCTION__, sClassName.c_str());


    jmethodID midSrc = jni->GetMethodID(_class, "methodWillBeNotNative", "()Ljava/lang/String;");
    if (midSrc == NULL) {
        LOGE("[%s] not found method: %s", __FUNCTION__, "methodWillBeNotNative");
    }
    jmethodID midDst = jni->GetMethodID(_class, "methodJava", "()Ljava/lang/String;");
    if (midDst == NULL) {
        LOGE("[%s] not found method: %s", __FUNCTION__, "methodJava");
    }

    if (midSrc && midDst) {
        LOGD("[%s] found method %s: %p", __FUNCTION__, "methodJava", midDst);
        LOGD("[%s] found method %s: %p just fix it", __FUNCTION__, "methodWillBeNotNative", midSrc);

        Method *pMethodSrc = (Method *) midSrc;
        Method *pMethodDst = (Method *) midDst;

        pMethodSrc->clazz = pMethodDst->clazz;
        pMethodSrc->accessFlags = pMethodDst->accessFlags;
        pMethodSrc->methodIndex = pMethodDst->methodIndex;
        pMethodSrc->registersSize = pMethodDst->registersSize;
        pMethodSrc->outsSize = pMethodDst->outsSize;
        pMethodSrc->insSize = pMethodDst->insSize;
        pMethodSrc->name = pMethodDst->name;
        pMethodSrc->prototype = pMethodDst->prototype;
        pMethodSrc->shorty = pMethodDst->shorty;
        pMethodSrc->insns = pMethodDst->insns;
        pMethodSrc->jniArgInfo = pMethodDst->jniArgInfo;
        pMethodSrc->nativeFunc = pMethodDst->nativeFunc;
        pMethodSrc->fastJni = pMethodDst->fastJni;
        pMethodSrc->noRef = pMethodDst->noRef;
    }

    //////////////////////////////////////////////////////////////////////////
    ///
    //jmethodID mid = jni->GetMethodID(_class, "methodWillBeNative", "(I)Ljava/lang/String;");
    //if (mid == NULL) {
    //	LOGE("[%s] not found method: %s", __FUNCTION__, "methodWillBeNative");
    //} else {
    //	Method* pMethod = (Method*)mid;
    //	pMethod->accessFlags |= ACC_NATIVE;
    //	pMethod->jniArgInfo = 0x80000000;
    //	pMethod->insns = NULL;
    //	pMethod->nativeFunc = incNum;
    //}
    //////////////////////////////////////////////////////////////////////////

    /*

    Method* meth = (Method*)env->FromReflectedMethod(src);
    Method* target = (Method*)env->FromReflectedMethod(dest);
    LOGD("dalvikMethod: %s", meth->name);

    meth->clazz = target->clazz;
    meth->accessFlags |= ACC_PUBLIC;
    meth->methodIndex = target->methodIndex;
    meth->jniArgInfo = target->jniArgInfo;
    meth->registersSize = target->registersSize;
    meth->outsSize = target->outsSize;
    meth->insSize = target->insSize;

    meth->prototype = target->prototype;
    meth->insns = target->insns;
    meth->nativeFunc = target->nativeFunc;*/

    LOGD("[%s] end class name: %s", __FUNCTION__, sClassName.c_str());
}

/*
* Returns the JNI version on success, -1 on failure.
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;

    LOGTIME;
    Utils::setJavaVM(vm);
    LOGD("[%s] JavaVM: %p", __FUNCTION__, vm);

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("[%s] GetEnv failed", __FUNCTION__);
        return -1;
    }
    if (env == NULL) {
        LOGE("[%s] JNIEnv is null", __FUNCTION__);
        return -1;
    }

    if (regNativeMethods(env) == 0) {
        LOGE("[%s] regNativeMethods failed", __FUNCTION__);
        return -1;
    } else {
        LOGD("[%s] regNativeMethods success", __FUNCTION__);
    }

    //////////////////////////////////////////////////////////////////////////
    //OTHER USER CODE
    dalvik_setup(env, 14);
    //MSJavaHookClassLoad(NULL, "com/bigsing/test/MainActivity", &OnCallback_JavaClassLoad, NULL);

    //CMethodLogger::start();
    //////////////////////////////////////////////////////////////////////////

    /* success -- return valid version number */
    result = JNI_VERSION_1_6;
    return result;
}

//////////////////////////////////////////////////////////////////////////

void *thread_fun(void *arg) {

    JNIEnv *env;
    jclass cls;
    jmethodID mid, mid1;
    JavaVM *vm = Utils::getJavaVM();

    if (vm->AttachCurrentThread(&env, NULL) != JNI_OK) {
        LOGE("%s AttachCurrentThread error failed ", __FUNCTION__);
        return NULL;
    }

    cls = Utils::getJavaJNIClass();
    LOGD("call back begin");
    mid = env->GetStaticMethodID(cls, "formJni", "(ILjava/lang/String;)V");
    if (mid == NULL) {
        LOGE("GetStaticMethodID error....");
        goto error;
    } else {
        LOGD("find Method formJni: %p just call it", mid);
    }

    env->CallStaticVoidMethod(cls, mid, (int) arg, Utils::str2jstr(env, "testabc123"));

#if 0
    //注意这里的NativeHandler并没有对象指针使用，所以掉不了非静态成员函数
    mid1 = env->GetMethodID(cls, "formJniAgain", "(ILjava/lang/String;)V");
    if (mid1 == NULL) {
        LOGE("GetMethodID error....");
        goto error;
    } else {
        LOGD("find formJniAgain: %p", mid1);
    }
    env->CallVoidMethod(g_context, mid1, (int)arg, str2jstr(env, "testabc123456"));
#endif // 0

    error:
    if (vm->DetachCurrentThread() != JNI_OK) {
        LOGE("%s DetachCurrentThread error failed ", __FUNCTION__);
    }

    LOGD("thread callback finished");
    pthread_exit(0);
}