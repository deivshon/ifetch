SRC_DIR = src
HS_DIR = hs

ifeq ($(dbg), true)
	CFLAGS = -Wall -Wextra -g
else
	CFLAGS = -Wall -Wextra -O2
endif

.PHONY: clean

ifetch: ifetch_main.o netutils_nm.o argutils_nm.o
	gcc $(CFLAGS) $^ -o ifetch

%_main.o: $(SRC_DIR)/%.c
	gcc $(CFLAGS) -c -o $@ $<

%_nm.o: $(SRC_DIR)/%.c $(HS_DIR)/%.h
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o ifetch
