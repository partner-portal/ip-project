#
# Copyright (c) 2017-2023, ProvenRun and/or its affiliates. All rights reserved.
#
# Providing BASE_DIR & BUILD_BASE is always a good idea
# $(BASE_DIR)/kconfig/sys_procs_pid.h must exist, please
# run make programs && make kconfig before

ifndef BASE_DIR
    $(error BASE_DIR has to be set by calling Makefile !)
endif

ifndef LIBNAME
    $(error LIBNAME has to be set by calling Makefile !)
endif

ifndef BUILD_BASE
BUILD_BASE := $(BASE_DIR)/build/$(VARIANT)
endif

ifndef LIB_SRC_DIR
LIB_SRC_DIR    := $(BUILD_BASE)/lib/src/$(LIBNAME)
endif

ifndef LIB_BUILD_DIR
LIB_BUILD_DIR  := $(BUILD_BASE)/lib/build/$(LIBNAME)
endif

INCLUDE_DIR := $(BUILD_BASE)/include

# This path cannot be overridden
LIB_DIR := $(BUILD_BASE)/lib

include $(BASE_DIR)/scripts/build/verbose.mk

TARGET_FILE := $(BUILD_BASE)/kconfig/target.mk
CONFIG_FILE := $(BUILD_BASE)/kconfig/config.mk

# Make sure $(plat)_config has been called.
ifneq ($(words $(wildcard $(TARGET_FILE) $(CONFIG_FILE))),2)

# Targets than can be called before config.
PRE_CONFIG_TARGETS := clean doc

ifneq ($(strip $(filter-out $(PRE_CONFIG_TARGETS), $(MAKECMDGOALS))),)
    $(error "No configuration found -- please run make $$(plat)_config first")
endif

endif # Check for CONFIG_FILE+TARGET_FILE

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

ifneq ($(strip $(filter-out $(PRE_CONFIG_TARGETS), $(MAKECMDGOALS))),)
# Include target compiler default tools and options
COMPILE_ARCH := $(ARCH)
include $(BASE_DIR)/scripts/build/common_target.mk
endif

FEATURES_DEFINES := $(addprefix -DCONFIG_,$(FEATURES))

# Common part for compile flags
# Use VFLAGS on the command line for temporary options
CFLAGS := $(TARGET_CFLAGS) $(FEATURES_DEFINES)
ASFLAGS := $(TARGET_ASFLAGS) $(FEATURES_DEFINES)
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
CFLAGS += -ffunction-sections -fdata-sections
CARGO_FLAGS += --release
LDFLAGS += --gc-sections
endif

ifeq ($(filter SIMD_USEREN, $(FEATURES)),)
# No FP support
ifeq ($(ARCH), armv8)
CFLAGS += -mgeneral-regs-only
endif # armv8
else
# Even when kernel has FP support, we still compile
#  libraries without FP support. This avoid heavy
#  context switches becauses memcpy used FP registers.
#CFLAGS      += -mfloat-abi=soft
endif # SIMD_USEREN

ifneq ($(filter ARMV8_BTI, $(FEATURES)),)
CFLAGS += -mbranch-protection=standard
endif # ARMV8_BTI

# Common include paths:
INCLUDES := \
    -I$(BASE_DIR)/include \
    -I$(BASE_DIR)/include/arch/$(ARCH) \
    -I$(INCLUDE_DIR)/pnc \
    -I$(INCLUDE_DIR) \
    -I$(BUILD_BASE)/plat \
    -I$(BASE_DIR)/plat


ifeq ($(PNC_LIBC),libstd)
# lib/std include paths
INCLUDES += \
    -isystem $(INCLUDE_DIR)/std
endif

ARCH_ARM_COMMON := armv7a armv8
ifneq ($(filter $(ARCH), $(ARCH_ARM_COMMON)),)
INCLUDES += -I$(BASE_DIR)/include/arch/arm-common
endif

ifneq ($(filter UBSAN UBSAN_NO_RECOVER, $(FEATURES)),)
CFLAGS += -fsanitize=undefined
endif
ifneq ($(filter UBSAN_NO_RECOVER, $(FEATURES)),)
CFLAGS += -fno-sanitize-recover=undefined
endif

CFLAGS += $(INCLUDES)
ASFLAGS += $(INCLUDES)

CFLAGS += $(VFLAGS)

# Set CXXFLAGS
include $(BASE_DIR)/scripts/build/common_cxx.mk
