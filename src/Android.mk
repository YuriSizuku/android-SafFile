LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := xhook
LOCAL_SRC_FILES := ../prebuilds/$(TARGET_ARCH_ABI)/libxhook.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/ 
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES  := xhook
LOCAL_SRC_FILES := yurihook.c \
                   safhook.c
LOCAL_MODULE := yurihook
include $(BUILD_SHARED_LIBRARY)  #BUILD_EXECUTABLE, BUILD_SHARED_LIBRARY