#
# Copyright (c) 2017-2023, ProvenRun and/or its affiliates. All rights reserved.
#
# Original author: Hadrien Barral
#

# Call platform-specific script.
# This script builds binary images for the platform
# Generic script arguments:
#   --secure: generate signed images suitable for secure boot
#   --single: see SINGLE_BINARY_IMAGE
#
# Available environement variables:
#   BASE_DIR, BUILD_BASE, ...
#

ifneq ($(wildcard $(IMAGE_SCRIPT)),)

IMAGE_SCRIPT_ARGS :=
ifneq ($(filter SECURE_BOOT, $(FEATURES)),)
IMAGE_SCRIPT_ARGS += --secure
endif # ($(filter SECURE_BOOT, $(FEATURES)),)
ifneq ($(filter SINGLE_BINARY_IMAGE, $(FEATURES)),)
IMAGE_SCRIPT_ARGS += --single
endif # ($(filter SINGLE_BINARY_IMAGE, $(FEATURES)),)
image:
	$(Q)if [ ! -x "$(IMAGE_SCRIPT)" ]; then \
	        echo "Error: File '$(IMAGE_SCRIPT)' is not executable" 1>&2; \
	        false; \
	    fi
	$(Q)+"$(IMAGE_SCRIPT)" $(IMAGE_SCRIPT_ARGS) $(ECHO_OUTPUT)

clean-image:
	$(Q)rm -rf $(BUILD_BASE)/images
else
image:
	@echo "ERROR: This platform does not support 'make image'"
	$(Q)false

clean-image:
	@echo "ERROR: This platform does not support 'make clean-image'"
	$(Q)false
endif # ($(wildcard $(IMAGE_SCRIPT)),)

.PHONY: image
