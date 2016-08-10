# See LICENSE file for license and copyright information

include ../config.mk
include ../colors.mk
include ../common.mk

include config.mk

PROJECT       = tests
SOURCE        = $(wildcard *.c)
OBJECTS       = $(addprefix ${BUILDDIR_RELEASE}/,${SOURCE:.c=.o})
OBJECTS_DEBUG = $(addprefix ${BUILDDIR_DEBUG}/,${SOURCE:.c=.o})
OBJECTS_GCOV  = $(addprefix ${BUILDDIR_GCOV}/,${SOURCE:.c=.o})

ifneq (${WITH_LIBFIU},0)
INCS += ${FIU_INC}
LIBS += ${FIU_LIB}
FIU_EXEC += ${FIU_RUN}
CPPFLAGS += -DFIU_ENABLE
endif

ifeq "${ARCH}" "armv7"
	include ../config-arm.mk
endif

ifneq (${HAVE_PAGEMAP_ACCESS},0)
CPPFLAGS += -DHAVE_PAGEMAP_ACCESS=${HAVE_PAGEMAP_ACCESS}
endif

ifneq ($(wildcard ${VALGRIND_SUPPRESSION_FILE}),)
VALGRIND_ARGUMENTS += --suppressions=${VALGRIND_SUPPRESSION_FILE}
endif

all: options ${PROJECT}

options:
ifeq "$(VERBOSE)" "1"
	$(ECHO) ${PROJECT} build options:
	$(ECHO) "CFLAGS  = ${CFLAGS}"
	$(ECHO) "LDFLAGS = ${LDFLAGS}"
	$(ECHO) "DFLAGS  = ${DFLAGS}"
	$(ECHO) "CC      = ${CC}"
endif

# release

${PROJECT}: options ${OBJECTS}
	$(QUIET)${MAKE} WITH_LIBFIU=${WITH_LIBFIU} -C .. libflush
	$(call colorecho,CC,$@)
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} -o $@ \
		${OBJECTS} ${LIBFLUSH_RELEASE} ${LIBS}

${OBJECTS}: config.mk ../config.mk ../libflush/version.h

${BUILDDIR_RELEASE}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

run: ${PROJECT}
	$(QUIET)${FIU_EXEC} ./${PROJECT}

# debug

debug: options ${PROJECT}-debug

${PROJECT}-debug: ${OBJECTS_DEBUG}
	$(QUIET)${MAKE} WITH_LIBFIU=${WITH_LIBFIU} -C .. libflush-debug
	$(call colorecho,CC,$@)
	$(QUIET)${CC} ${LDFLAGS} -o $@ \
		${OBJECTS_DEBUG} ${LIBFLUSH_DEBUG} ${LIBS}

${OBJECTS_DEBUG}: config.mk ../config.mk ../libflush/version.h

${BUILDDIR_DEBUG}/%.o: %.c
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(call colorecho,CC,$<)
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

run-debug: ${PROJECT}-debug
	$(QUIET)${FIU_EXEC} ./${PROJECT}-debug

# gcov

gcov: options ${PROJECT}-gcov

${PROJECT}-gcov: options ${OBJECTS_GCOV}
	$(QUIET)${MAKE} WITH_LIBFIU=${WITH_LIBFIU} -C .. libflush-gcov
	$(call colorecho,CC,$@)
	$(QUIET)${CC} ${LDFLAGS} ${GCOV_LDFLAGS} -o $@ \
		${OBJECTS_GCOV} ${LIBFLUSH_GCOV} ${LIBS}

${OBJECTS_GCOV}: config.mk ../config.mk ../libflush/version.h

${BUILDDIR_GCOV}/%.o: %.c
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(call colorecho,CC,$<)
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${GCOV_CFLAGS} ${DFLAGS} ${GCOV_DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

run-gcov: ${PROJECT}-gcov
	$(QUIET)${FIU_EXEC} ./${PROJECT}-gcov

../libflush/version.h:
	$(MAKE) -C .. libflush/version.h

valgrind: ${PROJECT}-debug
	 $(QUIET)G_SLICE=always-malloc G_DEBUG=gc-friendly ${FIU_EXEC} ${VALGRIND} ${VALGRIND_ARGUMENTS} ./${PROJECT}-debug

clean:
	$(call colorecho,RM, "Clean test files")
	$(QUIET)rm -rf ${PROJECT}
	$(QUIET)rm -rf ${PROJECT}-debug
	$(QUIET)rm -rf ${PROJECT}-gcov
	$(QUIET)rm -rf ${BUILDDIR}
	$(QUIET)rm -rf ${DEPENDDIR}

.PHONY: all options clean debug run

-include $(wildcard ${DEPENDDIR}/*.dep)
