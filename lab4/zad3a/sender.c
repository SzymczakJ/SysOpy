#include "stdio.h"
#include "stdlib.h"
#include <sys/types.h>
#include <signal.h>
#include "string.h"

int responseCount, sig2Received;
int recvData;

typedef enum {
    KILL,
    SIGQUEUE,
    SIGRT
}SendingMode;

void nullHandler(int signalNUmber) {}

void setSignals(void (*f)(int)) {
    signal(SIGUSR1, f);
    signal(SIGUSR2, f);
    signal(SIGRTMIN, f);
    signal(SIGRTMIN + 1, f);
}

void setSignalsQueued(void (*f)(int, siginfo_t *, void *)) {
    struct sigaction sa;
    sa.sa_sigaction = f;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGUSR1);
    sigaddset(&sa.sa_mask, SIGUSR2);
    sigaddset(&sa.sa_mask, SIGRTMIN);
    sigaddset(&sa.sa_mask, SIGRTMIN + 1);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGRTMIN, &sa, NULL);
    sigaction(SIGRTMIN + 1, &sa, NULL);
}

void responseHandler(int sigNumber) {
    if (sigNumber == SIGUSR1 || sigNumber == SIGRTMIN) responseCount++;
    if (sigNumber == SIGUSR2 || sigNumber == SIGRTMIN + 1) sig2Received = 1;
}

void queueResponseHandler(int signalNumber, siginfo_t *info, void *ucontext) {
    if (signalNumber == SIGUSR1 || signalNumber == SIGRTMIN)
        recvData = info->si_value.sival_int;
    responseHandler(signalNumber);
}

void waitForSIGUSR(int usr1, int usr2) {
    sigset_t usr2Set;
    sigfillset(&usr2Set);
    if (usr1) {
        sigdelset(&usr2Set, SIGUSR1);
        sigdelset(&usr2Set, SIGRTMIN);
    }
    if (usr2) {
        sigdelset(&usr2Set, SIGUSR2);
        sigdelset(&usr2Set, SIGRTMIN + 1);
    }
    sigdelset(&usr2Set, SIGINT);
    sigsuspend(&usr2Set);
}

void send(int pid, int signalNumber, SendingMode mode, int data) {
    switch (mode) {
        case KILL:
            kill(pid, signalNumber);
            break;
        case SIGQUEUE:
            sigqueue(pid, signalNumber, (const union sigval){.sival_int=data});
            break;
        case SIGRT:
            if (signalNumber == SIGUSR1) kill(pid, SIGRTMIN);
            else if (signalNumber == SIGUSR2) kill(pid, SIGRTMIN + 1);
            break;
    }
}

int main(int argc, char **argv) {
    pid_t catcherPid = atoi(argv[1]);
    int numberOfSignals = atoi(argv[2]);

    SendingMode mode;
    if (strcmp(argv[3], "KILL") == 0) mode = KILL;
    else if (strcmp(argv[3], "SIGQUEUE") == 0) mode = SIGQUEUE;
    else if (strcmp(argv[3], "SIGRT") == 0) mode = SIGRT;

    setSignals(nullHandler);
    sigqueue(catcherPid, SIGUSR2, (const union sigval){.sival_int=mode});
    waitForSIGUSR(0, 1);
    puts("Catcher notified of sender presence.");

    for (int i = 0; i <numberOfSignals; i++) {
        send(catcherPid, SIGUSR1, mode, 0);
    }

    puts("End of transmission.");
    send(catcherPid, SIGUSR2, mode, 0);

    responseCount = 0;
    sig2Received = 0;

    if (mode == SIGQUEUE)
        setSignalsQueued(queueResponseHandler);
    else
        setSignals(responseHandler);

    while (!sig2Received) {
        waitForSIGUSR(1, 1);
    }
    printf("Received %d signals.\n", responseCount);
    if (mode == SIGQUEUE) printf("Catcher received %d signals.\n", numberOfSignals);
    printf("Total number of signals sent: %d.\n", numberOfSignals);

    return 0;
}