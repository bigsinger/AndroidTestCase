#include $(call all-subdir-makefiles)

#############################################################################
#�ȱ��뾲̬��HookTest
#############################################################################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := test

#����������
LOCAL_CFLAGS    := -Werror -fvisibility=hidden

#����Դ�ļ�
MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/src/*.cpp)
LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

LOCAL_LDLIBS := -L$(LOCAL_PATH) -llog -ldl -lz

LOCAL_CPPFLAGS += -std=c++11  -fvisibility=hidden

#�������ļ�Ŀ¼
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
#############################################################################