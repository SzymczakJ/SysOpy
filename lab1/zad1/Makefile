CC = gcc -Wall -std=c11 -ggdb

all: static shared clean

static:
	$(CC) -c blckmem.c
	ar rcs libblckmem.a blckmem.o

shared:
	$(CC) -c -fPIC blckmem.c
	$(CC) -shared -fPIC -o blckmem.so blckmem.o

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so