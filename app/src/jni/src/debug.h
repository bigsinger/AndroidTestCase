
/*
����debug�����������release�²����ֵ��ַ�����ͳһ�ڴ˶���
����Ե��ַ������鶨��
*/

#pragma once
#include <android/log.h>
#include "Constant.h"


//�����LOGD��LOGE�е��ַ�������_DEBUG��İ汾������������ַ���
#ifdef _DEBUG
	#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__))
	#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))
#else
	#define LOGD(...)
	#define LOGE(...)
#endif

