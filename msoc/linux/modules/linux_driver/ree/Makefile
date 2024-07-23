# If KERNELRELEASE is defined, we have been invoked from the kernel build
# system and we can use its language.

ifneq ($(KERNELRELEASE),)

obj-m := pnc_ree.o
pnc_ree-objs := main.o shm.o session.o smc.o pnr_ring/light_ring.o

# Supported features: see Kconfig file for details if you don't plan to build
# this module thanks to menuconfig...
#
ccflags-y := \
	-Wno-declaration-after-statement \
	-Wno-unused-function \
	-Wno-error=date-time

PNC_CONFIG_MK := $(PWD)/ree/pnc_config.mk
-include $(PNC_CONFIG_MK)

CC_VERSION := $(shell $(CC) -dumpversion)
IS_VER_GE = $(shell printf "$2\n$1" | sort --version-sort --check=quiet && echo YES)

ifeq ($(ARCH), arm64)
ifeq ($(call IS_VER_GE,$(CC_VERSION),10.1), YES)
ccflags-y += -mno-outline-atomics
endif
endif

# Flags for specific files
CFLAGS_main.o := -DDEBUG
CFLAGS_dev.o := -DDEBUG

# Otherwise, we were called directly from the command line ; invoke the
# kernel build system.
else

PWD := $(shell pwd)
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
KERNELDIR ?= $(KERNEL_SRC)

default:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) -C $(KERNELDIR) \
		M=$(PWD) modules

endif

.PHONY: modules_install
modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

.PHONY: clean
clean:
	$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) ARCH=$(ARCH) -C $(KERNELDIR) \
		M=$(PWD) clean
	rm -f *.o *.ko

TZMODULE_SOURCES := \
	main.c \
	shm.c \
	session.c \
	smc.c \
	internal.h \
	ree.h \
	session.h \
	shm.h \
	smc.h \
	Makefile \
	misc/provencore/ree_session.h \
	pnr_ring/light_ring.c \
	pnr_ring/light_ring.h \
	pnr_ring/ring_util.h \
	pnr_ring/pnr_shared_mem.h \
	pnc_config.mk

TZMODULE_ARCHIVE := tzmodule.tar.gz
$(TZMODULE_ARCHIVE): $(TZMODULE_SOURCES)
	$(Q)tar czf $@ --transform 's#^#tzmodule/#' --dereference $^
