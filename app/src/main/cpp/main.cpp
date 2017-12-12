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
#include "MethodLogger.h"
#include "DeviceInfo.h"

void *thread_fun(void *arg);

extern "C" {
    JNIEXPORT jstring   JNICALL getStr(JNIEnv *, jclass, jobject, jint, jstring);
    JNIEXPORT jint      JNICALL getInt(JNIEnv *, jclass, jobject, jint, jstring);
    JNIEXPORT jstring   JNICALL
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
getStr(JNIEnv *jni, jclass clsJavaJNI, jobject jCtxObj, jint paramInt, jstring paramStr) {
    jstring jstrResult = NULL;

    switch (paramInt) {
        case CMD_INIT: {
            LOGD("context: %p", jCtxObj);
            jobject obj = Utils::getGlobalContext(jni);
            LOGD("context: %p", obj);
            //保存JVM
            JavaVM *vm = NULL;
            jni->GetJavaVM(&vm);
            Utils::setJavaVM(vm);
            Utils::setContext(jni->NewGlobalRef(jCtxObj));
            Utils::setJavaJNIClass((jclass) jni->NewGlobalRef(clsJavaJNI));

            LOGD("newThread begin");
            pthread_t pt;
            pthread_create(&pt, NULL, &thread_fun, (void *) paramStr);
            std::string s = Utils::fmt("jni: %p, jCtxObj: %p, g_jvm: %p, g_obj: %p", jni, jCtxObj,
                                       Utils::getJavaVM(), Utils::getContext());
            jstrResult = jni->NewStringUTF(s.c_str());
        }
            break;
        case CMD_GET_TEST_STR: {
            LOGD("[%s] CMD_GET_TEST_STR\n", __FUNCTION__);
            std::string s = Utils::fmt("%s Hello Android Native!",
                                       Utils::GetCurrentTimeStr().c_str());
            jstrResult = jni->NewStringUTF(s.c_str());
        }
            break;
        case CMD_GET_MAC: {
            string strMacs = Utils::getMacs();
            jstrResult = Utils::str2jstr(jni, strMacs);
        }
            break;
        case CMD_GET_FILE_TEXT: {
            string strText;
            string sFileName = Utils::jstr2str(jni, paramStr);
            LOGD("[%s] CMD_GET_FILE_TEXT readTextFile: %s", __FUNCTION__, sFileName.c_str());
            if (Utils::readTextFile(sFileName.c_str(), strText) == true) {
                jstrResult = Utils::str2jstr(jni, strText);
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

JNIEXPORT jobject    JNICALL Jump(JNIEnv *jni, jclass, jint nMethodId, jobject objArgs...) {
    LOGTIME;
    LOGD("[%s] MethodId: %d", __FUNCTION__, nMethodId);
    jobject result = NULL;
    va_list args;
    va_start(args, objArgs);

    switch (int(nMethodId)) {
        case 100: {
            //jobject dummy = va_arg(args, jobject);
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
            jintArray array = jni->NewIntArray(1);
            //给需要返回的数组赋值
            jint num[1] = {nMethodId};
            jni->SetIntArrayRegion(array, 0, 1, num);
            result = array;
        }
            break;
        case 101: {
            LOGD("[%s] 11", __FUNCTION__);
            jobject dummy = va_arg(args, jobject);
            dummy = va_arg(args, jobject);
            LOGD("[%s] 22", __FUNCTION__);
            //std::string s = Utils::jstr2str(jni, a);
            //LOGD("param a is %s", s.c_str());
            //s = Utils::fmt("from jni: %s", s.c_str());
            result = jni->NewStringUTF("101 is testB ");
        }
            break;
        case 102: {
            //对应testC函数，没有返回值
            jobject a = va_arg(args, jobject);
            LOGD("[%s] 44", __FUNCTION__);
            //std::string s = Utils::jstr2str(jni, (jstring)a);
            LOGD("[%s] 33", __FUNCTION__);
            //LOGD("param a is %s", s.c_str());
            result = NULL;
        }
            break;
        default:
            LOGE("[%s]unimplemented method id: %d", __FUNCTION__, (int) nMethodId);
            break;
    }

    va_end(args);
    return result;
}

/**
* Register native methods for all classes we know about.
* Table of methods associated with a single class.
*/
static int regNativeMethods(JNIEnv *jni) {
    int nResult = JNI_FALSE;
    int nError = 0;
    jclass clazz = NULL;

    //查找Java层对应的接口类
    clazz = jni->FindClass(Java_Interface_Class_Name);
    if (clazz != NULL) {
        JNINativeMethod methods[] = {
                {Native_Method_1_Name, Native_Method_1_Signature, (void *) getStr},
                {Native_Method_2_Name, Native_Method_2_Signature, (void *) getInt},
                {Native_Method_3_Name, Native_Method_3_Signature, (void *) Jump},
        };

        nError = jni->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
        if (nError >= 0) {
            nResult = JNI_TRUE;
        } else {
            LOGE("[%s] RegisterNatives error: %d", __FUNCTION__, nError);
        }

        jni->DeleteLocalRef(clazz);
    } else {
        //找不到会有异常，处理一下
        if(jni->ExceptionCheck() == JNI_TRUE){
            jni->ExceptionClear();
        }
        nResult = JNI_FALSE;
        LOGI("[%s] not found class: %s may be in other app process", __FUNCTION__, Java_Interface_Class_Name);
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

//handleBindApplication原函数地址
static void (*old_handleBindApplication)(JNIEnv *, jobject, ...);

//参数二的形式为：android.app.ActivityThread@41683ec8
//参数三的形式为：AppBindData{appInfo=ApplicationInfo{4178ead0 com.huawei.ChnUnicomAutoReg}}
//因此可以通过参数三的包名进行过滤
static void OnCallback_handleBindApplicaton(JNIEnv *jni, jobject jthis, jobject appBindData)
{
    LOGD("[%s] begin", __FUNCTION__);
    string strName;
    strName = Utils::getClassName(jni, appBindData);
    if ( (int)strName.size() > 0 ) {
        LOGD("[%s] class: %s", __FUNCTION__, strName.c_str());
    }

    (*old_handleBindApplication)(jni, jthis, appBindData);
    LOGD("[%s] end", __FUNCTION__);
}
bool Hook_handleBindApplication(JNIEnv *jni)
{
    LOGD("[%s] begin", __FUNCTION__);
    jclass ActivityThread = jni->FindClass("android/app/ActivityThread");
    if ( ActivityThread==NULL ) {
        LOGE("[%s] not found: android/app/ActivityThread, error: %s", __FUNCTION__, dlerror());
        return false;
    }

    jmethodID handleBindApplication = jni->GetMethodID(ActivityThread, "handleBindApplication", "(Landroid/app/ActivityThread$AppBindData;)V");
    if (handleBindApplication == NULL) {
        LOGE("[%s] not found: handleBindApplication, error: %s", __FUNCTION__, dlerror());
        return false;
    }

    old_handleBindApplication = NULL;
    MSJavaHookMethod(jni, ActivityThread, handleBindApplication, (void *) (&OnCallback_handleBindApplicaton), (void **) (&old_handleBindApplication));
    if ( old_handleBindApplication==NULL ) {
        LOGE("[%s] old_handleBindApplication is NULL, error: %s", __FUNCTION__, dlerror());
    }

    LOGD("[%s] end", __FUNCTION__);
    return true;
}

// this function pointer is purposely variadic
static void (*oldApplicationOnCreate)(JNIEnv *, jobject, ...);
static void newApplicationOnCreate(JNIEnv *jni, jobject thiz) {
    std:string sPackageName;
    Utils::getPackageName(jni, sPackageName);
    (*oldApplicationOnCreate)(jni, thiz);
    //Utils::getGlobalContext(jni);
    Utils::getPackageName(jni, sPackageName);
    LOGD("current process package name: %s", sPackageName.c_str());
}

void HookApplicationOnCreate(JNIEnv *jni){
    jclass cls = jni->FindClass("android/app/Application");
    jmethodID method = jni->GetMethodID(cls, "onCreate",  "()V" );
    MSJavaHookMethod(jni, cls, method, &newApplicationOnCreate, &oldApplicationOnCreate);
    jni->DeleteLocalRef(cls);
}
/*
* Returns the JNI version on success, -1 on failure.
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *jni = NULL;
    jint result = -1;

    LOGTIME;
    Utils::setJavaVM(vm);

    LOGD("Android OS: %s ( %d )", CDeviceInfo::getOsReleaseVer().c_str(), CDeviceInfo::getSdkInt());
    if (vm->GetEnv((void **) &jni, JNI_VERSION_1_6) != JNI_OK) {
        LOGE("[%s] GetEnv failed", __FUNCTION__);
        return -1;
    }
    ASSERT(jni);

    int nRet = regNativeMethods(jni);
    if (nRet == JNI_FALSE) {
        //可能是其他进程
        //////////////////////////////////////////////////////////////////////////
        //OTHER USER CODE
        jobject context = Utils::getApplication(jni);
        if (context == NULL) {
            //Hook_handleBindApplication(jni);
            HookApplicationOnCreate(jni);
        }

        CMethodLogger::start(jni);
        //////////////////////////////////////////////////////////////////////////
    }else{
        //在测试APP中
        MSJavaHookClassLoad(NULL, "com/bigsing/test/MainActivity", &OnCallback_JavaClassLoad, NULL);
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_6;
    return result;
}

//////////////////////////////////////////////////////////////////////////

void *thread_fun(void *arg) {

    JNIEnv *env;
    jclass cls;
    jmethodID mid = NULL, mid1 = NULL;
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