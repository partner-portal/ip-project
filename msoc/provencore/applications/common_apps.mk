#
# Copyright (c) 2016-2023, ProvenRun and/or its affiliates. All rights reserved.
#

ifndef BASE_DIR
    $(error BASE_DIR is not defined, must be set by calling Makefile)
endif

ifndef APPNAME
    $(error APPNAME is not defined, must be set by calling Makefile)
endif

ifndef BUILD_BASE
BUILD_BASE          := $(BASE_DIR)/build/$(VARIANT)
endif

# Note: this path cannot be overridden
APP_DIR := $(BUILD_BASE)/app

ifndef APP_SRC_DIR
APP_SRC_DIR         := $(APP_DIR)/src/$(APPNAME)
endif

# Note: $(BUILD_BASE)/app/build/$(APPNAME) cannot be changed as
# applications/Makefile looks for .binaries in this directory.
ifndef APP_BUILD_DIR
APP_BUILD_DIR       := $(APP_DIR)/build/$(APPNAME)/build
endif

# Note: there is no value in letting the user _choose_ $(APP_BUILD_OBJ_DIR), but
# the user should know where to add additional obj files.
APP_BUILD_OBJ_DIR   := $(APP_BUILD_DIR)/obj

INCLUDE_DIR := $(BUILD_BASE)/include

include $(BASE_DIR)/scripts/build/verbose.mk

TARGET_FILE := $(BUILD_BASE)/kconfig/target.mk
CONFIG_FILE := $(BUILD_BASE)/kconfig/config.mk

# Include current platform and kernel configuration.
-include $(TARGET_FILE) $(CONFIG_FILE)

# Check for minimal configuration.
ifneq ($(strip $(filter-out $(PRE_CONFIG_TARGETS), $(MAKECMDGOALS))),)
ifndef PNC_LIBC
    $(error PNC_LIBC is not defined)
endif
endif

# Applications use both kernel and user features.
FEATURES := $(kernel_FEATURES) $(user_FEATURES)

FEATURES += LIBC_$(shell echo $(PNC_LIBC) | tr a-z A-Z)

# Include target compiler default tools and options
COMPILE_ARCH := $(ARCH)
include $(BASE_DIR)/scripts/build/common_target.mk

FEATURES_DEFINES := $(addprefix -DCONFIG_,$(FEATURES))

# Common part for compile flags
# Use VFLAGS on the command line for temporary options
# Several application Makefiles define CFLAGS before including us,
# *then* define 'app_CFLAGS := $(CFLAGS) options'. Thus we must keep the
# old CFLAGS values.
CFLAGS := $(TARGET_CFLAGS) $(CFLAGS) $(FEATURES_DEFINES)
ASFLAGS := $(TARGET_ASFLAGS)
LDFLAGS := $(TARGET_LDFLAGS)
CARGO_FLAGS := $(TARGET_CARGO_FLAGS)

ifneq ($(DEBUG), 0)
CFLAGS += -g -fno-omit-frame-pointer -gdwarf-2 -O0
ASFLAGS += -g
ifneq ($(PROVENCORE_LLVM_BUILD),1)
ASFLAGS += -Wa,--gdwarf-2
endif
else
CFLAGS += -O1
CARGO_FLAGS += --release
LDFLAGS += --gc-sections
endif

# Common link paths:
LDFLAGS += -L$(BUILD_BASE)/lib -T $(APP_DIR)/pnc_app.ld

ifeq ($(filter SIMD_USEREN, $(FEATURES)),)
# No FP support
ifeq ($(ARCH), armv8)
CFLAGS += -mgeneral-regs-only
endif # armv8
else
# FP support enabled
ifeq ($(ARCH), armv7a)
# We use softfp so we can link with 'soft' libraries
# If you want to use the 'hard' ABI, it needs some work
#  to build multi-abi libraries (libs/sf/... && libs/hf/...)
CFLAGS += -mfloat-abi=softfp
endif # armv7a
endif # SIMD_USEREN

# GCC includes an incompatible built-in fork declaration :
#  int fork(void);
# instead of
#  pid_t fork(void);
CFLAGS += -fno-builtin-fork

ifneq ($(filter ARMV8_BTI, $(FEATURES)),)
CFLAGS += -mbranch-protection=standard
endif # ARMV8_BTI

# device.c/device.h are stored in $(BUILD_BASE)
BUILD_DEV := $(BUILD_BASE)/plat

# Common include paths
CFLAGS += \
    -I$(BASE_DIR)/include \
    -I$(BASE_DIR)/include/arch/$(ARCH) \
    -I$(INCLUDE_DIR)/pnc \
    -I$(BASE_DIR)/plat \
    -I$(BASE_DIR)/plat/$(PLAT) \
    -I$(INCLUDE_DIR) \
    -I$(BUILD_DEV) \
    -I$(APP_SRC_DIR)/include

ifeq ($(PNC_LIBC),libstd)
# lib/std include paths
CFLAGS += \
    -isystem $(INCLUDE_DIR)/std
endif

ARCH_ARM_COMMON := armv7a armv8
ifneq ($(filter $(ARCH), $(ARCH_ARM_COMMON)),)
CFLAGS += -I$(BASE_DIR)/include/arch/arm-common
endif

ifneq ($(strip $(filter-out $(PRE_CONFIG_TARGETS), $(MAKECMDGOALS))),)
ifeq ($(PNC_LIBC),libstd)
COMMON_LIBS := std pnc
else ifeq ($(PNC_LIBC),newlib)
COMMON_LIBS := gloss pnc
# The nostdlib option is left as we need to control the way the libc is linked
# for applications.
# LDFLAGS := $(filter-out -nostdlib,$(LDFLAGS))
else
$(error unrecognized option PNC_LIBC=$(PNC_LIBC))
endif
endif

ifneq ($(filter UBSAN UBSAN_NO_RECOVER, $(FEATURES)),)
CFLAGS += -fsanitize=undefined
COMMON_LIBS += ubsan
endif
ifneq ($(filter UBSAN_NO_RECOVER, $(FEATURES)),)
CFLAGS += -fno-sanitize-recover=undefined
endif

CFLAGS += $(VFLAGS)

# Set CXXFLAGS
include $(BASE_DIR)/scripts/build/common_cxx.mk
