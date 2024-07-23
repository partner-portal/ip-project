#
# Copyright (c) 2014-2023, ProvenRun and/or its affiliates. All rights reserved.
#

MAKEFLAGS += --no-builtin-rules

DEBUG               ?= 0
BASE_DIR            := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BASE_DIR            := $(BASE_DIR:/=)
BUILD_BASE          ?= $(abspath $(BASE_DIR)/build/$(VARIANT))
USER_PROGS_FILE     := $(BUILD_BASE)/.user_progs
SEL_USER_PROGS_FILE := $(BUILD_BASE)/.sel_user_progs
PROG_TIMESTAMP      := $(BUILD_BASE)/.user_progs_timestamp
USER_LIBS_FILE      := $(BUILD_BASE)/.user_libs
APPS_DEP_FILE       := $(BUILD_BASE)/.apps.dep
CONFIG_TIMESTAMP    := $(BUILD_BASE)/.config_timestamp
CONFIGS_DIR         := $(BASE_DIR)/configs

# Allow sub Makefile to decide if they need to compile for ProvenCore
# (instead of e.g. for Linux. Useful for prototyping.)
PROVENCORE_BUILD    := 1

# Unconditionnaly export these variables to sub Makefile
# They don't have to re-export them, it will be done automatically by make
export PROVENCORE_BUILD DEBUG BUILD_BASE BASE_DIR CONFIGS_DIR

include $(BASE_DIR)/scripts/build/verbose.mk

# All generated files or link should go in $(BUILD_BASE): for apps, libs and
# kconfig
APP_DIR             := $(BUILD_BASE)/app
APP_SRC_DIR         := $(BUILD_BASE)/app/src
APP_BUILD_DIR       := $(BUILD_BASE)/app/build
INCLUDE_DIR         := $(BUILD_BASE)/include
LIB_DIR             := $(BUILD_BASE)/lib
LIB_SRC_DIR         := $(BUILD_BASE)/lib/src
LIB_BUILD_DIR       := $(BUILD_BASE)/lib/build
KCONFIG_DIR         := $(BUILD_BASE)/kconfig
DOC_DIR             := $(BUILD_BASE)/doc

# Define generic paths for different platform config files:
CONFIG_FILE         := $(KCONFIG_DIR)/config.mk
TARGET_FILE         := $(KCONFIG_DIR)/target.mk
APP_FILE            := $(KCONFIG_DIR)/pnc_app.mk
IMAGE_SCRIPT        := $(KCONFIG_DIR)/do_image
KCONFIG_BIN         := $(KCONFIG_DIR)/kernel_config.bin

# Targets that can be called before config.
PRE_CONFIG_TARGETS := \
	%_config distclean

# Explicit default target.
all:
ifeq ($(strip $(MAKECMDGOALS)),)
    MAKECMDGOALS := all
endif

# Trick for autocompletion (%_config)
CONFIGS := $(notdir $(patsubst %/,%,$(shell ls -d $(CONFIGS_DIR)/*/)))
define AUTOCOMPLETE_CONFIG
$(1)_config:
endef
$(foreach dir,$(CONFIGS),$(eval $(call AUTOCOMPLETE_CONFIG,$(dir))))

# Enable parts of the Makefile dealing with configuration if make is
# called with not pre-config targets.
ifneq ($(strip $(filter-out $(PRE_CONFIG_TARGETS), $(MAKECMDGOALS))),)

# Make sure $(plat)_config has been called.
ifneq ($(words $(wildcard $(TARGET_FILE) $(CONFIG_FILE))),2)
    $(error "No configuration found -- please run make $$(plat)_config first")
endif

# Make sure python3 is intalled
ifeq (, $(shell which python3))
    $(error "No python3 in PATH, consider doing apt-get install python3")
endif

# Include current platform and kernel configuration.
include $(TARGET_FILE) $(CONFIG_FILE)

# Kernel does not require user features.
FEATURES := $(kernel_FEATURES)

# Include target compiler default tools and options
COMPILE_ARCH := $(KARCH)
include $(BASE_DIR)/scripts/build/common_target.mk

TAR = tar

all: world
	$(Q)$(MAKE) -C kconfig check

# Builds all recipes needed to build the final image
world: programs $(BUILD_BASE)/provencore.bin

# 'image' rule
include $(BASE_DIR)/scripts/build/do_image.mk

-include $(APP_FILE)

$(PROG_TIMESTAMP): FORCE programs-build ;

$(APP_DIR)/codes.tar: $(PROG_TIMESTAMP) $(KCONFIG_BIN) $(SEL_USER_PROGS_FILE) | $(APP_DIR)
	@echo "  ARCHIVE for kernel" $(ECHO_OUTPUT)
	$(eval APP_LIST := $(shell cat $(SEL_USER_PROGS_FILE) 2> /dev/null || echo ""))
	$(Q)$(TAR) -c -f $@ \
	    --owner=0 --group=0 --numeric-owner --mtime='1970-01-01UTC' \
	    -C $(KCONFIG_DIR) kernel_config.bin \
	    -C $(APP_BUILD_DIR) $(APP_LIST) $(ECHO_OUTPUT)

$(KCONFIG_BIN) $(SEL_USER_PROGS_FILE): kconfig-bin

kconfig-bin: $(CONFIG_TIMESTAMP) FORCE
	$(Q)$(MAKE) -C kconfig bin

$(INCLUDE_DIR)/metadata/metadata.h: $(SEL_USER_PROGS_FILE)
	$(Q)mkdir -p $(INCLUDE_DIR)/metadata
	$(Q)$(BASE_DIR)/tools/metadata-config/generate-metadata-header.sh \
	    -o $(INCLUDE_DIR)/metadata/metadata.h \
	    -I $(INCLUDE_DIR)/metadata \
	    -I $(BASE_DIR)/include/metadata \
	    `cat $(SEL_USER_PROGS_FILE)`

$(APP_DIR)/codes.header: $(APP_DIR)/app_header.bin | $(APP_DIR)
	$(eval APP_HEADER_SIZE := $(shell stat -c %s $<))
	$(Q)if [ "$(APP_HEADER_SIZE)" != "16" ]; then \
	    echo "Invalid application header size: $(APP_HEADER_SIZE)"; \
	    exit 1; \
	fi
	$(Q)cp $< $@.tmp
	$(Q)truncate -s %512 $@.tmp
	$(Q)mv $@.tmp $@

$(BUILD_BASE)/codes.bin: $(APP_DIR)/codes.header $(APP_DIR)/codes.tar
	$(Q)cat $^ > $@

programs-build: lib
	$(Q)$(MAKE) PROG_FILE=$(USER_PROGS_FILE) -C applications all

programs: $(BUILD_BASE)/codes.bin

clean-programs:
	$(Q)$(MAKE) -C applications clean
	$(Q)rm -f  $(BUILD_BASE)/codes.bin
	$(Q)rm -f  $(APP_DIR)/app_header.*
	$(Q)rm -f  $(APP_DIR)/codes.header
	$(Q)rm -f  $(APP_DIR)/codes.tar
	$(Q)rm -f  $(APPS_DEP_FILE)
	$(Q)rm -rf $(APP_SRC_DIR)
	$(Q)rmdir  --ignore-fail-on-non-empty $(APP_DIR)

##
# Configure applications and kernel.
# The dependence tree is:
#   - `config-apps` (1, 2)
#     - `config-kconfig`  (A1, A2)
#   - `config-libs`     (B1, B2)
##
config: FORCE config-kconfig config-libs
	$(Q)touch $(CONFIG_TIMESTAMP)

# Automatically call config if it was not called previously
$(CONFIG_TIMESTAMP):
	$(Q)$(MAKE) -C $(BASE_DIR) config

# 1. create symlinks for applications in $(USER_PROGS_FILE)
# 2. call applications depend
config-apps: tools
	$(Q)mkdir -p $(INCLUDE_DIR)/apps
	$(Q)\
	    for app in $(USER_APPS); do \
	        base="$$(dirname $$app)"; \
	        name="$$(basename $$app)"; \
	        path="$(BASE_DIR)/applications/$$app"; \
	        if [ -d $$path ]; then \
	            mkdir -p "$(APP_SRC_DIR)/$$base"; \
	            ln -snf $$path $(APP_SRC_DIR)/$$app; \
	            if [ -d $$path/include ]; then \
	                mkdir -p "$(INCLUDE_DIR)/apps/$$base"; \
	                ln -snf $$path/include $(INCLUDE_DIR)/apps/$$app; \
	            fi; \
	        fi; \
	    done
	$(Q)$(MAKE) -C applications depend LIBS_FILE=$(APPS_DEP_FILE)

$(APPS_DEP_FILE): config-apps

# A1. generate system headers
# A2. call application config
config-kconfig: config-apps
	$(Q)$(MAKE) -C kconfig headers
	$(Q)$(MAKE) -C applications config

# B1. build $(USER_LIBS_FILE) to contain all libraries in the transitive
#     closure of $(APPS_DEP_FILE)
#     For release: $(USER_LIBS_FILE) will contain only libs whose source is
#     available in $(LIB_SRC_DIR), otherwise expect lib binary to be stored in
#     $(LIB_DIR)
# B2. create symlinks for libraries in $(USER_LIBS_FILE)
#     For release: create symlinks for headers directories of libraries shipped
#     as binary
config-libs: $(APPS_DEP_FILE)
	+$(Q)if [ -s $< ]; then \
		xargs -a $< $(BASE_DIR)/scripts/build/dep/compute_closure.sh \
		    --kernel=$(BASE_DIR) \
			--create-as-needed \
			--release \
			--variant=$(VARIANT) \
			--incpath $(BASE_DIR)/include/libs \
			--libpath $(BASE_DIR)/lib > $(USER_LIBS_FILE); \
		fi
	$(Q)if [ -s $(USER_LIBS_FILE) ]; then \
	    $(MAKE) LIBS_FILE=$(USER_LIBS_FILE) -C lib config; \
	else \
	    rm -f $(USER_LIBS_FILE); \
	fi

# lib:
# build lib(s)
# set lib(s) for each app
lib: $(CONFIG_TIMESTAMP) $(INCLUDE_DIR)/metadata/metadata.h FORCE
	$(Q)$(MAKE) LIBS_FILE=$(USER_LIBS_FILE) -C lib all

clean-lib:
	$(Q)$(MAKE) LIBS_FILE=$(USER_LIBS_FILE) -C lib clean
	$(Q)rm -f  $(USER_LIBS_FILE)
	$(Q)rm -rf $(LIB_SRC_DIR)
	$(Q)rmdir  --ignore-fail-on-non-empty $(LIB_DIR)

$(APP_DIR):
	$(Q)mkdir -p $@

##
# Defines KERNEL_DATA_END and KERNEL_IMAGE_END
##
include $(BUILD_BASE)/kernel_limits.mk

##
# Generate a single binary containing the kernel and application binaries
# as would be produced by the option SINGLE_BINARY_IMAGE
# The binary is generated as $(BUILD_BASE)/provencore.bin
##
.PHONY: single-binary
single-binary: $(BUILD_BASE)/kernel.bin $(BUILD_BASE)/codes.bin
	@echo "Merging kernel.bin and codes.bin" $(ECHO_OUTPUT)
	$(eval padding := \
	    $(shell printf "%u - %u\n" $(KERNEL_IMAGE_END) $(KERNEL_DATA_END) | bc))
	$(Q)cp $(BUILD_BASE)/kernel.bin $(BUILD_BASE)/provencore.bin.tmp
	$(Q)truncate -s +$(padding) $(BUILD_BASE)/provencore.bin.tmp
	$(Q)truncate -s %4096 $(BUILD_BASE)/provencore.bin.tmp
	$(Q)cat $(BUILD_BASE)/provencore.bin.tmp $(BUILD_BASE)/codes.bin \
	    > $(BUILD_BASE)/provencore.bin
	$(Q)rm -f $(BUILD_BASE)/provencore.bin.tmp

ifneq ($(filter SINGLE_BINARY_IMAGE, $(FEATURES)),)

$(BUILD_BASE)/provencore.bin: single-binary ;

else

$(BUILD_BASE)/provencore.bin: $(BUILD_BASE)/kernel.bin
	$(Q)cp $< $@

endif

$(APP_DIR)/app_header.o: $(APP_DIR)/codes.tar $(BASE_DIR)/applications/app_header.c
	$(eval STATIC_ARCHIVES_SIZE := $(shell stat -c %s $<))
	$(Q)$(CC) \
	    $(TARGET_CFLAGS) -ffreestanding \
	    -DSTATIC_ARCHIVES_SIZE=$(STATIC_ARCHIVES_SIZE)UL \
	    -DCONFIG_KARCH_$(shell echo $(KARCH) | tr a-z A-Z) \
	    -DCONFIG_KARCH_BITS_$(shell echo $(KARCH_BITS) | tr a-z A-Z) \
	    -I$(BASE_DIR)/include/ \
	    -I$(BUILD_BASE)/kconfig \
	    -c $(BASE_DIR)/applications/app_header.c -o $@ -MD -MT $@

-include $(APP_DIR)/app_header.d

$(APP_DIR)/app_header.bin: $(APP_DIR)/app_header.o
	$(Q)$(OC) -O binary --only-section=.app.header $< $@

mrproper: clean

clean: clean-programs clean-lib
	@echo "  CLEAN root" $(ECHO_OUTPUT)
	$(Q)$(MAKE) -C kconfig clean
	$(Q)rm -f  $(BUILD_BASE)/provencore.bin
	$(Q)rm -rf $(BUILD_BASE)/dep
	$(Q)rm -f  $(CONFIG_TIMESTAMP)
	$(Q)rm -rf $(INCLUDE_DIR)



endif # End !PRE_CONFIG_TARGETS

%_config:
	@# We start from scratch, remove and replace the KCONFIG directory
	$(Q)rm -Rf $(KCONFIG_DIR)
	$(Q)mkdir -p $(KCONFIG_DIR)
	@# Check that the configuration contains at least the mandatory files
	@# These checks are not mandatory, as the build system will fail anyway
	@# at a later time with an explicit message, but it is always nice to fail early.
	$(Q)if ! [ -f "$(BASE_DIR)/configs/$*/kernel_config.dts" ]; then \
	        echo "Error: '$*' is not a valid configuration (kernel_config.dts not found)\n"; \
	        exit 1; \
	    fi
	$(Q)if ! [ -f "$(BASE_DIR)/configs/$*/pnc_app.mk" ]; then \
	        echo "Error: '$*' is not a valid configuration (pnc_app.mk not found)\n"; \
	        exit 1; \
	    fi
	$(Q)if ! [ -f "$(BASE_DIR)/configs/$*/config.mk" ]; then \
	        echo "Error: '$*' is not a valid configuration (config.mk not found)\n"; \
	        exit 1; \
	    fi
	$(Q)if ! [ -f "$(BASE_DIR)/configs/$*/target.mk" ]; then \
	        echo "Error: '$*' is not a valid configuration (target.mk not found)\n"; \
	        exit 1; \
	    fi
	$(Q)cp -LRf "$(BASE_DIR)/configs/$(@:_config=)/"* "$(KCONFIG_DIR)"
	$(Q)echo "$*" > "$(BUILD_BASE)/.config"
	@# Copy pre-built files
	$(Q)cp -r "$(BASE_DIR)/prebuilt/$(@:_config=)/". "$(BUILD_BASE)"
	@echo "Target '$*' configured..."

distclean:
	@echo "  DISTCLEAN $(VARIANT)" $(ECHO_OUTPUT)
	$(Q)rm -rf $(BUILD_BASE)

FORCE: ;

.PHONY: all clean distclean mrproper kconfig-bin \
        programs programs-build clean-programs clean-lib FORCE

.SUFFIXES:
