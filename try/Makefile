CFLAGS = -std=gnu99 -ggdb3 -pedantic 
new_file: new_file.o
	gcc -o new_file new_file.o
new_file.o: new_file.c
	gcc $(CFLAGS) -c new_file.c
