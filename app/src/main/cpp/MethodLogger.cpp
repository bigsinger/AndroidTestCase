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

////////////////////////////////////////////////////////////////
// this function pointer is purposely variadic
static void (*oldMethod)(JNIEnv *,...);
static void newMethod(JNIEnv *jni,...) {
    LOGD("newMethod BEGIN");
    va_list args;
    va_start(args, jni);
    (*oldMethod)(jni, args);
    va_end(args);
    LOGD("newMethod END");
}

//todo 不定参数的处理不对，但是BEGIN有输出，说明这个方法可以，日后参考MSJavaHookMethod的实现再看。
void hookJavaMethod(JNIEnv *env, jclass cls, jobject methodObj) {
    Method *method = (Method *) env->FromReflectedMethod(methodObj);
    MSJavaHookMethod(env, cls, (jmethodID) method, (void *)&newMethod, reinterpret_cast<void **>(&oldMethod));
}
////////////////////////////////////////////////////////////////

//枚举类的所有函数
//ref https://github.com/woxihuannisja/StormJiagu/blob/50dce517dfca667374fe9ba1c47f507f7d4ebd62/StormProtector/dexload/Utilload.cpp
void enumAllMethodOfClass(JNIEnv *jni, jclass cls, const std::string &sClassName) {
    static jclass javaClass = jni->FindClass("java/lang/Class");
    static jmethodID getNameOfClass = jni->GetMethodID(javaClass, "getName", "()Ljava/lang/String;");
    static jmethodID getDeclaredMethods = jni->GetMethodID(javaClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
    jobjectArray methodsArr = (jobjectArray) jni->CallObjectMethod(cls, getDeclaredMethods);
    if (methodsArr == NULL) {
        if(jni->ExceptionCheck() == JNI_TRUE){
            jni->ExceptionClear();
        }
        return;
    }
    int sizeMethods = jni->GetArrayLength(methodsArr);

    //Method类中有一个getSignature方法可以获取到方法签名.
    static jclass clsMethod = jni->FindClass("java/lang/reflect/Method");
    static jmethodID getSignature = jni->GetMethodID(clsMethod, "getSignature",
                                                     "()Ljava/lang/String;");
    static jmethodID getNameOfMethod = jni->GetMethodID(clsMethod, "getName", "()Ljava/lang/String;");
    static jmethodID getParameterTypes = jni->GetMethodID(clsMethod, "getParameterTypes",
                                                          "()[Ljava/lang/Class;");
    static jmethodID getReturnType = jni->GetMethodID(clsMethod, "getReturnType",
                                                      "()Ljava/lang/Class;");

    bool isCanContinue = true;
    for (int i = 0; isCanContinue && i < sizeMethods; ++i) {
        string sParams;
        string sMethodDesc;
        jobject methodObj = jni->GetObjectArrayElement(methodsArr, i); //get one method obj
        if (methodObj == NULL) {
            if(jni->ExceptionCheck() == JNI_TRUE){
                jni->ExceptionClear();
            }
            continue;
        }

        jobjectArray argsArr = static_cast<jobjectArray>(jni->CallObjectMethod(methodObj, getParameterTypes));
        jint sizeArgs = jni->GetArrayLength(argsArr);
        //循环获取每个参数的类型
        for (int j = 0; j < sizeArgs; ++j) {
            jobject argObj = jni->GetObjectArrayElement(argsArr, j);
            jstring jstrArgClassName = static_cast<jstring>(jni->CallObjectMethod(argObj, getNameOfClass));
            const char *szArgClassName = jni->GetStringUTFChars(jstrArgClassName, 0);
            if (j != sizeArgs - 1 && sizeArgs != 1) {
                sParams.append(szArgClassName);
                sParams.append(", ");
            } else {
                sParams.append(szArgClassName);
            }

            //释放参数对象
            jni->ReleaseStringUTFChars(jstrArgClassName, szArgClassName);
            jni->DeleteLocalRef(jstrArgClassName);
            jni->DeleteLocalRef(argObj);
        }//end for
        jni->DeleteLocalRef(argsArr);

        //获取函数名
        jstring jstrMethodName = (jstring) jni->CallObjectMethod(methodObj, getNameOfMethod);
        const char *szMethodName = jni->GetStringUTFChars(jstrMethodName, 0);

        //获取函数返回值类型
        jobject returnTypeObj = jni->CallObjectMethod(methodObj, getReturnType);
        jstring jstrRetTypeClassName = static_cast<jstring>(jni->CallObjectMethod(returnTypeObj, getNameOfClass));
        const char *szRetTypeClassName = jni->GetStringUTFChars(jstrRetTypeClassName, 0);

        //获取函数签名信息
        std::string sSig;
        jstring jstrSign = (jstring) jni->CallObjectMethod(methodObj, getSignature);
        const char *szSignature = jni->GetStringUTFChars(jstrSign, 0);
        sSig = szSignature;

        //签名是从Java层反射调用返回的，是以.分割的，这里要转换一下
        std::replace(sSig.begin(), sSig.end(), '.', '/');

        //释放关于签名的引用
        jni->ReleaseStringUTFChars(jstrSign, szSignature);
        jni->DeleteLocalRef(jstrSign);
        //格式化函数信息
        sMethodDesc = Utils::fmt("%s %s %s(%s); sig: %s", sClassName.c_str(), szRetTypeClassName,
                                 szMethodName, sParams.c_str(), sSig.c_str());
        LOGD("method: %s", sMethodDesc.c_str());

        if (strchr(szMethodName, '$') == NULL) {
            //设置函数为native
            isCanContinue = dalvik_hook_java_method(jni, cls, methodObj, sClassName.c_str(), szMethodName, sSig.c_str(),
                                                    sMethodDesc.c_str());
        }
        //hookJavaMethod(jni, cls, methodObj);



        //释放关于返回值的引用
        jni->ReleaseStringUTFChars(jstrRetTypeClassName, szRetTypeClassName);
        jni->DeleteLocalRef(jstrRetTypeClassName);
        jni->DeleteLocalRef(returnTypeObj);

        //释放关于函数名的引用
        jni->ReleaseStringUTFChars(jstrMethodName, szMethodName);
        jni->DeleteLocalRef(jstrMethodName);

        //释放函数对象
        jni->DeleteLocalRef(methodObj);
    }//end for

    jni->DeleteLocalRef(methodsArr);
}

static void *(*orign_loadClass)(JNIEnv *, jobject, jstring);

static void *OnCall_loadClass(JNIEnv *jni, jobject thiz, jstring jstrName) {
    string sClassName = Utils::jstr2str(jni, jstrName);
    jclass cls = (jclass) (*orign_loadClass)(jni, thiz, jstrName);
	if (sClassName.find("com.taobao.sophix.") != std::string::npos ||
		sClassName.find("com.nostra13.") != std::string::npos ||
		sClassName.find(".common.util.ToastUtil") != std::string::npos ||
		sClassName.find(".session.module.input.InputPanel") != std::string::npos ||
		sClassName.find(".activity.BaseActivity") != std::string::npos ||
		sClassName.find("YWApplication") != std::string::npos ||
		sClassName.find("base.activity.BaseToolbarActivity") != std::string::npos ||
		sClassName.find("MaterialCalendarView") != std::string::npos ||
		sClassName.find("java.lang.ClassLoader") != std::string::npos ||
		sClassName.find("$") != std::string::npos) {
		return cls;
	}
    if (cls && sClassName.find("com.netease.stone") != std::string::npos) {
        //假定为用户代码类
		LOGD("[%s] class name: %s hook begin", __FUNCTION__, sClassName.c_str());
		enumAllMethodOfClass(jni, cls, sClassName);
        LOGD("[%s] class name: %s hook end", __FUNCTION__, sClassName.c_str());
    }
    return cls;
}

static void *(*orign_loadClassBool)(JNIEnv *, jobject, jstring, jboolean);
static void *OnCall_loadClassBool(JNIEnv *jni, jobject thiz, jstring name, jboolean resolve) {
    string sClassName = Utils::jstr2str(jni, name);
    jclass cls = (jclass) (*orign_loadClassBool)(jni, thiz, name, resolve);
    if (cls && sClassName.find("com.") != std::string::npos) {
        //假定为用户代码类
        LOGD("[%s] class name: %s hook begin", __FUNCTION__, sClassName.c_str());
        enumAllMethodOfClass(jni, cls, sClassName);
        //会不会有异常，处理一下
        //todo 实测里面会有异常，有空找出来
        if(jni->ExceptionCheck() == JNI_TRUE){
            LOGE("[%s] Exception", __FUNCTION__);
            jni->ExceptionClear();
        }
        LOGD("[%s] class name: %s hook end", __FUNCTION__, sClassName.c_str());
    }
    return cls;
}

//当类被加载时触发的回调函数
static void OnClassLoad_ClassLoader(JNIEnv *jni, jclass clazz, void *arg) {
    LOGTIME;
    string sClassName = Utils::getClassName(jni, clazz);
    LOGD("[%s] begin class name: %s", __FUNCTION__, sClassName.c_str());


    jmethodID loadClassMethod = jni->GetMethodID(clazz, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    if (loadClassMethod == NULL) {
        LOGE("[%s] \"loadClass(String name)\" not found in class: %s", __FUNCTION__,
             sClassName.c_str());
    } else {
        orign_loadClass = NULL;
        MSJavaHookMethod(jni, clazz, loadClassMethod, (void *) (&OnCall_loadClass),
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

void hookClassLoader_loadClass(JNIEnv *jni){
    jclass clazz = jni->FindClass("java/lang/ClassLoader");
    if (clazz != NULL) {
        LOGD("java/lang/ClassLoader FOUND, HOOK loadClass");
//        jmethodID loadClassMethod = jni->GetMethodID(clazz, "loadClass",
//                                                     "(Ljava/lang/String;)Ljava/lang/Class;");
//        if (loadClassMethod == NULL) {
//            LOGE("[%s] \"loadClass(String name)\" not found in class: %s", __FUNCTION__,
//                 "java/lang/ClassLoader");
//        } else {
//            orign_loadClass = NULL;
//            MSJavaHookMethod(jni, clazz, loadClassMethod, (void *) (&OnCall_loadClass),
//                             (void **) (&orign_loadClass));
//            ASSERT(orign_loadClass);
//        }

        jmethodID loadClassBoolMethod = jni->GetMethodID(clazz, "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;");
        if (loadClassBoolMethod == NULL) {
            LOGE("[%s] \"loadClass(String name, boolean resolve)\" not found in class: %s", __FUNCTION__, "java/lang/ClassLoader");
        } else {
            orign_loadClassBool = NULL;
            MSJavaHookMethod(jni, clazz, loadClassBoolMethod, (void *)(&OnCall_loadClassBool), (void **)(&orign_loadClassBool));
            ASSERT(orign_loadClassBool);
        }
    }else{
        //找不到会有异常，处理一下
        if(jni->ExceptionCheck() == JNI_TRUE){
            jni->ExceptionClear();
        }
        LOGD("java/lang/ClassLoader NOT FOUND, HOOK IT ON CLASSLOAD");
        MSJavaHookClassLoad(NULL, "java/lang/ClassLoader", &OnClassLoad_ClassLoader, NULL);
    }
}


static void *(*orign_forName)(JNIEnv *, jobject, jstring, jboolean, jobject);
static void *OnCall_forName(JNIEnv *jni, jobject thiz, jstring name, jboolean initialize, jobject loader) {
    string sClassName = Utils::jstr2str(jni, name);
    jclass cls = (jclass) (*orign_forName)(jni, thiz, name, initialize, loader);
    if (sClassName.find("com.tencent.") != std::string::npos ) {
        return cls;
    }
    if (initialize== true && cls && sClassName.find("com.") != std::string::npos) {
        //假定为用户代码类
        LOGD("[%s] class name: %s hook begin", __FUNCTION__, sClassName.c_str());
        enumAllMethodOfClass(jni, cls, sClassName);
        //会不会有异常，处理一下
        //todo 实测里面会有异常，有空找出来
        if(jni->ExceptionCheck() == JNI_TRUE){
            LOGE("[%s] Exception", __FUNCTION__);
            jni->ExceptionClear();
        }
        LOGD("[%s] class name: %s hook end", __FUNCTION__, sClassName.c_str());
    }
    return cls;
}

//
//static void *(*orign_forName)(JNIEnv *, jstring);
//static void *OnCall_forName(JNIEnv *jni, jstring name) {
//    string sClassName = Utils::jstr2str(jni, name);
//    jclass cls = (jclass) (*orign_forName)(jni, name);
//    if (cls && sClassName.find("com.") != std::string::npos) {
//        //假定为用户代码类
//        LOGD("[%s] class name: %s hook begin", __FUNCTION__, sClassName.c_str());
//        enumAllMethodOfClass(jni, cls, sClassName);
//        //会不会有异常，处理一下
//        //todo 实测里面会有异常，有空找出来
//        if(jni->ExceptionCheck() == JNI_TRUE){
//            LOGE("[%s] Exception", __FUNCTION__);
//            jni->ExceptionClear();
//        }
//        LOGD("[%s] class name: %s hook end", __FUNCTION__, sClassName.c_str());
//    }
//    return cls;
//}

void hookClass_forName(JNIEnv *jni){
    jclass clazz = jni->FindClass("java/lang/Class");
    if (clazz != NULL) {
        LOGD("java/lang/Class FOUND, HOOK loadClass");
        jmethodID forNameMethod = jni->GetStaticMethodID(clazz, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
        if (forNameMethod == NULL) {
            LOGE("[%s] \"forName(String name, boolean initialize, ClassLoader loader)\" not found in class: %s", __FUNCTION__, "java/lang/Class");
        } else {
            orign_forName = NULL;
            MSJavaHookMethod(jni, clazz, forNameMethod, (void *)(&OnCall_forName), (void **)(&orign_forName));
            ASSERT(orign_forName);
        }

//        jmethodID forNameMethod = jni->GetStaticMethodID(clazz, "forName", "(Ljava/lang/String;)Ljava/lang/Class;");
//        if (forNameMethod == NULL) {
//            LOGE("[%s] \"forName(String name)\" not found in class: %s", __FUNCTION__, "java/lang/Class");
//        } else {
//            orign_forName = NULL;
//            MSJavaHookMethod(jni, clazz, forNameMethod, (void *)(&OnCall_forName), (void **)(&orign_forName));
//            ASSERT(orign_forName);
//        }
    }else{
        //找不到会有异常，处理一下
        if(jni->ExceptionCheck() == JNI_TRUE){
            jni->ExceptionClear();
        }
        LOGD("java/lang/Class NOT FOUND, HOOK IT ON CLASSLOAD");
        //MSJavaHookClassLoad(NULL, "java/lang/ClassLoader", &OnClassLoad_ClassLoader, NULL);
    }
}

bool CMethodLogger::start(JNIEnv *jni) {
    hookClass_forName(jni);
    return true;
}