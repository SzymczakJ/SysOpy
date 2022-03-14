CC = gcc
CFLAGS=-Wall -Wno-unused-variable

static:
	$(CC) -c blckmem.c
	ar rcs libblckmem.a blckmem.o

shared:
	$(CC) -c -fPIC blckmem.c
	$(CC) -shared -fPIC -o blckmem.so blckmem.o

clean:
	rm -f *.o *.a word_counter report2.txt *.so

test: static
	gcc word_counter.c -I../zad1 -lblckmem -L../zad2 -D TEST -o word_counter
	make report

report:
	rm -f ./report2.txt
	@printf "Word count test:\n" 2>/dev/null >> report2.txt
	./word_counter 2>/dev/null >> report2.txt


