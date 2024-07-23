#
# Copyright (c) 2021-2023, ProvenRun and/or its affiliates. All rights reserved.
#

#
# C++ flags definitions
#

ifndef CFLAGS
    $(error CFLAGS has to be set by calling Makefile !)
endif

CXXFLAGS := $(CFLAGS)

# Warnings
CXXFLAGS := $(filter-out -Wstrict-prototypes -Wmissing-prototypes, $(CXXFLAGS))
CXXFLAGS += -Woverloaded-virtual

# Dialect options
CXXFLAGS := $(filter-out -std=gnu11, $(CXXFLAGS))
CXXFLAGS += --std=c++11
CXXFLAGS += -fno-threadsafe-statics

# We have issues with Wexpansion-to-defined in include/log.h (see comment).
# Unfortunately, due to a gcc bug (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53431),
# pragma GCC diagnostic ignored "-Wexpansion-to-defined" does not work for g++,
# so we disable it globally. This probably breaks C++ compilation with GCC
# whose version is older than 7
CXXFLAGS += -Wno-expansion-to-defined
