
#include "Utils.h"
#include <time.h>
#include <dlfcn.h>
#include <errno.h>
#include <cstring>
#include "debug.h"
#include <sys/stat.h>
#include "include/cutils/properties.h"


///////////////////////////////////////////////////////
JavaVM *g_jvm = NULL;
jobject g_context = NULL;
jclass g_clsJNI = NULL;
///////////////////////////////////////////////////////


void Utils::setJavaVM(JavaVM *vm) {
    g_jvm = vm;
}

JavaVM *Utils::getJavaVM() {
    return g_jvm;
}

void Utils::setContext(jobject context) {
    g_context = context;
}

jobject Utils::getContext() {
    return g_context;
}

void Utils::setJavaJNIClass(jclass cls) {
    g_clsJNI = cls;
}

jclass Utils::getJavaJNIClass() {
    return g_clsJNI;
}

pthread_key_t s_threadKey;

bool Utils::getenv(JNIEnv **env) {
    bool bRet = false;
    switch (g_jvm->GetEnv((void **) env, JNI_VERSION_1_6)) {
        case JNI_OK:
            bRet = true;
            break;

        case JNI_EDETACHED:
            if (g_jvm->AttachCurrentThread(env, 0) < 0) {
                break;
            }
            if (pthread_getspecific(s_threadKey) == NULL) {
                pthread_setspecific(s_threadKey, env);
            }
            bRet = true;
            break;
        default:
            break;
    }

    return bRet;
}

jobject Utils::getGlobalContext(JNIEnv *env)
{
    jobject application = NULL;
    jclass activity_thread_clz = env->FindClass("android/app/ActivityThread");
    if (activity_thread_clz != NULL) {
        jmethodID currentApplication = env->GetStaticMethodID(
                activity_thread_clz, "currentApplication", "()Landroid/app/Application;");
        if (currentApplication != NULL) {
            application = env->CallStaticObjectMethod(activity_thread_clz, currentApplication);
        } else {
            LOGE("Cannot find method: currentApplication() in ActivityThread.");
        }
        env->DeleteLocalRef(activity_thread_clz);
    } else {
        LOGE("Cannot find class: android.app.ActivityThread");
    }
    return application;

//    //获取Activity Thread的实例对象
//    jclass clsActivityThread = env->FindClass("android/app/ActivityThread");
//    jmethodID currentActivityThread = env->GetStaticMethodID(clsActivityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
//    jobject at = env->CallStaticObjectMethod(clsActivityThread, currentActivityThread);
//    LOGD("context: %p", at);
//    //获取Application，也就是全局的Context
//    jmethodID getApplication = env->GetMethodID(clsActivityThread, "getApplication", "()Landroid/app/Application;");
//    jobject context = env->CallObjectMethod(at, getApplication);
//    env->DeleteLocalRef(clsActivityThread);
//    LOGD("context: %p", context);
//    return at;
}

//将string转换为jstring
jstring Utils::str2jstr(JNIEnv *env, const string &s, const char *encoding) {
    return str2jstr(env, s.c_str(), s.length(), encoding);
}

//请勿使用Android JNI出坑指南(bugly)中的代码，有BUG！！！
//将string转换为jstring
#if 1

jstring Utils::str2jstr(JNIEnv *env, const char *szText, const int nLen, const char *encoding) {
    jstring jstrResult = NULL;
    if (szText != NULL) {
        jstrResult = env->NewStringUTF(szText);
    }

    return jstrResult;
}

#else
jstring Utils::str2jstr(JNIEnv* env, const char* szText, const int nLen, const char* encoding) {
    jstring jstrResult;
    if (env != NULL) {
        jclass clazz = env->FindClass("java/lang/String");
        jmethodID init = env->GetMethodID(clazz, "<init>", "([BLjava/lang/String;)V");
        jbyteArray bytes = env->NewByteArray((jsize)nLen);
        env->SetByteArrayRegion(bytes, 0, nLen, (jbyte*)szText);
        jstring jencoding;
        if (encoding == NULL) {
            jencoding = env->NewStringUTF("utf-8");
        } else {
            jencoding = env->NewStringUTF("GB2312");
        }
        jstrResult = (jstring)env->NewObject(clazz, init, bytes, jencoding);
        env->DeleteLocalRef(bytes);
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(jencoding);
    } else {
        LOGE("[%s] failed: env is null", __FUNCTION__);
    }
    return jstrResult;
}
#endif

//请勿使用Android JNI出坑指南(bugly)中的代码，有BUG！！！

/// 使用完成后需要释放：jni->ReleaseStringUTFChars(jstr, szStr);
/// \param env
/// \param jstr
/// \return
const char *Utils::jstr2s(JNIEnv *env, const jstring jstr) {
    return env->GetStringUTFChars(jstr, 0);
}

//将jstring转换为string
string Utils::jstr2str(JNIEnv *env, const jstring jstr, const char *encoding) {
    string strResult;

    if (env != NULL) {
        if (jstr != NULL) {
            const char *s = NULL;
            s = env->GetStringUTFChars(jstr, NULL);
            strResult.assign(s);
            env->ReleaseStringUTFChars(jstr, s);
        }

        //char* rtn = NULL;
        //jstring jencoding;
        //if ( encoding==NULL ) {
        //	jencoding = env->NewStringUTF("GB2312");
        //}else{
        //	jencoding = env->NewStringUTF(encoding);	//"utf-8"
        //}

        //jclass strClass = env->FindClass("java/lang/String");
        //jmethodID getBytes= env->GetMethodID(strClass, "getBytes", "(Ljava/lang/String;)[B");
        //jbyteArray arr = (jbyteArray)env->CallObjectMethod(jstr, getBytes, jencoding);
        //jsize arrLen = env->GetArrayLength(arr);
        //jbyte* ba = env->GetByteArrayElements(arr, JNI_FALSE);

        //if( arrLen > 0 ){
        //	strResult.assign((const char*)ba, arrLen);
        //}else{
        //	LOGE("[%s] error GetArrayLength: %d", arrLen);
        //}

        //env->ReleaseByteArrayElements(arr, ba, 0);
        //env->DeleteLocalRef(arr);
        //env->DeleteLocalRef(strClass);
        //env->DeleteLocalRef(jencoding);
    } else {
        LOGE("[%s] failed: env is null", __FUNCTION__);
    }

    return strResult;
}

//获取文件大小
long Utils::get_file_size(const char *path) {
    long ret = 0;
    FILE *fp = fopen(path, "rb"); //打开一个文件， 文件必须存在，只运行读
    if (fp) {
        fseek(fp, 0, SEEK_END);
        ret = ftell(fp);
        fclose(fp);
    }

    return ret;
}

bool Utils::copyFile(const char *inFileName, const char *outFileName) {
    bool bSuccess = false;
    unsigned long ulSize = 0;

    FILE *inFile = fopen(inFileName, "r");
    if (inFile == NULL) {
        LOGE("[%s] open file: %s failed error: %s ", __FUNCTION__, inFileName, dlerror());
        return bSuccess;
    }
    FILE *outFile = fopen(outFileName, "wb+");
    if (outFile == NULL) {
        LOGE("[%s] open file: %s failed error: %s ", __FUNCTION__, outFileName, dlerror());
        fclose(outFile);
        return bSuccess;
    }

    fseek(inFile, 0L, SEEK_END);
    ulSize = ftell(inFile);
    fseek(inFile, 0L, SEEK_SET);
    char *buf = new char[ulSize + 1]();
    fread(buf, ulSize, 1, inFile);
    fclose(inFile);

    fwrite(buf, ulSize, 1, outFile);
    fflush(outFile);
    fclose(outFile);
    delete[] buf;
    bSuccess = true;

    return bSuccess;
}

bool Utils::saveFile(const void *addr, int len, const char *outFileName) {
    bool bSuccess = false;
    FILE *file = fopen(outFileName, "wb+");
    if (file != NULL) {
        fwrite(addr, len, 1, file);
        fflush(file);
        fclose(file);
        bSuccess = true;
        chmod(outFileName, S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
        LOGE("[%s] fopen failed, error: %s", __FUNCTION__, dlerror());
    }

    return bSuccess;
}


bool Utils::readTextFile(const char *fileName, string &strText) {
    bool bSuccess = false;
    char buff[1024] = {0};
    strText.clear();

    unsigned long ulSize = 0;
    FILE *fp = fopen(fileName, "r");
    if (fp != NULL) {
        //LOGE("feof 1");
        while (feof(fp) == 0) {
            //LOGE("feof 2");
            if (fgets(buff, sizeof(buff), fp) == NULL) {
                //LOGE("fgets continue");
                continue;
            }
            //LOGE("fgets got");
            strText.append(buff);
        }
        //LOGE("fgets end");
        bSuccess = true;
    } else {
        LOGE("[%s] open file: %s failed: %s", __FUNCTION__, fileName, strerror(errno));
    }

    return bSuccess;
}

jobject Utils::getApplication(JNIEnv *env) {
    jobject obj = NULL;
    if (env != NULL) {
        jclass cls = env->FindClass("android/app/ActivityThread");
        if (cls != NULL) {
            jmethodID currentApplicationMethod = env->GetStaticMethodID(cls, "currentApplication",
                                                                        "()Landroid/app/Application;");
            if (currentApplicationMethod != NULL) {
                obj = env->CallStaticObjectMethod(cls, currentApplicationMethod);
            } else {
                LOGE("[%s] can not found method: currentApplication", __FUNCTION__);
            }
            env->DeleteLocalRef(cls);
        } else {
            LOGE("[%s] can not found method: currentApplication", __FUNCTION__);
        }
    }
    return obj;
}

std::string Utils::getClassName(JNIEnv *env, jobject obj) {
    std::string sName;
    jclass cls = env->GetObjectClass(obj);

    // First get the class object
    jmethodID mid = env->GetMethodID(cls, "getClass", "()Ljava/lang/Class;");
    jobject clsObj = env->CallObjectMethod(obj, mid);

    // Now get the class object's class descriptor
    cls = env->GetObjectClass(clsObj);

    // Find the getName() method on the class object
    mid = env->GetMethodID(cls, "getName", "()Ljava/lang/String;");

    // Call the getName() to get a jstring object back
    jstring strObj = (jstring) env->CallObjectMethod(clsObj, mid);

    // Now get the c string from the java jstring object
    const char *str = env->GetStringUTFChars(strObj, NULL);

    // Print the class name
    //printf("\nCalling class is: %s\n", str);
    sName.assign(str);
    // Release the memory pinned char array
    env->ReleaseStringUTFChars(strObj, str);
    env->DeleteLocalRef(cls);

    return sName;
}

std::string Utils::getClassName(JNIEnv *env, jclass cls) {
    std::string sName;

    // Get the class object's class descriptor
    jclass clsClazz = env->GetObjectClass(cls);

    // Find the getSimpleName() method in the class object
    jmethodID methodId = env->GetMethodID(clsClazz, "getName", "()Ljava/lang/String;");
    jstring className = (jstring) env->CallObjectMethod(cls, methodId);

    // Now get the c string from the java jstring object
    const char *str = env->GetStringUTFChars(className, NULL);
    sName.assign(str);

    // And finally, don't forget to release the JNI objects after usage!!!!
    env->ReleaseStringUTFChars(className, str);
    env->DeleteLocalRef(clsClazz);

    return sName;
}

bool Utils::getPackageName(JNIEnv *env, string &strOut) {
    bool bSuccess = false;
    strOut.clear();

    jobject context = getApplication(env);
    if (context == NULL) {
        LOGE("[%s] 1", __FUNCTION__);
        return bSuccess;
    }

    jclass contextClass = env->GetObjectClass(context);
    if (contextClass == NULL) {
        LOGE("[%s] 2", __FUNCTION__);
        return bSuccess;
    }

    //得到getPackageManager方法的ID
    jmethodID getPackageManagerMethod = env->GetMethodID(contextClass, "getPackageManager",
                                                         "()Landroid/content/pm/PackageManager;");
    if (getPackageManagerMethod == NULL) {
        LOGE("[%s] 3", __FUNCTION__);
        return bSuccess;
    }

    //获得PackageManager对象
    jobject packageManager = env->CallObjectMethod(context, getPackageManagerMethod);
    jclass packageManagerClass = env->GetObjectClass(packageManager);
    if (packageManager == NULL || packageManagerClass == NULL) {
        LOGE("[%s] 4", __FUNCTION__);
        return bSuccess;
    }

    //得到getPackageName方法的ID
    jmethodID getPackageNameMethod = env->GetMethodID(contextClass, "getPackageName",
                                                      "()Ljava/lang/String;");

    //获取包名
    jstring name_str = static_cast<jstring>(env->CallObjectMethod(context, getPackageNameMethod));

    strOut = jstr2str(env, name_str);
    return true;
}

bool Utils::getPackagePath(JNIEnv *env, string &strOut) {
    string strPackageName;
    strOut = "";

    if (getPackageName(env, strPackageName)) {
        strOut = "/data/data/" + strPackageName;
        return true;
    }

    return false;
}

string Utils::getMacs() {
    bool bOK = false;
    string strMacs;
    string strText;

    bOK = readTextFile("/sys/class/net/wlan0/address", strText);
    if (bOK) {
        strMacs += strText;
    }
    bOK = readTextFile("/sys/class/net/eth0/address", strText);
    if (bOK && strMacs.find(strText) == string::npos) {
        strMacs += strText;
    }
    bOK = readTextFile("/sys/class/net/p2p0/address", strText);
    if (bOK && strMacs.find(strText) == string::npos) {
        strMacs += strText;
    }

    return strMacs;
}

std::string Utils::fmt(const char *lpszFormat, ...) {
    char b = 0;
    va_list argList;
    va_start(argList, lpszFormat);
    unsigned required = 1;
    required = vsnprintf(&b, 0, lpszFormat, argList) + 1;
    char bytes[required];
    vsnprintf(bytes, required, lpszFormat, argList);
    va_end(argList);

    return std::string(bytes);
}

std::string Utils::GetCurrentTimeStr(const char *lpszFormat/* = NULL*/) {
    time_t t;
    time(&t);
    char tmp[64] = {0};
    if (lpszFormat) {
        strftime(tmp, sizeof(tmp), lpszFormat, localtime(&t));
    } else {
        strftime(tmp, sizeof(tmp), "%Y-%m-%d_%H:%M:%S", localtime(&t));
    }
    return tmp;
}


//清除异常
bool clearException(JNIEnv *env) {
    jthrowable exception = env->ExceptionOccurred();
    if (exception != NULL) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }
    return false;
}


//类查找
jclass findAppClass(JNIEnv *env, const char *apn) {
    jclass clazzApplicationLoaders = env->FindClass("android/app/ApplicationLoaders");
    jthrowable exception = env->ExceptionOccurred();
    if (clearException(env)) {
        LOGE("No class : %s", "android/app/ApplicationLoaders");
        return NULL;
    }
    jfieldID fieldApplicationLoaders = env->GetStaticFieldID(clazzApplicationLoaders,
                                                             "gApplicationLoaders",
                                                             "Landroid/app/ApplicationLoaders;");
    if (clearException(env)) {
        LOGE("No Static Field :%s", "gApplicationLoaders");
        return NULL;
    }
    jobject objApplicationLoaders = env->GetStaticObjectField(clazzApplicationLoaders,
                                                              fieldApplicationLoaders);
    if (clearException(env)) {
        LOGE("GetStaticObjectField is failed [%s", "gApplicationLoaders");
        return NULL;
    }
    jfieldID fieldLoaders = env->GetFieldID(clazzApplicationLoaders, "mLoaders",
                                            "Landroid/util/ArrayMap;");
    if (clearException(env)) {
        LOGE("No Field :%s", "mLoaders");
        return NULL;
    }
    jobject objLoaders = env->GetObjectField(objApplicationLoaders, fieldLoaders);
    if (clearException(env)) {
        LOGE("No object :%s", "mLoaders");
        return NULL;
    }
    //??map??alues
    jclass clazzHashMap = env->GetObjectClass(objLoaders);
    jmethodID methodValues = env->GetMethodID(clazzHashMap, "values", "()Ljava/util/Collection;");
    jobject values = env->CallObjectMethod(objLoaders, methodValues);

    jclass clazzValues = env->GetObjectClass(values);
    jmethodID methodToArray = env->GetMethodID(clazzValues, "toArray", "()[Ljava/lang/Object;");
    if (clearException(env)) {
        LOGE("No Method:%s", "toArray");
        return NULL;
    }

    jobjectArray classLoaders = (jobjectArray) env->CallObjectMethod(values, methodToArray);
    if (clearException(env)) {
        LOGE("CallObjectMethod failed :%s", "toArray");
        return NULL;
    }

    int size = env->GetArrayLength(classLoaders);
    int i = 0;
    for (i = 0; i < size; i++) {
        jobject classLoader = env->GetObjectArrayElement(classLoaders, i);
        jclass clazzCL = env->GetObjectClass(classLoader);
        jmethodID loadClass = env->GetMethodID(clazzCL, "loadClass",
                                               "(Ljava/lang/String;)Ljava/lang/Class;");
        jstring param = env->NewStringUTF(apn);
        jclass tClazz = (jclass) env->CallObjectMethod(classLoader, loadClass, param);
        if (clearException(env)) {
            LOGE("No");
            continue;
        }
        return tClazz;
    }
    LOGE("No");
    return NULL;
}

/*
*
*/
jclass Utils::dvmFindJNIClass(JNIEnv *env, const char *classDesc) {
    jclass classObj = env->FindClass(classDesc);

    if (env->ExceptionCheck() == JNI_TRUE) {
        env->ExceptionClear();
    }
    jclass clazzHashMap = env->FindClass("android/util/ArrayMap");

    if (env->ExceptionCheck() == JNI_TRUE) {
        env->ExceptionClear();
    }

    if (classObj == NULL) {
        jclass clazzApplicationLoaders = env->FindClass("android/app/ApplicationLoaders");
        ASSERT(clazzApplicationLoaders);

        jfieldID fieldApplicationLoaders = env->GetStaticFieldID(
                clazzApplicationLoaders, "gApplicationLoaders",
                "Landroid/app/ApplicationLoaders;");
        ASSERT(fieldApplicationLoaders);

        jobject objApplicationLoaders = env->GetStaticObjectField(
                clazzApplicationLoaders, fieldApplicationLoaders);
        ASSERT(objApplicationLoaders);

        jfieldID fieldLoaders = NULL; //env->GetFieldID(clazzApplicationLoaders, "mLoaders", "Ljava/util/Map;");

        if (NULL == clazzHashMap) {
            fieldLoaders = env->GetFieldID(clazzApplicationLoaders, "mLoaders", "Ljava/util/Map;");
        } else {
            fieldLoaders = env->GetFieldID(clazzApplicationLoaders, "mLoaders",
                                           "Landroid/util/ArrayMap;");
        }

        ASSERT(fieldLoaders);

        jobject objLoaders = env->GetObjectField(objApplicationLoaders, fieldLoaders);
        ASSERT(objLoaders);

        if (NULL == clazzHashMap) {
            clazzHashMap = env->GetObjectClass(objLoaders);
        }
        assert(clazzHashMap);
        jmethodID methodValues = env->GetMethodID(clazzHashMap, "values",
                                                  "()Ljava/util/Collection;");
        assert(methodValues);
        jobject values = env->CallObjectMethod(objLoaders, methodValues);


        assert(values);
        jclass clazzValues = env->GetObjectClass(values);
        assert(clazzValues);
        jmethodID methodToArray = env->GetMethodID(clazzValues, "toArray", "()[Ljava/lang/Object;");
        jobjectArray classLoaders = (jobjectArray) env->CallObjectMethod(values,
                                                                         methodToArray);
        assert(classLoaders);

        int size = env->GetArrayLength(classLoaders);
        int i = 0;
        jstring param = env->NewStringUTF(classDesc);

        for (i = 0; i < size; i++) {
            jobject classLoader = env->GetObjectArrayElement(classLoaders, i);
            jclass clazzCL = env->GetObjectClass(classLoader);
            jmethodID loadClass = env->GetMethodID(clazzCL, "loadClass",
                                                   "(Ljava/lang/String;)Ljava/lang/Class;");
            classObj = (jclass) env->CallObjectMethod(classLoader, loadClass, param);

            if (env->ExceptionCheck() == JNI_TRUE) {
                env->ExceptionDescribe();
                env->ExceptionClear();
                continue;
            } else {
                break;
            }
        }
        if (i == size) {
            return NULL;
        }
    }

    return (jclass) env->NewGlobalRef(classObj);
}

const char *Utils::findLibrary(JNIEnv *env, const char *libName) {

    static jclass VMStack = env->FindClass("dalvik/system/VMStack");
    static jmethodID getSystemClassLoader = env->GetStaticMethodID(VMStack,
                                                                   "getCallingClassLoader",
                                                                   "()Ljava/lang/ClassLoader;");
    static jclass baseClassLoaderC = env->FindClass(
            "dalvik/system/BaseDexClassLoader");

    static jmethodID findLibrary = env->GetMethodID(baseClassLoaderC,
                                                    "findLibrary",
                                                    "(Ljava/lang/String;)Ljava/lang/String;");

    jobject classLoader = env->CallStaticObjectMethod(VMStack,
                                                      getSystemClassLoader);

    jstring libNameStr = env->NewStringUTF(libName);
    jstring libPath = (jstring) env->CallObjectMethod(classLoader, findLibrary,
                                                      libNameStr);

    env->DeleteLocalRef(libNameStr);

    if (libPath == NULL) {
        LOGD("libPath== NULL");
        return NULL;
    }
    return env->GetStringUTFChars(libPath, 0);
}

bool Utils::isArt(){
    char value[PROPERTY_VALUE_MAX] = {0};
    property_get("persist.sys.dalvik.vm.lib", value, "");
    return strncmp(value, "libart.so", strlen("libart.so")) == 0;
}