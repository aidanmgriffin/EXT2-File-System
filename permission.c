#include "functions.h"

int myaccess(char *filename, char mode)
{
    int r;

    // if super user, always have permission
    if (running->uid == 0)
        return 1;
    
    int ino = getino(filename);
    MINODE* mip = iget(dev, ino);

    if (mip->INODE.i_uid == running->uid)
    {
        r = access(filename, R_OK);
    }
}

