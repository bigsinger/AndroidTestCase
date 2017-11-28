#pragma once
#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string>
using namespace std;

std::string fmt(const char *lpszFormat, ...);

std::string GetCurrentTimeStr(const char* lpszFormat = NULL);


jstring str2jstr(JNIEnv* env, string s, const char* encoding = NULL);
jstring str2jstr(JNIEnv* env, const char* szText, const int nLen, const char* encoding = NULL);
string  jstr2str(JNIEnv* env, jstring jstr, const char *encoding = NULL);
string  formatInt(int n);


bool copyFile(const char *inFileName, const char *outFileName);
bool saveFile(const void* addr, int len, const char *outFileName);
bool readTextFile(const char *fileName, string&strText);

//[android jniǩ����֤(һ)](http://www.xiaobaiyey.com/598.html)
jobject getApplication(JNIEnv *env);

//��ȡ����
bool getPackageName(JNIEnv *env, string&strOut);

//��ȡapp��dataĿ¼��/data/data/com.youzu.android.snsgz
bool getPackagePath(JNIEnv *env, string&strOut);

string getMacs();