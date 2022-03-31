#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

int signalCallCounter;

void siginfoHandler(int sigNumber, siginfo_t *sigInfo, void *context) {
    puts("SIGINFO handler:\n");
    printf("PID: %d\n", sigInfo->si_pid);
    printf("Signal ID: %d\n", sigNumber);
    puts("Exiting SIGINFO handler\n");
}

void nodeferHandler(int sigNumber) {
    puts("Entering the SA_NODEFER handler\n");
    if (signalCallCounter < 3) {
        signalCallCounter++;
        puts("Raising another signal\n");
        raise(SIGUSR1);
        while (signalCallCounter < 3);
    }
    puts("Exiting the nodeferHandler\n");
}

void resethandHandler(int sigNumber) {
    puts("Entering the SA_RESETHAND handler\n");
}

int main(int argc, char **argv) {
    puts("SIGINFO flag test:\n");
    struct sigaction sig;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = SA_SIGINFO;
    sig.sa_sigaction = siginfoHandler;
    sigaction(SIGUSR1, &sig, NULL);
    raise(SIGUSR1);

    puts("SA_NODEFER flag test: \n");
    struct sigaction sig2;
    sigemptyset(&sig2.sa_mask);
    sig2.sa_flags = SA_NODEFER;
    sig2.sa_handler = nodeferHandler;
    signalCallCounter = 0;
    sigaction(SIGUSR1, &sig2, NULL);
    raise(SIGUSR1);

    puts("SA_RESETHAND flag test: \n");
    struct sigaction sig3;
    sigemptyset(&sig3.sa_mask);
    sig3.sa_flags = SA_RESETHAND;
    sig3.sa_handler = resethandHandler;
    sigaction(SIGUSR1, &sig3, NULL);
    raise(SIGUSR1);
    puts("Handler finished, next raise should become a default and terminate the program\n");
    raise(SIGUSR1);
    puts("This shouldn't show\n");
    return 0;
}