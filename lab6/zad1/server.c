#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <signal.h>
#include "server.h"
struct msgData {
    long mtype;
    char buffer[MSGMAX+1];
};

int serverRunning = 1;

int openServer() {
    key_t serverKey = SERVER_IPC_KEY;
    msgctl(serverKey, IPC_RMID, NULL); //kasuję kolejkę która mogłaby posiadać taki numer
    int messsageDescriptor = msgget(serverKey, IPC_CREAT | IPC_EXCL); //wyrzuca błąd jesli już jest taka kolejka
    if(messsageDescriptor == -1){
        int stillFailed = 0;
        if(errno == EEXIST){
            messsageDescriptor = msgget(serverKey, 0);
            if(msgctl(messsageDescriptor, IPC_RMID, 0) == -1)
                stillFailed = 1;
            else
                messsageDescriptor = msgget(serverKey, IPC_CREAT | IPC_EXCL | 0666);
        }
        if(stillFailed)
            return -1;
    }
    return messsageDescriptor;
}

void stopClient(clientList **clients, char *buffer) {
    int clientId;
    sscanf(buffer, "%d", &clientId);
    clientListRemove(clients, clientId);
}

void listClients(int serverKey, clientList *clients, char *buffer) {
    int clientId;
    sscanf(buffer, "%d", &clientId);
    clientList *clientData = clientListFind(clients, clientId);
    if (!clientData)
        return;
    struct {
        long mtype;
        char output[MSGMAX];
    } response;
    response.mtype = RESPONSE;
    response.output[MSGMAX - 1] = 0;
    response.output[0] = 0;
    while (clients != NULL) {
        size_t len = strlen(response.output);
        if (len > MSGMAX)
            break;
        sprintf(response.output + len, "client: %d\n", clients->clientID);
        clients = clients->next;
    }
    msgsnd(clientData->queueID, &response, MSGMAX, IPC_NOWAIT);
}

void connectClient(int* nextClientId, clientList **clients, char *inputBuffer) {
    int clientQueue;
    sscanf(inputBuffer, "%d", &clientQueue);
    clientListAppend(clients, *nextClientId, clientQueue);
    struct {
        long mtype;
        char buffer[60];
    }messageToClient;
    messageToClient.mtype = RESPONSE;
    sprintf(messageToClient.buffer, "%d\n", *nextClientId);
    printf("Initializing client. Received queue id: %d\n", clientQueue);
    msgsnd(clientQueue, &messageToClient, 80, IPC_NOWAIT);
    (*nextClientId)++;
}

void sendMessageToAll(clientList *clients, char *buffer) {
    char *messageContent = calloc(sizeof(char), strlen(buffer));
    strcpy(messageContent, buffer);
    strtok(buffer, " ");
    char *sequence = strtok(NULL, " \n");
    int clientId;
    sscanf(sequence, "%d", &clientId);
    struct {
        long mtype;
        char messageContent[MSGMAX];
    } message;
    message.mtype = MESSAGE2ALL;
    strcpy(message.messageContent, messageContent);
    message.messageContent[strlen(messageContent)] = 0;
    while (clients != NULL) {
        if (clients->clientID != clientId) msgsnd(clients->queueID, &message, MSGMAX, IPC_NOWAIT);
        clients = clients->next;
    }
}

void sendMessageToOne(clientList *clients, char *buffer) {
    int receiverId;
    char *sequence = strtok(buffer, " ");
    sscanf(sequence, "%d", &receiverId);
    sequence = strtok(NULL, "");
    struct {
        long mtype;
        char messageContent[MSGMAX];
    } message;
    message.mtype = MESSAGE2ONE;
    strcpy(message.messageContent, sequence);
    message.messageContent[strlen(sequence)] = 0;
    while (clients != NULL) {
        if (clients->clientID == receiverId) {
            msgsnd(clients->queueID, &message, MSGMAX, IPC_NOWAIT);
            break;
        }
        clients = clients->next;

    }
}

void serverLoop(int serverKey) {
    printf("Server starts.\n");
    int nextClientId = 0;
    clientList *clients = NULL;
    struct{
        long mtype;
        char buffer[MSGMAX+1];
    } receivedMessageData;
    while (serverRunning) {
        printf("Listening for messages.\n");
        size_t charsRead = msgrcv(serverKey, &receivedMessageData, sizeof(receivedMessageData) - sizeof(long), -100, MSG_NOERROR);
        printf("Message received.\n");
        receivedMessageData.buffer[charsRead] = '\0';

        switch(receivedMessageData.mtype) {
            case STOP:
                printf("Stopping client.\n");
                stopClient(&clients, receivedMessageData.buffer);
                break;
            case LIST:
                printf("Listing clients.\n");
                listClients(serverKey, clients, receivedMessageData.buffer);
                break;
            case INIT:
                printf("Initializing client.\n");
                connectClient(&nextClientId, &clients, receivedMessageData.buffer);
                break;
            case MESSAGE2ALL:
                printf("Message sent to all.");
                sendMessageToAll(clients, receivedMessageData.buffer);
                break;
            case MESSAGE2ONE:
                printf("Message to one.");
                sendMessageToOne(clients, receivedMessageData.buffer);
                break;
        }
    }
}

int main() {
    int serverKey = openServer();

    if (serverKey != -1) {
        serverLoop(serverKey);
    }
    else {
        printf("Server start failed.");
    }
    return 0;

}