#include "stdlib.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "string.h"

int main(int argc, char **argv) {
    int n = strtol(argv[1], NULL, 10);
    int motherPid = getpid();
    printf("PID procesu macierzystego: %d\n", getpid());
    for (int i = 0; i < n; i++) {
        fork();
        if (getpid() != motherPid) break;
    }
    printf("Ten napis pochodzi z procesu o PID: %d\n", getpid());
}