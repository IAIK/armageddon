# See LICENSE file for license and copyright information

include config.mk
include colors.mk
include common.mk

SOURCE = $(wildcard ${PROJECT}/*.c)

ifneq ($(wildcard ${VALGRIND_SUPPRESSION_FILE}),)
VALGRIND_ARGUMENTS += --suppressions=${VALGRIND_SUPPRESSION_FILE}
endif

OBJECTS       = $(addprefix ${BUILDDIR_RELEASE}/,${SOURCE:.c=.o})
OBJECTS_DEBUG = $(addprefix ${BUILDDIR_DEBUG}/,${SOURCE:.c=.o})

ifeq "${ARCH}" "armv7"
	include config-arm.mk
	CPPFLAGS += -DARM_DEVICE
endif

ifeq "${ARCH}" "armv8"
	include config-arm64.mk
	CPPFLAGS += -D__ARM_ARCH_8A__
endif

ifneq (${WITH_THREADS}, 0)
CPPFLAGS += -DWITH_THREADS
LIBS += -pthread
endif

ifneq (${WITH_ANDROID}, 0)
CPPFLAGS += -DWITH_ANDROID
endif

all: options ${PROJECT}

# pkg-config based version checks
.version-checks/%: config.mk
	$(QUIET)test $($(*)_VERSION_CHECK) -eq 0 || \
		${PKG_CONFIG} --atleast-version $($(*)_MIN_VERSION) $($(*)_PKG_CONFIG_NAME) || ( \
		echo "The minimum required version of $(*) is $($(*)_MIN_VERSION)" && \
		false \
	)
	@mkdir -p .version-checks
	$(QUIET)touch $@

options:
	@echo ${PROJECT} build options:
	@echo "CFLAGS  = ${CFLAGS}"
	@echo "LIBS    = ${LIBS}"
	@echo "DFLAGS  = ${DFLAGS}"
	@echo "CC      = ${CC}"

# release build

${OBJECTS}: config.mk .version-checks/LIBFLUSH

${BUILDDIR_RELEASE}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $@)
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF ${DEPENDDIR}/$@.dep

${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}: ${OBJECTS}
	$(call colorecho,CC,$@)
	@mkdir -p ${BUILDDIR_RELEASE}/${BINDIR}
	$(QUIET)${CC} ${SFLAGS} ${LDFLAGS} \
		-o ${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT} ${OBJECTS} ${LIBS}

${PROJECT}: ${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}

release: ${PROJECT}

run: release
	$(QUIET)./${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT}

# debug build

${OBJECTS_DEBUG}: config.mk .version-checks/LIBFLUSH

${BUILDDIR_DEBUG}/%.o: %.c
	$(call colorecho,CC,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $@)
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$@.dep

${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}: ${OBJECTS_DEBUG}
	$(call colorecho,CC,$@)
	@mkdir -p ${BUILDDIR_DEBUG}/${BINDIR}
	$(QUIET)${CC} ${LDFLAGS} \
		-o ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT} ${OBJECTS_DEBUG} ${LIBS}

debug: ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

run-debug: debug
	$(QUIET)./${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

# clean

clean:
	$(QUIET)rm -rf \
		${BUILDDIR} \
		${DEPENDDIR} \
		${TARFILE} \
		${TARDIR} \

valgrind: debug
	 $(QUIET)G_SLICE=always-malloc G_DEBUG=gc-friendly ${VALGRIND} ${VALGRIND_ARGUMENTS} \
		 ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

gdb: debug
	$(QUIET)cgdb ${BUILDDIR_DEBUG}/${BINDIR}/${PROJECT}

dist: clean
	$(QUIET)tar -czf $(TARFILE) --exclude=.gitignore \
		--transform 's,^,${PROJECT}-$(VERSION)/,' \
		`git ls-files`

install: all
	$(call colorecho,INSTALL,"executeable file")
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${PREFIX}/bin
	$(QUIET)install -m 755 ${BUILDDIR_RELEASE}/${BINDIR}/${PROJECT} ${DESTDIR}${PREFIX}/bin

uninstall:
	$(ECHO) removing executable file
	$(call colorecho,UNINSTALL,"executeable")
	$(QUIET)rm -f ${DESTDIR}${PREFIX}/bin/${PROJECT}

DEPENDS = ${DEPENDDIRS:^=${DEPENDDIR}/}$(addprefix ${DEPENDDIR}/,${OBJECTS:.o=.o.dep})
-include ${DEPENDS}

.PHONY: all options clean doc debug valgrind gdb dist install uninstall
