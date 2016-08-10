# See LICENSE file for license and copyright information

PROJECT = libflush
ARCH = x86

LIBFLUSH_VERSION_MAJOR = 0
LIBFLUSH_VERSION_MINOR = 0
LIBFLUSH_VERSION_REV   = 1

VERSION = ${LIBFLUSH_VERSION_MAJOR}.${LIBFLUSH_VERSION_MINOR}.${LIBFLUSH_VERSION_REV}

# If the API changes, the API version and the ABI version have to be bumped.
LIBFLUSH_VERSION_API = 1

# If the ABI breaks for any reason, this has to be bumped.
LIBFLUSH_VERSION_ABI = 1

# Rules for the SOMAJOR and SOMINOR.
# Before a release check perform the following checks against the last release:
# * If a function has been removed or the paramaters of a function have changed
#   bump SOMAJOR and set SOMINOR to 0.
# * If any of the exported datastructures have changed in a incompatible way
# 	bump SOMAJOR and set SOMINOR to 0.
# * If a function has been added bump SOMINOR.

SOMAJOR = 1
SOMINOR = 0
SOVERSION = ${SOMAJOR}.${SOMINOR}

# pkg-config binary
PKG_CONFIG ?= pkg-config

# paths
PREFIX ?= /usr
LIBDIR ?= ${PREFIX}/lib
INCLUDEDIR ?= ${PREFIX}/include
DEPENDDIR=.depend
BUILDDIR ?= build/${ARCH}
BUILDDIR_RELEASE ?= ${BUILDDIR}/release
BUILDDIR_DEBUG ?= ${BUILDDIR}/debug
BUILDDIR_GCOV ?= ${BUILDDIR}/gcov
BINDIR ?= bin

# libs
FIU_INC ?= $(shell ${PKG_CONFIG} --cflags libfiu)
FIU_LIB ?= $(shell ${PKG_CONFIG} --libs libfiu) -ldl

INCS =
LIBS = -lm

# flags
CFLAGS += -std=gnu11 -pedantic -Wall -Wextra -fPIC -O3 $(INCS)

# linker flags
LDFLAGS += -fPIC

# debug
DFLAGS = -O0 -g

# compiler
CC ?= gcc

# archiver
AR ?= ar

# strip
SFLAGS ?= -s

# gcov & lcov
GCOV_CFLAGS=-fprofile-arcs -ftest-coverage -fno-inline -fno-inline-small-functions -fno-default-inline
GCOV_LDFLAGS=-fprofile-arcs
LCOV_OUTPUT=gcov
LCOV_EXEC=lcov
LCOV_FLAGS=--base-directory . --directory ${BUILDDIR_GCOV} --capture --rc \
					 lcov_branch_coverage=1 --output-file ${BUILDDIR_GCOV}/$(PROJECT).info
GENHTML_EXEC=genhtml
GENHTML_FLAGS=--rc lcov_branch_coverage=1 --output-directory ${LCOV_OUTPUT} ${BUILDDIR_GCOV}/$(PROJECT).info

# libfiu
WITH_LIBFIU ?= 0
FIU_RUN ?= fiu-run -x

# set to something != 0 if you want verbose build output
VERBOSE ?= 0

# enable colors
COLOR ?= 1

# android
ANDROID_PLATFORM ?= android-21

# thread safe
WITH_PTHREAD ?= 0

# pagemap access
HAVE_PAGEMAP_ACCESS ?= 1

# time sources
TIME_SOURCES = (register perf monotonic_clock thread_counter)
TIME_SOURCE ?= register

# use eviction instead of flush
USE_EVICTION ?= 0

# Define device
DEVICE_CONFIGURATION ?= default
