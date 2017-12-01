#include "MethodLogger.h"
#include <jni.h>
#include "Utils.h"
#include "substrate/substrate.h"
#include "dalvik/dalvik_core.h"
#include "dalvik/object.h"



CMethodLogger::CMethodLogger() {
}


CMethodLogger::~CMethodLogger() {
}


//ö��������к���
//ref https://github.com/woxihuannisja/StormJiagu/blob/50dce517dfca667374fe9ba1c47f507f7d4ebd62/StormProtector/dexload/Utilload.cpp
void enumAllMethodOfClass(JNIEnv *env, jclass cls, const std::string&sClassName) {
	static jclass javaClass = env->FindClass("java/lang/Class");
	static jmethodID ClassgetName = env->GetMethodID(javaClass, "getName", "()Ljava/lang/String;");
	static jmethodID getDeclaredmethods = env->GetMethodID(javaClass, "getDeclaredMethods", "()[Ljava/lang/reflect/Method;");
	jobjectArray methods = (jobjectArray)env->CallObjectMethod(cls, getDeclaredmethods);
	int sizeMethods = env->GetArrayLength(methods);

	//Method������һ������ getSignature ���Ի�ȡ������ǩ��.
	static jclass Method = env->FindClass("java/lang/reflect/Method");
	static jmethodID getSignature = env->GetMethodID(Method, "getSignature", "()Ljava/lang/String;");
	static jmethodID getName = env->GetMethodID(Method, "getName", "()Ljava/lang/String;");
	static jmethodID getParameterTypes = env->GetMethodID(Method, "getParameterTypes", "()[Ljava/lang/Class;");
	static jmethodID getReturnType = env->GetMethodID(Method, "getReturnType", "()Ljava/lang/Class;");

	for (int i = 0; i < sizeMethods; i++) {
		jobject method = env->GetObjectArrayElement(methods, i);
		jstring name = (jstring)env->CallObjectMethod(method, getName);
		const char* szMethodName = env->GetStringUTFChars(name, 0);
		string sParams;
		string sMethodDesc;

		jobjectArray args = static_cast<jobjectArray>(env->CallObjectMethod(method, getParameterTypes));
		jint sizeArgs = env->GetArrayLength(args);
		//ѭ����ȡÿ������������
		for (int j = 0; j < sizeArgs; ++j) {
			jobject argClass = env->GetObjectArrayElement(args, j);
			//����Class getName����
			jstring jArgTypeName = static_cast<jstring>(env->CallObjectMethod(argClass, ClassgetName));
			string sArgTypeName = Utils::jstr2str(env, jArgTypeName);
			if (j != sizeArgs - 1 && sizeArgs != 1) {
				sParams = sParams + sArgTypeName + ", ";
			} else {
				sParams = sParams + sArgTypeName;
			}
			env->DeleteLocalRef(argClass);
			env->DeleteLocalRef(jArgTypeName);
		}//end for

		 //ƴ�ӷ���ֵ
		jobject retClass = env->CallObjectMethod(method, getReturnType);
		jstring jstrRetTypeName = static_cast<jstring>(env->CallObjectMethod(retClass, ClassgetName));
		string sRetTypeName = Utils::jstr2str(env, jstrRetTypeName);
		//�ͷ�����
		env->DeleteLocalRef(retClass);
		env->DeleteLocalRef(jstrRetTypeName);

		jstring sign = (jstring)env->CallObjectMethod(method, getSignature);
		const char* szSignature = env->GetStringUTFChars(sign, 0);
		sMethodDesc = Utils::fmt("%s %s %s(%s); sig: %s", sClassName.c_str(), sRetTypeName.c_str(), szMethodName, sParams.c_str(), szSignature);
		LOGD("method: %s", sMethodDesc.c_str());
		env->ReleaseStringUTFChars(sign, szSignature);
		env->ReleaseStringUTFChars(name, szMethodName);

		dalvik_dispatch(env, method, NULL, false, sMethodDesc.c_str());
	}//end for
}

static void*(*orign_loadClass)(JNIEnv *, jobject, jstring);
static void* OnCall_loadClass(JNIEnv *jni, jobject thiz, jstring name) {
	string sClassName = Utils::jstr2str(jni, name);
	LOGD("[%s] class name: %s", __FUNCTION__, sClassName.c_str());
	jclass cls = (jclass)(*orign_loadClass)(jni, thiz, name);
	if (sClassName.find("com.") != std::string::npos) {
		//�ٶ�Ϊ�û�������
		enumAllMethodOfClass(jni, cls, sClassName);
	}
	return cls;
}

static void*(*orign_loadClassBool)(JNIEnv *, jobject, jstring, jboolean);
static void* OnCall_loadClassBool(JNIEnv *jni, jobject thiz, jstring name, jboolean resolve) {
	string sClassName = Utils::jstr2str(jni, name);
	LOGD("[%s] class name: %s", __FUNCTION__, sClassName.c_str());
	return (*orign_loadClassBool)(jni, thiz, name, resolve);
}

//���౻����ʱ�����Ļص�����
static void OnClassLoad_ClassLoader(JNIEnv *jni, jclass _class, void *arg) {
	LOGTIME;
	string sClassName = Utils::getClassName(jni, _class);
	LOGD("[%s] begin class name: %s", __FUNCTION__, sClassName.c_str());


	jmethodID loadClassMethod = jni->GetMethodID(_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	if (loadClassMethod == NULL) {
		LOGE("[%s] \"loadClass(String name)\" not found in class: %s", __FUNCTION__, sClassName.c_str());
	} else {
		orign_loadClass = NULL;
		MSJavaHookMethod(jni, _class, loadClassMethod, (void *)(&OnCall_loadClass), (void **)(&orign_loadClass));
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
}