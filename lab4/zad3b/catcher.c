#include "stdio.h"
#include "stdlib.h"
#include <sys/types.h>
#include <signal.h>
#include "string.h"
#include <unistd.h>

typedef enum{
    KILL,
    SIGQUEUE,
    SIGRT
}SendingMode;

SendingMode mode;
pid_t senderPid;

void receive(int signalNumber, siginfo_t *info, void *ucontext) {
    mode = info->si_value.sival_int;
    senderPid = info->si_pid;
}

int numberOfReceived;
int receivedUSR2;

void recvHandler(int signalNumber) {
    if (signalNumber == SIGUSR1 || signalNumber == SIGRTMIN) numberOfReceived++;
    else if (signalNumber == SIGUSR2 || signalNumber == SIGRTMIN + 1) receivedUSR2 = 1;
}

void blockSIGUSRs(){
    sigset_t blockSet;
    sigaddset(&blockSet, SIGUSR2);
    sigaddset(&blockSet, SIGUSR1);
    sigaddset(&blockSet, SIGRTMIN);
    sigaddset(&blockSet, SIGRTMIN+1);
    sigprocmask(SIG_BLOCK, &blockSet, NULL);
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

void setSignals(void (*f)(int)) {
    signal(SIGUSR1, f);
    signal(SIGUSR2, f);
    signal(SIGRTMIN, f);
    signal(SIGRTMIN + 1, f);
}

int main(int argc, char **argv) {
    pid_t myPid = getpid();
    printf("Catcher pid is %d\n", myPid);

    struct sigaction recvAct;
    recvAct.sa_sigaction = receive;
    sigemptyset(&recvAct.sa_mask);
    sigaddset(&recvAct.sa_mask, SIGUSR2);
    recvAct.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &recvAct, NULL);
    waitForSIGUSR(0, 1);

    switch (mode) {
        case KILL:
            puts("KILL communication mode.\n");
            break;
        case SIGQUEUE:
            puts("SIGQUEUE communication mode.\n");
            break;
        case SIGRT:
            puts("SIGRT communication mode.\n");
            break;
    }

    send(senderPid, SIGUSR2, mode, 0);
    blockSIGUSRs();
    receivedUSR2 = 0;
    setSignals(recvHandler);
    while(!receivedUSR2) {
        waitForSIGUSR(1, 1);
        if (!receivedUSR2) send(senderPid, SIGUSR1, mode, 0);
    }
    printf("Received %d SIGUSR1\n", numberOfReceived);
    for (int i = 0; i < numberOfReceived; i++) {
        send(senderPid, SIGUSR1, mode, i + 1);
    }
    send(senderPid, SIGUSR2, mode, 0);
    puts("Catcher ended his job");

    return 0;
}