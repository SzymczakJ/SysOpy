#include "stdlib.h"
#include <stdio.h>
#include <sys/types.h>
#include<sys/wait.h>
#include <unistd.h>
#include "string.h"
#include <sys/times.h>
#include <dirent.h>
#include <sys/stat.h>

int n, maxDepth;
char fullpath[900];
char directPath[900];
char command[1000];
char *searchedString;

void printIfFound() {
    sprintf(command, "grep -c %s %s\n", searchedString, fullpath);
    FILE *pf = popen(command, "r");

    if (!feof(pf)) {
        char sign = getc(pf);
        if (sign != '0') {
            printf("%s %s %d", fullpath, directPath, getpid());
        }
    }
}

void recurThroughFiles() {
    n += 1;
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;

    int directLen = strlen(directPath);
    if (directLen != 0) {
        directPath[directLen++] = '/';
    }
    n = strlen(fullpath);
    fullpath[n++] = '/';
    fullpath[n] = '\0';

    dp = opendir(fullpath);
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) continue;
        strcpy(&fullpath[n], dirp->d_name);
        strcpy(&directPath[directLen], dirp->d_name);
        lstat(fullpath, &statbuf);
        if ( S_ISDIR(statbuf.st_mode) == 1) {
            if (fork() == 0) {
                recurThroughFiles();
                return;
            }
        }
        else if (S_ISREG(statbuf.st_mode) == 1) printIfFound();
    }
}

int main(int argc, char **argv) {
    strcpy(directPath, "");
    strcpy(fullpath, argv[1]);

    searchedString = calloc(sizeof(char), strlen(argv[2]) + 1);
    strcpy(searchedString, argv[2]);

    maxDepth = atoi(argv[3]);
    n = 0;
    recurThroughFiles();
}