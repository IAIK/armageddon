# See LICENSE file for license and copyright information

CHECK_INC ?= $(shell pkg-config --cflags check)
CHECK_LIB ?= $(shell pkg-config --libs check)

INCS += ${CHECK_INC} ${FIU_INC} -I ../libflush
LIBS += ${CHECK_LIB} ${FIU_LIB} -lpthread -Wl,--whole-archive -Wl,--no-whole-archive
LDFLAGS += -rdynamic

LIBFLUSH_RELEASE=../${BUILDDIR_RELEASE}/libflush.a
LIBFLUSH_DEBUG=../${BUILDDIR_DEBUG}/libflush.a
LIBFLUSH_GCOV=../${BUILDDIR_GCOV}/libflush.a
#
# valgrind
VALGRIND = valgrind
VALGRIND_ARGUMENTS = --tool=memcheck --leak-check=yes --leak-resolution=high \
	--show-reachable=yes --log-file=libflush-valgrind.log
VALGRIND_SUPPRESSION_FILE = libflush.suppression
