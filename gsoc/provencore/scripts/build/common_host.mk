#
# Copyright (c) 2017-2023, ProvenRun and/or its affiliates. All rights reserved.
#

# Host compiler settings
HOST_CC ?= gcc
HOST_CFLAGS := -std=gnu11 -Wall -Wextra -O0
WEXTRA_FLAGS := -Wshadow -Wcast-align -Wstrict-prototypes -Wundef \
                -Winit-self -Wmissing-prototypes
HOST_CFLAGS += $(WEXTRA_FLAGS)
ifneq ($(COMPAT32),)
HOST_CFLAGS += -m32
HOST_LDFLAGS += -m32
endif

ifndef KARCH
    KARCH := $(ARCH)
endif
