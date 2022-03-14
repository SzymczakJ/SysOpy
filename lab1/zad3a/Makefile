CC=gcc
CFLAGS=-Wall -Wno-unused-variable
LIB_NAME=blckmem
LIB_PATH=../zad2
MAIN_PROG_LOCATION=../zad2
MAIN_PROG_FILE_C=word_counter.c
MAIN_PROG_PATH=$(MAIN_PROG_LOCATION)/$(MAIN_PROG_FILE_C)
REPORT_FILENAME=report3a.txt
REPORT_CMD=2>/dev/null >> $(REPORT_FILENAME)


static:
	make -C $(LIB_PATH) static
	$(CC) $(MAIN_PROG_PATH) -I$(LIB_PATH) -l$(LIB_NAME) -L$(LIB_PATH) -D TEST -o word_counter

shared:
	make -C $(LIB_PATH) shared
	$(CC) $(MAIN_PROG_PATH) -I$(LIB_PATH) -l$(LIB_NAME) -L$(LIB_PATH) -D TEST -o word_counter

dynamic:
	make -C $(LIB_PATH) shared
	$(CC) $(MAIN_PROG_PATH) -ldl -D LIB_DYNAMIC -D TEST -o word_counter

clean:
	make -C $(LIB_PATH) clean
	make -C $(MAIN_PROG_LOCATION) clean
	rm -f $(REPORT_FILENAME) word_counter

test: clean
	printf "static:\n" $(REPORT_CMD)
	make static
	make report

	printf "shared:\n" $(REPORT_CMD)
	make shared
	make report

#	printf "dynamic:\n" $(REPORT_CMD)
#	make dynamic
#	make report


report:
	@printf "Word count test:\n" $(REPORT_CMD)
	./word_counter $(REPORT_CMD)