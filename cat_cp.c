#include "functions.h"

int mycat(char * filename)
{
    char mybuf[BLKSIZE];
    int n, i;
    //printf("%s\n", filename);
    fd = myopen(filename, 0); // need myopen
    // printf("\ntest\n");
    if (fd == -1) // file doesnt exist
        return -1;
    while (n = myread(fd, mybuf, BLKSIZE))
    {
        mybuf[n] = 0;
        char *cp = mybuf;
        while (*cp != '\0') 
        {
            if (*cp == '\n') 
            {
                printf("\n");
            } 
            else 
            {
                printf("%c", *cp);
            }
            cp++;
        }
    }
    printf("\n\r");
	myclose(fd); // need myclose
    return;
}

int mycp(char *source, char *dest)
{
    int fd = myopen(source, O_RDONLY);
    int gd = myopen(dest, O_WRONLY);
    char buf[BLKSIZE];

    while (n = myread(fd, buf, BLKSIZE))
    {
        mywrite(gd, buf, n);
        memset(buf,0,BLKSIZE);
    }
    myclose(fd);
    myclose(gd);
}