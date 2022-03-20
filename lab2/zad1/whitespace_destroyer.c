#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "string.h"
#include <sys/times.h>

int libCopyingHelper(FILE *file, int *numberOfCharactersToRead);
void libWhitespaceDestroyer(char *sourceFileName, char *targetFileName);
int sysCopyingHelper(int file, int *numberOfCharactersToRead);
void start_timer();
void stop_timer();


clock_t clock_t_begin, clock_t_end;
struct tms times_start_buffer, times_end_buffer;

void start_timer(){
    clock_t_begin = times(&times_start_buffer);
}

void stop_timer(){
    clock_t_end = times(&times_end_buffer);
}

double calc_time(clock_t s, clock_t e) {
    return ((double) (e - s) / (double) sysconf(_SC_CLK_TCK));
}

void print_times(){
    printf("real %.6fs user %.6fs sys %.6fs\n",
           calc_time(clock_t_begin, clock_t_end),
           calc_time(times_start_buffer.tms_cutime, times_end_buffer.tms_cutime),
           calc_time(times_start_buffer.tms_cstime, times_end_buffer.tms_cstime));
}

void libWhitespaceDestroyer(char *sourceFileName, char *targetFileName) {
    FILE *sourceFile = fopen(sourceFileName, "r+");
    FILE *targetFile = fopen(targetFileName, "w");

    int writeLineToFileFlag = 0;
    char *readingArray = calloc(sizeof(char), 256);
    int biggestLineSize = 256;
    int numberOfCharactersToRead = 0;
    while (!feof(sourceFile)) {
        writeLineToFileFlag = libCopyingHelper(sourceFile, &numberOfCharactersToRead);
        if (numberOfCharactersToRead > biggestLineSize) {
            biggestLineSize = numberOfCharactersToRead;
            readingArray = (char *) realloc(readingArray, biggestLineSize);
        }
        fread(readingArray, sizeof(char), numberOfCharactersToRead, sourceFile);
        if (writeLineToFileFlag) fwrite(readingArray, sizeof(char), numberOfCharactersToRead, targetFile);
        writeLineToFileFlag = 0;
    }

    free(readingArray);
    fclose(sourceFile);
    fclose(targetFile);
}

int libCopyingHelper(FILE *file, int *numberOfCharactersToRead) {
    fpos_t startingPos;
    fgetpos(file, &startingPos);

    short characterInLineFlag = 0;
    int i = 1;
    int lineCharacter = fgetc(file);
    while (!feof(file) && lineCharacter != '\n') {
        if (!characterInLineFlag && !isspace(lineCharacter)) characterInLineFlag = 1;
        i++;
        lineCharacter = fgetc(file);
    }

    fsetpos(file, &startingPos);
    *numberOfCharactersToRead = i;
    return characterInLineFlag;
}

void sysWhitespaceDestroyer(char *sourceFileName, char *targetFileName) {
    int sourceFile = open(sourceFileName, O_RDONLY);
    int targetFile = open(targetFileName, O_WRONLY | O_CREAT);

    int writeLineToFileFlag = 0;
    char *readingArray = calloc(sizeof(char), 256);
    int biggestLineSize = 256;
    int numberOfCharactersToRead = 0;
    while (read(sourceFile, readingArray, 1) == 1) {
        lseek(sourceFile, -1, SEEK_CUR);
        writeLineToFileFlag = sysCopyingHelper(sourceFile, &numberOfCharactersToRead);
        if (numberOfCharactersToRead > biggestLineSize) {
            biggestLineSize = numberOfCharactersToRead;
            readingArray = (char *) realloc(readingArray, biggestLineSize);
        }
        read(sourceFile, readingArray, numberOfCharactersToRead);
        if (writeLineToFileFlag) write(targetFile, readingArray, numberOfCharactersToRead);
    }

    free(readingArray);
    close(sourceFile);
    close(targetFile);
}

int sysCopyingHelper(int file, int *numberOfCharactersToRead) {
    short characterInLineFlag = 0;
    int i = 1;
    char *lineCharacter = calloc(sizeof(char), 1);
    while (read(file, lineCharacter, 1) == 1 && lineCharacter[0] != '\n') {
        if (!characterInLineFlag && !isspace(lineCharacter[0])) characterInLineFlag = 1;
        i++;
    }

//    if (read(file, lineCharacter, 1) != 1 && i == 1) i = 0;
    lseek(file, -i, SEEK_CUR);
    *numberOfCharactersToRead = i;
    return characterInLineFlag;
}

int main(int argc, char** argv) {
#ifdef TEST
    printf("time using sys:\n");
    start_timer();
    sysWhitespaceDestroyer("from", "to");
    stop_timer();
    print_times();
    printf("time using lib:\n");
    start_timer();
    libWhitespaceDestroyer("from", "to");
    stop_timer();
    print_times();
    return 0;
#endif
    if (argc < 3) {
        char firstFile[100];
        char *f = firstFile;
        size_t buffSize = 100;
        char secondFile[100];
        char *s = secondFile;
        int firstNumbers = getline(&f, &buffSize, stdin);
        int secondNumbers = getline(&s, &buffSize, stdin);
        firstFile[firstNumbers - 1] = '\0';
        secondFile[secondNumbers - 1] = '\0';
        libWhitespaceDestroyer(firstFile, secondFile);
    } else {
        libWhitespaceDestroyer(argv[1], argv[2]);
    }
}