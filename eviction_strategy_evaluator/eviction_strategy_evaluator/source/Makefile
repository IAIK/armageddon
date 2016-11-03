# See LICENSE file for license and copyright information

include ${LIBFLUSH_SOURCE}/config.mk
include ${LIBFLUSH_SOURCE}/common.mk
include ${LIBFLUSH_SOURCE}/colors.mk

PROJECT = executable
SOURCE  = $(wildcard *.c)
OBJECTS = $(addprefix ${BUILDDIR_RELEASE}/,${SOURCE:.c=.o})
OBJECTS_DEBUG = $(addprefix ${BUILDDIR_DEBUG}/,${SOURCE:.c=.o})

INCS += -I${LIBFLUSH_INC}

LIBFLUSH_RELEASE=${LIBFLUSH_BUILDDIR}/release/libflush.a
LIBFLUSH_DEBUG=${LIBFLUSH_BUILDDIR}/debug/libflush.a

ifeq "${ARCH}" "x86"
	LDFLAGS += -pthread
endif

ifeq "${ARCH}" "armv7"
	include ${LIBFLUSH_SOURCE}/config-arm.mk
	LDFLAGS += -pie
endif

ifeq "${ARCH}" "armv8"
	include ${LIBFLUSH_SOURCE}/config-arm64.mk
	LDFLAGS += -pie
endif

all: options ${PROJECT}

options:
	${ECHO} ${PROJECT} build options:
	${ECHO} "CFLAGS  = ${CFLAGS}"
	${ECHO} "LDFLAGS = ${LDFLAGS}"
	${ECHO} "LIBS    = ${LIBS}"
	${ECHO} "CC      = ${CC}"

# release build

${OBJECTS}: ${LIBFLUSH_SOURCE}/config.mk

${BUILDDIR_RELEASE}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}: ${OBJECTS}
	$(call colorecho,CC,$@)
	@mkdir -p ${BUILDDIR_RELEASE}/${BINDIR}
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} \
		-o ${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT} ${OBJECTS} ${LIBS} ${LIBFLUSH_RELEASE}

${PROJECT}: ${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}

run: ${PROJECT}
		${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}

# debug build

${OBJECTS_DEBUG}: ${LIBFLUSH_SOURCE}/config.mk

${BUILDDIR_DEBUG}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}: ${OBJECTS_DEBUG} dependencies-debug
	$(call colorecho,CC,$@)
	@mkdir -p ${BUILDDIR_DEBUG}/${BINDIR}
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} \
		-o ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT} ${OBJECTS_DEBUG} ${LIBS} ${LIBFLUSH_DEBUG}

debug: ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

run-debug: debug
		${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

dependencies-debug:
	$(QUIET)${MAKE} WITH_LIBFIU=1 -C .. debug

# debugging

gdb: debug
	$(QUIET)${GDB} ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

# clean

clean:
	$(QUIET)rm -rf ${PROJECT}.so ${OBJECTS} .depend ${PROJECT}.gcda ${PROJECT}.gcno

.PHONY: all options clean debug run dependencies dependencies-debug gdb

-include $(wildcard .depend/*.dep)
