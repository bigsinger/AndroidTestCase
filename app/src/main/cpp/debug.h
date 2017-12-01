
/*
����debug�����������release�²����ֵ��ַ�����ͳһ�ڴ˶���
����Ե��ַ������鶨��
*/

#pragma once

#include <android/log.h>
#include "Constant.h"


#define _DEBUG

//�����LOGD��LOGE�е��ַ�������_DEBUG��İ汾������������ַ���
#ifdef _DEBUG
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#define LOGE(...)
#endif

