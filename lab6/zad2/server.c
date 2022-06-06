
#include <errno.h>
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "defines.h"

mqd_t getQueueDescriptor(){
    struct mq_attr mqAttr;
    mqAttr.mq_flags = O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL;
    mqAttr.mq_maxmsg = 10;
    mqAttr.mq_msgsize = MSG_MAX;
    mqAttr.mq_curmsgs = 0;

    //create the server's queue
    errno = 0;
    mqd_t queueDesc = mq_open(serverQueueName, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL, 0666, &mqAttr);
    if(queueDesc == -1){
        puts("Unable to open the server's queue. Perhaps it already exists - trying to delete it");
        puts(strerror(errno));
        mq_unlink(serverQueueName);
        queueDesc = mq_open(serverQueueName, O_RDONLY | O_NONBLOCK | O_CREAT | O_EXCL, 0666, &mqAttr);
        if(queueDesc == -1){
            puts("Failed to open the queue even after unlinking it.");
            exit(0);
        }
    }
    return queueDesc;
}

int running = 1;

void intHandler(int signo){
    running = 0;
}

void sendMessagesToAll(clientList *clients, char *buffer) {
    char *messageContent = calloc(sizeof(char), strlen(buffer));
    strcpy(messageContent, buffer);
    strtok(buffer, " ");
    char *sequence = strtok(NULL, " \n");
    int clientId;
    sscanf(sequence, "%d", &clientId);
    char responseBuffer[MSG_MAX+1];
    responseBuffer[MSG_MAX] = 0;
    responseBuffer[0] = 0;
    clientList *clCp = clients;
    while (clCp != NULL) {
        mq_send(clCp->queueID, messageContent, MSG_MAX, MESSAGE2ALL);
        clCp = clCp->next;
    }
}

void sendMessageToOne(clientList *clients, char *buffer) {
    int receiverId;
    char *sequence = strtok(buffer, " ");
    sscanf(sequence, "%d", &receiverId);
    sequence = strtok(NULL, "");
    while (clients != NULL) {
        if (clients->clientID == receiverId) {
            mq_send(clients->queueID, sequence, MSG_MAX, MESSAGE2ONE);
            break;
        }
        clients = clients->next;
    }

}

void handleInit(clientList ** clients, char * buffer, int * nextClientID){
    //get client's queue name
    char clientQName[71];
    sscanf(buffer, "%70s", clientQName);
    mqd_t queueID = mq_open(clientQName, O_WRONLY | O_NONBLOCK);
    if(queueID < 0){
        puts("Init failed - unable to open queue.");
        return;
    }

    clientListInsert(clients, *nextClientID, queueID, clientQName);
    char sndBuff[MSG_MAX+1];
    sndBuff[MSG_MAX] = 0;
    sprintf(sndBuff, "%d\n", *nextClientID);
    mq_send(queueID, sndBuff, MSG_MAX, RESPONSE);
    (*nextClientID)++;
    printf("INIT handled (%s) -> %d\n", clientQName, *nextClientID-1);
}

void handleList(clientList ** clients, char * buffer){
    int clientID;
    char responseBuffer[MSG_MAX];
    responseBuffer[0] = 0;
    responseBuffer[MSG_MAX - 1] = 0;
    sscanf(buffer, "%d", &clientID);
    clientList * clientData = clientListFind(*clients, clientID);
    if(!clientData)
        return;

    clientList * clCp = *clients;
    while(clCp != NULL){
        ssize_t oLen = strlen(responseBuffer);
        if(oLen > MSG_MAX - 120)
            break;
        sprintf(responseBuffer + oLen, "client: %d\n", clCp->clientID);
        clCp = clCp->next;
    }
    mq_send(clientData->queueID, responseBuffer, MSG_MAX, RESPONSE);
}

void handleStop(clientList ** clients, char * buffer){
    int clID;
    sscanf(buffer, "%d", &clID);
    clientList * cl = clientListFind(*clients, clID);
    if(!cl)
        return;

    mq_close(cl->queueID);
    clientListRemove(clients, clID);
}

void serverLoop(mqd_t queueDesc){
    messages messageType;

    char buffer[MSG_MAX+1];
    buffer[MSG_MAX] = 0;

    int nextClientID = 0;

    clientList * clients = NULL;

    while(running){
        if(mq_receive(queueDesc, buffer, MSG_MAX, &messageType) == -1){
            continue;
        }
        switch(messageType){
            case STOP:
                puts("Handling STOP");
                handleStop(&clients, buffer);
                break;
            case LIST:
                puts("Handling LIST");
                handleList(&clients, buffer);
                break;
            case INIT:
                puts("HANDLING INIT");
                handleInit(&clients, buffer, &nextClientID);
                break;
            case MESSAGE2ALL:
                puts("Handling 2ALL");
                sendMessagesToAll(clients, buffer);
                break;
            case MESSAGE2ONE:
                puts("Handling 2ONE");
                sendMessageToOne(clients, buffer);
                break;
            default:
                puts("Undefined request");
                break;
        }
    }

    clientList * tempC = clients;
    char nullMessage[MSG_MAX];
    memset(nullMessage, 0, MSG_MAX);
    while(tempC != NULL){
        mq_send(tempC->queueID, nullMessage, MSG_MAX, SERVER_DOWN);
        mq_close(tempC->queueID);
        tempC = tempC->next;
    }


    clientListFree(clients);
    mq_close(queueDesc);
    mq_unlink(serverQueueName);
}

int main(int argc, char ** argv){
    mqd_t queueDesc = getQueueDescriptor();
    signal(SIGINT, intHandler);
    puts("Server up and running");
    serverLoop(queueDesc);

    return 0;
}