#include "stdlib.h"
#include <stdio.h>
#include <sys/types.h>
#include<sys/wait.h>
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

double func(double x) {
    return 4 * (x * x + 1);
}

double calculateIntegralOnInterval(double start, double end, double delta) {
    double currentPoint = start;
    double integralSum = 0;
    while (currentPoint < end) {
        integralSum += delta * func(currentPoint);
        currentPoint += delta;
    }
    return integralSum;
}

int main(int argc, char **argv) {
#ifdef TEST
    start_timer();
#endif

    char sing = '\0';
    char *eptr = &sing;
    int n = atoi(argv[2]);
    double delta = strtod(argv[1], &eptr);
    int motherPID = getpid();

    char resultString[1100];
    char fileName[20];
    FILE *currentFile;
    double intervalStart = 0;
    double intervalEnd = 1 / (double) n;
    double intervalResult;
    for (int i = 0; i < n; i++) {
        fork();
        if (getpid() != motherPID) {
            intervalResult = calculateIntegralOnInterval(intervalStart, intervalEnd, delta);
            sprintf(resultString, "%lf", intervalResult);
            sprintf(fileName, "w%d.txt", i + 1);
            currentFile = fopen(fileName, "w");
            fwrite(resultString, 1, strlen(resultString), currentFile);
            fclose(currentFile);
            return 0;
        }
        intervalStart += 1 / (double) n;
        intervalEnd += 1 / (double) n;
    }

    for (int i = 0; i < n; i++) wait(NULL);

    double finalRes = 0;
    for (int i = 0; i < n; i++) {
        sprintf(fileName, "w%d.txt", i + 1);
        currentFile = fopen(fileName, "r");
        fread(resultString, 1, 9, currentFile);
        resultString[8] = '\0';
        finalRes += strtod(resultString, &eptr);
        fclose(currentFile);
    }
#ifdef TEST
    stop_timer();
    print_times();
#endif

#ifndef TEST
    printf("%lf", finalRes);
#endif

    return 0;
}