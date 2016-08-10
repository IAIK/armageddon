LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include ../config.mk
LOCAL_MODULE := libflush
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../
LOCAL_SRC_FILES := ../obj/local/$(TARGET_ARCH_ABI)/libflush.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_CFLAGS += ${CFLAGS}
LOCAL_MODULE := example
LOCAL_SRC_FILES := main.c
LOCAL_SHARED_LIBRARIES := libflush
include $(BUILD_EXECUTABLE)
