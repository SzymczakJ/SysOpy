#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include "stdio.h"
#include <sys/stat.h>
#include <time.h>
#include <ftw.h>


static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock;

int describeFile(char *pathName, struct stat *statPtr, int type, struct FTW *pfwt);

int describeFile(char *pathName, struct stat *statPtr, int type, struct FTW *pfwt) {
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
                default:
                    printf("\tdir");
                    ndir++;
                    break;
            }
            break;
        default:
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
    return 0;
}

int main(int argc, char **argv) {
    int flags = FTW_PHYS;
    void *func = describeFile;
    nftw(argv[1], func, 5, flags);
    printf("regular files = %7ld\n", nreg);
    printf("directories= %7ld\n", ndir);
    printf("block special = %7ld\n", nblk);
    printf("char special = %7ld\n", nchr);
    printf("FIFOs = %7ld\n", nfifo);
    printf("symbolic links = %7ld\n", nslink);
    printf("sockets = %7ld\n", nsock);
}