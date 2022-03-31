#include "stdio.h"
#include "stdlib.h"
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>

typedef enum {
    IGNORE,
    HANDLER,
    MASK,
    PENDING
}mode;

void handleSignal();

void pendingSignal() {
    sigset_t pendingSet;
    sigpending(&pendingSet);
    if (sigismember(&pendingSet, SIGUSR1)) puts("SIGUSR1 is pending");
    else puts("No signal");
}

void handleSignal() {
    printf("signal was captured and handled\n");
}

int main(int argc, char **argv) {
    mode opMode;

    if (strcmp(argv[1], "ignore") == 0) { opMode = IGNORE; }
    else if (strcmp(argv[1], "handler") == 0) { opMode = HANDLER; }
    else if (strcmp(argv[1], "mask") == 0) { opMode = MASK; }
    else if (strcmp(argv[1], "pending") == 0) { opMode = PENDING; }

    int runningAsExecChild = 0;
    if (argc == 3 && strcmp(argv[2], "CHILD_EXEC") == 0) {
        runningAsExecChild = 1;
        puts("Running as executed child");
//        signal(SIGUSR1, handleSignal);
    } else {
        switch (opMode) {
            case IGNORE:
                signal(SIGUSR1, SIG_IGN);
                break;
            case HANDLER:
                signal(SIGUSR1, handleSignal);
                break;
            case MASK:
            case PENDING: {
                sigset_t blockingSet;
                sigemptyset(&blockingSet);
                sigaddset(&blockingSet, SIGUSR1);
                sigprocmask(SIG_BLOCK, &blockingSet, NULL);
                }
                break;
        }
    }

    if (!(runningAsExecChild && opMode == PENDING)) raise(SIGUSR1);

    if (opMode == PENDING) pendingSignal();

    if (runningAsExecChild) {
        puts("Child process ending without interruption");
        return 0;
    }

    pid_t childPid;
    puts("Starting child with fork");
    if ((childPid = fork()) == 0) {
        if (opMode != PENDING) {
            raise(SIGUSR1);
        } else {
            pendingSignal();
        }
        puts("Child process ending without interruption");
        return 0;
    }

    wait(NULL);
    puts("Forked child finished execution.");
    puts("Starting child with exec");
    if((childPid = fork()) == 0) {
        char **newArg = calloc(sizeof(char*), 4);
        for (int i = 0; i < 2; i++) {
            newArg[i] = calloc(sizeof(char), strlen(argv[i]));
            strcpy(newArg[i], argv[i]);
        }
        newArg[2] = "CHILD_EXEC";
        newArg[3] = NULL;
        execv(argv[0], newArg);
        return 0;
    }

    wait(NULL);
    puts("Executed child finished execution.");

    return 0;
}


