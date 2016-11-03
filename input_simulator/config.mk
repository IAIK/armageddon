# See LICENSE file for license and copyright information

# project
PROJECT = input-simulator
VERSION = 0.0.1

# arch
ARCH = x86

# pkg-config binary
PKG_CONFIG ?= pkg-config

# paths
PREFIX ?= /usr
DEPENDDIR ?= .depend
BUILDDIR ?= build/${ARCH}
BUILDDIR_RELEASE ?= ${BUILDDIR}/release
BUILDDIR_DEBUG ?= ${BUILDDIR}/debug
BINDIR ?= bin

# libs
INCS =
LIBS =

# compiler flags
CFLAGS += -std=c11 -pedantic -Wall -Wno-format-zero-length -Wextra -O3 $(INCS)

# debug
DFLAGS ?= -g

# linker flags
LDFLAGS += -rdynamic

# compiler
CC ?= gcc

# strip
SFLAGS ?= -s

# valgrind
VALGRIND = valgrind
VALGRIND_ARGUMENTS = --tool=memcheck --leak-check=yes --leak-resolution=high \
	--show-reachable=yes --log-file=${PROJECT}-valgrind.log
VALGRIND_SUPPRESSION_FILE = ${PROJECT}.suppression

# set to something != 0 if you want verbose build output
VERBOSE ?= 0

# colors
COLOR ?= 1

# dist
TARFILE = ${PROJECT}-${VERSION}.tar.gz
TARDIR = ${PROJECT}-${VERSION}

# android
ANDROID_PLATFORM ?= android-21

# device configuration
DEVICE_CONFIGURATION ?= ZEROFLTE
