#include $(call all-subdir-makefiles)

#############################################################################
#先编译静态的HookTest
#############################################################################
MK_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := hooktest

#函数名隐藏
LOCAL_CFLAGS    := -Werror -fvisibility=hidden

#包含库文件目录
LOCAL_PATH = $(MK_PATH)/../main/cpp
LOCAL_C_INCLUDES := $(JNI_H_INCLUDE) \
					$(LOCAL_PATH)	\
					$(LOCAL_PATH)/include
					
#编译源文件
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/*.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/demo/*.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/dalvik/*.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/libcutils/*.c)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/art/*.cpp) $(LOCAL_PATH)/art/art_quick_dexposed_invoke_handler.S
LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

#LOCAL_LDLIBS := -L$(LOCAL_PATH) -llog -ldl -lz
LOCAL_LDLIBS := -L$(LOCAL_PATH) -llog -ldl  ../main/cpp/3rd/libsubstratedvm.so ../main/cpp/3rd/libsubstrate.so

LOCAL_CPPFLAGS += -std=c++14  -fvisibility=hidden


include $(BUILD_SHARED_LIBRARY)
#############################################################################