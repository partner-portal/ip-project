#
# Copyright (c) 2015-2023, ProvenRun and/or its affiliates. All rights reserved.
#

# some FEATURES defined in the configs/foobar.mk files may depend
# on each other, we test that the configuration is sound

CHECK_FEATURES_DEP := 1

define implies_one
ifneq ($(filter $(strip $(1)), $(FEATURES)),)
ifeq ($(filter $(strip $(2)), $(FEATURES)),)
    $$(warning $(strip $(1)) requires the feature $(strip $(2)) which is not set)
    CHECK_FEATURES_DEP := 0
endif
endif
endef

##
# Declare the relation $(1) => /\ $(2) where $(1) is the name of one feature,
# and $(2) a list of features which $(1) depends upon.
##
define implies
$(foreach feat,$(2),$(eval $(call implies_one,$(1),$(feat))))
endef

define excludes_one
ifneq ($(filter $(strip $(1)), $(FEATURES)),)
ifneq ($(filter $(strip $(2)), $(FEATURES)),)
    $$(warning $(strip $(1)) and $(strip $(2)) cannot be set together)
    CHECK_FEATURES_DEP := 0
endif
endif
endef

##
# Declare the relation $(1) => !(\/ $(2)) where $(1) is the name of one feature,
# and $(2) a list of features which $(1) excludes.
##
define excludes
$(foreach feat,$(2),$(eval $(call excludes_one,$(1),$(feat))))
endef

##
# Declare a set of features that are mutually exclusive.
# The second argument indicates whether exactly one of the group must be set.
##
define incompatible_select
$(eval TEMP := $(filter $(1),$(sort $(FEATURES))))
ifneq ($(words $(TEMP)),1)
ifneq ($(words $(TEMP)),0)
    $$(warning $(TEMP) cannot be set together)
    CHECK_FEATURES_DEP := 0
else
ifeq ($(2),yes)
    $$(warning exactly one of $(strip $(1)) must be set)
    CHECK_FEATURES_DEP := 0
endif
endif
endif
endef

define incompatible
$(eval $(call incompatible_select, $(1),no))
endef

define select
$(eval $(call incompatible_select, $(1),yes))
endef

$(call implies, NS_SMP,              TRUSTZONE YIELD_TO_NON_SECURE V7_SECURE_MONITOR)
$(call implies, YIELD_TO_NON_SECURE, TRUSTZONE)
$(call implies, ARMV8_BTI,           ARCH_ARMV8)

$(call select,  NO_SECURE_MONITOR V8_SECURE_MONITOR V7_SECURE_MONITOR KARCH_RISCV64)

# One which cannot be expressed with the helpers: SMART && KARCH_ARVM8 => !ARCH_ARMV7A
$(eval TEMP := $(filter ARCH_ARMV7A KARCH_ARMV8 SMART,$(sort $(FEATURES))))
ifeq ($(words $(TEMP)), 3)
    $(warning '$(TEMP)' cannot be set together. You probably want ARCH:=armv8.)
    CHECK_FEATURES_DEP := 0
endif

ifeq ($(PNC_LIBC), newlib)
ifneq ($(filter ARCH_ARMV8, $(FEATURES)),)
ifeq ($(filter SIMD_USEREN, $(FEATURES)),)
    $(warning 'PNC_LIBC=newlib' with 'ARCH_ARMV8' requires feature 'SIMD_USEREN' which is not set.)
    CHECK_FEATURES_DEP := 0
endif
endif
endif
