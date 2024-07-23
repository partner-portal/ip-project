#
# Copyright (c) 2019-2023, ProvenRun and/or its affiliates. All rights reserved.
#
# Original author: Hadrien Barral
#

# Newlib toolchain overlay settings
# See services/toolchain/README.md for documentation

# Overlays below need these definitions
PNC_TOOLCHAIN_NEWLIB_CC_BACKEND   := $(CC)
PNC_TOOLCHAIN_NEWLIB_CXX_BACKEND  := $(CXX)
PNC_TOOLCHAIN_NEWLIB_AS_BACKEND   := $(AS)
PNC_TOOLCHAIN_NEWLIB_LD_BACKEND   := $(LD)

ifeq ($(COMPILE_ARCH),armv7a)
PNC_TOOLCHAIN_NEWLIB_TARGET       := arm-pnc
else ifeq ($(COMPILE_ARCH),armv8)
PNC_TOOLCHAIN_NEWLIB_TARGET       := aarch64-pnc
else ifeq ($(COMPILE_ARCH),riscv64)
PNC_TOOLCHAIN_NEWLIB_TARGET       := riscv64-pnc
else
$(error Unsupported COMPILE_ARCH value)
endif

# Skip the overlay if the current toolchain already includes ProvenCore support.
ifneq ($(CC),$(PNC_TOOLCHAIN_NEWLIB_TARGET)-gcc)

export PNC_TOOLCHAIN_NEWLIB_CC_BACKEND
export PNC_TOOLCHAIN_NEWLIB_CXX_BACKEND
export PNC_TOOLCHAIN_NEWLIB_AS_BACKEND
export PNC_TOOLCHAIN_NEWLIB_LD_BACKEND
export PNC_TOOLCHAIN_NEWLIB_TARGET

# Toolchain definitions: we overlay a few tools
CROSS_TOOLCHAIN := $(PATH_TO_SERVICES)/toolchain/bin/any-pnc-
CC_TOOL         := gcc
CXX_TOOL        := g++
AS_TOOL         := gcc
LD_TOOL         := ld
CC              := $(CROSS_TOOLCHAIN)$(CC_TOOL)
CXX             := $(CROSS_TOOLCHAIN)$(CXX_TOOL)
AS              := $(CROSS_TOOLCHAIN)$(AS_TOOL)
LD              := $(CROSS_TOOLCHAIN)$(LD_TOOL)

endif
