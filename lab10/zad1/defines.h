//
// Created by kubson on 31.05.2022.
//

#ifndef SYSOPY_DEFINES_H
#define SYSOPY_DEFINES_H

#define BUF_SIZE 120
#include <pthread.h>
#include <stdio.h>
#include <sys/un.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

int getTimeMs() {
    struct timespec spec;
    clock_gettime(1, &spec);
    return spec.tv_sec * 1000 + spec.tv_nsec / 1e6;
}

#endif //SYSOPY_DEFINES_H
