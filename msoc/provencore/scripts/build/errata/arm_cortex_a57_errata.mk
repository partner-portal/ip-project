#
# Copyright (c) 2019-2023, ProvenRun and/or its affiliates. All rights reserved.
#

CPU_ERRATA := ARM_ERRATA_A57_813420 ARM_ERRATA_A57_813419 ARM_ERRATA_A57_826974

# This workaround may have some side effect regarding performance
# When we are sure that there is an empty page between address which can be executed
# and address mapping devices which are read sensitive, it can be discarded.
CPU_ERRATA := ARM_ERRATA_A57_859972
