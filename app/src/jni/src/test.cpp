
/*
这是一个测试so，模拟使用方的so集成anep环境
*/


#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "debug.h"
#include <dlfcn.h>
#include <string>
using namespace std;


//全局的vm指针
JavaVM* g_vm = NULL;

/*考虑到扩展性，如果有新的功能要添加，就按照paramInt来派发，以后不再增加接口。
**参数int 为1时：读取Build.prop的相关信息
**参数string 为备用接口，可为Null,仅供添加新功能时使用，调用getDeviceInfo功能时没有使用
**
*/
extern "C" {
	JNIEXPORT jstring JNICALL  getInfo(JNIEnv *, jclass, jobject, jint , jstring );
	JNIEXPORT jstring JNICALL  Java_com_bigsing_NativeHandler_getString(JNIEnv *, jclass, jobject, jint , jstring );
}

JNIEXPORT jstring JNICALL Java_com_bigsing_NativeHandler_getString(JNIEnv *env, jclass arg, jobject jCtxObj, jint paramInt, jstring paramStr)
{
	return env->NewStringUTF("hell native by getString3");
}

int getInt(int i)
{
	return i + 100;
}

/*
**考虑到扩展性，如果有新的功能要添加，就按照paramInt来派发，以后不再增加接口。
**参数说明：
**jobject jCtxObj：传入的Conetent 
**jint paramInt：int 为1时，调用getDeviceInfo函数,为2时调用isRoot函数
** jstring paramStr：可为null,仅供以后添加新功能时使用，注参数为1时调用getDeviceInfo功能时没有使用
*/

JNIEXPORT jstring JNICALL getInfo(JNIEnv *env, jclass arg, jobject jCtxObj, jint paramInt, jstring paramStr)
{
	if ((int)paramInt == 1){
		LOGD("[+] %d\n", getInt(1));
		return env->NewStringUTF("hell native2");
	}else if((int)paramInt == 2){
		return NULL;
		//return isRoot(env,arg);	

	}else{
		return NULL;
	}
}


/**
* Table of methods associated with a single class.
*/
//static JNINativeMethod gMethods[] = {
//	{ "start", "(Landroid/content/Context;)V", (void*)start},
//	{ "stop", "()V", (void*)stop},
//};
/*
* Register native methods for all classes we know about.
*/
static int registerNativeMethods(JNIEnv* env)
{
	int nError = 0;
	jclass clazz = NULL;

	clazz = env->FindClass("com/bigsing/NativeHandler");
	if (clazz == NULL) {
		LOGE("clazz is null");
		return JNI_FALSE;
	}


	string strMethod3Name = "getInfo";
	string strMethod3Sig = "(Landroid/content/Context;ILjava/lang/String;)Ljava/lang/String;";

	JNINativeMethod methods[] = { 
		{ strMethod3Name.c_str(), strMethod3Sig.c_str(), (void*)getInfo },
	};

	nError = env->RegisterNatives(clazz, methods, sizeof(methods)/sizeof(JNINativeMethod) );
	if ( nError < 0 ) {
		LOGE("RegisterNatives methods error: %d num: %d", nError, sizeof(methods)/sizeof(JNINativeMethod));
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

//////////////////////////////////////////////////////////////////////////


/*
* Returns the JNI version on success, -1 on failure.
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;

	g_vm = vm;
	LOGD("JNI_OnLoad:vm %p", vm);

	if(vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK){
		return -1;
	}
	assert(env != NULL);

	if (!registerNativeMethods(env)) {
		LOGE("registerNativeMethods failed");
		return -1;
	}

	/* success -- return valid version number */
	result = JNI_VERSION_1_6;

	return result;
}
