#
# Copyright (c) 2016-2023, ProvenRun and/or its affiliates. All rights reserved.
#
# Must be called with BASE_DIR set

ifndef BASE_DIR
    $(error BASE_DIR has to be set by calling Makefile !)
endif

PROG_FILE       ?= $(BUILD_BASE)/.user_progs
PROG_TIMESTAMP  := $(BUILD_BASE)/.user_progs_timestamp
TARGET_FILE     := $(BUILD_BASE)/kconfig/target.mk
CONFIG_FILE     := $(BUILD_BASE)/kconfig/config.mk
TEST_FILE       := $(BUILD_BASE)/kconfig/pnc_tests.mk
APP_FILE        := $(BUILD_BASE)/kconfig/pnc_app.mk
APP_DIR         := $(BUILD_BASE)/app
APP_SRC_DIR     := $(BUILD_BASE)/app/src
APP_BUILD_DIR   := $(BUILD_BASE)/app/build

include $(BASE_DIR)/scripts/build/verbose.mk

include $(CONFIG_FILE)
include $(TARGET_FILE)

##
# If APP_FILE exists, we must include it as it allows cleaning applications
# In some cases, 'clean' is called here and APP_FILE does not exist. Hopefully,
# applications have already been cleaned.
# Include test framework if the test declaration is present.
##
-include $(APP_FILE)
-include $(TEST_FILE)

# Applications use both kernel and user features.
FEATURES := $(kernel_FEATURES) $(user_FEATURES)

# Include target compiler default tools and options.
COMPILE_ARCH := $(ARCH)
include $(BASE_DIR)/scripts/build/common_target.mk

ifneq ($(PNC_TESTS),)
# Need to trigger build of tests
USER_APPS += tests-framework
endif

all: programs

# Include rules for building binaries.
include $(BASE_DIR)/scripts/build/core/rules_binary.mk

programs: $(PROG_TIMESTAMP)

doc: app-doc

config: app-config

depend: app-depend clean-libs-file $(PROG_FILE)
	$(Q)cat $(LIBS_FILE) | gawk '!seen[$$0]++' > $(LIBS_FILE).tmp
	$(Q)mv $(LIBS_FILE).tmp $(LIBS_FILE)

##
# Dummy intermediate targets.
# Do not remove, they must be present in case USER_APPS is empty.
##
app-programs:

app-doc:

app-config:

app-depend:

app-clean:

# Reset the file containing libraries
clean-libs-file:
	$(Q)echo pnc > $(LIBS_FILE)
	$(Q)for l in $(COMPILED_LIBS); do \
		echo "$$l" >> $(LIBS_FILE); \
	done

FEATURES_DEFINES := $(addprefix -DCONFIG_,$(FEATURES))

PNC_APP_LINKERSCRIPT := $(APP_DIR)/pnc_app.ld
PNC_APP_LINKERFILE := $(BASE_DIR)/scripts/pnc_app.ld.S
PNC_APP_ASFLAGS := $(PLATFORM_ASFLAGS) $(FEATURES_DEFINES)
$(eval $(call BUILD_LINKER_SCRIPT,$(PNC_APP_LINKERSCRIPT),PNC_APP))

clean: app-clean
	@echo "  APP CLEAN" $(ECHO_OUTPUT)
	$(Q)rm -f  $(PROG_FILE)
	$(Q)rm -f  $(PROG_TIMESTAMP)
	$(Q)rm -f  $(PNC_APP_LINKERSCRIPT)
	$(Q)rm -f  $(PNC_APP_LINKERSCRIPT).d
	$(Q)rmdir --ignore-fail-on-non-empty $(APP_BUILD_DIR)

##
# Generate the rules for building a single application.
# $(eval $(call BUILD_USER_PROGS,$(app)) adds the following rules to
# the context:
#   $(app):
#   $(app)-doc:
#   $(app)-config:
#   $(app)-depend:
#   $(app)-clean:
##
define BUILD_USER_PROGS

app-programs: $(1)
$(1): $(PNC_APP_LINKERSCRIPT)
	$(Q)$$(MAKE) -C $(APP_SRC_DIR)/$(1) all APPNAME=$(1)

app-doc: $(1)-doc
$(1)-doc:
	$(Q)$$(MAKE) -C $(APP_SRC_DIR)/$(1) doc APPNAME=$(1)

app-config: $(1)-config
$(1)-config: | $(APP_BUILD_DIR)/$(1)
	$(Q)$$(MAKE) -C $(APP_SRC_DIR)/$(1) config APPNAME=$(1)

app-depend: $(1)-depend
$(1)-depend: clean-libs-file
	$(Q)$$(MAKE) -C $(APP_SRC_DIR)/$(1) depend APPNAME=$(1) \
		LIBS_FILE=$(LIBS_FILE)

app-clean: $(1)-clean
$(1)-clean:
	$(Q)if [ -d "$(APP_SRC_DIR)/$(1)" ]; then \
		$$(MAKE) -C $(APP_SRC_DIR)/$(1) clean APPNAME=$(1); \
	fi

$(APP_BUILD_DIR)/$(1):
	$(Q)mkdir -p $$@

.PHONY: $(1) $(1)-doc $(1)-config $(1)-depend $(1)-clean

endef

##
# Generate the rules for building all configured applications.
##
$(foreach app, $(USER_APPS),$(eval $(call BUILD_USER_PROGS,$(app))))

##
# Generate the .user_progs file.
# .user_progs is not modified if the contents would not be changed.
##
$(PROG_FILE): app-depend $(APP_FILE) | $(BUILD_BASE)
	$(eval ALL_APPS := $(USER_APPS) $(PRECOMPILED_APPS))
	$(eval USER_PROGS := $(ALL_APPS:%=$(APP_BUILD_DIR)/%/.binaries))
	@echo "" | sort - $(USER_PROGS) | paste -s -d ' ' | tr -d '\n' > $@.tmp
	$(Q)if [ -r $@ ] && cmp -s $@ $@.tmp; then \
		rm -f $@.tmp; \
	else \
		mv -f $@.tmp $@; \
	fi

##
# Track when user programs gets (re)built.
##
$(PROG_TIMESTAMP): app-programs | $(APP_BUILD_DIR)
	@echo "  CHK     $(PROG_TIMESTAMP)" $(ECHO_OUTPUT)
	$(Q)MOST_RECENT=`cd $(APP_BUILD_DIR) && cat $(PROG_FILE) |                 \
	                 tr -d '\n' | xargs -n 1 -r date +"%s%N" -r | sort -nr |   \
	                 head -n 1`;                                               \
	if [ -r $(PROG_TIMESTAMP) ] ; then                                         \
	    CURR_PROG_TIMESTAMP=`cat $(PROG_TIMESTAMP)`;                           \
	fi;                                                                        \
	if ! test "$$MOST_RECENT" = "$$CURR_PROG_TIMESTAMP" ; then                 \
	    echo "  UPD     $(PROG_TIMESTAMP)" $(ECHO_OUTPUT);                     \
	    echo $$MOST_RECENT > $(PROG_TIMESTAMP);                                \
	fi

$(BUILD_BASE):
	$(Q)mkdir -p $@

$(APP_BUILD_DIR):
	$(Q)mkdir -p $@

FORCE: ;
.PHONY: FORCE all mrproper clean-libs-file \
	programs doc config depend clean \
	app-programs app-doc app-config app-depend app-clean
