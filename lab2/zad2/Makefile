compile:
	gcc -c countSigns.c
	gcc countSigns.o -o main

clean:
	rm -f *.o
	rm -f main
	rm -f *.txt

test:
	rm -f *.txt
	gcc countSigns.c -D TEST -o main
	./main 2>/dev/null >> pomiar_zad_1.txt