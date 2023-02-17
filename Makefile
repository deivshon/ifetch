SRC_DIR = src
HS_DIR = hs

DEST_DIR ?=
INSTALL_DIR = /usr/bin
ETC_CONFIG_DIR = /etc/ifetch

ifeq ($(DBG), true)
	CFLAGS = -Wall -Wextra -g
else
	CFLAGS = -Wall -Wextra -O2
endif

.PHONY: clean

ifetch: ifetch.o netutils.o argutils.o
	gcc $(CFLAGS) $^ -o ifetch

ifetch.o: $(SRC_DIR)/ifetch.c $(HS_DIR)/ifetch.h $(HS_DIR)/netutils.h
	gcc $(CFLAGS) -c -o $@ $<

argutils.o: $(SRC_DIR)/argutils.c $(HS_DIR)/argutils.h $(HS_DIR)/ifetch.h
	gcc $(CFLAGS) -c -o $@ $<

%.o: $(SRC_DIR)/%.c $(HS_DIR)/%.h
	gcc $(CFLAGS) -c -o $@ $<

install: ifetch
	mkdir -p $(DEST_DIR)$(INSTALL_DIR)
	cp ifetch $(DEST_DIR)$(INSTALL_DIR)/ifetch
	chmod 755 $(DEST_DIR)$(INSTALL_DIR)/ifetch
	mkdir -p $(DEST_DIR)$(ETC_CONFIG_DIR)
	cp -rn defaults/* $(DEST_DIR)$(ETC_CONFIG_DIR)
	mkdir -p $(DEST_DIR)/usr/share/doc/ifetch/
	cp README.md $(DEST_DIR)/usr/share/doc/ifetch/

uninstall:
	rm -f $(INSTALL_DIR)/ifetch

purge:
	rm -rf $(ETC_CONFIG_DIR)
	rm -rf /usr/share/doc/ifetch

clean:
	rm -f *.o ifetch
