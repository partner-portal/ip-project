#
# Copyright (c) 2019-2023, ProvenRun and/or its affiliates. All rights reserved.
#

CPU_ERRATA := ARM_ERRATA_A53_827319 ARM_ERRATA_A53_826319 CONFIG_ARM_ERRATA_A53_824069 CONFIG_ARM_ERRATA_A53_855873 CONFIG_ARM_ERRATA_A53_819472

CPU_ERRATA := CONFIG_ARM_ERRATA_A53_836870

# Only needed when EL0 is using Aarch32
CPU_ERRATA := ARM_ERRATA_A53_845719

# Do not include compiler options when compiling applications as they are using armv7 compiler which does not know these new (and A53 specific) options
ifeq ($(COMPILE_ARCH), armv8)
# Erratum 835769
PLATFORM_CFLAGS  += -mfix-cortex-a53-835769
PLATFORM_ASFLAGS += -mfix-cortex-a53-835769
ifneq ($(PROVENCORE_LLVM_BUILD),1)
PLATFORM_LDFLAGS += --fix-cortex-a53-835769
endif

# Erratum 843419
ifneq ($(PROVENCORE_LLVM_BUILD),1)
PLATFORM_CFLAGS  += -mfix-cortex-a53-843419
PLATFORM_ASFLAGS += -mfix-cortex-a53-843419
endif
PLATFORM_LDFLAGS += --fix-cortex-a53-843419
endif # COMPILE_ARCH == armv8
