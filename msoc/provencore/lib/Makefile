#
# Copyright (c) 2016-2023, ProvenRun and/or its affiliates. All rights reserved.
#
# Providing BASE_DIR & BUILD_BASE is always a good idea
# $(BASE_DIR)/kconfig/sys_procs_pid.h must exist, please
# run make programs && make kconfig before

# Base directory of the whole project. We should be in $(BASE_DIR)/lib
ifndef BASE_DIR
	$(error BASE_DIR has to be set by calling Makefile !)
endif

ifneq ($(wildcard $(LIBS_FILE)),)
	USER_LIBS := $(shell cat $(LIBS_FILE))
endif

LIB_DIR         := $(BUILD_BASE)/lib
LIB_SRC_DIR     := $(BUILD_BASE)/lib/src
LIB_BUILD_DIR   := $(BUILD_BASE)/lib/build

include $(BASE_DIR)/scripts/build/verbose.mk

all:

config:

clean:
	$(Q)rm -rf $(LIB_BUILD_DIR)

doc:

##
# Generate the rules for building a single library.
# $(eval $(call BUILD_USER_LIB,$(app)) adds the following rules to
# the context:
#   $(lib):
#   $(lib)-doc:
#   $(lib)-clean:
##
define BUILD_USER_LIB

all: $(1)
$(1):
	$(Q)$$(MAKE) -C $(LIB_SRC_DIR)/$(1) LIBNAME=$(1) all

config: $(1)-config
$(1)-config:
	$(Q)$$(MAKE) -C $(LIB_SRC_DIR)/$(1) LIBNAME=$(1) config

doc: $(1)-doc
$(1)-doc:
	$(Q)$$(MAKE) -C $(LIB_SRC_DIR)/$(1) LIBNAME=$(1) doc

clean: $(1)-clean
$(1)-clean:
	$(Q)$$(MAKE) -C $(LIB_SRC_DIR)/$(1) LIBNAME=$(1) clean

.PHONY: $(1) $(1)-doc $(1)-clean

endef

# Build each of the required static libraries
$(foreach lib, $(USER_LIBS),$(eval $(call BUILD_USER_LIB,$(lib))))

.PHONY: all doc clean
