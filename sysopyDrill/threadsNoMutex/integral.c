#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct {
    double start;
    double end;
    int steps;
}jobInfo;

void *function(void *data) {
    jobInfo *info = data;
    double integral = 0;
    double delta = 1.0 / 5 / info->steps;
    double x;
    for (int i = 0; i < info->steps; i++) {
        x = i * delta + info->start;
        integral += (4.0 / (x * x + 1)) * delta;
    }
    double *res = malloc(sizeof(int));
    *res = integral;
    return (void*)res;
}

int main() {
    int numberOfThreads = 5;
    int numberOfSteps = 10000;
    int start = 0;
    int end = 1;

    pthread_t threads[5];
    jobInfo* info = malloc(sizeof(jobInfo) * 5);
    for (int i = 0; i < 5; i++) {
        info[i].steps = numberOfSteps;
        info[i].start = start + 1.0 / 5 * i;
        info[i].end = start + 1.0 / 5 * (i + 1);
        pthread_create(&threads[i], NULL, function, &info[i]);
    }

    double *res;
    double sum;
    for (int i = 0; i < 5; i++) {
        if (pthread_join(threads[i], (void **) &res) == 0) {
            double *convertedRes = (double*)res;
            sum += *convertedRes;
            free(res);
        }
    }
    printf("res: %f\n", sum);

    free(info);
}