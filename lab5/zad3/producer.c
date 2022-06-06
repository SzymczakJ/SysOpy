#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <wait.h>
#include <limits.h>

void produce(int myNumber, int charactersPerRead, char * fifoPath, char * inputPath) {
    FILE * input = fopen(inputPath, "r");
    FILE * pipe = fopen(fifoPath, "w");
    int readOffset = 1;
    int number = myNumber;
    while(number > 0){
        readOffset++;
        number = number / 10;
    }

    char * writeBuffer = calloc(charactersPerRead + 100, sizeof(char));
    writeBuffer[readOffset + charactersPerRead + 1] = 0;
    char * readBuffer = writeBuffer + readOffset;

    sprintf(writeBuffer, "%d:", myNumber);
    setvbuf(pipe, NULL, _IONBF, 0);

    int numberOfCharactersRead;
    do {
        usleep((rand() % 20) * 1000);
        numberOfCharactersRead = fread(readBuffer, sizeof(char ), charactersPerRead, input);
        for (int i = numberOfCharactersRead; i < charactersPerRead; i++) {
            readBuffer[i] = 0;
        }
        for (size_t i = 0; i < numberOfCharactersRead; i++) {
            if (readBuffer[i] == '\n') readBuffer[i] = ' ';
        }
        fwrite(writeBuffer, sizeof(char ), readOffset + charactersPerRead, pipe);
    } while (numberOfCharactersRead > 0);

    free(writeBuffer);
    fclose(input);
}

int main(int argc, char **argv) {
    int myNumber = atoi(argv[2]);
    int charactersPerRead = atoi(argv[4]);
    char * fifoPath = argv[1];
    char * inputPath = argv[3];

    produce(myNumber, charactersPerRead, fifoPath, inputPath);
    return 0;
}
