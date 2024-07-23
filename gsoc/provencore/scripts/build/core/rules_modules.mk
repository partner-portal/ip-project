#
# Copyright (c) 2017-2023, ProvenRun and/or its affiliates. All rights reserved.
#
define SOURCES_TO_OBJS
	$(patsubst %.c,%.o,$(filter %.c,$(1))) \
	$(patsubst %.S,%.o,$(filter %.S,$(1)))
endef

define BUILD_C_FILE
	# $1: module name
	# $2: source file path
	# $3: source file possible dependencies
	$(eval BUILD_DIR    := $(BUILD_BASE)/$(1))
	$(eval FULLSRC      := $(addprefix $(BASE_DIR)/,$(2)))
	$(eval OBJ          := $(addprefix $(BUILD_DIR)/,$(2:.c=.o)))
	$(eval DEPS         := $(patsubst %.o,%.d,$(OBJ)))
	$(eval LOCAL_CFLAGS := $($(1)_CFLAGS))

$(OBJ): $(FULLSRC) $(3)
	@echo "  CC      $(2)" $(ECHO_OUTPUT)
	$$(Q)mkdir -p $$(dir $$@)
	$$(Q)$$(CC) $(LOCAL_CFLAGS) -ffile-prefix-map=$(BASE_DIR)/= -c $$< -o $$@ -MD -MT $$@

-include $(DEPS)

endef

define BUILD_S_FILE
	# $1: module name
	# $2: source file path
	# $3: source file possible dependencies
	$(eval BUILD_DIR     := $(BUILD_BASE)/$(1))
	$(eval FULLSRC       := $(addprefix $(BASE_DIR)/,$(2)))
	$(eval OBJ           := $(addprefix $(BUILD_DIR)/,$(2:.S=.o)))
	$(eval DEPS          := $(patsubst %.o,%.d,$(OBJ)))
	$(eval LOCAL_ASFLAGS := $($(1)_ASFLAGS))

$(OBJ): $(FULLSRC) $(3)
	@echo "  AS      $(2)" $(ECHO_OUTPUT)
	$$(Q)mkdir -p $$(dir $$@)
	$(Q)$$(AS) $(LOCAL_ASFLAGS) -ffile-prefix-map=$(BASE_DIR)/= -c $$< -o $$@ -MD -MT $$@

-include $(DEPS)
endef

define BUILD_OBJS
	# $1: module name
	# $2: list of source files paths
	# $3: module sources dependencies
	$(eval C_OBJS := $(filter %.c,$(2)))
	$(eval REMAIN := $(filter-out %.c,$(2)))
	$(eval $(foreach obj,$(C_OBJS),$(call BUILD_C_FILE,$(1),$(obj),$(3))))

	$(eval S_OBJS := $(filter %.S,$(REMAIN)))
	$(eval REMAIN := $(filter-out %.S,$(REMAIN)))
	$(eval $(foreach obj,$(S_OBJS),$(call BUILD_S_FILE,$(1),$(obj),$(3))))

	$(and $(REMAIN),$(error Unexpected source files present: $(REMAIN)))
endef

define BUILD_MODULE
	# $1: module name
	$(eval BUILD_DIR  := $(BUILD_BASE)/$(1))
	$(eval SOURCES    := $($(1)_SOURCES))
	$(eval OBJS       := $(addprefix $(BUILD_DIR)/,$(call SOURCES_TO_OBJS,$(SOURCES))))
	$(eval KOBJS      := $(BUILD_DIR)/$(1).a)
	$(eval LOCAL_LDFLAGS := $($(1)_LDFLAGS))

	$(eval $(call BUILD_OBJS,$(1),$(SOURCES),$($(1)_DEPS)))

$(KOBJS): $(OBJS)
	@echo "  AR      $$@" $(ECHO_OUTPUT)
	$(Q)rm -f $$@ # ar cannot handle 'r' correctly with 'T'
	$(Q)$(AR) crDT $$@ $(OBJS)

.PHONY: $(1)
$(1): $(KOBJS)

endef
