SRC_DIR = src

ifetch: ifetch.o netutils.o argutils.o
	gcc $^ -o ifetch

%.o: $(SRC_DIR)/%.c
	gcc -c -o $@ $< 

clean:
	rm *.o ifetch
