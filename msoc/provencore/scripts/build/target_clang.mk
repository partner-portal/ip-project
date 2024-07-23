#
# Copyright (c) 2018-2023, ProvenRun and/or its affiliates. All rights reserved.
#
# Original author: Hadrien Barral
#

# See doc/dev/llvm_build.md for documentation.

#
# Clang compiler settings
#

# GNU_CROSS_COMPILE* should point to a GCC-compatible toolchain
ifeq ($(COMPILE_ARCH), armv7a)
ifndef GNU_CROSS_COMPILE
    $(error GNU_CROSS_COMPILE must be set by the user !)
endif
GNU_CROSS_TOOLCHAIN := $(GNU_CROSS_COMPILE)
endif

ifeq ($(COMPILE_ARCH), armv8)
ifndef GNU_CROSS_COMPILE64
    $(error GNU_CROSS_COMPILE64 must be set by the user !)
endif
GNU_CROSS_TOOLCHAIN := $(GNU_CROSS_COMPILE64)
endif

ifeq ($(COMPILE_ARCH), riscv64)
ifndef GNU_CROSS_COMPILE_RISCV64
    $(error GNU_CROSS_COMPILE_RISCV64 must be set by the user !)
endif
GNU_CROSS_TOOLCHAIN := $(GNU_CROSS_COMPILE_RISCV64)
endif

ifndef GNU_CROSS_TOOLCHAIN
    $(error Unknown arch $(COMPILE_ARCH) (target_clang.mk)!)
endif

# CLANG_LIBC_*_INCLUDE_PATH should point to the libc include path
ifeq ($(COMPILE_ARCH), armv7a)
ifndef PNC_CLANG_LIBC_ARMV7A_INCLUDE_PATH
    $(error PNC_CLANG_LIBC_ARMV7A_INCLUDE_PATH must be set by the user !)
endif
endif

ifeq ($(COMPILE_ARCH), armv8)
ifndef PNC_CLANG_LIBC_AARCH64_INCLUDE_PATH
    $(error PNC_CLANG_LIBC_AARCH64_INCLUDE_PATH must be set by the user !)
endif
endif

ifeq ($(COMPILE_ARCH), riscv64)
ifndef PNC_CLANG_LIBC_RISCV64_INCLUDE_PATH
    $(error PNC_CLANG_LIBC_RISCV64_INCLUDE_PATH must be set by the user !)
endif
endif

#
# Toolchain definitions
#

CC_TOOL    := clang
CPP_TOOL   := cpp
AS_TOOL    := clang
# llvm-ar handles -t in a strange way...
# It is not compatible with our current setup
#AR_TOOL    = llvm-ar
AR_TOOL    := ar
LD_TOOL    := ld.lld
# llvm-objcopy fails on empty relocation section.
# I suspect it might be an llvm bug
#OC_TOOL    = llvm-objcopy
OC_TOOL    := objcopy
# llvm has no "ranlib"
RANLIB_TOOL  := ranlib
# llvm has no "readelf"
READELF_TOOL := readelf
# llvm has no "strip" (flag in the linker instead)
STRIP_TOOL := strip
NM_TOOL := llvm-nm

CC      := $(CROSS_TOOLCHAIN)$(CC_TOOL)
CPP     := $(CROSS_TOOLCHAIN)$(CPP_TOOL)
AS      := $(CROSS_TOOLCHAIN)$(AS_TOOL)
AR      := $(GNU_CROSS_TOOLCHAIN)$(AR_TOOL)
ifeq ($(COMPILE_ARCH), riscv64)
#FIXME:LLVM: lld does not support RISC-V yet
LD      := $(GNU_CROSS_TOOLCHAIN)ld
else
LD      := $(CROSS_TOOLCHAIN)$(LD_TOOL)
endif
OC      := $(GNU_CROSS_TOOLCHAIN)$(OC_TOOL)
RANLIB  := $(GNU_CROSS_TOOLCHAIN)$(RANLIB_TOOL)
READELF := $(GNU_CROSS_TOOLCHAIN)$(READELF_TOOL)
STRIP   := $(GNU_CROSS_TOOLCHAIN)$(STRIP_TOOL)
NM      := $(CROSS_TOOLCHAIN)$(NM_TOOL)

#
# Default toolchain flags
#

# Unlike gcc, clang is usually not packaged with a libc. We however need
# definitions from the standard headers
ifeq ($(COMPILE_ARCH), armv7a)
PLATFORM_CFLAGS  += --target=armv7a-none-eabi -mno-implicit-float
PLATFORM_ASFLAGS += --target=armv7a-none-eabi

PLATFORM_CFLAGS += -idirafter "$(PNC_CLANG_LIBC_ARMV7A_INCLUDE_PATH)"
endif

ifeq ($(COMPILE_ARCH), armv8)
PLATFORM_CFLAGS  += --target=aarch64-none-elf -mno-implicit-float
PLATFORM_ASFLAGS += --target=aarch64-none-elf
#FIXME:LLVM: Find doc about gcc 'w' modifier
TARGET_CFLAGS += -Wno-asm-operand-widths

PLATFORM_CFLAGS += -idirafter "$(PNC_CLANG_LIBC_AARCH64_INCLUDE_PATH)"
endif

ifeq ($(COMPILE_ARCH), riscv64)
PLATFORM_CFLAGS  += --target=riscv64-unknown-elf
PLATFORM_ASFLAGS += --target=riscv64-unknown-elf
RISCV64_MCMODEL := medium

PLATFORM_CFLAGS += -idirafter "$(PNC_CLANG_LIBC_RISCV64_INCLUDE_PATH)"
endif

# By default, llvm has strange include paths:
#  - for 'armv7a-none-eabi' and 'riscv64-unknown-elf': `include`
#  - for 'aarch64-none-elf': `/usr/include` and `/usr/local/include`
#  The '--sysroot=/dev/null' part removes them
PLATFORM_CFLAGS += --sysroot=/dev/null

# We know typedef redefinition is C11
TARGET_CFLAGS += -Wno-typedef-redefinition
