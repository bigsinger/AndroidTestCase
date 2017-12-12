
/*

*/

#pragma once

#include <android/log.h>
#include "Constant.h"
#include "TimeLog.h"

#define _DEBUG



#define ASSERT(V)                \
    if(V == NULL){                                    \
        LOGE("[%s] %s is null. %s::%d", __FUNCTION__, #V, __FILE__, __LINE__);    \
        exit(-1);                                    \
    }

#ifdef _DEBUG
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGTIME    CTimeLog timeLogger(__FUNCTION__)
//#define LOGTIME(s)    CTimeLog timeLogger(#s)
#else
#define LOGD(...)
#define LOGTIME
#endif

//错误日志无论如何都要输出
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

