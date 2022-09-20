SRC_DIR = src
HS_DIR = hs

ifeq ($(dbg), false)
	CFLAGS = -Wall -Wextra -O2
else
	CFLAGS = -Wall -Wextra -g
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

clean:
	rm -f *.o ifetch
