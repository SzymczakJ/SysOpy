#ifndef SYSOPY_SERVER_H
#define SYSOPY_SERVER_H
#define SERVER_IPC_KEY 3333
#define MSGMAX 4096
#include <stdlib.h>
#include <string.h>

typedef enum{
    STOP = 1,
    LIST,
    INIT,
    MESSAGE2ALL,
    MESSAGE2ONE,
    RESPONSE,
    MESSAGE,
    SERVER_DOWN
}messages;

typedef struct client__list{
    struct client__list * next;
    int clientID;
    int queueID;
}clientList;

void clientListAppend(clientList **list, int Id, int queueId) {
    clientList *client = calloc(1, sizeof(clientList));
    client->next = *list;
    *list = client;
    client->queueID = queueId;
    client->clientID = Id;
};

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
#endif