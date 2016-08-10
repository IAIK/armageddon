# See LICENSE file for license and copyright information
#
include ../config.mk
include ../common.mk
include ../colors.mk
include config.mk

DOXYGEN_SOURCES=$(wildcard ../libflush/*.h) Doxyfile
HTML_SOURCES=$(wildcard *.rst) conf.py

SPHINX_OPTS+=-d $(SPHINX_BUILDDIR)/doctrees

ifeq ($(shell which $(SPHINX_BIN) >/dev/null 2>&1; echo $$?), 1)
	$(error The '$(SPHINX_BIN)' command was not found.)
endif

all: html

clean:
	$(call colorecho,RM,"Clean documentation")
	$(QUIET)rm -rf $(SPHINX_BUILDDIR)/

$(SPHINX_BUILDDIR)/html/index.html: $(HTML_SOURCES) $(SPHINX_BUILDDIR)/doxygen/xml/index.xml
	$(call colorecho,DOC,"Build HTML documentation")
	$(QUIET)$(SPHINX_BIN) -b html $(SPHINX_OPTS) . $(SPHINX_BUILDDIR)/html

$(SPHINX_BUILDDIR)/doxygen/xml/index.xml: $(DOXYGEN_SOURCES)
	$(QUIET)mkdir -p $(SPHINX_BUILDDIR)
	$(call colorecho,DOC,"Run doxygen")
	$(QUIET)$(DOXYGEN_BIN) Doxyfile

# TODO: Make a better test for breathe and sphinx_rtd_theme
ifeq ($(shell which $(DOXYGEN_BIN) >/dev/null 2>&1 && $(PYTHON_BIN) -c "import breathe; import sphinx_rtd_theme" >/dev/null 2>&1; echo $$?), 0)
html: $(SPHINX_BUILDDIR)/html/index.html
else
html:
endif

.PHONY: clean html man all
