#include $(call all-subdir-makefiles)

#############################################################################
#�ȱ��뾲̬��HookTest
#############################################################################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := hooktest

#����������
LOCAL_CFLAGS    := -Werror -fvisibility=hidden

#����Դ�ļ�
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/../main/cpp/*.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/../main/cpp/dalvik/*.cpp)
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/../main/cpp/art/*.cpp) ../main/cpp/art/art_quick_dexposed_invoke_handler.S
LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

#LOCAL_LDLIBS := -L$(LOCAL_PATH) -llog -ldl -lz
LOCAL_LDLIBS := -L$(LOCAL_PATH) -llog -ldl ../main/cpp/3rd/libsubstrate-dvm.so ../main/cpp/3rd/libsubstrate.so

LOCAL_CPPFLAGS += -std=c++11  -fvisibility=hidden

#�������ļ�Ŀ¼
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
#############################################################################