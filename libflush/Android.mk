LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include config.mk

LOCAL_MODULE := libflush_shared
LOCAL_MODULE_FILENAME := libflush

LOCAL_CFLAGS += ${CFLAGS}

LOCAL_SRC_FILES := $(wildcard libflush/*.c)
LOCAL_SRC_FILES += $(wildcard libflush/eviction/*.c)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES += $(wildcard libflush/armv7/*.c)
USE_EVICTION = 1
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
LOCAL_SRC_FILES += $(wildcard libflush/armv8/*.c)
LOCAL_CFLAGS += -D__ARM_ARCH_8A__
endif

ifeq (,$(findstring ${TIME_SOURCE},${TIME_SOURCES}))
$(error ${TIME_SOURCE} is an invalid time source. $(TIME_SOURCES))
else
TIME_SOURCE_UC = $(shell echo $(TIME_SOURCE) | tr a-z A-Z)
LOCAL_CFLAGS += -DTIME_SOURCE=TIME_SOURCE_${TIME_SOURCE_UC}
endif

ifneq (${DEVICE_CONFIGURATION},0)
ifneq ("$(wildcard ${DEVICE_CONFIGURATION})","")
LOCAL_CFLAGS += -DDEVICE_CONFIGURATION=${DEVICE_CONFIGURATION}
else
LOCAL_CFLAGS += -DDEVICE_CONFIGURATION="strategies/${DEVICE_CONFIGURATION}.h"
endif
endif

ifneq (${USE_EVICTION},0)
LOCAL_CFLAGS += -DUSE_EVICTION=${USE_EVICTION}
endif

ifneq (${HAVE_PAGEMAP_ACCESS},0)
LOCAL_CFLAGS += -DHAVE_PAGEMAP_ACCESS=${HAVE_PAGEMAP_ACCESS}
endif

ifneq (${ANDROID_PLATFORM},0)
LOCAL_CFLAGS += -DANDROID_PLATFORM=$(subst android-,,${ANDROID_PLATFORM})
endif

ifneq (${WITH_PTHREAD},0)
LOCAL_CFLAGS += -DPTHREAD_ENABLE
endif

# include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
