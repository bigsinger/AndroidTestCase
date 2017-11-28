
/*
凡是debug下期望输出而release下不出现的字符串，统一在此定义
相关性的字符串分组定义
*/

#pragma once
#include <android/log.h>
#include "Constant.h"


#define _DEBUG

//输出，LOGD，LOGE中的字符串在无_DEBUG宏的版本不会出现明文字符串
#ifdef _DEBUG
	#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__))
	#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))
#else
	#define LOGD(...)
	#define LOGE(...)
#endif

