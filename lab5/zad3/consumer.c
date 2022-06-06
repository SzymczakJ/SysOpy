#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/file.h>
#include <string.h>
#include <wait.h>
#include <stdbool.h>
#include <limits.h>

void fileAppend(FILE *fileHandle, char *text, size_t lineNumber) {
    int fileHandleNumber = fileno(fileHandle);
    flock(fileHandleNumber, LOCK_EX);
    int numberOfLines = 1;
    char c;
    fseek(fileHandle, 0, SEEK_SET);
    int before = -1;
    while (fread(&c, sizeof(char), 1, fileHandle) > 0) {
        if ((c == '\n' || c == 0x0) && lineNumber == numberOfLines) {
            before = ftell(fileHandle) - 1;
        }
        if (c == '\n') numberOfLines++;
    }

    int fileSize = ftell(fileHandle);
    if (before != -1) {
        fseek(fileHandle, before, SEEK_SET);
        char *contentAfter = malloc(sizeof(char) * (fileSize - before));
        fread(contentAfter, sizeof(char), fileSize - before, fileHandle);
        fseek(fileHandle, before, SEEK_SET);
        fwrite(text, sizeof(char), strlen(text), fileHandle);
        fwrite(contentAfter, sizeof(char), fileSize - before, fileHandle);
        free(contentAfter);
    } else {
        while (lineNumber > numberOfLines) {
            fwrite("\n", sizeof (char), 1, fileHandle);
            numberOfLines++;
        }
        fwrite(text, sizeof(char ), strlen(text), fileHandle);
    }
    fseek(fileHandle, 0, SEEK_SET);
    flock(fileHandleNumber, LOCK_UN);
}

void consume(FILE * fifoHandle, FILE * outputHandle, int numberOfCharactersRead) {
    setvbuf(fifoHandle, NULL, _IONBF, 0);
    int fifoDesc = fileno(fifoHandle);
    int incomingFrom;
    int result;

    while (flock(fifoDesc, LOCK_EX) == 0 && (result = fscanf(fifoHandle, "%d:", &incomingFrom)) >= 0) {
        if (result == 0) {
            flock(fifoDesc, LOCK_UN);
            continue;
        }

        char buffer[numberOfCharactersRead + 1];
        buffer[numberOfCharactersRead] = 0;
        int readLen = 0;
        while(readLen < numberOfCharactersRead) {
            readLen += fread(buffer + readLen, sizeof(char ), numberOfCharactersRead - readLen, fifoHandle);
        }

        buffer[readLen] = 0;
        flock(fifoDesc, LOCK_UN);
        fileAppend(outputHandle, buffer, incomingFrom);
    }
}

int main(int argc, char ** argv) {
    int numberOfCharactersRead;
    numberOfCharactersRead = atoi(argv[3]);
    FILE * fifoHandle, * outputHandle;

    fifoHandle = fopen(argv[1], "r");
    outputHandle = fopen(argv[2], "w+r");
    consume(fifoHandle, outputHandle, numberOfCharactersRead);

    fclose(fifoHandle);
    fclose(outputHandle);
    return 0;
}
