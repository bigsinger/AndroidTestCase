#include $(call all-subdir-makefiles)

#############################################################################
#先编译静态的HookTest
#############################################################################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := test

#函数名隐藏
LOCAL_CFLAGS    := -Werror -fvisibility=hidden

#编译源文件
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/src/*.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/src/dalvik/*.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/src/art/*.cpp) src/art/art_quick_dexposed_invoke_handler.S
LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

#LOCAL_LDLIBS := -L$(LOCAL_PATH) -llog -ldl -lz
LOCAL_LDLIBS := -L$(LOCAL_PATH) -llog -ldl 3rd/libsubstrate-dvm.so 3rd/libsubstrate.so

LOCAL_CPPFLAGS += -std=c++11  -fvisibility=hidden

#包含库文件目录
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
#############################################################################