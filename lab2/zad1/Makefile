compile:
	gcc -c whitespace_destroyer.c
	gcc whitespace_destroyer.o -o main

clean:
	rm -f *.o
	rm -f main

test:
	rm -f *.txt
	gcc whitespace_destroyer.c -D TEST -o main
	./main 2>/dev/null >> pomiar_zad_1.txt