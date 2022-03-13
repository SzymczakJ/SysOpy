CC=gcc
CFLAGS=-Wall -Wno-unused-variable

static:
	make -C ../zad1 static

clean:
	make -C ../zad1 clean
	rm -f .o word_counter

test: static
	$(CC) $(CFLAGS) word_counter.c -I../zad1 -lblckmem -L../zad1 -D PERFORMANCE_TESTS -o word_counter

