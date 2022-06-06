#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "unistd.h"
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>

const key_t semaphoreSetKey = 0xabcd0101;
const key_t sharedMemoryKey = 0xabcd1010;
const size_t CONTAINER_SEMAPHORE = 0;

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
};

typedef struct {
    int places[5];
}Container;

Container *container;

void giver(int semDesc) {
    printf("Giver starts.\n");
    pid_t myPid = getpid();
    while (1) {
        usleep((rand() % 2000 + 3000) * 1000);
        int itemType = rand() % 10;
        printf("%d zara włoze item o numerze %d", myPid, itemType);
        struct sembuf sem = {
                .sem_num = CONTAINER_SEMAPHORE,
                .sem_op = -1,
                .sem_flg = 0
        };
        int inserted = 0;
        while (!inserted) {
            sem.sem_op = -1;
            semop(semDesc, &sem, 1);
            for (int i = 0; i < 5; i++) {
                if (container->places[i] == -1) {
                    container->places[i] = itemType;
                    printf("%d wkladam item %d na miejsce %d \n", myPid, itemType, i);
                    inserted = 1;
                    break;
                }
            }
            sem.sem_op = 1;
            semop(semDesc, &sem, 1);
        }
    }
}

void taker(int semDesc) {
    printf("taker starts\n");
    pid_t myPID = getpid();
    while(1) {
        usleep((rand() % 2000 + 3000) * 1000);
        struct sembuf sem = {
                .sem_num = CONTAINER_SEMAPHORE,
                .sem_op = -1,
                .sem_flg = 0
        };
        int taken = 0;
        while (!taken) {
            sem.sem_op = -1;
            semop(semDesc, &sem, 1);
            printf("%d aktualny stan: [%d %d %d %d %d]\n", myPID, container->places[0], container->places[1], container->places[2], container->places[3], container->places[4]);
            for (int i = 0; i < 5; i++) {
                if (container->places[i] != -1) {
                    printf("%d wyciagam item %d\n", myPID, container->places[i]);
                    container->places[i] = -1;
                    taken = 1;
                    break;
                }
            }
            sem.sem_op = 1;
            semop(semDesc, &sem, 1);
        }

    }

}

int main() {
    srand(time(NULL));
    int semDesc = semget(semaphoreSetKey, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semDesc == -1) {
        semDesc = semget(semaphoreSetKey, 1, 0666);
        if (semaphoreSetKey != -1) {
            semctl(semDesc, 0, IPC_RMID);
            semDesc = semget(semaphoreSetKey, 1, IPC_CREAT | IPC_EXCL | 0666);
        } else return 0;
    }
    int memDesc = shmget(sharedMemoryKey, sizeof(Container), IPC_CREAT | IPC_EXCL | 0666);
    if (memDesc == -1) {
        memDesc = shmget(sharedMemoryKey, 0, 0666);
        if (memDesc != -1) {
            shmctl(memDesc, IPC_RMID, NULL);
            memDesc = shmget(memDesc, sizeof(Container), IPC_CREAT | IPC_EXCL | 0666);
        } else return 0;
    }
    container = shmat(memDesc, NULL, 0);
    for (int i = 0; i < 5; i++) {
        container->places[i] = -1;
    }

    union semun opts;
    opts.val = 1;
    semctl(semDesc, CONTAINER_SEMAPHORE, SETVAL, opts);

    pid_t childPID;
    pid_t childPIDs[10];
    for (int i = 0; i < 5; i++) {
        if ((childPID = fork() == 0)) {
            giver(semDesc);
            return 0;
        } else {
            childPIDs[i] = childPID;
        }
    }

    for (int i = 0; i < 5; i++) {
        if ((childPID = fork()) == 0) {
            taker(semDesc);
            return 0;
        }
        else {
            childPIDs[i + 5] = childPID;
        }
    }

    for (int i = 0; i < 10; i++) {
        waitpid(childPIDs[i], NULL, 0);
    }
    semctl(semaphoreSetKey, 0, IPC_RMID);
    shmdt(container);
    shmctl(sharedMemoryKey, IPC_RMID, NULL);
    return 0;
}