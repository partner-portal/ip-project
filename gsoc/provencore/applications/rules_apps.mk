#
# Copyright (c) 2016-2023, ProvenRun and/or its affiliates. All rights reserved.
#

# Must be set by caller (defaults defined in common_apps.mk):
# BUILD_BASE, default $(BASE_DIR)/build/$(VARIANT)
# APP_DIR, set to $(BUILD_BASE)/app
# APP_SRC_DIR, default $(APP_DIR)/src/$(APPNAME)
# APP_BUILD_DIR, default $(APP_DIR)/build/$(APPNAME)/build
# APP_BUILD_OBJ_DIR, set to $(APP_BUILD_DIR)/obj

# Patch linking for newlib
ifeq ($(PNC_LIBC),newlib)
# $(shell ..) does not export the environment defined by the current Makefile
# variables, as is the case for commands executed for a target rules.
# We have to export the variables manually here, because the newlib
# toolchain's $(CC) depends on variables defined in target_newlib.mk
MAKE_ENV := $(shell echo '$(.VARIABLES)' | awk -v RS=' ' '/^[a-zA-Z0-9_]+$$/')
SHELL_EXPORT := $(foreach v,$(MAKE_ENV),$(v)='$($(v))')

# Add libc and libgcc as external libraries
ifeq ($(PROVENCORE_LLVM_BUILD),1)
ifeq ($(ARCH), armv8)
# FIXME: LLVM returns no suitable libgcc path for this architecture
LIBGCC_FILE_NAME :=
else
LIBGCC_FILE_NAME := $(shell $(SHELL_EXPORT) $(CC) $(TARGET_CFLAGS) -print-libgcc-file-name)
endif
else
LIBGCC_FILE_NAME := $(shell $(SHELL_EXPORT) $(CC) $(TARGET_CFLAGS) -print-libgcc-file-name)
endif
EXT_LIBS := -lc $(LIBGCC_FILE_NAME)
else
EXT_LIBS :=
endif

LIB_DIR := $(BUILD_BASE)/lib
INCLUDE_DIR := $(BUILD_BASE)/include
APP_DEP_DIR := $(BUILD_BASE)/dep/app

all: do_build

config: do_config

depend: add_lib_dependencies
	@echo "  DEPEND  $(APPNAME)" $(ECHO_OUTPUT)
	$(Q)mkdir -p $(APP_DIR)/build/$(APPNAME)
	$(Q)echo "$(addprefix $(APPNAME)/build/,$(APPLICATIONS))" \
		> $(APP_DIR)/build/$(APPNAME)/.binaries

add_lib_dependencies:
	$(Q)for i in $(COMMON_LIBS) $(LIB_DEPENDENCIES); do \
		echo $$i >> $(LIBS_FILE); \
	done

METACONFIG = $(BUILD_BASE)/tools/metadata-config/metadata-config
METADTD = $(BASE_DIR)/tools/metadata-config/metadata-config.dtd

define BUILD_APPLICATION

LIB_DEPENDENCIES += $($(1)_LIBS)

$(1)_OBJ_DIR := $(APP_BUILD_OBJ_DIR)/$(1)
$(1)_LIB_DEP := $(APP_DEP_DIR)/$(APPNAME)/$(1).dep

$(1)_C_OBJS := $(addprefix $$($(1)_OBJ_DIR)/, \
    $(patsubst %.c,%.o,$(filter %.c,$($(1)_SOURCES))))
$(1)_C_DEPS := $(addprefix $$($(1)_OBJ_DIR)/, \
    $(patsubst %.c,%.d,$(filter %.c,$($(1)_SOURCES))))
$(1)_CXX_OBJS := $(addprefix $$($(1)_OBJ_DIR)/, \
    $(patsubst %.cpp,%.o,$(filter %.cpp,$($(1)_SOURCES))))
$(1)_CXX_DEPS := $(addprefix $$($(1)_OBJ_DIR)/, \
    $(patsubst %.cpp,%.d,$(filter %.cpp,$($(1)_SOURCES))))
$(1)_AS_OBJS := $(addprefix $$($(1)_OBJ_DIR)/, \
    $(patsubst %.S,%.o,$(filter %.S,$($(1)_SOURCES))))
$(1)_AS_DEPS := $(addprefix $$($(1)_OBJ_DIR)/, \
    $(patsubst %.S,%.d,$(filter %.S,$($(1)_SOURCES))))
$(1)_RUST_LIB := $(addprefix $$($(1)_OBJ_DIR)/, \
    $(patsubst %,lib%.a,$($(1)_CRATE)))

ifeq ($(PROVENCORE_LLVM_BUILD),1)
$$(if $$($(1)_CXX_OBJS),$$(error C++ build not supported with LLVM))
endif
ifneq ($(PNC_LIBC),newlib)
$$(if $$($(1)_CXX_OBJS),$$(error C++ build only supported with newlib))
endif

ifeq ($($(1)_STDCXX),)
$(1)_STDCXX := $(STDCXX)
endif

$$(if $$(and $$($(1)_STDCXX),$$(filter-out stdc++ supc++,$$($(1)_STDCXX))), \
    $$(error STDCXX should be either empty, "stdc++" or "supc++"))

ifeq ($($(1)_CXXFLAGS),)
$(1)_CXXFLAGS = $(CXXFLAGS)
endif

# crtbegin.o must come first in the object list (otherwise leading to an issue
# during stack unwinding when an exception is thrown).
$(1)_OBJS := $$(if $$($(1)_STDCXX), \
    $$(shell $$(SHELL_EXPORT) $(CXX) $$($(1)_CXXFLAGS) -print-file-name=crtbegin.o))

$(1)_OBJS += $$($(1)_C_OBJS) $$($(1)_AS_OBJS) \
    $(addprefix $$($(1)_OBJ_DIR)/, $($(1)_CUSTOM_OBJS))
$(1)_OBJS += $$($(1)_CXX_OBJS)
$(1)_DEPS := $$($(1)_C_DEPS) $$($(1)_AS_DEPS)
$(1)_DEPS += $$($(1)_CXX_DEPS)

-include $$($(1)_DEPS)

ifeq ($($(1)_CFLAGS),)
$(1)_CFLAGS = $(CFLAGS)
endif

ifeq ($($(1)_CPPFLAGS),)
$(1)_CPPFLAGS = $(CPPFLAGS)
endif

ifeq ($($(1)_LDFLAGS),)
$(1)_LDFLAGS = $(LDFLAGS)
endif

$(1)_LDLIBS := $(shell $(BASE_DIR)/scripts/build/dep/compute_graph.py -C $(BUILD_BASE) --ldflags $($(1)_LIBS))
$(1)_ARLIBS := $(foreach lib, $($(1)_LIBS) $(COMMON_LIBS), \
    $(LIB_DIR)/lib$(notdir $(lib)).a)

# Add ABI version segment
$(1)_LDLIBS += $(LIB_DIR)/abi_version.o

# Remove whole-archive option to make sure it is not applied to the
# common libraries group.
# Note: whole-archive applies to archives linked following the option,
# and no-whole-archive turns off whole-achive, so users can still define
# whole-archive for all the libraries except COMMON_LIBS + EXT_LIBS
$(1)_LDLIBS += --no-whole-archive

# libstdc++ is linked after user libraries so that C++ user libraries can
# import symbols from it
$(1)_LDFLAGS += $$(if $$($(1)_STDCXX), \
    -L $$(dir $$(shell $$(SHELL_EXPORT) $(CXX) $$($(1)_CXXFLAGS) -print-file-name=lib$$($(1)_STDCXX).a)))
$(1)_LDLIBS += $$(if $$($(1)_STDCXX),-l$$($(1)_STDCXX))

# Link common libraries as a single group.
$(1)_LDLIBS += --start-group $(addprefix -l, $(COMMON_LIBS)) $(EXT_LIBS) --end-group

# crtend.o must come at the end as it contains a zero terminator FDE in
# .eh_frame. Otherwise, the end of FDEs cannot be detected during stack
# unwinding when an exception is thrown.
# https://github.com/riscv/riscv-gcc/issues/142
$(1)_LDLIBS += $$(if $$($(1)_STDCXX), \
    $$(shell $$(SHELL_EXPORT) $(CXX) $$($(1)_CXXFLAGS) -print-file-name=crtend.o))

$(1)_METADATA_BIN := $(APP_BUILD_DIR)/$(1).metadata.bin
$(1)_METADATA_HEADER := \
	$(INCLUDE_DIR)/metadata/$(APPNAME)/$$(patsubst build/,,$(1))_metadata.h
$(1)_METACONFIG_BIN := $(APPNAME)/build/$(1)

$$($(1)_METADATA_BIN): $($(1)_METADATA) | $(APP_BUILD_DIR)
	$(Q)mkdir -p $$(dir $$($(1)_METADATA_HEADER))
	@echo "  METACONFIG $$@" $(ECHO_OUTPUT)
	$(Q)$(METACONFIG) --dtd=$(METADTD) --meta=$($(1)_METADATA) \
		--bin=$$($(1)_METACONFIG_BIN) --output=$$($(1)_METADATA_BIN) \
		--header=$$($(1)_METADATA_HEADER)

depend: $$($(1)_METADATA_BIN)

$$($(1)_OBJ_DIR)/%.o: %.c
	@echo "  CC      $$<" $(ECHO_OUTPUT)
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(CC) $$($(1)_CPPFLAGS) $$($(1)_CFLAGS) -c $$< -o $$@ -MD -MT $$@

$$($(1)_OBJ_DIR)/%.o: %.cpp
	@echo "  CXX     $$<" $(ECHO_OUTPUT)
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(CXX) $$($(1)_CPPFLAGS) $$($(1)_CXXFLAGS) -c $$< -o $$@ -MD -MT $$@

$$($(1)_OBJ_DIR)/%.o: %.S
	@echo "  AS      $$<" $(ECHO_OUTPUT)
	$(Q)mkdir -p $$(dir $$@)
	$(Q)$(AS) $$($(1)_ASFLAGS) -c $$< -o $$@ -MD -MT $$@


.PHONY: FORCE
ifeq ($($(1)_CARGO_FLAGS),)
$(1)_CARGO_FLAGS = $(CARGO_FLAGS)
endif

ifeq ($($(1)_RUSTFLAGS),)
$(1)_RUSTFLAGS = $(RUSTFLAGS)
endif

# Append all the config options to the rust flags
# a config.mk option declared as
#
#     user_FEATURES += SOME_OPTION=some_value
#
# becomes a rust flag passed to cargo as
#
#     --cfg SOME_OPTION="some_value"
#
# Some care must be taken as:
# - cargo requires all options values to be quoted with double quote
# - cargo apparently does not support 0-length value (so `--cfg FOO=""` is not
#   supported, `--cfg FOO` must be used instead
#
$(1)_RUSTFLAGS += $(shell echo '$(FEATURES)' | awk '                \
	BEGIN {RS=" +"}                                                 \
	{	if (match($$0, /^([a-zA-Z0-9_]+)="?(([^"]|\\")+)"?/, a)) { \
			print "--cfg " a[1] "=\"" a[2] "\"";                    \
		}                                                           \
		else if (match($$0, /^[a-zA-Z0-9_]+/, a)) {                 \
			print "--cfg " a[0] ;                                   \
		}                                                           \
		else {                                                      \
		}                                                           \
	}')


$$($(1)_RUST_LIB): FORCE
	@echo "  CARGO   $$($(1)_CRATE)" $(ECHO_OUTPUT)
	$(Q)mkdir -p $$(dir $$@)
	$(Q)CROSS_COMPILE=$(CROSS_TOOLCHAIN) CONFIG_VARIABLES="$(FEATURES)" \
	    CARGO_TARGET_DIR="$(APP_BUILD_DIR)/cargo-target" \
	    RUSTFLAGS='$$($(1)_RUSTFLAGS)' \
	    $(CARGO) build $$($(1)_CARGO_FLAGS) \
	    --out-dir $$(dir $$@)

$(APP_BUILD_DIR)/$(1): $$($(1)_METADATA_BIN) $$($(1)_OBJS) $$($(1)_RUST_LIB) $$($(1)_ARLIBS)
	@echo "  EXE     $$@" $(ECHO_OUTPUT)
	$(Q)$(LD) -z max-page-size=0x1000 \
	    $(if $($(1)_CRATE),--undefined=main) \
	    $$($(1)_OBJS) \
	    $$($(1)_RUST_LIB) \
	    $$($(1)_LDFLAGS) $$($(1)_LDLIBS) \
	    -o $$@.nometa
	$(Q)$(OC) --add-section \
		.pnc.metadata=$$($(1)_METADATA_BIN) $$@.nometa $$@.nostrip
ifeq ($(DEBUG), 0)
	@echo "  STRIP   $$@" $(ECHO_OUTPUT)
	$(Q)$(STRIP) -s $$@.nostrip -o $$@
else
	$(Q)cp $$@.nostrip $$@
endif
	@echo $(ECHO_OUTPUT)
	@echo "Built $$@ successfully" $(ECHO_OUTPUT)
	@echo $(ECHO_OUTPUT)

$$($(1)_LIB_DEP):
	@echo "  DEPEND  $$@" $(ECHO_OUTPUT)
	$(Q)mkdir -p $$(dir $$@)
	$$(Q)$(BASE_DIR)/scripts/build/dep/compute_graph.py -C $(BUILD_BASE) \
		$$($(1)_LIBS) > $$@

do_build : $(APP_BUILD_DIR)/$(1) $$($(1)_LIB_DEP)

do_config : $(1)_config

.PHONY: do_build do_config $(1)_config

endef # BUILD_APPLICATION

$(APP_BUILD_DIR):
	$(Q)mkdir -p $(APP_BUILD_DIR)

get_lib_dependencies:
	@echo $(LIB_DEPENDENCIES)

# Build each application(s)
$(foreach app,$(APPLICATIONS),$(eval $(call BUILD_APPLICATION,$(app))))

.PHONY: all do_clean depend add_lib_dependencies get_lib_dependencies config

do_clean:
	@echo "  CLEAN $(APPNAME)" $(ECHO_OUTPUT)
	$(Q)rm -rf $(APP_DIR)/build/$(APPNAME)

clean: do_clean
