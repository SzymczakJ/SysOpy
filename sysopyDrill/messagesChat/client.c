#include "sys/msg.h"
#include "sys/ipc.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "pthread.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"


typedef enum{
    MESSAGE = 1,
    INIT
}messages;

typedef struct {
    int clientQueue;
}threadInfo;

void *listenMessages(void *data) {
    int *info = data;
    int clientQueue = *info;
    printf("my descriptor %d\n", clientQueue);
    struct {
        long mtype;
        char someText[4000];
    }message;
    while (1) {
        msgrcv(clientQueue, &message, sizeof(message), -100, MSG_NOERROR);
        printf("dostalem wiadomosc %s\n", message.someText);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    int serverDesc = msgget(3333, 0);
    printf("server desc %d", serverDesc);
    if (serverDesc == -1) {
        printf("Unable to open serva\n");
        return 0;
    }

    char *homePath = getenv("HOME");
    key_t queueKey = ftok(homePath, rand() % 256);
    int clientQueue = msgget(queueKey, IPC_CREAT | IPC_EXCL | 0666);
    if (clientQueue == -1) {
        printf("Unable to open client queue.");
        return 0;
    }
    printf("Client started with msgID %d.\n", clientQueue);

    pthread_t listening_thread;
    pthread_create(&listening_thread, NULL, listenMessages, &clientQueue);


    struct {
        long mtype;
        char someText[4000];
    }message;
    message.mtype = INIT;
    sprintf(message.someText, "%d", clientQueue);
    msgsnd(serverDesc, &message, 128, IPC_NOWAIT);

    message.mtype = MESSAGE;
    while (1) {
        printf("giv messg:");
        scanf("%s", message.someText);
        msgsnd(serverDesc, &message, 4200, IPC_NOWAIT);
    }
}

#pragma clang diagnostic pop