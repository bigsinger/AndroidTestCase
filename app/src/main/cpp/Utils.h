#pragma once

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string>
#include <android/log.h>
#include "debug.h"
#include "TimeLog.h"

using namespace std;

///////////////////////////////////////////////////////
//请不要直接使用以下变量，使用Utils::getxxx来读取，使用Utils::setxxx来设置
extern JavaVM *g_jvm;
extern jobject g_context;
extern jclass g_clsJNI;        //注册Native函数的类
///////////////////////////////////////////////////////

namespace Utils {
    void setJavaVM(JavaVM *vm);

    JavaVM *getJavaVM();

    void setContext(jobject context);

    jobject getContext();

    void setJavaJNIClass(jclass cls);

    jclass getJavaJNIClass();

    bool getenv(JNIEnv **env);

    jobject getGlobalContext(JNIEnv *env);

    //字符串格式化函数
    std::string fmt(const char *lpszFormat, ...);

    //获取当前日期时间的字符串，格式：2017-11-28_17:35:01
    std::string GetCurrentTimeStr(const char *lpszFormat = NULL);

    //string -> jstring
    jstring str2jstr(JNIEnv *env, const string &s, const char *encoding = NULL);

    //string -> jstring
    jstring str2jstr(JNIEnv *env, const char *szText, const int nLen, const char *encoding = NULL);

    //jstring -> char*
    const char *jstr2s(JNIEnv *env, const jstring jstr);

    //jstring -> string
    string jstr2str(JNIEnv *env, const jstring jstr, const char *encoding = NULL);

    //获取文件大小
    long get_file_size(const char *path);

    //复制文件inFileName到outFileName
    bool copyFile(const char *inFileName, const char *outFileName);

    //保存一段内存数据到文件outFileName
    bool saveFile(const void *addr, int len, const char *outFileName);

    //读取文本文件fileName的内容到strText
    bool readTextFile(const char *fileName, string &strText);

    //获取当前application的对象。ref: [android jni签名验证(一)](http://www.xiaobaiyey.com/598.html)
    jobject getApplication(JNIEnv *env);

    //获取对象的类名
    std::string getClassName(JNIEnv *env, jobject obj);

    std::string getClassName(JNIEnv *env, jclass cls);

    //获取当前应用的包名
    bool getPackageName(JNIEnv *env, string &strOut);

    //获取app的data目录(末尾不含斜杠)：/data/data/com.xxx.xxx
    bool getPackagePath(JNIEnv *env, string &strOut);

    //分别从/sys/class/net/wlan0/address /sys/class/net/eth0/address /sys/class/net/p2p0/address文件中读取mac，如果不重复则拼接。
    string getMacs();

    const char *findLibrary(JNIEnv *env, const char *libName);

    //dvmDecodeIndirectRef
    jclass getObjectFromJobject(JNIEnv *jni, jobject);

    jclass dvmFindJNIClass(JNIEnv *env, const char *classDesc);

    bool isArt();
}