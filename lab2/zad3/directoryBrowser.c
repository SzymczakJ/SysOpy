#include "stdio.h"
#include "stdlib.h"
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include "string.h"

void describeFile(char *pathName, struct stat *statPtr, int type);
void recurThroughFiles();

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock;
static char fullpath[1000];

#define FTW_F 1
#define FTW_D 2

void recurThroughFiles() {
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int n;

    lstat(fullpath, &statbuf);
    if (S_ISDIR(statbuf.st_mode) == 0) {
        describeFile(fullpath, &statbuf, FTW_F);
        return;
    }

    describeFile(fullpath, &statbuf, FTW_D);

    n = strlen(fullpath);
    fullpath[n++] = '/';
    fullpath[n] = '\0';

    dp = opendir(fullpath);
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) continue;
        strcpy(&fullpath[n], dirp->d_name);
        recurThroughFiles();
    }
    fullpath[n - 1] = '\0';

    closedir(dp);
}

void browseDirectiories(char *pathName) {
    strcpy(fullpath, pathName);
    recurThroughFiles();
}

void describeFile(char *pathName, struct stat *statPtr, int type) {
    char actualpath [1000];
    char *ptr;

    ptr = realpath(pathName, actualpath);
    printf("%s", ptr);
    printf("\t%ld", statPtr->st_nlink);

    switch (type) {
        case FTW_F:
            switch (statPtr->st_mode & S_IFMT) {
                case S_IFREG:
                    printf("\tfile");
                    nreg++;
                    break;
                case S_IFBLK:
                    printf("\tblock dev");
                    nblk++;
                    break;
                case S_IFCHR:
                    printf("\tchar dev");
                    nchr++;
                    break;
                case S_IFIFO:
                    printf("\tfifo");
                    nfifo++;
                    break;
                case S_IFLNK:
                    printf("\tslink");
                    nslink++;
                    break;
                case S_IFSOCK:
                    printf("\tsock");
                    nsock++;
                    break;
            }
            break;
        case FTW_D:
            printf("\tdir");
            ndir++;
            break;
    }

    printf("\t%ld", statPtr->st_size);

    struct tm *visit_time=localtime(&statPtr->st_atime);
    char buffer[80];
    strftime(buffer,10,"%b",visit_time);
    printf("\t%4d %s %2d ", visit_time->tm_year+1900,buffer,visit_time->tm_mday);

    struct tm *time_stamp=localtime(&statPtr->st_mtime);
    char bufferPrim[80];
    strftime(bufferPrim,10,"%b",time_stamp);
    printf("\t%4d %s %2d \n", time_stamp->tm_year+1900,buffer,time_stamp->tm_mday);
}

int main(int argc, char **argv) {
    browseDirectiories(argv[1]);
    printf("regular files = %7ld\n", nreg);
    printf("directories= %7ld\n", ndir);
    printf("block special = %7ld\n", nblk);
    printf("char special = %7ld\n", nchr);
    printf("FIFOs = %7ld %%\n", nfifo);
    printf("symbolic links = %7ld\n", nslink);
    printf("sockets = %7ld\n", nsock);
}