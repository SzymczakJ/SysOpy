#include "sys/ipc.h"
#include <sys/msg.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <errno.h>


typedef enum{
    MESSAGE = 1,
    INIT
}messages;

int clients[5];
int clientCounter;

void init(char *buffer) {
    int clientDesc;
    sscanf(buffer, "%d", &clientDesc);
    clients[clientCounter] = clientDesc;
    clientCounter++;
}

void send(char *buffer) {
    struct {
        long mtype;
        char messageContent[1000];
    } message;
    message.mtype = MESSAGE;
//    sprintf(message.messageContent, "%d dupa\n", 123);
    strcpy(message.messageContent, buffer);
    message.messageContent[strlen(buffer)] = 0;
    printf("i send %s\n", message.messageContent);
    for (int i = 0; i < clientCounter; i++) {
        msgsnd(clients[i], &message, 1000, IPC_NOWAIT);
    }
}

int main() {
    clientCounter = 0;

    key_t serverKey = 3333;
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

    printf("Server starts.\n");
    struct{
        long mtype;
        char buffer[4097];
    } receivedMessageData;

    while (1) {
        printf("Listening for messages.\n");
        size_t charsRead = msgrcv(messsageDescriptor, &receivedMessageData, sizeof(receivedMessageData) - sizeof(long), -100, MSG_NOERROR);
        printf("Message received: %s\n", receivedMessageData.buffer);
        receivedMessageData.buffer[charsRead] = 0;

        switch (receivedMessageData.mtype) {
            case INIT:
                init(receivedMessageData.buffer);
                printf("%d\n", clients[clientCounter - 1]);
                send("dupa");
                break;
            case MESSAGE:
                send(receivedMessageData.buffer);
                break;
        }
    }
}