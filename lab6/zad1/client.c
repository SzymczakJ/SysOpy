#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <time.h>

int clientQueue;
int serverQueue;
int myId;

int inputAvailable(){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return (FD_ISSET(0,  &fds));
}

void sigintHandler(int signo){

    struct{
        long rType;
        char data[MSGMAX];
    } request;

    puts("Quitting.");
    request.rType = STOP;
    sprintf(request.data, "%d", myId);
    msgsnd(serverQueue, &request, MSGMAX, 0);
    msgctl(clientQueue, IPC_RMID, 0);

    exit(0);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
void clientLoop(int serverQueue, int myQueue, int myId) {
    struct {
        long mtype;
        char data[MSGMAX];
    } request;
    char *command = calloc(100, sizeof(char));
    size_t nRead = 100;
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    printf(">");
    fflush(stdout);
    while (1) {
        if (inputAvailable() > 0) {
            getline(&command, &nRead, stdin);
            if (strncmp(command, "LIST", 4) == 0) {
                request.mtype = LIST;
                sprintf(request.data, "%d", myId);
                msgsnd(serverQueue, &request, MSGMAX, 0);
                msgrcv(myQueue, &request, MSGMAX, RESPONSE, 0);
                printf("%s", request.data);
            } else if (strcmp(command, "STOP") == 0) {
                sigintHandler(SIGINT);
            }
            else if (strncmp(command, "2ALL", strlen("2ALL")) == 0) {
                request.mtype = MESSAGE2ALL;
                time_t t;
                time(&t);
                strtok(command, " ");
                char *content = strtok(NULL, "");
                sprintf(request.data, "client: %d\non %s writes:\n%s", myId, ctime(&t), content);
                msgsnd(serverQueue, &request, MSGMAX, 0);
            }
            else if (strncmp(command, "2ONE", strlen("2ONE")) == 0) {
                request.mtype = MESSAGE2ONE;
                time_t t;
                time(&t);
                strtok(command, " ");
                char *content = strtok(NULL, " ");
                int receiverId;
                sscanf(content, "%d", &receiverId);
                content = strtok(NULL, "");
                sprintf(request.data, "%d client: %d\non %s writes:\n%s", receiverId, myId, ctime(&t), content);
                msgsnd(serverQueue, &request, MSGMAX, 0);
            }
            else{
                puts("Available commands: LIST | STOP | 2ALL [ID] [content] | 2ONE [ID] [content]");
            }
            printf("\n>");
            fflush(stdout);
        }
        if (msgrcv(myQueue, &request, MSGMAX, -100, IPC_NOWAIT) > 0) {
            if (request.mtype == MESSAGE2ALL || request.mtype == MESSAGE2ONE) {
                printf("%s", request.data);
                fflush(stdout);
            } else if (request.mtype == SERVER_DOWN){
                printf("Server is shutting down.");
                sigintHandler(SIGINT);
            }
        }
    }
}
#pragma clang diagnostic pop



int main() {
    srand(time(NULL));

    int msgd = msgget(SERVER_IPC_KEY, 0);
    if (msgd == -1) {
        printf("Unable to open server queue");
        return 0;
    }
    serverQueue = msgd;

    char *homePath = getenv("HOME");
    key_t queueKey = ftok(homePath, rand() % 256);
    clientQueue = msgget(queueKey, IPC_CREAT | IPC_EXCL | 0666);
    if (clientQueue == -1) {
        printf("Unable to open client queue.");
        return 0;
    }
    printf("Client started with msgID %d.\n", clientQueue);
    signal(SIGINT, sigintHandler);

    struct {
        long mtype;
        char request[MSGMAX];
    } initRequest;
    initRequest.mtype = INIT;
    sprintf(initRequest.request, "%d", clientQueue);
    msgsnd(msgd, &initRequest, 128, IPC_NOWAIT);
    msgrcv(clientQueue, &initRequest, MSGMAX, RESPONSE, 0);
    sscanf(initRequest.request, "%d", &myId);
    clientLoop(msgd, clientQueue, myId);

    msgctl(clientQueue, IPC_RMID, 0);
    return 0;
}

#pragma clang diagnostic pop