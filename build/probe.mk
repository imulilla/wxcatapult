# $Id: probe.mk,v 1.6 2004/05/09 21:52:15 mthuurne Exp $
#
# Replacement for "configure".
# Performs some test compiles, to check for headers and functions.
# Unlike configure, it does not run any test code, so it is more friendly for
# cross compiles.

# This Makefile needs parameters to operate; check that they were specified:
# - output directory
ifeq ($(OUTDIR),)
$(error Missing parameter: OUTDIR)
endif
# - command line to invoke compiler + options
ifeq ($(COMPILE),)
$(error Missing parameter: COMPILE)
endif

# Result files.
LOG:=$(OUTDIR)/probe.log
OUTMAKE:=$(OUTDIR)/probed_defs.mk


# Headers
# =======

CHECK_HEADERS:=$(addsuffix _H, \
	WX XRC XML )

WX_HEADER:=<wx/version.h>
WX_CFLAGS:=`wx-config --cflags 2>> $(LOG)`

XML_HEADER:=<libxml/parser.h>
XML_CFLAGS:=`xml2-config --cflags 2>> $(LOG)`

XRC_HEADER:=<wx/xrc/xml.h>
XRC_CFLAGS:=$(WX_CFLAGS)


# Libraries
# =========

CHECK_LIBS:=WX XRC XML

WX_LDFLAGS:=`wx-config --libs 2>> $(LOG)`
WX_RESULT:=`wx-config --version`

XML_LDFLAGS:=`xml2-config --libs 2>> $(LOG)`
XML_RESULT:=`xml2-config --version`

XRC_LDFLAGS:=`((wx-config --libs > /dev/null) && (wx-config --libs | sed -e "s/-lwx_\([^-]*\)-\([^ ]*\)/& -lwx_\1_xrc-\2/")) 2>> $(LOG)`
XRC_RESULT:=yes


# Implementation
# ==============

CHECK_TARGETS:=$(CHECK_HEADERS) $(CHECK_LIBS)
PRINT_LIBS:=$(addsuffix -print,$(CHECK_LIBS))

.PHONY: all init check-targets print-libs $(CHECK_TARGETS) $(PRINT_LIBS)

# Default target.
all: check-targets print-libs
check-targets: $(CHECK_TARGETS)
print-libs: $(PRINT_LIBS)

# Create empty log and result files.
init:
	@echo "Probing target system..."
	@mkdir -p $(OUTDIR)
	@echo "Probing system:" > $(LOG)
	@echo "# Automatically generated by build system." > $(OUTMAKE)
	@echo "# Non-empty value means found, empty means not found." >> $(OUTMAKE)
	@echo "PROBE_MAKE_INCLUDED:=true" >> $(OUTMAKE)

# Probe for header:
# Try to include the header.
$(CHECK_HEADERS): init
	@echo > $(OUTDIR)/$@.cc
	@if [ -n "$($(@:%_H=%)_PREHEADER)" ]; then \
		echo "#include $($(@:%_H=%)_PREHEADER)"; fi >> $(OUTDIR)/$@.cc
	@echo "#include $($(@:%_H=%)_HEADER)" >> $(OUTDIR)/$@.cc
	@if FLAGS="$($(@:%_H=%_CFLAGS))" && $(COMPILE) $(CXXFLAGS) $$FLAGS \
		-c $(OUTDIR)/$@.cc -o $(OUTDIR)/$@.o 2>> $(LOG); \
	then echo "Found header: $(@:%_H=%)" >> $(LOG); \
	     echo "HAVE_$@:=true" >> $(OUTMAKE); \
	else echo "Missing header: $(@:%_H=%)" >> $(LOG); \
	     echo "HAVE_$@:=" >> $(OUTMAKE); \
	fi
	@rm -f $(OUTDIR)/$@.cc $(OUTDIR)/$@.o

# Probe for library:
# Try to link dummy program to the library.
$(CHECK_LIBS): init
	@echo "int main(char **argv, int argc) { return 0; }" > $(OUTDIR)/$@.cc
	@if FLAGS="$($@_LDFLAGS)" && $(COMPILE) $(CXXFLAGS) \
		$(OUTDIR)/$@.cc -o $(OUTDIR)/$@.exe $(LINK_FLAGS) $$FLAGS 2>> $(LOG); \
	then echo "Found library: $@" >> $(LOG); \
	     echo "HAVE_$@_LIB:=$($@_RESULT)" >> $(OUTMAKE); \
	else echo "Missing library: $@" >> $(LOG); \
	     echo "HAVE_$@_LIB:=" >> $(OUTMAKE); \
	fi
	@rm -f $(OUTDIR)/$@.cc $(OUTDIR)/$@.exe

# Print the flags for using a certain library (CFLAGS and LDFLAGS).
$(PRINT_LIBS): check-targets
	@echo "$(@:%-print=%)_CFLAGS:=$($(@:%-print=%)_CFLAGS)" >> $(OUTMAKE)
	@echo "$(@:%-print=%)_LDFLAGS:=$($(@:%-print=%)_LDFLAGS)" >> $(OUTMAKE)

