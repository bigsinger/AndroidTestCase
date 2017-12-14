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
#include "demo/Demo.h"


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
bool Hook_handleBindApplication(JNIEnv *jni) {
    LOGD("[%s] begin", __FUNCTION__);
    jclass ActivityThread = jni->FindClass("android/app/ActivityThread");
    if (ActivityThread == NULL) {
        LOGE("[%s] not found: android/app/ActivityThread, error: %s", __FUNCTION__, dlerror());
        return false;
    }

    jmethodID handleBindApplication = jni->GetMethodID(ActivityThread, "handleBindApplication",
                                                       "(Landroid/app/ActivityThread$AppBindData;)V");
    if (handleBindApplication == NULL) {
        LOGE("[%s] not found: handleBindApplication, error: %s", __FUNCTION__, dlerror());
        return false;
    }

    old_handleBindApplication = NULL;
    MSJavaHookMethod(jni, ActivityThread, handleBindApplication,
                     (void *) (&OnCallback_handleBindApplicaton),
                     (void **) (&old_handleBindApplication));
    if (old_handleBindApplication == NULL) {
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
        }else{
            std:string sPackageName;
            Utils::getPackageName(jni, sPackageName);
            LOGD("current process package name: %s", sPackageName.c_str());
        }

        CMethodLogger::start(jni);
        //////////////////////////////////////////////////////////////////////////
    }else{
        //在测试APP中
        testDemo(jni);
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_6;
    return result;
}