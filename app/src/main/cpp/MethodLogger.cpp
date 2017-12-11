#include "MethodLogger.h"
#include <jni.h>
#include <algorithm>
#include "Utils.h"
#include "substrate/substrate.h"
#include "dalvik/dalvik_core.h"
#include "dalvik/object.h"

jclass g_javaClass;
jclass g_ObjectClass;
jclass g_MethodClass;
jclass g_javaFieldClass;
jclass g_javaMemberClass;

jmethodID g_getNameOfClass;
jmethodID g_getDeclaredMethods;
jmethodID g_getSignature;
jmethodID g_getNameOfMethod;
jmethodID g_getParameterTypes;
jmethodID g_getReturnType;
jmethodID g_getDeclaredField;
jmethodID g_getInt;
jmethodID g_getModifiers;
jmethodID g_toString;


std::list<std::string>   g_lstExcludedClassName;
std::list<std::string>   g_lstIncludedClassName;


CMethodLogger::CMethodLogger() {
}


CMethodLogger::~CMethodLogger() {
}


bool CMethodLogger::start(JNIEnv *jni) {
    g_lstExcludedClassName.clear();
    g_lstIncludedClassName.clear();
    init(jni);
    //hookBydvmResolveClass(jni);
    hookByloadClass(jni);
    //hookByforName(jni);
    return true;
}
////////////////////////////////////////////////////////////////



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

/*
 * Convert a slot number to a method pointer.
 */
Method* dvmSlotToMethod(ClassObject* clazz, int slot)
{
    if (slot < 0) {
        slot = -(slot+1);
        assert(slot < clazz->directMethodCount);
        return &clazz->directMethods[slot];
    } else {
        assert(slot < clazz->virtualMethodCount);
        return &clazz->virtualMethods[slot];
    }
}


//枚举类的所有函数
//ref https://github.com/woxihuannisja/StormJiagu/blob/50dce517dfca667374fe9ba1c47f507f7d4ebd62/StormProtector/dexload/Utilload.cpp
void enumAllMethodOfClass(JNIEnv *jni, jclass clazz, const std::string &sClassName) {
    jobjectArray methodsArr = (jobjectArray) jni->CallObjectMethod(clazz, g_getDeclaredMethods);
    if (methodsArr == NULL) {
        if(jni->ExceptionCheck() == JNI_TRUE){
            jni->ExceptionClear();
        }
        return;
    }

    jclass MethodClass = jni->FindClass("java/lang/reflect/Method");
    ClassObject* pCls = (ClassObject*) dvmDecodeIndirectRef_fnPtr(dvmThreadSelf_fnPtr(), clazz);

    int sizeMethods = jni->GetArrayLength(methodsArr);
    for (int i = 0; i < sizeMethods; ++i) {
        string sParams;
        string sMethodDesc;
        jobject methodObj = jni->GetObjectArrayElement(methodsArr, i); //get one method obj
        if (methodObj == NULL) {
            if(jni->ExceptionCheck() == JNI_TRUE){
                jni->ExceptionClear();
            }
            continue;
        }


        //获取函数名
        jstring jstrMethodName = (jstring) jni->CallObjectMethod(methodObj, g_getNameOfMethod);
        const char *szMethodName = jni->GetStringUTFChars(jstrMethodName, 0);

        //....
        //ref https://github.com/xiaobaiyey/dexload/blob/4f4679de20c1282589530e16e91c9bdea0e530a1/dexload/Utilload.cpp
        jint nSlot = 0;
        jfieldID slotId = jni->GetFieldID(MethodClass, "slot", "I");
        nSlot = jni->GetIntField(methodObj, slotId);
//        jclass clsOneMethod = jni->GetObjectClass(methodObj);
//        jstring slotName = jni->NewStringUTF("slot");
//        jobject mfield = jni->CallObjectMethod(clsOneMethod, getDeclaredField, slotName);
//
//        if (mfield == NULL) {
//            LOGE("nulllllll");
//            if(jni->ExceptionCheck() == JNI_TRUE){
//                jni->ExceptionClear();
//            }
//        }else{
//            LOGE("getInt2 %p ", getInt);
//            nSlot = jni->CallIntMethod(mfield, getInt, methodObj);
//        }
        //jni->DeleteLocalRef(slotName);
        //jni->DeleteLocalRef(mfield);
        //jni->DeleteLocalRef(clsOneMethod);
        /////////////////////

        jobjectArray argsArr = static_cast<jobjectArray>(jni->CallObjectMethod(methodObj, g_getParameterTypes));
        jint sizeArgs = jni->GetArrayLength(argsArr);
        //循环获取每个参数的类型
        for (int j = 0; j < sizeArgs; ++j) {
            jobject argObj = jni->GetObjectArrayElement(argsArr, j);
            jstring jstrArgClassName = static_cast<jstring>(jni->CallObjectMethod(argObj, g_getNameOfClass));
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


        //获取函数返回值类型
        jobject returnTypeObj = jni->CallObjectMethod(methodObj, g_getReturnType);
        jstring jstrRetTypeClassName = static_cast<jstring>(jni->CallObjectMethod(returnTypeObj, g_getNameOfClass));
        const char *szRetTypeClassName = jni->GetStringUTFChars(jstrRetTypeClassName, 0);

        //获取函数签名信息
        std::string sSig;
        jstring jstrSign = (jstring) jni->CallObjectMethod(methodObj, g_getSignature);
        const char *szSignature = jni->GetStringUTFChars(jstrSign, 0);
        sSig = szSignature;

        //签名是从Java层反射调用返回的，是以.分割的，这里要转换一下
        std::replace(sSig.begin(), sSig.end(), '.', '/');

        //释放关于签名的引用
        jni->ReleaseStringUTFChars(jstrSign, szSignature);
        jni->DeleteLocalRef(jstrSign);


        if (strchr(szMethodName, '$') == NULL && (strstr(szMethodName, "on") == szMethodName
                || strstr(sSig.c_str(), "String")
                || strstr(szRetTypeClassName,"String")
                                                 )) {
            //格式化函数信息
            sMethodDesc = Utils::fmt("%s %s %s(%s); slotId: %p slot: %d sig: %s", sClassName.c_str(), szRetTypeClassName,
                                     szMethodName, sParams.c_str(), slotId, nSlot, sSig.c_str());
            LOGD("method: %s", sMethodDesc.c_str());

            //设置函数为native
            jmethodID mid = NULL;
            jint nModifiers = jni->CallIntMethod(methodObj, g_getModifiers);
            if (nModifiers & ACC_STATIC) {
                //mid = jni->GetStaticMethodID(clazz, szMethodName, sSig.c_str());
            }else{
                //mid = jni->GetMethodID(clazz, szMethodName, sSig.c_str());
            }
            //todo FromReflectedMethod等函数会触发其他类的加载，也就是loadClass会被调用，由于本函数已经在loadClass中，
            //这样会引起递归调用。而且由于触发了其他类的加载，使得类的加载顺序混乱，容易崩溃。
            //Method *method = (Method *) jni->FromReflectedMethod(methodObj);
            Method *method = (Method *) mid;
            method = dvmSlotToMethod(pCls, nSlot);
            //method = (Method *)dvmGetMethodFromReflect_fnPtr(srcMethod);
            //Object* pmethod = dvmDecodeIndirectRef(jni, srcMethod);
            //method = dvmGetMethodFromReflectObj(pmethod);
            if (method) {
                dalvik_hook_method(method, sClassName.c_str(), szMethodName, sSig.c_str(), sMethodDesc.c_str());
            }
        }
        //hookJavaMethod(jni, clazz, methodObj);

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
    jni->DeleteLocalRef(MethodClass);
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

//当类被加载时触发的回调函数
static void OnClassLoad_EveryClass(JNIEnv *jni, jclass clazz, void *arg) {
    string *sClassName = (string *) arg;
    if (sClassName->find("com.") != std::string::npos) {
        //假定为用户代码类
        LOGD("[%s] class name: %s hook begin", __FUNCTION__, sClassName->c_str());
        enumAllMethodOfClass(jni, clazz, *sClassName);
        //会不会有异常，处理一下
        //todo 实测里面会有异常，有空找出来
        if (jni->ExceptionCheck() == JNI_TRUE) {
            LOGE("[%s] Exception", __FUNCTION__);
            jni->ExceptionClear();
        }
        LOGD("[%s] class name: %s hook end", __FUNCTION__, sClassName->c_str());
    }
    delete sClassName;
}


static void *(*orign_forName)(JNIEnv *, jobject, jstring, jboolean, jobject);
static void *OnCall_forName(JNIEnv *jni, jobject thiz, jstring name, jboolean initialize, jobject loader) {
    string sClassName = Utils::jstr2str(jni, name);
    jclass cls = (jclass) (*orign_forName)(jni, thiz, name, initialize, loader);
    if (sClassName.find("com.tencent.") != std::string::npos ||
            sClassName.find(".wrapper.") != std::string::npos
            ) {
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

///
/// \param referrer referrer->descriptor类型为 Lcom/xxx/xxx;
/// \param classIdx
/// \param fromUnverifiedConstant
/// \return
static ClassObject *(*orign_dvmResolveClass)(const ClassObject *referrer, u4 classIdx, bool fromUnverifiedConstant);
static ClassObject *new_dvmResolveClass(const ClassObject *referrer, u4 classIdx, bool fromUnverifiedConstant){
    ClassObject *pRet = NULL;
    pRet = (*orign_dvmResolveClass)(referrer, classIdx, fromUnverifiedConstant);
    //DvmDex* pDvmDex = referrer->pDvmDex;
    //char *className = dexStringByTypeIdx(pDvmDex->pDexFile, classIdx);

    //过滤java的类，带有com的才处理
    if (strstr(referrer->descriptor, "com/") == NULL ||
            strstr(referrer->descriptor, "/xtool/")) {
        return pRet;
    }

    std::string sClassName;
    std::string sMethodDesc;
    char *newclassDesc = dvmDescriptorToName_fnPtr(referrer->descriptor);
    if (newclassDesc) {
        sClassName = newclassDesc;
        free(newclassDesc);
    }else{
        //去掉第一个字符L和最后一个分号;
        sClassName.assign(referrer->descriptor + 1, strlen(referrer->descriptor) - 2);
    }

    //JNIEnv *jni = NULL;
    //Utils::getenv(&jni);
    //jclass clazz = NULL;
    //clazz = jni->FindClass(sClassName.c_str());
    //clazz = dvmFindJNIClass(jni, sClassName.c_str());
    //clazz = (jclass) addLocalReference(ts.self(), (Object *) pRet);
//    if (clazz == NULL) {
//        LOGE("[%s] clazz is NULL, dvmFindJNIClass return NULL", __FUNCTION__);
//        if (jni->ExceptionCheck() == JNI_TRUE) {
//            jni->ExceptionClear();
//        }
//    }

    LOGD("\n");
    LOGD("------------------------------------------------");
    LOGD("[%s] --> BEGIN class name: %s directMethodCount: %d virtualMethodCount: %d\n", __FUNCTION__, referrer->descriptor, referrer->directMethodCount, referrer->virtualMethodCount);
    for (int i = 0; i < referrer->directMethodCount; ++i) {
        Method *pMethod = &referrer->directMethods[i];
        sMethodDesc = Utils::fmt("%s::%s sig: %s", sClassName.c_str(), pMethod->name, pMethod->shorty);
        LOGD("[%s] directMethod: %s", __FUNCTION__, sMethodDesc.c_str());
        //todo 这里应该传入一个全一点的签名，例如：Lcom/xxx/xxx;Z  到后面dvmGetMethodParamTypes解析的时候会出错
        dalvik_hook_method(pMethod, sClassName.c_str(), pMethod->name, pMethod->shorty, sMethodDesc.c_str());
    }
    for (int i = 0; i < referrer->virtualMethodCount; ++i) {
        Method *pMethod = &referrer->virtualMethods[i];
        sMethodDesc = Utils::fmt("%s::%s sig: %s", sClassName.c_str(), pMethod->name, pMethod->shorty);
        LOGD("[%s] virtualMethod: %s", __FUNCTION__, sMethodDesc.c_str());
        //todo 这里应该传入一个全一点的签名，例如：Lcom/xxx/xxx;Z  到后面dvmGetMethodParamTypes解析的时候会出错
        dalvik_hook_method(pMethod, sClassName.c_str(), pMethod->name, pMethod->shorty, sMethodDesc.c_str());
    }

//    if (clazz) {
//        jni->DeleteLocalRef(clazz);
//    }
    LOGD("[%s] --> END class name: %s", __FUNCTION__, referrer->descriptor);
    LOGD("------------------------------------------------");
    LOGD("\n");

    return pRet;
}

////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////

static void *(*orign_loadClassBool)(JNIEnv *, jobject, jstring, jboolean);
static void *OnCall_loadClassBool(JNIEnv *jni, jobject thiz, jstring name, jboolean resolve) {
    string sClassName = Utils::jstr2str(jni, name);
    jclass cls = (jclass) (*orign_loadClassBool)(jni, thiz, name, resolve);
    if(cls && CMethodLogger::isCanHookThisClass(sClassName.c_str())){
        //假定为用户代码类
        LOGD("[%s] class name: %s hook begin", __FUNCTION__, sClassName.c_str());
        enumAllMethodOfClass(jni, cls, sClassName);
        //todo 实测里面会有异常，有空找出来
        if (jni->ExceptionCheck() == JNI_TRUE) {
            LOGE("[%s] Exception!!!", __FUNCTION__);
            jni->ExceptionClear();
        }
        LOGD("[%s] class name: %s hook end", __FUNCTION__, sClassName.c_str());
    }

    return cls;
}

void hook_loadClass(JNIEnv *jni,jclass clazz) {
#if 0
    jmethodID loadClassMethod = jni->GetMethodID(clazz, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
        if (loadClassMethod == NULL) {
            LOGE("[%s] \"loadClass(String name)\" not found in class: %s", __FUNCTION__,
                 "java/lang/ClassLoader");
        } else {
            orign_loadClass = NULL;
            MSJavaHookMethod(jni, clazz, loadClassMethod, (void *) (&OnCall_loadClass),
                             (void **) (&orign_loadClass));
            ASSERT(orign_loadClass);
        }
#endif
    //因为loadClass最终是调用loadClass(string, false)，所以hook这个函数就可以了
    jmethodID loadClassBoolMethod = jni->GetMethodID(clazz, "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;");
    if (loadClassBoolMethod == NULL) {
        LOGE("[%s] loadClass not found in class: %s", __FUNCTION__, "java/lang/ClassLoader");
    } else {
        orign_loadClassBool = NULL;
        MSJavaHookMethod(jni, clazz, loadClassBoolMethod, (void *)(&OnCall_loadClassBool), (void **)(&orign_loadClassBool));
        ASSERT(orign_loadClassBool);
    }
}

//当ClassLoader类被加载时触发的回调函数
static void OnClassLoad_ClassLoader(JNIEnv *jni, jclass clazz, void *arg) {
    string sClassName = Utils::getClassName(jni, clazz);
    LOGD("[%s] begin class name: %s", __FUNCTION__, sClassName.c_str());
    hook_loadClass(jni, clazz);
    LOGD("[%s] end class name: %s", __FUNCTION__, sClassName.c_str());
}

void CMethodLogger::hookByloadClass(JNIEnv *jni) {
    jclass clazz = jni->FindClass("java/lang/ClassLoader");
    if (clazz != NULL) {
        LOGD("java/lang/ClassLoader FOUND, HOOK loadClass");
        hook_loadClass(jni, clazz);
        jni->DeleteLocalRef(clazz);
    }else{
        //找不到会有异常，处理一下
        if(jni->ExceptionCheck() == JNI_TRUE){
            jni->ExceptionClear();
        }
        LOGD("java/lang/ClassLoader NOT FOUND, HOOK IT ON CLASSLOAD");
        MSJavaHookClassLoad(NULL, "java/lang/ClassLoader", &OnClassLoad_ClassLoader, NULL);
    }
}

void CMethodLogger::hookByforName(JNIEnv *jni){
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
    }else{
        //找不到会有异常，处理一下
        if(jni->ExceptionCheck() == JNI_TRUE){
            jni->ExceptionClear();
        }
        LOGD("java/lang/Class NOT FOUND, HOOK IT ON CLASSLOAD");
    }
}

/// 通过hook dvmResolveClass的方式在类初始化的时候处理其所有函数
/// \param jni
void CMethodLogger::hookBydvmResolveClass(JNIEnv *jni) {
    ASSERT(dvmResolveClass_fnPtr);
    MSHookFunction(dvmResolveClass_fnPtr, new_dvmResolveClass, &orign_dvmResolveClass);
}

bool dalvik_hook_method(Method *method, const char *szClassName, const char *szMethodName, const char *szSig, const char *szDesc) {
    //LOGD("[%s] begin", __FUNCTION__);
    if (method->nativeFunc == (DalvikBridgeFunc) nativeFunc_logMethodCall) {
        LOGD("[%s] method had been hooked", __FUNCTION__);
        return true;
    }

    if (method->accessFlags == ACC_PUBLIC
        || method->accessFlags == ACC_PRIVATE
           ||   method->accessFlags == (ACC_STATIC | ACC_PUBLIC)
                || method->accessFlags == (ACC_STATIC | ACC_PRIVATE)
            ) {

    }else {
        if (method->accessFlags & ACC_ABSTRACT ||
            method->accessFlags & ACC_CONSTRUCTOR ||
            method->accessFlags & ACC_NATIVE ||
            method->accessFlags & ACC_INTERFACE ||
            method->accessFlags & ACC_SYNCHRONIZED ||
            method->accessFlags & ACC_FINAL ||
            method->accessFlags & ACC_VOLATILE ||
            method->accessFlags & ACC_SYNTHETIC ||
            method->accessFlags & ACC_DECLARED_SYNCHRONIZED ||
            method->accessFlags & ACC_STRICT ||
            method->accessFlags & ACC_ANNOTATION
                ) {
            //抽象方法不处理
            //LOGD("[%s] the method can not surpport accessFlags: %08X", __FUNCTION__, method->accessFlags);
            return true;
        }
        return true;
    }

    //没有设置替换的目标，仅仅显示一个调用日志。保存原始方法的副本后再做修改。
    Method *pOriginalMethod = (Method *) malloc(sizeof(Method));
    memcpy(pOriginalMethod, method, sizeof(Method));

    // init info
    HookInfo *info = new HookInfo;
    info->originalMethod = pOriginalMethod;
    info->sClassName = szClassName;
    info->sMethodName = szMethodName;
    info->sMethodSig = szSig;
    info->sMethodDesc = szDesc;

//    int argsSize = dvmComputeMethodArgsSize_fnPtr(method);
//    if (dvmIsStaticMethod(method) == false) {
//        argsSize++;
//    }
    method->accessFlags |= ACC_NATIVE;
    //method->registersSize = method->insSize = argsSize;
    method->registersSize = method->insSize;
    method->outsSize = 0;
    //method->jniArgInfo = dvmComputeJniArgInfo(method->shorty);
    method->insns = (u2 *) info;
    method->nativeFunc = (DalvikBridgeFunc) nativeFunc_logMethodCall;

    //LOGD("[%s] end", __FUNCTION__);
    return true;
}


//ref : Hook Java中返回值问题的解决和修改https://github.com/Harold1994/program_total/blob/7659177e1d562a8396d0ee9c9eda9f36a12727e5/program_total/3%E6%B3%A8%E5%85%A5%E7%9A%84so%E6%96%87%E4%BB%B6/documents/HookJava%E4%B8%AD%E8%BF%94%E5%9B%9E%E5%80%BC%E9%97%AE%E9%A2%98%E7%9A%84%E8%A7%A3%E5%86%B3%E5%92%8C%E4%BF%AE%E6%94%B9.md
void nativeFunc_logMethodCall(const u4 *args, JValue *pResult, const Method *method, void *self) {
    Object *result = NULL;
    HookInfo* info = (HookInfo*)method->insns; //get hookinfo pointer from method-insns
    LOGD("[%s] --> %s::%s %s", __FUNCTION__, info->sClassName.c_str(), info->sMethodName.c_str(), info->sMethodSig.c_str());

    Method* originalMethod = (Method*)(info->originalMethod);
    if (info->returnType == NULL) {
        info->returnType = dvmGetBoxedReturnType_fnPtr(originalMethod);
        if (info->returnType == NULL) {
            LOGE("[%s] error 1", __FUNCTION__);
            if (dvmCheckException_fnPtr(self)) {
                LOGE("[%s] error 1 Exception", __FUNCTION__);
            }
        }
    }
    //在替换方法的时候可能有些参数类型并没有被加载呢，所以要在调用的时候获取。
    if (info->paramTypes == NULL) {
        info->paramTypes = dvmGetMethodParamTypes(originalMethod, info->sMethodSig.c_str());
        if (info->paramTypes == NULL) {
            LOGE("[%s] error 2", __FUNCTION__);
            if (dvmCheckException_fnPtr(self)) {
                LOGE("[%s] error 2 Exception", __FUNCTION__);
                Object *excep = dvmGetException_fnPtr(self);
                jni_env->Throw((jthrowable) excep);
            }
        }
    }
    ArrayObject *argArr = NULL;
    if (dvmIsStaticMethod(originalMethod)== false) {
        Object* thisObject = (Object*)args[0];
        argArr = dvmBoxMethodArgs(originalMethod, args + 1);
        if (dvmCheckException_fnPtr(self)) {
            LOGE("[%s] error 3", __FUNCTION__);
            //Object *excep = dvmGetException_fnPtr(self);
            //jni_env->Throw((jthrowable) excep);
            jni_env->ExceptionClear();
        } else {
            result = dvmInvokeMethod_fnPtr(thisObject, originalMethod, argArr, info->paramTypes, info->returnType, true);
        }
    }else{
        argArr = dvmBoxMethodArgs(originalMethod, args);
        if (dvmCheckException_fnPtr(self)) {
            LOGE("[%s] error 4", __FUNCTION__);
            //Object *excep = dvmGetException_fnPtr(self);
            //jni_env->Throw((jthrowable) excep);
            jni_env->ExceptionClear();
        } else {
            result = dvmInvokeMethod_fnPtr(NULL, originalMethod, argArr, info->paramTypes, info->returnType, true);
        }
    }

    if (dvmCheckException_fnPtr(self)) {
        LOGE("[%s] error 5", __FUNCTION__);
        Object *excep = dvmGetException_fnPtr(self);
        //jni_env->Throw((jthrowable) excep);
        jni_env->ExceptionClear();
    }
    dvmReleaseTrackedAlloc_fnPtr((Object *)argArr, self);

    if (result != NULL) {

        switch (info->returnType->primitiveType) {
            case PRIM_NOT: {
                pResult->l = result;
                const char *szObj2String = NULL;
#if 0
                jstring jstrObj = (jstring)jni_env->CallObjectMethod((jobject)(result), g_toString);
                if (dvmCheckException_fnPtr(self)) {
                    Object *excep = dvmGetException_fnPtr(self);
                    jni_env->ExceptionClear();
                } else {
                    szObj2String = jni_env->GetStringUTFChars(jstrObj, 0);
                }

                LOGD("[%s] <-- %s::%s ret object %p %s", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str(), result, szObj2String);

                //释放
                if (jstrObj) {
                    jni_env->ReleaseStringUTFChars(jstrObj, szObj2String);
                    jni_env->DeleteLocalRef(jstrObj);
                }
#endif

//                JNIEnv *env = NULL;
//                Utils::getenv(&env);
//
//                jobject* i = reinterpret_cast<jobject*>(&result[1]);
//                jobject jobj = (jobject)*i;
//
//
//                char *newclassDesc = dvmDescriptorToName_fnPtr(info->returnType->descriptor);
//                jclass retClass = env->FindClass(newclassDesc);
//                info->toStringMethod = env->GetMethodID(retClass, "toString", "()Ljava/lang/String;");
//                free(newclassDesc);
//
//                jstring msg = (jstring) env->CallObjectMethod(jobj, info->toStringMethod);
//                env->DeleteLocalRef(retClass);
//                const char *szMsg = env->GetStringUTFChars(msg, 0);
//                LOGD("nativeFunc_logMethodCall: ret object %s", szMsg);
//                env->ReleaseStringUTFChars(msg, szMsg);
//                env->DeleteLocalRef(msg);
            }
                break;
            case PRIM_VOID: {
                pResult->l = NULL;
                LOGD("[%s] <-- %s::%s ret void", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str());
            }
                break;
            case PRIM_BOOLEAN: {
                unsigned char *i = reinterpret_cast<unsigned char *>(&result[1]);
                pResult->z = *i;
                LOGD("[%s] <-- %s::%s ret bool %d", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->z);
            }
                break;
            case PRIM_BYTE: {
                signed char *i = reinterpret_cast<signed char *>(&result[1]);
                pResult->b = *i;
                LOGD("[%s] <-- %s::%s ret byte %c", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->b);
            }
                break;
            case PRIM_CHAR: {
                unsigned short *i = reinterpret_cast<unsigned short *>(&result[1]);
                pResult->c = *i;
                LOGD("[%s] <-- %s::%s ret char %c", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->c);
            }
                break;
            case PRIM_SHORT: {
                signed short *i = reinterpret_cast<signed short *>(&result[1]);
                pResult->s = *i;
                LOGD("[%s] <-- %s::%s ret byte %c", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->s);
            }
                break;
            case PRIM_INT: {
                if (result != NULL) {
                    int *i = reinterpret_cast<int *>(&result[1]);
                    pResult->i = *i;
                    LOGD("[%s] <-- %s::%s ret int %d", __FUNCTION__, info->sClassName.c_str(),
                         info->sMethodName.c_str(), pResult->i);
                } else {
                    pResult->l = result;
                    LOGE("[%s] <-- %s::%s ret int , but NULL returned", __FUNCTION__,
                         info->sClassName.c_str(), info->sMethodName.c_str());
                }
            }
                break;
            case PRIM_LONG: {
                long *i = reinterpret_cast<long *>(&result[1]);
                pResult->j = *i;
                LOGD("[%s] <-- %s::%s ret long %d", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str(), (int) pResult->j);
            }
                break;
            case PRIM_FLOAT: {
                float *i = reinterpret_cast<float *>(&result[1]);
                pResult->f = *i;
                LOGD("[%s] <-- %s::%s ret fload %f", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->f);
            }
                break;
            case PRIM_DOUBLE: {
                double *i = reinterpret_cast<double *>(&result[1]);
                pResult->d = *i;
                LOGD("[%s] <-- %s::%s ret double %f", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str(), pResult->d);
            }
                break;
            default: {
                pResult->l = (void *) result;
                LOGD("[%s] <-- %s::%s ret unkonw type", __FUNCTION__, info->sClassName.c_str(),
                     info->sMethodName.c_str());
            }
                break;
        }
    }else {
        pResult->l = (void *) result;
        LOGD("[%s] <-- %s::%s ret NULL", __FUNCTION__, info->sClassName.c_str(),
             info->sMethodName.c_str());
    }
}

void CMethodLogger::init(JNIEnv *jni) {
    addExcludedClassName("android.");
    addExcludedClassName(".tencent.");
    addExcludedClassName(".hotfix.");
    addExcludedClassName(".push");
    addExcludedClassName(".bugrpt.");
    addExcludedClassName(".fastjson.");
    addExcludedClassName(".util");
    addExcludedClassName(".common");
    addExcludedClassName(".service");

    addIncludedClassName("com.");
    addIncludedClassName("cn.");
    addIncludedClassName("im.yixin");
//    addIncludedClassName("Activity");
//    addIncludedClassName("activity");
//    addIncludedClassName(".view.");

    g_javaClass = jni->FindClass("java/lang/Class");
    g_getNameOfClass = jni->GetMethodID(g_javaClass, "getName", "()Ljava/lang/String;");
    g_getDeclaredMethods = jni->GetMethodID(g_javaClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");

    //Method类中有一个getSignature方法可以获取到方法签名.
    g_MethodClass = jni->FindClass("java/lang/reflect/Method");
    g_getSignature = jni->GetMethodID(g_MethodClass, "getSignature", "()Ljava/lang/String;");
    g_getNameOfMethod = jni->GetMethodID(g_MethodClass, "getName", "()Ljava/lang/String;");
    g_getParameterTypes = jni->GetMethodID(g_MethodClass, "getParameterTypes", "()[Ljava/lang/Class;");
    g_getReturnType = jni->GetMethodID(g_MethodClass, "getReturnType", "()Ljava/lang/Class;");
    g_getDeclaredField = jni->GetMethodID(g_javaClass, "getDeclaredField", "(Ljava/lang/String;)Ljava/lang/reflect/Field;");

    g_javaFieldClass = jni->FindClass("java/lang/reflect/Field");
    g_getInt = jni->GetMethodID(g_javaFieldClass, "getInt", "(Ljava/lang/Object;)I");

    g_javaMemberClass = jni->FindClass("java/lang/reflect/Member");
    g_getModifiers = jni->GetMethodID(g_javaMemberClass, "getModifiers", "()I");
    ASSERT(g_getModifiers);

    g_ObjectClass = jni->FindClass("java/lang/Object");
    g_toString = jni->GetMethodID(g_ObjectClass, "toString", "()Ljava/lang/String;");
    ASSERT(g_toString);
}

void CMethodLogger::addExcludedClassName(const char * lpszName) {
    g_lstExcludedClassName.push_back(std::string(lpszName));
}
void CMethodLogger::addIncludedClassName(const char * lpszName) {
    g_lstIncludedClassName.push_back(std::string(lpszName));
}

bool CMethodLogger::isCanHookThisClass(const std::string&sClassName) {
    if (g_lstExcludedClassName.empty()==false) {
        for(auto i:g_lstExcludedClassName) {
            if (sClassName.find(i) != std::string::npos) {
                return false;
                break;
            }
        }
    }


    if (g_lstIncludedClassName.empty() == false) {
        for(auto i:g_lstIncludedClassName) {
            if (sClassName.find(i) != std::string::npos) {
                return true;
                break;
            }
        }
        return false;
    }

    return true;
}