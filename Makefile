SRC_DIR = src
HS_DIR = hs

PACKAGE_INSTALL_DIR = /usr/bin
LOCAL_COMPILE_INSTALL_DIR = /usr/local/bin

ETC_CONFIG_DIR = /etc/ifetch

ifeq ($(dbg), false)
	CFLAGS = -Wall -Wextra -O2
else
	CFLAGS = -Wall -Wextra -g
endif

ifeq ($(pkg), true)
	INSTALL_DIR = $(PACKAGE_INSTALL_DIR)
else
	INSTALL_DIR = $(LOCAL_COMPILE_INSTALL_DIR)
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
	mkdir -p $(INSTALL_DIR)
	sudo cp ifetch $(INSTALL_DIR)/ifetch
	chmod 711 $(INSTALL_DIR)/ifetch
	mkdir -p $(ETC_CONFIG_DIR)
	cp -r defaults/* $(ETC_CONFIG_DIR)

uninstall:
	rm -f $(INSTALL_DIR)/ifetch

purge:
	sudo rm -rf $(ETC_CONFIG_DIR)

clean:
	rm -f *.o ifetch
