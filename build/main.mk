# $Id: main.mk,v 1.31 2004/11/14 19:05:33 h_oudejans Exp $
#
# Makefile for openMSX Catapult
# =============================
#
# Uses a similar approach as the openMSX build system.

# Functions
# =========

# Function to check a variable has been defined and has a non-empty value.
# Usage: $(call DEFCHECK,VARIABLE_NAME)
DEFCHECK=$(strip \
	$(if $(filter _undefined,_$(origin $(1))), \
		$(error Variable $(1) is undefined) ) \
	)

# Function to check a boolean variable has value "true" or "false".
# Usage: $(call BOOLCHECK,VARIABLE_NAME)
BOOLCHECK=$(strip \
	$(call DEFCHECK,$(1)) \
	$(if $(filter-out _true _false,_$($(1))), \
		$(error Value of $(1) ("$($(1))") should be "true" or "false") ) \
	)

# Will be added to by platform specific Makefile, by flavour specific Makefile
# and by this Makefile.
# Note: LDFLAGS are passed to the linker itself, LINK_FLAGS are passed to the
#       compiler in the link phase.
CXXFLAGS:=
LDFLAGS:=
LINK_FLAGS:=
SOURCES:=	
	
# Logical Targets
# ===============

# Logical targets which require dependency files.
DEPEND_TARGETS:=all default install
# Logical targets which do not require dependency files.
NODEPEND_TARGETS:=clean probe
# Mark all logical targets as such.
.PHONY: $(DEPEND_TARGETS) $(NODEPEND_TARGETS)

# Default target; make sure this is always the first target in this Makefile.
MAKECMDGOALS?=default
default: all


# Base Directories
# ================

SOURCES_PATH:=src
DIALOGS_PATH:=dialogs
BITMAPS_PATH:=resources/bitmaps
MAKE_PATH:=build
BUILD_BASE:=derived

# Customisation
# =============

include $(MAKE_PATH)/custom.mk
$(call DEFCHECK,INSTALL_BASE)

# Version
# =======

include $(MAKE_PATH)/version.mk
PACKAGE_FULL:=$(PACKAGE_NAME)-$(PACKAGE_VERSION)
CHANGELOG_REVISION:=\
	$(shell sed -ne "s/\$$Id: ChangeLog,v \([^ ]*\).*/\1/p" ChangeLog)

# Platforms
# =========

# Note:
# A platform currently specifies both the host platform (performing the build)
# and the target platform (running the created binary). When we have real
# experience with cross-compilation, a more sophisticated system can be
# designed.

ifeq ($(origin CATAPULT_TARGET_CPU),environment)
ifeq ($(origin CATAPULT_TARGET_OS),environment)
# Do not perform autodetection if platform was specified by the user.
else # CATAPULT_TARGET_OS not from environment
$(error You have specified CATAPULT_TARGET_CPU but not CATAPULT_TARGET_OS)
endif # CATAPULT_TARGET_OS
else # CATAPULT_TARGET_CPU not from environment
ifeq ($(origin CATAPULT_TARGET_OS),environment)
$(error You have specified CATAPULT_TARGET_OS but not CATAPULT_TARGET_CPU)
else # CATAPULT_TARGET_OS not from environment

DETECTSYS_PATH:=$(BUILD_BASE)/detectsys
DETECTSYS_MAKE:=$(DETECTSYS_PATH)/detectsys.mk
DETECTSYS_SCRIPT:=$(MAKE_PATH)/detectsys.sh

-include $(DETECTSYS_MAKE)

$(DETECTSYS_MAKE): $(DETECTSYS_SCRIPT)
	@echo "Autodetecting native system:"
	@mkdir -p $(@D)
	@sh $< > $@

endif # CATAPULT_TARGET_OS
endif # CATAPULT_TARGET_CPU	
	
PLATFORM:=
ifneq ($(origin CATAPULT_TARGET_OS),undefined)
ifneq ($(origin CATAPULT_TARGET_CPU),undefined)
PLATFORM:=$(CATAPULT_TARGET_CPU)-$(CATAPULT_TARGET_OS)
endif
endif

# Ignore rest of Makefile if autodetection was not performed yet.
# Note that the include above will force a reload of the Makefile.
ifneq ($(PLATFORM),)

# Load CPU specific settings.
$(call DEFCHECK,CATAPULT_TARGET_CPU)
include $(MAKE_PATH)/cpu-$(CATAPULT_TARGET_CPU).mk
# Check that all expected variables were defined by OS specific Makefile:
# - endianess
$(call BOOLCHECK,BIG_ENDIAN)
# - flavour (user selectable; platform specific default)
$(call DEFCHECK,CATAPULT_FLAVOUR)

# Load OS specific settings.
$(call DEFCHECK,CATAPULT_TARGET_OS)
include $(MAKE_PATH)/platform-$(CATAPULT_TARGET_OS).mk
# Check that all expected variables were defined by OS specific Makefile:
# - executable file name extension
#$(call DEFCHECK,EXEEXT)
# - platform supports symlinks?
#$(call BOOLCHECK,USE_SYMLINK)	
	
# Flavours
# ========

# Load flavour specific settings.
include $(MAKE_PATH)/flavour-$(CATAPULT_FLAVOUR).mk

# Paths
# =====

SEDSCRIPT:=$(MAKE_PATH)/wxg2xrc.sed
CUSTOM_MAKE:=$(MAKE_PATH)/custom.mk
PROBE_SCRIPT:=$(MAKE_PATH)/probe.mk
COMPONENTS_MAKE:=$(MAKE_PATH)/components.mk

BUILD_PATH:=$(BUILD_BASE)/$(PLATFORM)-$(CATAPULT_FLAVOUR)

OBJECTS_PATH:=$(BUILD_PATH)/obj

BINARY_PATH:=$(BUILD_PATH)/bin
BINARY_FILE:=catapult$(EXEEXT)
BINARY_FULL=$(BINARY_PATH)/$(BINARY_FILE) # allow override

LOG_PATH:=$(BUILD_PATH)/log
RESOURCES_PATH:=$(BUILD_PATH)/resources
XRC_PATH:=$(RESOURCES_PATH)/dialogs

CONFIG_PATH:=$(BUILD_PATH)/config
PROBE_MAKE:=$(CONFIG_PATH)/probed_defs.mk
CONFIG_HEADER:=$(CONFIG_PATH)/config.h
VERSION_HEADER:=$(CONFIG_PATH)/Version.ii
	
# Configuration
# =============	
		
include $(MAKE_PATH)/info2code.mk
ifneq ($(filter $(DEPEND_TARGETS),$(MAKECMDGOALS)),)
-include $(PROBE_MAKE)
ifeq ($(PROBE_MAKE_INCLUDED),true)
include $(COMPONENTS_MAKE)
$(call BOOLCHECK,COMPONENT_CORE)
endif
endif


# Filesets
# ========

SOURCES+= \
	CatapultPage \
	wxCatapultApp \
	wxCatapultFrm \
	CatapultConfigDlg \
	ConfigurationData \
	PipeReadThread \
	wxToggleButtonXmlHandler \
	CatapultXMLParser \
	SessionPage \
	InputPage \
	StatusPage \
	VideoControlPage \
	AudioControlPage \
	MiscControlPage \
	openMSXController \
	FullScreenDlg \
	ScreenShotDlg \
	Version \
	RomTypeDlg

OBJECTS_FULL:=$(addprefix $(OBJECTS_PATH)/, $(addsuffix .o,$(SOURCES)))

XRC_FULL:=$(addprefix $(XRC_PATH)/, \
	catapult.xrc \
	config.xrc \
	session.xrc \
	misccontrols.xrc \
	videocontrols.xrc \
	audiocontrols.xrc \
	status.xrc \
	input.xrc \
	fullscreen.xrc \
	screenshot.xrc \
	)

BITMAPS:=$(addprefix $(BUILD_PATH)/,$(wildcard $(BITMAPS_PATH)/*.bmp))

DEPEND_PATH:=$(BUILD_PATH)/dep
DEPEND_FULL:=$(addprefix $(DEPEND_PATH)/,$(addsuffix .d, $(SOURCES)))
DEPEND_FLAGS:=
# Empty definition of used headers, so header removal doesn't break things.
DEPEND_FLAGS+=-MP


# Compiler flags
# ==============

CXXFLAGS+=-pipe -Wall
CXXFLAGS+=-I$(CONFIG_PATH)
CXXFLAGS+= $(XRC_CFLAGS) $(XML_CFLAGS)
LDFLAGS+= $(XRC_LDFLAGS) $(XML_LDFLAGS)

# Strip binary?
CATAPULT_STRIP?=false
$(call BOOLCHECK,CATAPULT_STRIP)
ifeq ($(CATAPULT_STRIP),true)
  LDFLAGS+=--strip-all
endif

# Build Rules
# ===========

# Do not build if core component dependencies are not met.
ifeq ($(COMPONENT_CORE),false)
DUMMY:=$(shell rm -f $(PROBE_MAKE))
$(error Cannot build Catapult because essential libraries are unavailable. \
Please install the needed libraries and rerun (g)make)
endif

ifeq ($(PROBE_MAKE_INCLUDED),true)
# If probe was succesful, it's safe to use wx-config.
CXX:=$(shell wx-config --cxx)
endif

# Force a probe if "probe" target is passed explicitly.
ifneq ($(filter probe,$(MAKECMDGOALS)),)
probe: $(PROBE_MAKE)
.PHONY: $(PROBE_MAKE)
endif

# Probe for libraries.
# TODO: It would be cleaner to include probe.mk and probe-results.mk,
#       instead of executing them in a sub-make.
$(PROBE_MAKE): $(PROBE_SCRIPT)
	@OUTDIR=$(@D) COMPILE=g++ MAKE_LOCATION=$(MAKE_PATH) CURRENT_OS=$(CATAPULT_TARGET_OS)\
		$(MAKE) --no-print-directory -f $<
	@PROBE_MAKE=$(PROBE_MAKE) MAKE_PATH=$(MAKE_PATH) \
		$(MAKE) --no-print-directory -f $(MAKE_PATH)/probe-results.mk

# TODO: Relying on CONFIG_HEADER being built before BINARY_FULL,
#       this might break parallelized builds.
all: $(CONFIG_HEADER) $(VERSION_HEADER) config $(BINARY_FULL) $(XRC_FULL) $(BITMAPS)

config:
	@echo "Build configuration"
	@echo "  Platform: $(PLATFORM)"
	@echo "  Flavour:  $(CATAPULT_FLAVOUR)"

$(BINARY_FULL): $(OBJECTS_FULL)
	@echo "Linking $(BINARY_FILE)..."
	@mkdir -p $(@D)
	@$(CXX) -o $@ $(OBJECTS_FULL) $(LDFLAGS) 

# Compile and generate dependency files in one go.
DEPEND_SUBST=$(patsubst $(SOURCES_PATH)/%.cpp,$(DEPEND_PATH)/%.d,$<)
$(OBJECTS_FULL): $(OBJECTS_PATH)/%.o: $(SOURCES_PATH)/%.cpp $(DEPEND_PATH)/%.d
	@echo "Compiling $(<:$(SOURCES_PATH)/%.cpp=%)..."
	@mkdir -p $(@D)
	@mkdir -p $(patsubst $(OBJECTS_PATH)%,$(DEPEND_PATH)%,$(@D))
	@$(CXX) $(DEPEND_FLAGS) -MMD -MF $(DEPEND_SUBST) \
		-o $@ $(CXXFLAGS) -c $<
	@touch $@ # Force .o file to be newer than .d file.

$(XRC_FULL): $(XRC_PATH)/%.xrc: $(DIALOGS_PATH)/%.wxg $(SEDSCRIPT)
	@echo "Converting $(@:$(XRC_PATH)/%=%)..."
	@mkdir -p $(@D)
	@sed -f $(SEDSCRIPT) $< > $@

$(BITMAPS): $(BUILD_PATH)/%: %
	@echo "Copying $(<:$(BITMAPS_PATH)/%=%)..."
	@mkdir -p $(@D)
	@cp $< $@

clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_PATH)


# Installation and Binary Packaging
# =================================

CATAPULT_PREBUILT?=false
$(call BOOLCHECK,CATAPULT_PREBUILT)

INSTALL_DOCS:=release-notes.txt release-history.txt
CATAPULT_INSTALL?=$(INSTALL_BASE)
# Allow full customization of locations, used by Debian packaging.
INSTALL_BINARY_DIR?=$(CATAPULT_INSTALL)/bin
INSTALL_SHARE_DIR?=$(CATAPULT_INSTALL)
INSTALL_DOC_DIR?=$(CATAPULT_INSTALL)/doc


ifeq ($(CATAPULT_PREBUILT),true)
# TODO: Prebuilt is used only on win32, but using this is not clean.
BINARY_FILE:=catapult.exe
FILES_ONLY:=true
install: check_build
else
FILES_ONLY:=false
install: all
endif
	@echo "Installing to $(CATAPULT_INSTALL):"
	@echo "  Executable..."
	@mkdir -p $(INSTALL_BINARY_DIR)
ifeq ($(CATAPULT_PREBUILT),true)
	@cp -f $(BINARY_FULL) $(INSTALL_BINARY_DIR)/$(BINARY_FILE)
else
	@strip -o $(INSTALL_BINARY_DIR)/$(BINARY_FILE) $(BINARY_FULL)
endif
	@echo "  Data files..."
	@mkdir -p $(INSTALL_SHARE_DIR)
	@cp -rf $(RESOURCES_PATH) $(INSTALL_SHARE_DIR)/
	@echo "  Documentation..."
	@mkdir -p $(INSTALL_DOC_DIR)
	@cp -f README GPL AUTHORS $(INSTALL_DOC_DIR)
	@cp -f $(addprefix doc/,$(INSTALL_DOCS)) $(INSTALL_DOC_DIR)
	@mkdir -p $(INSTALL_DOC_DIR)/manual
	@cp -f $(addprefix doc/manual/,*.html *.css *.png) \
		$(INSTALL_DOC_DIR)/manual
ifeq ($(CATAPULT_PREBUILT),false)
ifneq ($(CATAPULT_NO_DESKTOP_HOOKS),true)
	@echo "  Desktop hooks..."
	@mkdir -p $(INSTALL_SHARE_DIR)/resources/icons
	@cp -rf src/catapult.xpm $(INSTALL_SHARE_DIR)/resources/icons
	@if [ -d /usr/share/applications -a -w /usr/share/applications ]; \
		then sed -e "s|%INSTALL_BASE%|$(INSTALL_SHARE_DIR)|" \
			desktop/openMSX-Catapult.desktop \
			> /usr/share/applications/openMSX-Catapult.desktop; \
		else mkdir -p ~/.local/share/applications && \
			sed -e "s|%INSTALL_BASE%|$(INSTALL_SHARE_DIR)|" \
			desktop/openMSX-Catapult.desktop \
			> ~/.local/share/applications/openMSX-Catapult.desktop; \
		fi
endif
ifeq ($(SYMLINK_FOR_BINARY),true)
	@echo "  Creating symlink..."
	@if [ -d /usr/local/bin -a -w /usr/local/bin ]; \
		then ln -sf $(INSTALL_BINARY_DIR)/$(BINARY_FILE) \
			/usr/local/bin/$(BINARY_FILE); \
		else if [ -d ~/bin ]; \
			then ln -sf $(INSTALL_BINARY_DIR)/$(BINARY_FILE) \
				~/bin/$(BINARY_FILE); \
			fi; \
		fi
endif
	@echo "  Setting permissions..."
	@chmod -R a+rX $(INSTALL_SHARE_DIR)
endif # CATAPULT_PREBUILT
	@echo "Installation complete... have fun!"

TARGET_CATAPULT:=$(wildcard $(BINARY_PATH)/$(BINARY_FILE))

check_build:
ifeq ($(TARGET_CATAPULT),)
	$(error Create Catapult first)
endif 

# Source Packaging
# ================

DIST_BASE:=$(BUILD_BASE)/dist
DIST_PATH:=$(DIST_BASE)/$(PACKAGE_FULL)

DIST_FULL:= \
	GNUmakefile ChangeLog AUTHORS GPL README
DIST_FULL+=$(addprefix $(SOURCES_PATH)/, \
	*.h *.cpp *.rc *.ico *.xpm \
	)
DIST_FULL+=$(addprefix $(MAKE_PATH)/, \
	*.mk *.sed *.dsp *.dsw \
	)
DIST_FULL+=$(DIALOGS_PATH)/*.wxg
DIST_FULL+=$(BITMAPS_PATH)/*.bmp
DIST_FULL+=$(addprefix doc/, \
	*.txt \
	$(addprefix manual/, \
		*.html *.css *.png \
		) \
	)
DIST_FULL+=$(addprefix desktop/, \
	openMSX-Catapult.desktop \
	)

dist: $(DETECTSYS_SCRIPT)
	@echo "Removing any old distribution files..."
	@rm -rf $(DIST_PATH)
	@echo "Gathering files for distribution..."
	@mkdir -p $(DIST_PATH)
	@cp -p --parents $(DIST_FULL) $(DIST_PATH)
	@echo "Creating tarball..."
	@cd $(DIST_BASE) ; GZIP=--best tar zcf $(PACKAGE_FULL).tar.gz $(PACKAGE_FULL)


# Dependencies
# ============

# Include dependency files.
ifneq ($(filter $(DEPEND_TARGETS),$(MAKECMDGOALS)),)
  -include $(DEPEND_FULL)
endif

# Generate dependencies that do not exist yet.
# This is only in case some .d files have been deleted;
# in normal operation this rule is never triggered.
$(DEPEND_FULL):

endif # PLATFORM