#
# Copyright (c) 2017-2023, ProvenRun and/or its affiliates. All rights reserved.
#

#
# Target compiler settings
#

ifndef BASE_DIR
    $(error BASE_DIR has to be set by calling Makefile !)
endif

# Path to the target toolchain tools
ifndef COMPILE_ARCH
    $(error COMPILE_ARCH has to be set by calling Makefile !)
endif

ifeq ($(COMPILE_ARCH), armv7a)
    CROSS_TOOLCHAIN := $(CROSS_COMPILE)
    ELFFORMAT := elf32-littlearm
    ELFARCH := arm
    CARGO_TARGET := armv7a-none-eabi
else ifeq ($(COMPILE_ARCH), armv8)
    CROSS_TOOLCHAIN := $(CROSS_COMPILE64)
    ELFFORMAT := elf64-littleaarch64
    ELFARCH := aarch64
    CARGO_TARGET := aarch64-unknown-none-softfloat
else ifeq ($(COMPILE_ARCH), riscv64)
    CROSS_TOOLCHAIN := $(CROSS_COMPILE_RISCV64)
    ELFFORMAT := elf64-littleriscv
    ELFARCH := riscv:rv64
    CARGO_TARGET := riscv64imac-unknown-none-elf
else
    $(error common_target.mk: Unsupported architecture (COMPILE_ARCH): $(COMPILE_ARCH))
endif

TARGET_FILE := $(BASE_DIR)/build/$(VARIANT)/kconfig/target.mk

# Defines CPU
-include $(TARGET_FILE)

ifndef KARCH
    KARCH := $(ARCH)
endif

TARGET_CFLAGS  :=
TARGET_LDFLAGS :=
TARGET_ASFLAGS :=
TARGET_RSFLAGS := -C panic=abort

#
# Default toolchain definitions
#

CC_TOOL      := gcc
CXX_TOOL     := g++
CPP_TOOL     := cpp
AS_TOOL      := gcc
AR_TOOL      := ar
LD_TOOL      := ld
OC_TOOL      := objcopy
RANLIB_TOOL  := ranlib
READELF_TOOL := readelf
STRIP_TOOL   := strip
NM_TOOL      := nm
CARGO_TOOL   := cargo

CC      := $(CROSS_TOOLCHAIN)$(CC_TOOL)
CXX     := $(CROSS_TOOLCHAIN)$(CXX_TOOL)
CPP     := $(CROSS_TOOLCHAIN)$(CPP_TOOL)
AS      := $(CROSS_TOOLCHAIN)$(AS_TOOL)
AR      := $(CROSS_TOOLCHAIN)$(AR_TOOL)
LD      := $(CROSS_TOOLCHAIN)$(LD_TOOL)
OC      := $(CROSS_TOOLCHAIN)$(OC_TOOL)
RANLIB  := $(CROSS_TOOLCHAIN)$(RANLIB_TOOL)
READELF := $(CROSS_TOOLCHAIN)$(READELF_TOOL)
STRIP   := $(CROSS_TOOLCHAIN)$(STRIP_TOOL)
NM      := $(CROSS_TOOLCHAIN)$(NM_TOOL)
CARGO   := $(CARGO_TOOL)

#
# Other presets
#

# LLVM/Clang toolchain. /!\ EXPERIMENTAL: do not use for releases.
# See the doc/dev/llvm_build.md for documentation.
ifeq ($(PROVENCORE_LLVM_BUILD),1)
include $(BASE_DIR)/scripts/build/target_clang.mk
endif

CC_VERSION := $(shell $(CC) -dumpversion)

# Newlib support for applications.
# See the services/toolchain/README.md for documentation.
ifeq ($(PNC_LIBC),newlib)
include $(BASE_DIR)/scripts/build/target_newlib.mk
endif

#
# Default toolchain flags
#
# PLATFORM_ flags are the bare minimum flags to compile for the target
# TARGET_ flags are usual common flags to compile for the target
#

WEXTRA_FLAGS := -Wshadow -Wcast-align -Wstrict-prototypes -Wundef \
                -Winit-self -Wmissing-prototypes -Wsign-conversion

PLATFORM_CFLAGS += -std=gnu11
ifeq ($(KARCH), riscv64)

ifeq ($(RISCV64_MCMODEL),)
RISCV64_MCMODEL := medany
endif
ifneq ($(ISA),)
PLATFORM_CFLAGS += -march=$(ISA) -mabi=lp64  -mcmodel=$(RISCV64_MCMODEL)
PLATFORM_ASFLAGS += -march=$(ISA) -mabi=lp64  -mcmodel=$(RISCV64_MCMODEL)
else
    $(error "Error: No ISA is set for this config.")
endif # ISA

else
ifneq ($(CPU),)
PLATFORM_CFLAGS  += -mcpu=$(CPU)
PLATFORM_ASFLAGS += -mcpu=$(CPU)
else
    $(error "Error: No CPU is set for this config.")
endif # CPU
# For now, just define errata file for Arm cores
CPU_ERRATA_FILE := \
    $(BASE_DIR)/scripts/build/errata/arm_$(subst -,_,$(CPU))_errata.mk
ifneq ("$(wildcard $(CPU_ERRATA_FILE))","")
-include $(CPU_ERRATA_FILE)
endif
endif # KARCH == riscv64

IS_VER_GE = $(shell printf "$2\n$1" | sort --version-sort --check=quiet && echo YES)

ifeq ($(COMPILE_ARCH), armv8)
# -moutline-atomics became default with gcc 10.1 but we do not have an
# implementation of the functions that select at runtime between LSE and
# exclusive monitor instructions. Until then, request gcc to statically pick
# the best implementation based on the architectural variant selected with
# -mcpu.
ifeq ($(call IS_VER_GE,$(CC_VERSION),10.1), YES)
PLATFORM_CFLAGS += -mno-outline-atomics
endif
endif

TARGET_CFLAGS += $(PLATFORM_CFLAGS)                                       \
                 -Wall -Wextra $(WEXTRA_FLAGS)                            \
                 -fno-common -fdiagnostics-show-option                    \
                 -D__PROVENCORE_BUILD__ -D__CUSTOM_FILE_IO__              \
                 -DDEBUG=$(DEBUG)

# Asserts are enabled by default
NDEBUG ?= 0
ifneq ($(NDEBUG), 0)
TARGET_CFLAGS += -DNDEBUG
endif

ifneq ($(PROVENCORE_LLVM_BUILD),1)
TARGET_CFLAGS += -I$(BASE_DIR)/include/compiler/gcc
else
TARGET_CFLAGS += -I$(BASE_DIR)/include/compiler/clang
endif

TARGET_LDFLAGS += $(PLATFORM_LDFLAGS) \
                  -nostdlib \
                  -z noexecstack

TARGET_ASFLAGS += $(PLATFORM_ASFLAGS) \
                  -DDEBUG=$(DEBUG)

TARGET_CARGO_FLAGS := -Z unstable-options \
                      --target $(CARGO_TARGET) \
                      $(CARGO_QUIET)

#
# Add common "CONFIG_*" variables
#

ifeq ($(KARCH), armv7a)
    KARCH_BITS := 32
else ifeq ($(KARCH), armv8)
    KARCH_BITS := 64
else ifeq ($(KARCH), riscv64)
    KARCH_BITS := 64
else
    $(error common_target.mk: Unsupported architecture (KARCH): $(KARCH))
endif

ifeq ($(ARCH), armv7a)
    ARCH_BITS := 32
else ifeq ($(ARCH), armv8)
    ARCH_BITS := 64
else ifeq ($(ARCH), riscv64)
    ARCH_BITS := 64
else
    $(error common_target.mk: Unsupported architecture (ARCH): $(ARCH))
endif

FEATURES += \
    PLAT_$(shell echo $(PLAT) | tr a-z A-Z) \
    KARCH_$(shell echo $(KARCH) | tr a-z A-Z) \
    ARCH_$(shell echo $(ARCH) | tr a-z A-Z) \
    KARCH_BITS_$(shell echo $(KARCH_BITS) | tr a-z- A-Z_) \
    ARCH_BITS_$(shell echo $(ARCH_BITS) | tr a-z- A-Z_) \
    CPU_$(shell echo $(CPU) | tr a-z-. A-Z_) \
    LINKER_FORMAT=$(ELFFORMAT) \
    LINKER_ARCH=$(ELFARCH)

ifdef ISA
FEATURES += ISA_$(shell echo $(ISA) | tr a-z A-Z)
endif

ifdef FLAVOR
FEATURES += $(addprefix FLAVOR_,$(shell echo $(FLAVOR) | tr a-z A-Z))
endif

ifdef CPU_ERRATA
FEATURES += $(shell echo $(CPU_ERRATA) | tr a-z- A-Z_)
endif
