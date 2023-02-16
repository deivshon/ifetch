SRC_DIR = src
HS_DIR = hs

DESTDIR = /usr/bin
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
	mkdir -p $(DESTDIR)
	cp ifetch $(DESTDIR)/ifetch
	chmod 755 $(DESTDIR)/ifetch
	mkdir -p $(ETC_CONFIG_DIR)
	cp -rn defaults/* $(ETC_CONFIG_DIR)

uninstall:
	rm -f $(DESTDIR)/ifetch

purge:
	rm -rf $(ETC_CONFIG_DIR)

clean:
	rm -f *.o ifetch
