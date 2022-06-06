#include <stdio.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/sem.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

const key_t semaphoreSetKey = 0xabcd0101;
const key_t sharedMemoryKey = 0xabcd1010;

const size_t OVEN_SEMAPHORE_ID = 0;
const size_t TABLE_SEMAPHORE_ID = 1;

pid_t * children = NULL;
size_t childIndex = 0;
int semaphoreDescriptor = -1;
int memoryDescriptor = -1;

typedef struct{
    int pizzas;
}PizzaContainer;

typedef struct{
    int places[5];
}Table;

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};

PizzaContainer * oven;
Table * table;

const size_t shrMemSize = sizeof(PizzaContainer) + sizeof(Table);

bool keepWorking = true;

void intHandler(int signo){
    shmdt(oven);
    shmdt(table);
    exit(0);
}

int createSemaphores() {
    int semaphoreDescriptor = semget(semaphoreSetKey, 2, IPC_CREAT | IPC_EXCL | 0666);
    if (semaphoreDescriptor == -1) {
        semaphoreDescriptor = semget(semaphoreSetKey, 2, 0666);
        if (semaphoreDescriptor != -1) {
            semctl(semaphoreDescriptor, 0, IPC_RMID);
            semaphoreDescriptor = semget(semaphoreSetKey, 2, IPC_CREAT | IPC_EXCL | 0666);
        }
    }
    return semaphoreDescriptor;
}

int createSharedMemory() {
    int memoryDescriptor = shmget(sharedMemoryKey, shrMemSize, IPC_CREAT | IPC_EXCL | 0666);
    if (memoryDescriptor == -1) {
        memoryDescriptor = shmget(sharedMemoryKey, 0, 0666);
        if (memoryDescriptor != -1) {
            shmctl(memoryDescriptor, IPC_RMID, NULL);
            memoryDescriptor = shmget(memoryDescriptor, shrMemSize, IPC_CREAT | IPC_EXCL | 0666);
        }
    }
    return memoryDescriptor;
}

unsigned long getTimestamp(){
    struct timespec spec;
    clock_gettime(1, &spec);
    return spec.tv_sec*1000 + spec.tv_nsec / 1000000;
}

void houseClosing(int s) {
    for (size_t i = 0; i < childIndex; i++) {
        kill(children[i], SIGINT);
    }
}

void cookLoop() {
    printf("Cook starting.\n");
    signal(SIGINT, intHandler);
    pid_t myPid = getpid();
    srand(time(NULL) ^ (myPid << 8));
    while (keepWorking) {
        int pizzaType = rand() % 10;
        printf("(%d %lu) Przygotowywuje pizze: %d\n", myPid, getTimestamp(), pizzaType);
        int sleepDelay = (rand() % 1000 + 1000) * 1000;
        usleep(sleepDelay);
        struct sembuf sem = {.sem_num = OVEN_SEMAPHORE_ID, .sem_op = -1, .sem_flg = 0};
        int inserted = 0;
        int nInOven = 0;
        while(!inserted) {
            sem.sem_op = -1;
            semop(semaphoreDescriptor, &sem, 1);
            if (oven->pizzas < 4) {
                oven->pizzas++;
                inserted = 1;
            }
            nInOven = oven->pizzas;
            sem.sem_op = 1;
            semop(semaphoreDescriptor, &sem, 1);
        }
        printf("(%d %lu) Dodalem pizze: %d. Liczba pizz w piecu: %d.\n", myPid, getTimestamp(), pizzaType, nInOven);
        sleepDelay = (rand() % 1000 + 4000) * 1000;
        usleep(sleepDelay);

        sem.sem_op = -1;
        semop(semaphoreDescriptor, &sem, 1);
        oven->pizzas--;
        nInOven = oven->pizzas;
        sem.sem_op = 1;
        semop(semaphoreDescriptor, &sem, 1);

        sem.sem_num = TABLE_SEMAPHORE_ID;
        inserted = 0;
        int onTable = 0;
        while (!inserted) {
            sem.sem_op = -1;
            semop(semaphoreDescriptor, &sem, 1);
            onTable = 0;
            for (int i = 0; i < 5; i++) {
                if (table->places[i] == -1) {
                    table->places[i] = pizzaType;
                    inserted = 1;
                    break;
                }
            }
            for (int i = 0; i < 5; i++) {
                if (table->places[i] != -1) {
                    onTable++;
                }
            }
            sem.sem_op = 1;
            semop(semaphoreDescriptor, &sem, 1);
        }
        printf("(%d %lu) Wyjmuje pizze: %d. Liczba pizz w piecu: %d. Liczba pizz na stole: %d.\n",
               myPid, getTimestamp(), pizzaType, nInOven, onTable);
    }
    intHandler(-1);
}

void driverLoop() {
    printf("Driver starting.\n");
    signal(SIGINT, intHandler);
    pid_t myPid = getpid();
    int pizzaType = 0;
    while (keepWorking) {
        struct sembuf sem = {.sem_num = TABLE_SEMAPHORE_ID, .sem_op = -1, .sem_flg = 0};
        int taken = 0;
        int onTable = 0;
        while (!taken) {
            sem.sem_op = -1;
            semop(semaphoreDescriptor, &sem, 1);
            onTable = 0;
            for (int i = 0; i < 5; i++) {
                if (table->places[i] != -1) {
                    pizzaType = table->places[i];
                    table->places[i] = -1;
                    taken = 1;
                    break;
                }
            }
            for (int i = 0; i < 5; i++) {
                if (table->places[i] != -1) {
                    onTable++;
                }
            }
            sem.sem_op = 1;
            semop(semaphoreDescriptor, &sem, 1);
        }
        printf("(%d %lu) Pobieram pizze %d. Liczba pizz na stole: %d.\n", myPid, getTimestamp(), pizzaType, onTable);
        int sleepDelay = (rand() % 1000 + 4000) * 1000;
        usleep(sleepDelay);
        printf("(%d %lu) Dostarczam pizze: %d.\n", myPid, getTimestamp(), pizzaType);
        usleep(sleepDelay);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Not enough args.");
        return 0;
    }
    int nCooks = atoi(argv[1]);
    int nDrivers = atoi(argv[2]);
    semaphoreDescriptor = createSemaphores();
    memoryDescriptor = createSharedMemory();
    if (semaphoreDescriptor == -1) {
        printf("Unable to create semaphores");
        return 0;
    }
    if (memoryDescriptor == -1) {
        printf("Unable to create shared memory.");
        return 0;
    }

    oven = shmat(memoryDescriptor, NULL, 0);
    table = (Table*)((void*)oven + sizeof(PizzaContainer));
    oven->pizzas = 0;
    for (int i = 0; i < 5; i++) {
        table->places[i] = -1;
    }

    union semun opts;
    opts.val = 1;
    semctl(semaphoreDescriptor, OVEN_SEMAPHORE_ID, SETVAL, opts);
    semctl(semaphoreDescriptor, TABLE_SEMAPHORE_ID, SETVAL, opts);

    pid_t childPID = 0;
    for (size_t i = 0; i < nCooks; i++) {
        if ((childPID = fork()) == 0) {
            cookLoop();
            return 0;
        }
        else {
            children = realloc(children, sizeof(pid_t)*++childIndex);
            children[childIndex - 1] = childPID;
        }
    }

    for (size_t i = 0; i < nDrivers; i++) {
        if ((childPID = fork()) == 0) {
            driverLoop();
            return 0;
        }
        else {
            children = realloc(children, sizeof(pid_t)*++childIndex);
            children[childIndex - 1] = childPID;
        }
    }

    signal(SIGINT, houseClosing);

    for(size_t i = 0; i < childIndex; i++){
        waitpid(children[i], NULL, 0);
    }

    free(children);
    semctl(semaphoreSetKey, 0, IPC_RMID);
    shmdt(oven);
    shmctl(sharedMemoryKey, IPC_RMID, NULL);
    return 0;
}