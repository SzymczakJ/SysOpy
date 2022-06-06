#include "stdio.h"
#include "stdlib.h"
#include <pthread.h>
#include <time.h>
#include "unistd.h"

typedef struct {
    int counter;
    pthread_mutex_t counterMutex;

    int shouldSantaWakeUp;
    pthread_mutex_t wakeUpMutex;
    pthread_cond_t wakeUpCond;
    pthread_cond_t asleepCond;
}threadInfo;

void *alarmAndIncrement(void *data) {
    threadInfo *info = data;
    for (int i = 0; i < 50; i++) {
        usleep(1000000);
        pthread_mutex_lock(&info->counterMutex);
        info->counter++;
        printf("counter++\n");
        pthread_mutex_unlock(&info->counterMutex);
        usleep(2000000);
        pthread_mutex_lock(&info->wakeUpMutex);
        while (info->shouldSantaWakeUp) {
            pthread_cond_wait(&info->asleepCond, &info->wakeUpMutex);
        }
        info->shouldSantaWakeUp = 1;
        printf("budze skurwysyna\n");
        pthread_cond_broadcast(&info->wakeUpCond);
        pthread_mutex_unlock(&info->wakeUpMutex);
    }
    return NULL;
}

void *santaLoop(void *data) {
    threadInfo *info = data;
    for (int i = 0; i < 100; i++) {
        pthread_mutex_lock(&info->wakeUpMutex);
        while(!info->shouldSantaWakeUp) {
            pthread_cond_wait(&info->wakeUpCond, &info->wakeUpMutex);
        }
        printf("o gurwa kto mnie budzi:DD\n");
        info->shouldSantaWakeUp = 0;
        printf("ide spac\n");
        pthread_cond_broadcast(&info->asleepCond);
        pthread_mutex_unlock(&info->wakeUpMutex);
    }
    return NULL;
}

int main() {
    threadInfo info = {
            .counter = 0,
            .counterMutex = PTHREAD_MUTEX_INITIALIZER,

            .shouldSantaWakeUp = 0,
            .wakeUpMutex = PTHREAD_MUTEX_INITIALIZER,
            .wakeUpCond = PTHREAD_COND_INITIALIZER,
            .asleepCond = PTHREAD_COND_INITIALIZER
    };

    pthread_t santa;
    pthread_t incrementators[3];
    pthread_create(&santa, NULL, santaLoop, &info);
    for (int i = 0; i < 3; i++) {
        pthread_create(&(incrementators[i]), NULL, alarmAndIncrement, &info);
    }

    pthread_join(santa, NULL);
    usleep(1000000);
    for (int i = 0; i < 3; i++) {
        pthread_join(incrementators[i], NULL);
    }

    return 0;
}