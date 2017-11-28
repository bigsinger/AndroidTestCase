
/*
这是一个测试so，模拟使用方的so集成anep环境
*/


#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
#include <string>
using namespace std;
#include "debug.h"
#include "Constant.h"
#include "Common.h"


//全局的vm指针
JavaVM* g_vm = NULL;


extern "C" {
	JNIEXPORT jstring	JNICALL  getStr(JNIEnv *, jclass, jobject, jint, jstring);
	JNIEXPORT jint		JNICALL  getInt(JNIEnv *, jclass, jobject, jint, jstring);
	JNIEXPORT jstring	JNICALL  Java_com_bigsing_NativeHandler_getString(JNIEnv *, jclass, jobject, jint, jstring);
}


//该函数不需要动态注册
JNIEXPORT jstring JNICALL Java_com_bigsing_NativeHandler_getString(JNIEnv *env, jclass arg, jobject jCtxObj, jint paramInt, jstring paramStr) {
	return getStr(env, arg, jCtxObj, paramInt, paramStr);
}


/*
**jobject	jCtxObj：	传入的Context 
**jint		paramInt：	作为命令ID，不同的命令ID执行不同的功能
**string	paramStr：	参数字符串

返回值：Java层返回string
*/
JNIEXPORT jstring JNICALL getStr(JNIEnv *env, jclass arg, jobject jCtxObj, jint paramInt, jstring paramStr) {
	jstring jstrResult = NULL;

	switch (paramInt) {
	case CMD_GET_INFO:
		break;
	case CMD_GET_TEST_STR:
	{
		LOGD("[%s] CMD_GET_TEST_STR\n", __FUNCTION__);
		std::string s = fmt("%s Hello Android Native!", GetCurrentTimeStr().c_str());
		jstrResult = env->NewStringUTF(s.c_str());
	}
	break;
	case CMD_GET_MAC:
	{
		string strMacs = getMacs();
		jstrResult = str2jstr(env, strMacs.c_str(), strMacs.length());
	}
	break;
	case CMD_GET_FILE_TEXT:
	{
		string strText;
		string sFileName = jstr2str(env, paramStr);
		LOGD("[%s] CMD_GET_FILE_TEXT readTextFile: %s", __FUNCTION__, sFileName.c_str());
		if (readTextFile(sFileName.c_str(), strText) == true) {
			jstrResult = str2jstr(env, strText.c_str(), strText.length());
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
JNIEXPORT jint JNICALL getInt(JNIEnv *env, jclass arg, jobject jCtxObj, jint paramInt, jstring paramStr) {
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

/**
* Register native methods for all classes we know about.
* Table of methods associated with a single class.
*/
static int regNativeMethods(JNIEnv* env) {
	int		nResult = JNI_FALSE;
	int		nError = 0;
	jclass	clazz = NULL;

	//查找Java层对应的接口类
	clazz = env->FindClass(Java_Interface_Class_Name);
	if (clazz != NULL) {
		JNINativeMethod methods[] = {
			{ Native_Method_1_Name, Native_Method_1_Signature, (void*)getStr },
			{ Native_Method_2_Name, Native_Method_2_Signature, (void*)getInt },
		};

		nError = env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
		if (nError >= 0) {
			nResult = JNI_TRUE;
		} else {
			LOGE("[%s] RegisterNatives error: %d", __FUNCTION__, nError);
		}
	} else {
		LOGE("[%s] FindClass error, not found class: %s", __FUNCTION__, Java_Interface_Class_Name);
	}


	return nResult;
}

//////////////////////////////////////////////////////////////////////////


/*
* Returns the JNI version on success, -1 on failure.
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint	result = -1;

	g_vm = vm;
	LOGD("[%s] JavaVM: %p", __FUNCTION__, vm);

	if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
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
		LOGE("[%s] regNativeMethods success", __FUNCTION__);
	}

	/* success -- return valid version number */
	result = JNI_VERSION_1_6;
	return result;
}