#include "defines.h"
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/select.h>
#include <string.h>
#include <time.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
mqd_t myQueueDesc;
int myID;
mqd_t serverQueueDesc;
mqd_t anotherQ;
char conBuff[100];
char myQueueName[200];
char messageBuffer[MSG_MAX+1];
int inChatMode = -1;

mqd_t createMyQueue(){
    pid_t myPid = getpid();
    sprintf(myQueueName, "/clientQueue_%d", myPid);

    struct mq_attr mqAttr;
    mqAttr.mq_flags = O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL;
    mqAttr.mq_maxmsg = 10;
    mqAttr.mq_msgsize = MSG_MAX;
    mqAttr.mq_curmsgs = 0;

    mqd_t queueDesc = mq_open(myQueueName, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL, 0666, &mqAttr);
    if(queueDesc == -1){
        puts(strerror(errno));
        puts("Unable to open client's queue. Perhaps it exists - trying to delete it");
        mq_unlink(myQueueName);
        queueDesc = mq_open(myQueueName, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL, 0666, &mqAttr);
        if(queueDesc == -1){
            puts("Failed to open the queue after removing it.");
            return -1;
        }
    }
    return queueDesc;
}

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

void intHandler(int signo){


    puts("Quitting");
    sprintf(messageBuffer, "%d", myID);
    mq_send(serverQueueDesc, messageBuffer, MSG_MAX, STOP);
    mq_close(serverQueueDesc);
    mq_close(myQueueDesc);
    mq_unlink(myQueueName);
    exit(0);
}

void clientLoop(){
    char * inpB = NULL;
    size_t getlineN = 100;
    char *command = calloc(100, sizeof(char));
    size_t nRead = 0;
    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    printf(">");
    fflush(stdout);
    while(1){
        if(inputAvailable() > 0){
            getline(&command, &getlineN, stdin);
            if(strncmp(command, "LIST", 4) == 0){
                sprintf(messageBuffer, "%d", myID);
                mq_send(serverQueueDesc, messageBuffer, MSG_MAX, LIST);
                do{
                    errno = 0;
                    nRead = mq_receive(myQueueDesc, messageBuffer, MSG_MAX, NULL);
                }while(errno == EAGAIN || nRead == 0);
                puts(messageBuffer);
            }
            else if (strncmp(command, "2ALL", strlen("2ALL")) == 0) {
                char responseBuffer[MSG_MAX];
                time_t t;
                time(&t);
                strtok(command, " ");
                char *content = strtok(NULL, "");
                sprintf(responseBuffer, "client: %d\non %s writes:\n%s", myID, ctime(&t), content);
                mq_send(serverQueueDesc, responseBuffer, MSG_MAX, MESSAGE2ALL);
            }
            else if (strncmp(command, "2ONE", strlen("2ONE")) == 0) {
                time_t t;
                time(&t);
                strtok(command, " ");
                char *content = strtok(NULL, " ");
                int receiverId;
                sscanf(content, "%d", &receiverId);
                content = strtok(NULL, "");
                char requestContent[MSG_MAX];
                sprintf(requestContent, "%d client: %d\non %s writes:\n%s", receiverId, myID, ctime(&t), content);
                mq_send(serverQueueDesc, requestContent, MSG_MAX, MESSAGE2ONE);
            }
            else if(strncmp(command, "STOP", 4) == 0){
                intHandler(SIGINT);
            }
            else{
                puts("Available commands: LIST | STOP | 2ALL [ID] [content] | 2ONE [ID] [content]");
            }
            printf("\n>");
            fflush(stdout);
        }
        unsigned prio;
        errno = 0;
        nRead = mq_receive(myQueueDesc, messageBuffer, MSG_MAX, &prio);
        if(errno == 0 && nRead > 0){
            if(prio == SERVER_DOWN){
                puts("Server is shutting down...");
                intHandler(SIGINT);
            }
            else if (prio == MESSAGE2ALL || prio == MESSAGE2ONE) {
                printf("%s", messageBuffer);
            }
        }
    }
}

int main(int argc, char ** argv){
    messageBuffer[MSG_MAX] = 0;
    messageBuffer[0] = 0;

    serverQueueDesc = mq_open(serverQueueName, O_WRONLY);
    if(serverQueueDesc < 0){
        puts("Unable to upen server's queue.");
        return 0;
    }

    if((myQueueDesc = createMyQueue()) == -1){
        return 0;
    }
    printf("Client started with queue name %s\n", myQueueName);

    signal(SIGINT, intHandler);
    sprintf(messageBuffer, "%s", myQueueName);
    mq_send(serverQueueDesc, messageBuffer, MSG_MAX, INIT);
    puts("Waiting for server's response");
    ssize_t nRecv = 0;
    do{
        errno = 0;
        nRecv = mq_receive(myQueueDesc, messageBuffer, MSG_MAX, NULL);
    }while(errno == EAGAIN || nRecv == 0);
    sscanf(messageBuffer, "%d", &myID);
    printf("Server responded. My ID is: %d\n", myID);

    clientLoop();

    return 0;
}
#pragma clang diagnostic pop