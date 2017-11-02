
#include "Common.h"
#include <dlfcn.h>
#include <errno.h>
#include <sys/stat.h>
#include "debug.h"


//将string转换为jstring
jstring str2jstr(JNIEnv* env, string s, const char* encoding)
{
	return str2jstr(env, s.c_str(), s.length(), encoding);
}

jstring str2jstr(JNIEnv* env, const char* szText, const int nLen, const char* encoding)
{
	jstring jstrResult;

	if(env != NULL){
		jclass clazz = env->FindClass("java/lang/String");
		jmethodID init = env->GetMethodID(clazz, "<init>", "([BLjava/lang/String;)V");
		jbyteArray bytes = env->NewByteArray((jsize)nLen);
		env->SetByteArrayRegion(bytes, 0, nLen, (jbyte*)szText);

		jstring jencoding;
		if ( encoding == NULL ) {
			jencoding = env->NewStringUTF("utf-8");
		}else{
			jencoding = env->NewStringUTF("GB2312");
		}
		jstrResult = (jstring)env->NewObject(clazz, init, bytes, jencoding);
		env->DeleteLocalRef(bytes);
		env->DeleteLocalRef(clazz);
		env->DeleteLocalRef(jencoding);
	}else{
		LOGE("[%s] failed: env is null", __FUNCTION__);
	}

	return jstrResult;
}


//将jstring转换为string
string jstr2str(JNIEnv* env, jstring jstr, const char *encoding)
{
	string strResult;

	if(env != NULL){
		char* rtn = NULL;
		jstring jencoding;
		if ( encoding==NULL ) {
			jencoding = env->NewStringUTF("GB2312");
		}else{
			jencoding = env->NewStringUTF(encoding);	//"utf-8"
		}

		jclass strClass = (env)->FindClass("java/lang/String");
		jmethodID getBytes= env->GetMethodID(strClass, "getBytes", "(Ljava/lang/String;)[B");
		jbyteArray arr = (jbyteArray)env->CallObjectMethod(jstr, getBytes, jencoding);
		jsize arrLen = env->GetArrayLength(arr);
		jbyte* ba = env->GetByteArrayElements(arr, JNI_FALSE);

		if( arrLen > 0 ){
			strResult.assign((const char*)ba, arrLen);
		}else{
			LOGE("[%s] error GetArrayLength: %d", arrLen);
		}

		env->ReleaseByteArrayElements(arr, ba, 0);
		env->DeleteLocalRef(arr);
		env->DeleteLocalRef(strClass);
		env->DeleteLocalRef(jencoding);
	}else{
		LOGE("[%s] failed: env is null", __FUNCTION__);
	}
	
	return strResult;
}


string formatInt(int n)
{
	char szBuff[260] = {0};
	snprintf(szBuff, 10, "%d", n);
	return szBuff;
}


bool copyFile(const char *inFileName, const char *outFileName)
{
	bool bSuccess = false;
	unsigned long ulSize = 0;

	FILE *inFile = fopen(inFileName, "r");
	if ( inFile==NULL ) {
		LOGE("[%s] open file: %s failed error: %s ", __FUNCTION__, inFileName, dlerror());
		return bSuccess;
	}
	FILE *outFile = fopen(outFileName, "wb+");
	if ( outFile==NULL ) {
		LOGE("[%s] open file: %s failed error: %s ", __FUNCTION__, outFileName, dlerror());
		fclose(outFile);
		return bSuccess;
	}

	fseek(inFile, 0L, SEEK_END);
	ulSize = ftell(inFile);
	fseek(inFile, 0L, SEEK_SET);
	char *buf = new char [ulSize + 1]();
	fread(buf, ulSize, 1, inFile);
	fclose(inFile);

	fwrite(buf, ulSize, 1, outFile);
	fflush(outFile);
	fclose(outFile);
	delete [] buf;	
	bSuccess = true;

	return bSuccess;
}

bool saveFile(const void* addr, int len, const char *outFileName)
{
	bool bSuccess = false;
	FILE* file = fopen(outFileName, "wb+");
	if (file != NULL) {
		fwrite(addr, len, 1, file);
		fflush(file);
		fclose(file);
		bSuccess = true;
		chmod(outFileName, S_IRWXU | S_IRWXG | S_IRWXO);
	}else{
		LOGE("[%s] fopen failed, error: %s", __FUNCTION__, dlerror());
	}

	return bSuccess;
}


bool readTextFile(const char *fileName, string&strText)
{
	bool bSuccess = false;
	char buff[1024] = {0};
	strText.clear();

	unsigned long ulSize = 0;
	FILE *fp = fopen(fileName, "r");
	if ( fp != NULL ) {
		//LOGE("feof 1");
		while ( feof(fp) == 0 ){
			//LOGE("feof 2");
			if ( fgets(buff, sizeof(buff), fp) == NULL ) {
				//LOGE("fgets continue");
				continue;
			}
			//LOGE("fgets got");
			strText.append(buff);
		}
		//LOGE("fgets end");
		bSuccess = true;
	}else{
		LOGE("[%s] open file: %s failed: %s", __FUNCTION__, fileName, strerror(errno));
	}

	return bSuccess;
}

jobject getApplication(JNIEnv *env) 
{
	jclass localClass = env->FindClass("android/app/ActivityThread");
	if (localClass!=NULL) {
		jmethodID getapplication = env->GetStaticMethodID(localClass, "currentApplication", "()Landroid/app/Application;");
		if (getapplication!=NULL) {
			jobject application = env->CallStaticObjectMethod(localClass, getapplication);
			return application;
		}else{
			LOGE("[%s] 1", __FUNCTION__);
			return NULL;
		}
	}
	return NULL;
}

bool getPackageName(JNIEnv *env, string&strOut) 
{
	bool ret = false;
	strOut = "";

	jobject context = getApplication(env);
	if ( context==NULL ) {
		LOGE("[%s] 1", __FUNCTION__);
		return ret;
	}

	jclass  activity = env->GetObjectClass(context);
	if ( activity==NULL ) {
		LOGE("[%s] 2", __FUNCTION__);
		return ret;
	}

	//得到getPackageManager方法的ID
	jmethodID methodID_func = env->GetMethodID(activity, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	if ( methodID_func==NULL ) {
		LOGE("[%s] 3", __FUNCTION__);
		return ret;
	}

	//获得PackageManager对象
	jobject packageManager = env->CallObjectMethod(context,methodID_func);
	jclass packageManagerclass = env->GetObjectClass(packageManager);
	if ( packageManager==NULL || packageManagerclass==NULL ) {
		LOGE("[%s] 4", __FUNCTION__);
		return ret;
	}

	//得到getPackageName方法的ID
	jmethodID methodID_pack = env->GetMethodID(activity,"getPackageName", "()Ljava/lang/String;");

	//获取包名
	jstring name_str = static_cast<jstring>(env->CallObjectMethod(context, methodID_pack));

	strOut = jstr2str(env, name_str);
	return true;
}

bool getPackagePath(JNIEnv *env, string&strOut)
{
	string strPackageName;
	strOut = "";

	if ( getPackageName(env, strPackageName) ) {
		strOut = "/data/data/" + strPackageName;
		return true;
	}

	return false;
}

string getMacs()
{
	bool bOK = false;
	string strMacs;
	string strText;

	bOK = readTextFile("/sys/class/net/wlan0/address", strText);
	if ( bOK ) {
		strMacs += strText;
	}	
	bOK = readTextFile("/sys/class/net/eth0/address", strText);
	if ( bOK && strMacs.find(strText) == string::npos ) {
		strMacs += strText;
	}
	bOK = readTextFile("/sys/class/net/p2p0/address", strText);
	if ( bOK && strMacs.find(strText) == string::npos ) {
		strMacs += strText;
	}

	return strMacs;
}