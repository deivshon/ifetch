SRC_DIR = src
HS_DIR = hs

ifetch: ifetch_main.o netutils_nm.o argutils_nm.o
	gcc $^ -o ifetch

%_main.o: $(SRC_DIR)/%.c
	gcc -c -o $@ $<

%_nm.o: $(SRC_DIR)/%.c $(HS_DIR)/%.h
	gcc -c -o $@ $<

clean:
	rm *.o ifetch
