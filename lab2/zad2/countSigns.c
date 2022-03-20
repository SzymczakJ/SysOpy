#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "string.h"
#include <sys/times.h>

clock_t clock_t_begin, clock_t_end;
struct tms times_start_buffer, times_end_buffer;

void start_timer(){
    clock_t_begin = times(&times_start_buffer);
}

void stop_timer(){
    clock_t_end = times(&times_end_buffer);
}

double calc_time(clock_t s, clock_t e) {
    return ((double) (e - s) / (double) sysconf(_SC_CLK_TCK));
}


void print_times(){
    printf("real %.6fs user %.6fs sys %.6fs\n",
           calc_time(clock_t_begin, clock_t_end),
           calc_time(times_start_buffer.tms_cutime, times_end_buffer.tms_cutime),
           calc_time(times_start_buffer.tms_cstime, times_end_buffer.tms_cstime));
}

void libCountSign(char *fileName, char sign, int *numberOfSignPointer, int *linesWithSignPointer) {
    FILE *file = fopen(fileName, "r");

    int newLineFlag = 1;
    int numberOfSign = 0;
    int linesWithSign = 0;
    char *currentSign = calloc(sizeof(char), 2);
    currentSign[1] = '\0';
    while(!feof(file)) {
        fread(currentSign, sizeof(char), 1, file);
        if (feof(file)) break;
        if (currentSign[0] == sign) {
            numberOfSign++;
            if (newLineFlag) {
                linesWithSign++;
                newLineFlag = 0;
            }
        } else if (*currentSign == '\n') {
            newLineFlag = 1;
        }
    }
    *numberOfSignPointer = numberOfSign;
    *linesWithSignPointer = linesWithSign;
    fclose(file);
}

void sysCountSign(char *fileName, char sign, int *numberOfSignPointer, int *linesWithSignPointer) {
    int file = open(fileName, O_RDONLY);

    int newLineFlag = 1;
    int numberOfSign = 0;
    int linesWithSign = 0;
    char *currentSign = calloc(sizeof(char), 1);
    while(read(file, currentSign, 1) == 1) {
        if (currentSign[0] == sign) {
            numberOfSign++;
            if (newLineFlag) {
                linesWithSign++;
                newLineFlag = 0;
            }
        } else if (*currentSign == '\n') {
            newLineFlag = 1;
        }
    }
    *numberOfSignPointer = numberOfSign;
    *linesWithSignPointer = linesWithSign;
    close(file);
}

int main(int argc, char** argv) {
    int numberOfSign;
    int linesWithSign;

#ifdef TEST
    printf("time using lib:\n");
    start_timer();
    libCountSign("testFile", 'o', &numberOfSign, &linesWithSign);
    stop_timer();
    print_times();
    printf("time using sys:\n");
    start_timer();
    sysCountSign("testFile", 'o', &numberOfSign, &linesWithSign);
    stop_timer();
    print_times();
    return 0;
#endif
    sysCountSign(argv[1], argv[2][0], &numberOfSign, &linesWithSign);
    char result[100];
    sprintf(result, "number of signs:%d lines with sign:%d", numberOfSign, linesWithSign);
    puts(result);
}