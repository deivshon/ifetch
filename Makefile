SRC_DIR = src
HS_DIR = hs

ifeq ($(dbg), true)
	CFLAGS = -Wall -Wextra -g
else
	CFLAGS = -Wall -Wextra -O2
endif

.PHONY: clean

ifetch: ifetch.o netutils.o argutils.o
	gcc $(CFLAGS) $^ -o ifetch

%.o: $(SRC_DIR)/%.c $(HS_DIR)/%.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o ifetch
