# Define Android specific variables
ANDROID_NDK_PATH = /opt/android-ndk
ANDROID_TOOLCHAIN_BIN = ${ANDROID_NDK_PATH}/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin
ANDROID_SYSROOT = ${ANDROID_NDK_PATH}/platforms/${ANDROID_PLATFORM}/arch-arm64

ANDROID_CC = ${ANDROID_TOOLCHAIN_BIN}/aarch64-linux-android-gcc
ANDROID_CC_FLAGS = --sysroot=${ANDROID_SYSROOT}

ANDROID_INCLUDES = -I ${ANDROID_NDK_PATH}/platforms/${ANDROID_PLATFORM}/arch-arm64/usr/include
ANDROID_CFLAGS = ${ANDROID_INCLUDES} -march=armv8-a -fPIE
ANDROID_LDFLAGS = ${ANDROID_INCLUDES} -march=armv8-a -fPIE

CC = ${ANDROID_CC} ${ANDROID_CC_FLAGS}
CFLAGS += ${ANDROID_CFLAGS} -flto
LDFLAGS += -fPIE -flto
