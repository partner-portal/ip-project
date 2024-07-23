#
# Copyright (c) 2017-2023, ProvenRun and/or its affiliates. All rights reserved.
#

define BUILD_LINKER_SCRIPT
	# $1: path of linker file to generate
	# $2: binary name
	# Expected is that:
	# - $(2)_LINKERFILE was defined with path to linker file source
	# - $(2)_LINKERADDON was defined with path to external .o addon added as
	#   specific section.
	$(eval LSOURCE  := $($(2)_LINKERFILE))
	$(eval LADDON   := $($(2)_LINKERADDON))
	$(eval DEPS     := $(1).d)
	$(eval LOCAL_ASFLAGS := $($(2)_ASFLAGS))

$(1): $(LSOURCE) $(LADDON)
	@echo "  PP      $$@" $(ECHO_OUTPUT)
	$$(Q)mkdir -p $$(dir $$@)
	$$(Q)$$(AS) $(LOCAL_ASFLAGS) \
		-P -E -o $$@ $$< -MD -MF $$@.d -MT $$@

-include $(DEPS)
endef

define BUILD_BINARY_MODULE
	# $1: module name
	# $2: binary name
	# Expected is that
	#  - $(1)_DIR was defined to indicate path to module sources (e.g
	#    path to module Makefile...)
	$(eval OBJ := $(addprefix $(BUILD_BASE)/,$(1)/$(1).a))

$(OBJ): FORCE
	@echo "  MODULE $(1) for $(2)" $(ECHO_OUTPUT)
	$(Q)$$(MAKE) -C $($(1)_DIR) $(1)
endef

define BUILD_BINARY
	# $1: binary name
	# Expected is that:
	#  - $(1)_CORE was defined with name of CORE module for this binary
	#  - $(1)_MODULES was defined with list of any additional module for
	#    this binary
	#  - $(1)_ENTRY_POINT was defined with name of binary's entry point
	#    symbol
	$(eval BIN_DIR      := $(BUILD_BASE)/$(1))
	$(eval MODULES_OBJ  := $(addprefix $(BUILD_BASE)/,$(foreach mod,$($(1)_MODULES),$(mod)/$(mod).a)))
	$(eval LINKERFILE   := $(BIN_DIR)/$(1).ld)
	$(eval MAPFILE      := $(BIN_DIR)/$(1).map)
	$(eval ELFSYM       := $(BIN_DIR)/$(1).sym)
	$(eval ELFDBG       := $(BIN_DIR)/$(1).dbg)
	$(eval ELF          := $(BIN_DIR)/$(1).elf)
	$(eval ALL_OBJS     := $(BIN_DIR)/$(1).objs.a)
	$(eval BIN          := $(BUILD_BASE)/$(1).bin)

	# Create rules to generate linker file
	$(eval $(call BUILD_LINKER_SCRIPT,$(LINKERFILE),$(1)))

	# Check if binary's CORE module is already available
ifeq ($($(1)_CORE_OBJ), )
	$(1)_CORE_OBJ     := $(addprefix $(BUILD_BASE)/,$(foreach core,$($(1)_CORE),$(core)/$(core).a))
	# Create rules to generate binary's CORE module
	$(eval $(foreach core,$($(1)_CORE),$(eval $(call BUILD_BINARY_MODULE,$(core),$(1)))))
endif

	# Create rules to generate other binary's MODULES
	$(eval $(foreach mod,$($(1)_MODULES),$(eval $(call BUILD_BINARY_MODULE,$(mod),$(1)))))

$(BIN_DIR):
	$$(Q)mkdir -p "$$@"

$(ELF): $$($(1)_CORE_OBJ) $(MODULES_OBJ) $(LINKERFILE)
	$(Q)rm -f $(ALL_OBJS) # ar cannot handle 'r' correctly with 'T'
	$(Q)$(AR) crDT $(ALL_OBJS) $$($(1)_CORE_OBJ) $(MODULES_OBJ)
	@echo "  LD      $$@" $(ECHO_OUTPUT)
	$(Q)LIST_OBJS=`$(AR) t $(ALL_OBJS)` && \
	$(LD) -o $$@.nostrip $$($(1)_LDFLAGS) -Map=$(MAPFILE) \
		--script $(LINKERFILE) --entry $($(1)_ENTRY_POINT) \
		-L$(BIN_DIR) $$$${LIST_OBJS}
ifneq ($(filter DEBUG_KERNEL_BACKTRACE, $(FEATURES)),)
	@echo "  DBG     $(ELFDBG)" $(ECHO_OUTPUT)
	$$(Q)cp $$@.nostrip $(ELFSYM)
	$$(Q)$(OC) --only-keep-debug $(ELFSYM)
	$$(Q)$(OC) --rename-section .data=.dbg_data \
		-I binary -O $(ELFFORMAT) -B $(ELFARCH) \
		$(ELFSYM) $(ELFDBG)
	@echo "  LD      $$<" $(ECHO_OUTPUT)
	$(Q)LIST_OBJS=`$(AR) t $(ALL_OBJS)` && \
	$(LD) -o $$@.nostrip $$($(1)_LDFLAGS) -Map=$(MAPFILE) \
		--script $(LINKERFILE) --entry $($(1)_ENTRY_POINT) \
		-L$(BIN_DIR) $$$${LIST_OBJS} $(ELFDBG)
endif
ifeq ($(DEBUG), 0)
	@echo "  STRIP   $$@" $(ECHO_OUTPUT)
	$$(Q)$$(STRIP) -o $$@ -s $$@.nostrip
else
	$(Q)cp $$@.nostrip $$@
endif

$(BIN): $(ELF) | $(BIN_DIR)
	@echo "  BIN     $$@" $(ECHO_OUTPUT)
	$$(Q)$$(OC) -O binary $$< $$@
	@echo $(ECHO_OUTPUT)
	@echo "Built $$@ successfully" $(ECHO_OUTPUT)
	@echo $(ECHO_OUTPUT)

.PHONY: $(1)
$(1): $(BIN)

endef
