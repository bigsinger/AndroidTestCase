
/*
����һ������so��ģ��ʹ�÷���so����anep����
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


//ȫ�ֵ�vmָ��
JavaVM* g_vm = NULL;

/*���ǵ���չ�ԣ�������µĹ���Ҫ��ӣ��Ͱ���paramInt���ɷ����Ժ������ӽӿڡ�
**����int Ϊ1ʱ����ȡBuild.prop�������Ϣ
**����string Ϊ���ýӿڣ���ΪNull,��������¹���ʱʹ�ã�����getDeviceInfo����ʱû��ʹ��
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
**���ǵ���չ�ԣ�������µĹ���Ҫ��ӣ��Ͱ���paramInt���ɷ����Ժ������ӽӿڡ�
**����˵����
**jobject jCtxObj�������Conetent 
**jint paramInt��int Ϊ1ʱ������getDeviceInfo����,Ϊ2ʱ����isRoot����
** jstring paramStr����Ϊnull,�����Ժ�����¹���ʱʹ�ã�ע����Ϊ1ʱ����getDeviceInfo����ʱû��ʹ��
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
