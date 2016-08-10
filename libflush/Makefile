# See LICENSE file for license and copyright information

include config.mk
include colors.mk
include common.mk

SOURCE = $(wildcard ${PROJECT}/*.c ${PROJECT}/eviction/*.c)

ifeq "${ARCH}" "x86"
	SOURCE += $(wildcard ${PROJECT}/x86/*.c)
	LDFLAGS += -pthread
endif

ifeq "${ARCH}" "armv7"
	SOURCE += $(wildcard ${PROJECT}/armv7/*.c)
	USE_EVICTION = 1
	include config-arm.mk
endif

ifeq "${ARCH}" "armv8"
	SOURCE += $(wildcard ${PROJECT}/armv8/*.c)
	CPPFLAGS += -D__ARM_ARCH_8A__
	include config-arm64.mk
endif

OBJECTS         = $(addprefix ${BUILDDIR_RELEASE}/,${SOURCE:.c=.o})
OBJECTS_DEBUG   = $(addprefix ${BUILDDIR_DEBUG}/,${SOURCE:.c=.o})
OBJECTS_GCOV    = $(addprefix ${BUILDDIR_GCOV}/,${SOURCE:.c=.o})
HEADERS         = $(filter-out ${PROJECT}/version.h, \
                  $(wildcard \
                    ${PROJECT}/*.h \
                  ))
HEADERS_INSTALL = ${HEADERS} ${PROJECT}/version.h

ifeq (,$(findstring ${TIME_SOURCE},${TIME_SOURCES}))
$(error ${TIME_SOURCE} is an invalid time source. $(TIME_SOURCES))
else
TIME_SOURCE_UC = $(shell echo $(TIME_SOURCE) | tr a-z A-Z)
CPPFLAGS += -DTIME_SOURCE=TIME_SOURCE_${TIME_SOURCE_UC}
endif

ifneq (${DEVICE_CONFIGURATION},0)
ifneq ("$(wildcard ${DEVICE_CONFIGURATION})","")
CPPFLAGS += -DDEVICE_CONFIGURATION=${DEVICE_CONFIGURATION}
else
CPPFLAGS += -DDEVICE_CONFIGURATION="strategies/${DEVICE_CONFIGURATION}.h"
endif
endif

ifneq (${USE_EVICTION},0)
CPPFLAGS += -DUSE_EVICTION=${USE_EVICTION}
endif

ifneq (${HAVE_PAGEMAP_ACCESS},0)
CPPFLAGS += -DHAVE_PAGEMAP_ACCESS=${HAVE_PAGEMAP_ACCESS}
endif

ifneq (${ANDROID_PLATFORM},0)
CPPFLAGS += -DANDROID_PLATFORM=$(subst android-,,${ANDROID_PLATFORM})
endif

ifneq (${WITH_PTHREAD},0)
CPPFLAGS += -DPTHREAD_ENABLE
LIBS += -pthread
endif

ifneq (${WITH_LIBFIU},0)
INCS += ${FIU_INC}
LIBS += ${FIU_LIB}
CPPFLAGS += -DFIU_ENABLE
endif

all: options ${PROJECT}

options:
	$(ECHO) ${PROJECT} build options:
	$(ECHO) "CFLAGS   = ${CFLAGS}"
	$(ECHO) "CPPFLAGS = ${CPPFLAGS}"
	$(ECHO) "LDFLAGS  = ${LDFLAGS}"
	$(ECHO) "DFLAGS   = ${DFLAGS}"
	$(ECHO) "CC       = ${CC}"

${PROJECT}/version.h: ${PROJECT}/version.h.in config.mk
	$(QUIET)sed 's/ZVMAJOR/${LIBFLUSH_VERSION_MAJOR}/' < ${PROJECT}/version.h.in | \
		sed 's/ZVMINOR/${LIBFLUSH_VERSION_MINOR}/' | \
		sed 's/ZVREV/${LIBFLUSH_VERSION_REV}/' | \
		sed 's/ZVAPI/${LIBFLUSH_VERSION_API}/' | \
		sed 's/ZVABI/${LIBFLUSH_VERSION_ABI}/' > ${PROJECT}/version.h

# release build

${OBJECTS}:  config.mk ${PROJECT}/version.h

${BUILDDIR_RELEASE}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $@)
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF ${DEPENDDIR}/$@.dep

${PROJECT}: ${PROJECT}/version.h static shared

static: ${BUILDDIR_RELEASE}/${PROJECT}.a
shared: ${BUILDDIR_RELEASE}/${PROJECT}.so.${SOVERSION}

${BUILDDIR_RELEASE}/${PROJECT}.a: ${OBJECTS}
	$(call colorecho,AR,$@)
	$(QUIET)${AR} rcs ${BUILDDIR_RELEASE}/${PROJECT}.a ${OBJECTS}

${BUILDDIR_RELEASE}/${PROJECT}.so.${SOVERSION}: ${OBJECTS}
	$(call colorecho,LD,$@)
	$(QUIET)${CC} -Wl,-soname,${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} \
		-o ${BUILDDIR_RELEASE}/${PROJECT}.so.${SOVERSION} ${OBJECTS} ${LIBS}

release: options ${PROJECT}

# debug build

${OBJECTS_DEBUG}: config.mk ${PROJECT}/version.h

${BUILDDIR_DEBUG}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $@)
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$@.dep

${PROJECT}-debug: ${PROJECT}/version.h static-debug shared-debug

static-debug: ${BUILDDIR_DEBUG}/${PROJECT}.a
shared-debug: ${BUILDDIR_DEBUG}/${PROJECT}.so.${SOVERSION}

${BUILDDIR_DEBUG}/${PROJECT}.a: ${OBJECTS_DEBUG}
	$(call colorecho,AR,${PROJECT}.a)
	$(QUIET)${AR} rc ${BUILDDIR_DEBUG}/${PROJECT}.a ${OBJECTS_DEBUG}

${BUILDDIR_DEBUG}/${PROJECT}.so.${SOVERSION}: ${OBJECTS_DEBUG}
	$(call colorecho,LD,${PROJECT}.so.${SOMAJOR})
	$(QUIET)${CC} -Wl,-soname,${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} \
		-o ${BUILDDIR_DEBUG}/${PROJECT}.so.${SOVERSION} ${OBJECTS_DEBUG} ${LIBS}

debug: options ${PROJECT}-debug

# gcov build

${OBJECTS_GCOV}: config.mk ${PROJECT}/version.h

${BUILDDIR_GCOV}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $@)
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${GCOV_CFLAGS} ${DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$@.dep

${PROJECT}-gcov: ${PROJECT}/version.h static-gcov shared-gcov

static-gcov: ${BUILDDIR_GCOV}/${PROJECT}.a
shared-gcov: ${BUILDDIR_GCOV}/${PROJECT}.so.${SOVERSION}

${BUILDDIR_GCOV}/${PROJECT}.a: ${OBJECTS_GCOV}
	$(call colorecho,AR,${PROJECT}.a)
	$(QUIET)${AR} rc ${BUILDDIR_GCOV}/${PROJECT}.a ${OBJECTS_GCOV}

${BUILDDIR_GCOV}/${PROJECT}.so.${SOVERSION}: ${OBJECTS_GCOV}
	$(call colorecho,LD,${PROJECT}.so.${SOMAJOR})
	$(QUIET)${CC} -Wl,-soname,${PROJECT}.so.${SOMAJOR} -shared ${LDFLAGS} ${GCOV_LDFLAGS} \
		-o ${BUILDDIR_GCOV}/${PROJECT}.so.${SOVERSION} ${OBJECTS_GCOV} ${LIBS}

gcov: options ${PROJECT}-gcov
	$(QUIET)${MAKE} -C tests run-gcov
	$(call colorecho,LCOV,"Analyse data")
	$(QUIET)${LCOV_EXEC} ${LCOV_FLAGS}
	$(call colorecho,LCOV,"Generate report")
	$(QUIET)${GENHTML_EXEC} ${GENHTML_FLAGS}

clean:
	$(call colorecho,RM, "Clean objects and builds")
	$(QUIET)rm -rf ${BUILDDIR}

	$(call colorecho,RM, "Clean android specific objects and builds")
	$(QUIET)rm -rf obj libs

	$(call colorecho,RM, "Clean pkg-config files")
	$(QUIET)rm -rf ${PROJECT}.pc

	$(call colorecho,RM, "Clean dependencies")
	$(QUIET)rm -rf ${DEPENDDIR}

	$(call colorecho,RM, "Clean distribution files")
	$(QUIET)rm -rf ${PROJECT}-${VERSION}.tar.gz
	$(QUIET)rm -rf ${PROJECT}.info
	$(QUIET)rm -rf ${PROJECT}/version.h

	$(call colorecho,RM, "Clean code analysis")
	$(QUIET)rm -rf ${LCOV_OUTPUT}
	$(QUIET)rm -rf gcov

	$(QUIET)${MAKE} -C tests clean
	$(QUIET)${MAKE} -C doc clean

doc:
	$(QUIET)${MAKE} -C doc

test: ${PROJECT}
	$(QUIET)${MAKE} -C tests run

example: ${PROJECT}
	$(QUIET)${MAKE} -C example

${PROJECT}.pc: ${PROJECT}.pc.in config.mk
	$(QUIET)echo project=${PROJECT} > ${PROJECT}.pc
	$(QUIET)echo version=${VERSION} >> ${PROJECT}.pc
	$(QUIET)echo apiversion=${LIBFLUSH_VERSION_API} >> ${PROJECT}.pc
	$(QUIET)echo abiversion=${LIBFLUSH_VERSION_ABI} >> ${PROJECT}.pc
	$(QUIET)echo includedir=${INCLUDEDIR} >> ${PROJECT}.pc
	$(QUIET)echo libdir=${LIBDIR} >> ${PROJECT}.pc
	$(QUIET)cat ${PROJECT}.pc.in >> ${PROJECT}.pc

install-static: static
	$(call colorecho,INSTALL,"Install static library")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}
	$(QUIET)install -m 644 ${BUILDDIR_RELEASE}/${PROJECT}.a ${DESTDIR}${LIBDIR}

install-shared: shared
	$(call colorecho,INSTALL,"Install shared library")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}
	$(QUIET)install -m 644 ${BUILDDIR_RELEASE}/${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}
	$(QUIET)ln -s ${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}/${PROJECT}.so.${SOMAJOR} || \
		echo "Failed to create ${PROJECT}.so.${SOMAJOR}. Please check if it exists and points to the correct version of ${PROJECT}.so."
	$(QUIET)ln -s ${PROJECT}.so.${SOVERSION} ${DESTDIR}${LIBDIR}/${PROJECT}.so || \
		echo "Failed to create ${PROJECT}.so. Please check if it exists and points to the correct version of ${PROJECT}.so."

install: options install-static install-shared install-headers

install-headers: ${PROJECT}/version.h ${PROJECT}.pc
	$(call colorecho,INSTALL,"Install pkg-config file")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${LIBDIR}/pkgconfig
	$(QUIET)install -m 644 ${PROJECT}.pc ${DESTDIR}${LIBDIR}/pkgconfig
	$(call colorecho,INSTALL,"Install header files")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${INCLUDEDIR}/${PROJECT}
	$(QUIET)for header in ${HEADERS_INSTALL}; do \
		mkdir -m 755 -p ${DESTDIR}${INCLUDEDIR}/`dirname $$header`; \
		install -m 644 $$header ${DESTDIR}${INCLUDEDIR}/$$header; \
	done

uninstall: uninstall-headers
	$(call colorecho,UNINSTALL,"Remove library files")
	$(QUIET)rm -f ${LIBDIR}/${PROJECT}.a ${LIBDIR}/${PROJECT}.so.${SOVERSION} \
		${LIBDIR}/${PROJECT}.so.${SOMAJOR} ${LIBDIR}/${PROJECT}.so

uninstall-headers:
	$(call colorecho,UNINSTALL,"Remove header files")
	$(QUIET)rm -rf ${INCLUDEDIR}/${PROJECT}
	$(call colorecho,UNINSTALL,"Remove pkg-config file")
	$(QUIET)rm -f ${LIBDIR}/pkgconfig/${PROJECT}.pc

.PHONY: all options clean debug test dist install install-headers \
	uninstall ninstall-headers ${PROJECT} ${PROJECT}-debug static shared \
	install-static install-shared

DEPENDS = ${DEPENDDIRS:^=${DEPENDDIR}/}$(addprefix ${DEPENDDIR}/,${OBJECTS:.o=.o.dep})
-include ${DEPENDS}
