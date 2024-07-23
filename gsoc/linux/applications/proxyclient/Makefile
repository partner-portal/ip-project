
Q := @

CFLAGS += -I.

proxyclient: proxyclient.c provencore.c
	@echo "  CC      $<"
	$(Q)$(CC) $(CFLAGS) --std=gnu99 $(LDFLAGS) $^ -o $@

all: proxyclient

clean:
	@rm -f proxyclient

install: proxyclient
	install -D -m 755 $< $(INSTALL_PATH)/usr/bin

.PHONY: clean
