#ifndef SYSOPY_DEFINES_H
#define SYSOPY_DEFINES_H
#include <stdlib.h>
#include <string.h>
#define MSG_MAX 4096
const char serverQueueName [] = "/posixServerQueue";

typedef enum{
    SERVER_DOWN,
    MESSAGE,
    RESPONSE,
    INIT,
    LIST,
    MESSAGE2ALL,
    MESSAGE2ONE,
    STOP
}messages;


typedef struct client__list{
    struct client__list * next;
    char * queueName;
    int clientID;
    int queueID;
}clientList;

void clientListFree(clientList * list){
    if(list != NULL){
        clientList * next = list->next;
        free(list->queueName);
        free(list);
        clientListFree(next);
    }
}

void clientListInsert(clientList ** list, int ID, int queueID, char * queueName){
    clientList * nl = calloc(1, sizeof(clientList));
    nl->next = *list;
    *list = nl;
    nl->queueID = queueID;
    nl->clientID = ID;
    nl->queueName = malloc(sizeof(char)*(strlen(queueName)+1));
    nl->queueName[strlen(queueName)] = 0;
    strcpy(nl->queueName, queueName);
}

clientList * clientListFind(clientList * list, int ID){
    if(list == NULL)
        return list;
    if(ID == list->clientID)
        return list;
    return clientListFind(list->next, ID);
}

void clientListRemove(clientList ** list, int ID){
    clientList * hd = *list;
    clientList * prev = NULL;
    while(hd != NULL){
        if (hd->clientID == ID){
            free(hd->queueName);
            if(hd == *list){
                *list = hd->next;
                free(hd);
            }
            else{
                prev->next = hd->next;
                free(hd);
            }
            return;
        }
        prev = hd;
        hd = hd->next;
    }
}
#endif //SYSOPY_DEFINES_H
