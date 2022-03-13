blckmem.o: blckmem.c blckmem.h
	gcc -c blckmem.c

libblckmem.a: blckmem.o
	ar rcs libblckmem.a blckmem.o

libblckmem.so: blckmem.c blckmem.h
	gcc -fPIC -c blckmem.c
	gcc -shared blckmem.o -o libblckmem.so

clean:
	rm -f *.o
	rm -f *.a