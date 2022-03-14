CC=gcc
CFLAGS=-Wall -Wno-unused-variable

static:
	$(CC) -c blckmem.c
	ar rcs libblckmem.a blckmem.o

clean:
	rm -f *.o *.a word_counter main report2.txt

test: static
	gcc word_counter.c -I../zad1 -lblckmem -L../zad2 -D TEST -o word_counter
	make report

report:
	rm -f ./report2.txt
	@printf "Word count test:\n" 2>/dev/null >> report2.txt
	./word_counter 2>/dev/null >> report2.txt