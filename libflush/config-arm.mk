# Define Android specific variables
ANDROID_NDK_PATH = /opt/android-ndk
ANDROID_TOOLCHAIN_BIN = ${ANDROID_NDK_PATH}/toolchains/llvm/prebuilt/linux-x86_64/bin
ANDROID_SYSROOT = ${ANDROID_NDK_PATH}/toolchains/llvm/prebuilt/linux-x86_64/sysroot

ANDROID_CC = ${ANDROID_TOOLCHAIN_BIN}/armv7a-linux-androideabi21-clang
ANDROID_CC_FLAGS = --sysroot=${ANDROID_SYSROOT}

ANDROID_INCLUDES = -I ${ANDROID_NDK_PATH}/platforms/${ANDROID_PLATFORM}/arch-arm/usr/include
ANDROID_CFLAGS = ${ANDROID_INCLUDES} -march=armv7-a -fPIE
ANDROID_LDFLAGS = ${ANDROID_INCLUDES} -march=armv7-a -fPIE

CC = ${ANDROID_CC} ${ANDROID_CC_FLAGS}
CFLAGS += ${ANDROID_CFLAGS}
LDFLAGS += -fPIE
