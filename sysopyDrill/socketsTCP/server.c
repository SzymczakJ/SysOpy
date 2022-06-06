#include "stdio.h"
#include "stdlib.h"
#include <netinet/in.h>
#include <sys/un.h>
#include "unistd.h"
#include <pthread.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int webSocket;
int localSocket;

typedef struct {
    int cliDesc;
    char *meanMessage;
}roastInfo;

void *sendRoastToClient(void *args) {
    roastInfo *info = args;

    for (int i = 0; i < 20; i++) {
        usleep(1000 * 1000);
        printf("i send to %d\n", info->cliDesc);
        send(info->cliDesc, info->meanMessage, strlen(info->meanMessage) + 1, MSG_DONTWAIT);
    }

    return NULL;
}

void unbind(void){
    close(webSocket);
    close(localSocket);
}

int main() {
    short port = 21374;
    char *socketPath = "/tmp/foo";
    struct sockaddr_in webSocketStruct;
    struct sockaddr_un localSocketStruct;

    //init websocketa
    if ((webSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        printf("socket dup\n");
        return 0;
    }
    webSocketStruct.sin_family = AF_INET;
    webSocketStruct.sin_port = htons(port);
    webSocketStruct.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(webSocket, (struct sockaddr*)&webSocketStruct, sizeof(webSocketStruct)) == -1) {
        printf("bind dup\n");
        return 0;
    }

    //init localsocketa
    if ((localSocket = socket(AF_LOCAL, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1) {
        printf("socket dup\n");
        return 0;
    }
    unlink(socketPath);
    strcpy(localSocketStruct.sun_path, socketPath);
    localSocketStruct.sun_family = AF_LOCAL;
    if (bind(localSocket, (struct sockaddr*)&localSocketStruct, sizeof(localSocketStruct)) == -1) {
        printf("bind dup\n");
        return 0;
    }

    pthread_t threads[5];
    int threadCounter = 0;

    listen(webSocket, 12);
    listen(localSocket, 12);
    while (threadCounter < 5) {
        int cliDesc = -1;
        cliDesc = accept(webSocket, NULL, NULL);
        if (cliDesc == -1) {
            cliDesc = accept(localSocket, NULL, NULL);
        }
        if (cliDesc != -1) {
            printf("got sth from client\n");
            char message[100];
            int nread = read(cliDesc, message, 100);
            message[nread] = 0;
            printf("got some mean message: %s\n", message);
            roastInfo *args = malloc(sizeof(roastInfo));
            args->cliDesc = cliDesc;
            args->meanMessage = "u scum";
            pthread_create(&threads[threadCounter], NULL, sendRoastToClient, args);
            threadCounter++;
        }
    }

    atexit(unbind);

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
#pragma clang diagnostic pop