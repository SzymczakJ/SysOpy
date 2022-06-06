#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
typedef struct{
    int shouldSantaWakeUp;
    pthread_cond_t santaWakeUpCond;
    pthread_mutex_t santaWakeUpMutex;

    pthread_cond_t IDTakenCond;
    pthread_mutex_t IDTakenMutex;
    int reindeerID;
    int reindeerIDTaken;
    int nReindeersWaiting;
    pthread_mutex_t reindeersWaitingMutex;
    int reindeersToGo[9];
    int reindeersCanGo;
    pthread_cond_t reindeersGoCond;
    pthread_mutex_t reindeersGoMutex;

    pthread_cond_t elfIDTakenCond;
    pthread_mutex_t elfIDTakenMutex;
    int elfID;
    int elfIDTaken;
    int elvesWaiting[3];
    pthread_mutex_t elvesWaitingMutex;
    pthread_cond_t elvesWaitingCond;
    int elfToGo[10];
    pthread_cond_t elvesGoCond;
    pthread_mutex_t elvesGoMutex;

}FactoryData;

void *elfLoop(void *data) {
    FactoryData *factoryData = data;
    pthread_mutex_lock(&factoryData->elfIDTakenMutex);
    int myID = factoryData->elfID;
    factoryData->elfIDTaken = 1;
    pthread_cond_broadcast(&factoryData->elfIDTakenCond);
    pthread_mutex_unlock(&factoryData->elfIDTakenMutex);

    while(1){
        usleep((rand() % 3000 + 2000)*1000);
        int takenPosition = 0;
        pthread_mutex_lock(&factoryData->elvesWaitingMutex);
        while(!takenPosition){
            for(int i = 0; i < 3; i++){
                if(factoryData->elvesWaiting[i] == -1){
                    factoryData->elvesWaiting[i] = myID;
                    takenPosition = 1;
                    printf("Elf: czeka %d elfów na mikołaja, %d\n", i+1, myID);
                    if(i == 2){
                        printf("Elf: wybudzam Mikołaja, %d\n", myID);
                        pthread_mutex_lock(&factoryData->santaWakeUpMutex);
                        factoryData->shouldSantaWakeUp = 1;
                        pthread_mutex_unlock(&factoryData->santaWakeUpMutex);
                        pthread_cond_signal(&factoryData->santaWakeUpCond);
                    }
                    break;
                }
            }
            if(!takenPosition){
                printf("Elf: czeka na powrót elfów, %d\n", myID);
                pthread_cond_wait(&factoryData->elvesWaitingCond, &factoryData->elvesWaitingMutex);
            }
        }
        pthread_mutex_unlock(&factoryData->elvesWaitingMutex);

        pthread_mutex_lock(&factoryData->elvesGoMutex);
        while(!factoryData->elfToGo[myID]){
            pthread_cond_wait(&factoryData->elvesGoCond, &factoryData->elvesGoMutex);
        }
        factoryData->elfToGo[myID] = 0;
        printf("Elf: Mikołaj rozwiązuje problem, %d\n", myID);
        pthread_mutex_unlock(&factoryData->elvesGoMutex);
        usleep((rand() % 1000 + 1000)*1000);
    }

    return NULL;
}

void *reindeerLoop(void *data) {
    FactoryData *factoryData = data;
    pthread_mutex_lock(&factoryData->IDTakenMutex);
    int myID = factoryData->reindeerID;
    factoryData->reindeerIDTaken = 1;
    pthread_cond_broadcast(&factoryData->IDTakenCond);
    pthread_mutex_unlock(&factoryData->IDTakenMutex);
    while(1) {
        usleep((rand() % 5000 + 5000)*1000);
        pthread_mutex_lock(&factoryData->reindeersWaitingMutex);
        factoryData->nReindeersWaiting++;
        printf("Renifer: czeka %d reniferów na Mikołaja, %d\n", factoryData->nReindeersWaiting, myID);

        if (factoryData->nReindeersWaiting == 9) {
            pthread_mutex_lock(&factoryData->santaWakeUpMutex);
            printf("Renifer: wybudzam Mikołaja, %d\n", myID);
            factoryData->shouldSantaWakeUp = 1;
            pthread_cond_broadcast(&factoryData->santaWakeUpCond);
            pthread_mutex_unlock(&factoryData->santaWakeUpMutex);
        }

        pthread_mutex_unlock(&factoryData->reindeersWaitingMutex);

        pthread_mutex_lock(&factoryData->reindeersGoMutex);
        while(!factoryData->reindeersToGo[myID]) {
            pthread_cond_wait(&factoryData->reindeersGoCond, &factoryData->reindeersGoMutex);
        }
        factoryData->reindeersToGo[myID] = 0;
        pthread_mutex_unlock(&factoryData->reindeersGoMutex);
    }

    return NULL;
}

void *santaLoop(void *data) {
    FactoryData * factoryData = data;
    while (1) {
        pthread_mutex_lock(&factoryData->santaWakeUpMutex);

        while (!factoryData->shouldSantaWakeUp) {
            pthread_cond_wait(&factoryData->santaWakeUpCond, &factoryData->santaWakeUpMutex);
        }

        factoryData->shouldSantaWakeUp = 0;

        pthread_mutex_unlock(&factoryData->santaWakeUpMutex);

        int deliverToys = 0;

        pthread_mutex_lock(&factoryData->reindeersWaitingMutex);
        if(factoryData->nReindeersWaiting == 9){
            factoryData->nReindeersWaiting = 0;
            pthread_mutex_lock(&factoryData->reindeersGoMutex);
            for(int i = 0; i < 9; i++){
                factoryData->reindeersToGo[i] = 1;
            }
            puts("Mikołaj: dostarczam zabawki");
            usleep((rand() % 2000 + 2000)*1000);
            factoryData->reindeersCanGo = 1;
            pthread_mutex_unlock(&factoryData->reindeersGoMutex);
            pthread_cond_broadcast(&factoryData->reindeersGoCond);
            deliverToys = true;
        }
        pthread_mutex_unlock(&factoryData->reindeersWaitingMutex);

        if(deliverToys){

        }

        pthread_mutex_lock(&factoryData->elvesWaitingMutex);

        int threeWaiting = 1;
        for(int i = 0; i < 3; i++){
            if(factoryData->elvesWaiting[i] == -1)
                threeWaiting = 0;
        }
        pthread_mutex_unlock(&factoryData->elvesWaitingMutex);

        if (threeWaiting) {
            pthread_mutex_lock(&factoryData->elvesGoMutex);
            pthread_mutex_lock(&factoryData->elvesWaitingMutex);
            printf("Mikołaj: rozwiązuję problemy elfów %d %d %d\n",
                   factoryData->elvesWaiting[0],
                   factoryData->elvesWaiting[1],
                   factoryData->elvesWaiting[2]);
            for(int i = 0; i < 3; i++){
                factoryData->elfToGo[factoryData->elvesWaiting[i]] = 1;
            }
            pthread_cond_broadcast(&factoryData->elvesGoCond);
            pthread_mutex_unlock(&factoryData->elvesGoMutex);
            usleep((rand() % 1000 + 1000)*1000);
            for(int i = 0; i < 3; i++){
                factoryData->elvesWaiting[i] = -1;
            }
            pthread_cond_broadcast(&factoryData->elvesWaitingCond);
            pthread_mutex_unlock(&factoryData->elvesWaitingMutex);
        }

        puts("Mikołaj: zasypiam");
    }

    return NULL;
}

int main(int argc, char **argv) {
    srand(time(NULL));

    FactoryData fd = {
            .shouldSantaWakeUp = false,
            .santaWakeUpCond = PTHREAD_COND_INITIALIZER,
            .santaWakeUpMutex = PTHREAD_MUTEX_INITIALIZER,

            .IDTakenCond = PTHREAD_COND_INITIALIZER,
            .IDTakenMutex = PTHREAD_MUTEX_INITIALIZER,
            .reindeerID = 0,
            .reindeerIDTaken = 0,
            .nReindeersWaiting = 0,
            .reindeersWaitingMutex = PTHREAD_MUTEX_INITIALIZER,
            .reindeersGoCond = PTHREAD_COND_INITIALIZER,
            .reindeersToGo = {false},
            .reindeersGoMutex = PTHREAD_MUTEX_INITIALIZER,

            .elfIDTakenCond = PTHREAD_COND_INITIALIZER,
            .elfIDTakenMutex = PTHREAD_MUTEX_INITIALIZER,
            .elfID = 0,
            .elfIDTaken = 0,
            .elvesWaiting = {-1, -1, -1},
            .elvesWaitingMutex = PTHREAD_MUTEX_INITIALIZER,
            .elvesWaitingCond = PTHREAD_COND_INITIALIZER,
            .elfToGo = {false},
            .elvesGoMutex = PTHREAD_MUTEX_INITIALIZER,
            .elvesGoCond = PTHREAD_COND_INITIALIZER
    };

    pthread_t santa;
    pthread_t reindeers[9];
    pthread_t elves[10];

    pthread_create(&santa, NULL, santaLoop, &fd);
    for (int i = 0; i < 9; i++) {
        pthread_mutex_lock(&fd.IDTakenMutex);
        fd.reindeerID = i;
        fd.reindeerIDTaken = 0;
        pthread_create(&reindeers[i], NULL, reindeerLoop, &fd);
        while (!fd.reindeerIDTaken) {
            pthread_cond_wait(&fd.IDTakenCond, &fd.IDTakenMutex);
        }
        pthread_mutex_unlock(&fd.IDTakenMutex);
    }

    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&fd.elfIDTakenMutex);
        fd.elfID = i;
        fd.elfIDTaken = 0;
        pthread_create(&elves[i], NULL, elfLoop, &fd);
        while (!fd.elfIDTaken) {
            pthread_cond_wait(&fd.elfIDTakenCond, &fd.elfIDTakenMutex);
        }
        pthread_mutex_unlock(&fd.elfIDTakenMutex);
    }

    pthread_join(santa, NULL);
    for(int i = 0; i < 9; i++){
        pthread_join(reindeers[i], NULL);
    }
    for(int i = 0; i < 10; i++){
        pthread_join(elves[i], NULL);
    }


    return 0;
}
#pragma clang diagnostic pop