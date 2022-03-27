compile:
	gcc -c calculateIntegral.c
	gcc calculateIntegral.o -o main

clean:
	rm -f *.o
	rm -f main
	rm -f w*

test:
	rm -f raport.txt
	gcc calculateIntegral.c -D TEST -o main
	@printf "Jeden proces liczący\n" 2>/dev/null >> raport.txt
	./main 0.0000001 1 2>/dev/null >> raport.txt
	@printf "Dwa procesy liczące\n" 2>/dev/null >> raport.txt
	./main 0.0000001 2 2>/dev/null >> raport.txt
	@printf "Cztery procesy liczące\n" 2>/dev/null >> raport.txt
	./main 0.0000001 4 2>/dev/null >> raport.txt
	@printf "Osiem procesów liczących\n" 2>/dev/null >> raport.txt
	./main 0.0000001 8 2>/dev/null >> raport.txt
	@printf "Dwanaście procesów liczących\n" 2>/dev/null >> raport.txt
	./main 0.0000001 12 2>/dev/null >> raport.txt
