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
//�벻Ҫֱ��ʹ�����±�����ʹ��Utils::getxxx����ȡ��ʹ��Utils::setxxx������
extern JavaVM *g_jvm;
extern jobject g_context;
extern jclass g_clsJNI;        //ע��Native��������
///////////////////////////////////////////////////////


#define ASSERT(V)                \
    if(V == NULL){                                    \
        LOGE("[%s] %s is null.", __FUNCTION__, #V);    \
        exit(-1);                                    \
    }


namespace Utils {
    void setJavaVM(JavaVM *vm);

    JavaVM *getJavaVM();

    void setContext(jobject context);

    jobject getContext();

    void setJavaJNIClass(jclass cls);

    jclass getJavaJNIClass();

    bool getenv(JNIEnv **env);

    //�ַ�����ʽ������
    std::string fmt(const char *lpszFormat, ...);

    //��ȡ��ǰ����ʱ����ַ�������ʽ��2017-11-28_17:35:01
    std::string GetCurrentTimeStr(const char *lpszFormat = NULL);

    //string -> jstring
    jstring str2jstr(JNIEnv *env, const std::string &s);

    //jstring -> string
    string jstr2str(JNIEnv *env, jstring jstr);

    //��ȡ�ļ���С
    long get_file_size(const char *path);

    //�����ļ�inFileName��outFileName
    bool copyFile(const char *inFileName, const char *outFileName);

    //����һ���ڴ����ݵ��ļ�outFileName
    bool saveFile(const void *addr, int len, const char *outFileName);

    //��ȡ�ı��ļ�fileName�����ݵ�strText
    bool readTextFile(const char *fileName, string &strText);

    //��ȡ��ǰapplication�Ķ���ref: [android jniǩ����֤(һ)](http://www.xiaobaiyey.com/598.html)
    jobject getApplication(JNIEnv *env);

    //��ȡ���������
    std::string getClassName(JNIEnv *env, jobject obj);

    std::string getClassName(JNIEnv *env, jclass cls);

    //��ȡ��ǰӦ�õİ���
    bool getPackageName(JNIEnv *env, string &strOut);

    //��ȡapp��dataĿ¼(ĩβ����б��)��/data/data/com.xxx.xxx
    bool getPackagePath(JNIEnv *env, string &strOut);

    //�ֱ��/sys/class/net/wlan0/address /sys/class/net/eth0/address /sys/class/net/p2p0/address�ļ��ж�ȡmac��������ظ���ƴ�ӡ�
    string getMacs();

    const char *findLibrary(JNIEnv *env, const char *libName);
}