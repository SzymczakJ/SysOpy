#include "stdlib.h"
#include "stdio.h"
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>


int main(int argc, char **argv) {
    struct sockaddr_in webSocketStruct;
    struct sockaddr_un localSocketStruct;
    int conFd;
    short port = 21374;
    char *addr = "127.0.0.1";
    char *socketPath = "/tmp/foo";

    if (strcmp(argv[1], "WEB") == 0) {
        printf("wchodze w weba\n");
        if ((conFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            printf("socka problem\n");
            return 0;
        }
        struct in_addr inAddr;
        if (inet_pton(AF_INET, addr, &inAddr) == 0) {
            printf("wrong network address\n");
            return 0;
        }
        webSocketStruct.sin_port = htons(port);
        webSocketStruct.sin_addr = inAddr;
        webSocketStruct.sin_family = AF_INET;
        if (connect(conFd, (struct sockaddr*)&webSocketStruct, sizeof(webSocketStruct)) == -1 ) {
            printf("connect problem");
            return 0;
        }
    } else {
        if ((conFd = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) {
            printf("socka problem\n");
            return 0;
        }

        localSocketStruct.sun_family = AF_LOCAL;
        strcpy(localSocketStruct.sun_path, socketPath);
        if (connect(conFd, (struct sockaddr*)&localSocketStruct, sizeof(localSocketStruct)) == -1 ) {
            printf("connect problem");
            return 0;
        }
    }
    write(conFd, "cho na solo", strlen("cho na solo") + 1);
    char buffer[100];
    int nread;
    while(1) {
        nread = read(conFd, buffer, 100);
        if (nread < 1) break;
        printf("I gor some mean mesg%s\n", buffer);
    }
    printf("koncze prace");

    return 0;
}
