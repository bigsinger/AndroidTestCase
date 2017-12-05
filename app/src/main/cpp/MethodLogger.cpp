#include "MethodLogger.h"
#include <jni.h>
#include <algorithm>
#include "Utils.h"
#include "substrate/substrate.h"
#include "dalvik/dalvik_core.h"
#include "dalvik/object.h"


CMethodLogger::CMethodLogger() {
}


CMethodLogger::~CMethodLogger() {
}


//枚举类的所有函数
//ref https://github.com/woxihuannisja/StormJiagu/blob/50dce517dfca667374fe9ba1c47f507f7d4ebd62/StormProtector/dexload/Utilload.cpp
void enumAllMethodOfClass(JNIEnv *env, jclass cls, const std::string &sClassName) {
    static jclass javaClass = env->FindClass("java/lang/Class");
    static jmethodID getNameOfClass = env->GetMethodID(javaClass, "getName", "()Ljava/lang/String;");
    static jmethodID getDeclaredMethods = env->GetMethodID(javaClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
    jobjectArray methodsArr = (jobjectArray) env->CallObjectMethod(cls, getDeclaredMethods);
    if (methodsArr==NULL) {
        return;
    }
    int sizeMethods = env->GetArrayLength(methodsArr);

    //Method类中有一个getSignature方法可以获取到方法签名.
    static jclass Method = env->FindClass("java/lang/reflect/Method");
    static jmethodID getSignature = env->GetMethodID(Method, "getSignature",
                                                     "()Ljava/lang/String;");
    static jmethodID getNameOfMethod = env->GetMethodID(Method, "getName", "()Ljava/lang/String;");
    static jmethodID getParameterTypes = env->GetMethodID(Method, "getParameterTypes",
                                                          "()[Ljava/lang/Class;");
    static jmethodID getReturnType = env->GetMethodID(Method, "getReturnType",
                                                      "()Ljava/lang/Class;");

    for (int i = 0; i < sizeMethods; ++i) {
        string sParams;
        string sMethodDesc;
        jobject methodObj = env->GetObjectArrayElement(methodsArr, i); //get one method obj

        jobjectArray argsArr = static_cast<jobjectArray>(env->CallObjectMethod(methodObj, getParameterTypes));
        jint sizeArgs = env->GetArrayLength(argsArr);
        //循环获取每个参数的类型
        for (int j = 0; j < sizeArgs; ++j) {
            jobject argObj = env->GetObjectArrayElement(argsArr, j);
            jstring jstrArgClassName = static_cast<jstring>(env->CallObjectMethod(argObj, getNameOfClass));
            const char *szArgClassName = env->GetStringUTFChars(jstrArgClassName, 0);
            if (j != sizeArgs - 1 && sizeArgs != 1) {
                sParams.append(szArgClassName);
                sParams.append(", ");
            } else {
                sParams.append(szArgClassName);
            }

            //释放参数对象
            env->ReleaseStringUTFChars(jstrArgClassName, szArgClassName);
            env->DeleteLocalRef(jstrArgClassName);
            env->DeleteLocalRef(argObj);
        }//end for
        env->DeleteLocalRef(argsArr);

        //获取函数名
        jstring jstrMethodName = (jstring) env->CallObjectMethod(methodObj, getNameOfMethod);
        const char *szMethodName = env->GetStringUTFChars(jstrMethodName, 0);

        //获取函数返回值类型
        jobject returnTypeObj = env->CallObjectMethod(methodObj, getReturnType);
        jstring jstrRetTypeClassName = static_cast<jstring>(env->CallObjectMethod(returnTypeObj,
                                                                             getNameOfClass));
        const char *szRetTypeClassName = env->GetStringUTFChars(jstrRetTypeClassName, 0);

        //获取函数签名信息
        jstring jstrSign = (jstring) env->CallObjectMethod(methodObj, getSignature);
        const char *szSignature = env->GetStringUTFChars(jstrSign, 0);

        //格式化函数信息
        sMethodDesc = Utils::fmt("%s %s %s(%s); sig: %s", sClassName.c_str(), szRetTypeClassName,
                                 szMethodName, sParams.c_str(), szSignature);
        LOGD("method: %s", sMethodDesc.c_str());

        //设置函数为native
        dalvik_hook_java_method(env, cls, methodObj, sClassName.c_str(), szMethodName, szSignature, sMethodDesc.c_str());


        //释放关于签名的引用
        env->ReleaseStringUTFChars(jstrSign, szSignature);
        env->DeleteLocalRef(jstrSign);

        //释放关于返回值的引用
        env->ReleaseStringUTFChars(jstrRetTypeClassName, szRetTypeClassName);
        env->DeleteLocalRef(jstrRetTypeClassName);
        env->DeleteLocalRef(returnTypeObj);

        //释放关于函数名的引用
        env->ReleaseStringUTFChars(jstrMethodName, szMethodName);
        env->DeleteLocalRef(jstrMethodName);

        //释放函数对象
        env->DeleteLocalRef(methodObj);
    }//end for

    env->DeleteLocalRef(methodsArr);
}

static void *(*orign_loadClass)(JNIEnv *, jobject, jstring);

static void *OnCall_loadClass(JNIEnv *jni, jobject thiz, jstring jstrName) {
    string sClassName = Utils::jstr2str(jni, jstrName);
    LOGD("[%s] class name: %s", __FUNCTION__, sClassName.c_str());
    jclass cls = (jclass) (*orign_loadClass)(jni, thiz, jstrName);
    if (cls && sClassName.find("com.") != std::string::npos) {
        //假定为用户代码类
        enumAllMethodOfClass(jni, cls, sClassName);
    }
    return cls;
}

static void *(*orign_loadClassBool)(JNIEnv *, jobject, jstring, jboolean);

static void *OnCall_loadClassBool(JNIEnv *jni, jobject thiz, jstring name, jboolean resolve) {
    string sClassName = Utils::jstr2str(jni, name);
    LOGD("[%s] class name: %s", __FUNCTION__, sClassName.c_str());
    return (*orign_loadClassBool)(jni, thiz, name, resolve);
}

//当类被加载时触发的回调函数
static void OnClassLoad_ClassLoader(JNIEnv *jni, jclass _class, void *arg) {
    LOGTIME;
    string sClassName = Utils::getClassName(jni, _class);
    LOGD("[%s] begin class name: %s", __FUNCTION__, sClassName.c_str());


    jmethodID loadClassMethod = jni->GetMethodID(_class, "loadClass",
                                                 "(Ljava/lang/String;)Ljava/lang/Class;");
    if (loadClassMethod == NULL) {
        LOGE("[%s] \"loadClass(String name)\" not found in class: %s", __FUNCTION__,
             sClassName.c_str());
    } else {
        orign_loadClass = NULL;
        MSJavaHookMethod(jni, _class, loadClassMethod, (void *) (&OnCall_loadClass),
                         (void **) (&orign_loadClass));
        ASSERT(orign_loadClass);
    }

#if 0
    jmethodID loadClassBoolMethod = jni->GetMethodID(_class, "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;");
    if (loadClassBoolMethod == NULL) {
        LOGE("[%s] \"loadClass(String name, boolean resolve)\" not found in class: %s", __FUNCTION__, sClassName.c_str());
    } else {
        orign_loadClassBool = NULL;
        MSJavaHookMethod(jni, _class, loadClassBoolMethod, (void *)(&OnCall_loadClassBool), (void **)(&orign_loadClassBool));
        ASSERT(orign_loadClassBool);
    }
#endif // 0

    LOGD("[%s] end class name: %s", __FUNCTION__, sClassName.c_str());
}


bool CMethodLogger::start() {
    MSJavaHookClassLoad(NULL, "java/lang/ClassLoader", &OnClassLoad_ClassLoader, NULL);
    return true;
}