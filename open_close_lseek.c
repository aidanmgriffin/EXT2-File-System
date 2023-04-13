#include "functions.h"

/* Open

**

Algorithm of open:
(1) Get file's minode
(2) Check if file is regular
(3) Check if file has been already opened if mode is w|rw|append
(4) Create oftp table entry and set to minode of filename.
(5) Set offset to 0, unless append. Set offset to INODE size
(6) Set minode time to current time
(7) Search proc for first free fd entry. Fill entry with opentable entry
(8) Return index as file descriptor.

 */

int myopen(char* filename, int mode)
{

    // (1) Get file's minode.

    int ino = getino(filename);

    if (ino == 0)
    { // file doesn't exist
        if (mode == 0 || mode == 3)
        {
            printf("File doesn't exist");
            return -1;
        }
        else
        {
            mycreat(filename);
            ino = getino(filename);
        }
    }

    MINODE *mip = iget(dev, ino);
    

    //check for regular file and permission
    if(!S_ISREG(mip->INODE.i_mode)){ 
        printf("Not a regular file\n");
        iput(mip);
        return -1;
    }

    for(int i = 0; i < NFD; i++)
    {
        if(running->fd[i] == NULL)
            break;
        if(running->fd[i] != 0 )
        {
            if(running->fd[i]->mode != 0)
            {
                printf("File already opened\n");
                return -1;
            }
        }
    }

   OFT* oftp;
   oftp = (OFT *)malloc(sizeof(OFT));
   oftp->mode = mode;
   oftp->refCount = 1;
   oftp->minodePtr = mip;

   switch(mode){
        case 0: 
            oftp->offset = 0; //r: offset = 0
            break;
        case 1: 
            mytruncate(mip); //w: truncate file to 0 size
            oftp->offset = 0;
            break;
        case 2:
            oftp->offset = 0; //rw: do not truncate file
            break;
        case 3: 
            oftp->offset = mip->INODE.i_size; //append size
            break;
        default: 
            printf("Invalid Node\n");
            return(-1);
   }
   
   //Search for FIRST FREE fd[index] entry with the lowest index in PROC

   int i = 0;

//fd is array of 10 elements that are intialized to 0. Search for first free(0) entry with the lowest index in PROC running
   for(i = 0; i < NFD; i++)
   {
       if(running->fd[i] == 0){
           running->fd[i] = oftp;
           break;
       }
   }

   if(mode == 0)
   {
       mip->INODE.i_atime = time(0L);
   }
   else{
       mip->INODE.i_atime = time(0L);
       mip->INODE.i_mtime = time(0L);
       mip->dirty = 1;
   }


   //I is the file descriptor to be returned.
   return i;
}

int mytruncate(MINODE *mip)
{
    char buf[256];
    char buf2[256];
    char buf3[256];

    //direct
    for(int i = 0; i < 12; i++)
    {
        bdalloc(dev, mip->INODE.i_block[i]);
    }

    //indirect
    if(mip->INODE.i_block[12] != NULL)
    {
        get_block(dev, mip->INODE.i_block[12], buf);
        for(int i = 0; i < 256; i++)
        {
            bdalloc(dev, buf[i]);
        }   
    }
    bdalloc(dev, mip->INODE.i_block[12]);

    if(mip->INODE.i_block[13] != NULL)
    {
        get_block(dev, mip->INODE.i_block[13], buf2);

        for(int i = 0; i < 256; i++)
        {
            get_block(dev, buf2[i], buf3);
            for(int j = 0; j < 256; j++)
            {
                bdalloc(dev, buf3[j]);
            }
            bdalloc(dev, buf2[i]);
        }
        bdalloc(dev, mip->INODE.i_block[13]);
    }

    mip->INODE.i_atime = time(0L);
    mip->INODE.i_mtime = time(0L);
    mip->INODE.i_size = 0;
    mip->dirty = 1;
}

int myclose(int fd)
{
    if(running->fd[fd] == NULL)
    {
        printf("Error! No file opened at fd %d!\n", fd);
        return -1;
    }

    OFT *oftp = running->fd[fd];
    // remove the file from the running fd list
    running->fd[fd] = NULL;
    oftp->refCount--;

    if(oftp->refCount > 0)
        return 0;
    // if we didnt return, that means thers only one more OFT entry
    // get MINODE of oft
    MINODE *mip = oftp->minodePtr;
    // dispoe of MINODE
    iput(mip);
}

/* LSEEK

**Must check given position is within the bounds of [0, Filesize-1]
**offset is the distance from the original address in the structure
**used for read and write
 */
int mylseek(int fd, int position)
{
    // Find OFT entry from fd
    OFT *oftp;
    oftp = running->fd[fd];

    // Set original position
    int op = oftp->offset;

    if(position < 0 || position > oftp->minodePtr->INODE.i_size - 1)
    {
        printf("Position not within bounds of [0, Filesize-1]\n");
        return -1;
    }

    // set OFT entry's offset to given position
    oftp->offset = position;

    // return original position
    return op;
}

int mypfd()
{
    int i = 0;
    
    printf("fd\tmode\toffset\tINODE\n");
    printf("--\t----\t------\t-----\n");
    for(i = 0; i < NFD; i++) {
        if(running->fd[i] == NULL)
            break;
        printf("%d\t", i);
        switch(running->fd[i]->mode){ //might have to test if it doesn't play along with the tests. Should offset be 0 for the first?
            case 0:
                printf("READ\t%d\t[%d %d]\n", running->fd[i]->offset, running->fd[i]->minodePtr->dev, running->fd[i]->minodePtr->ino);
                break;
            case 1:
                printf("WRITE\t%d\t[%d %d]\n", running->fd[i]->offset, running->fd[i]->minodePtr->dev, running->fd[i]->minodePtr->ino);
                break;
            case 2:
                printf("READWRITE\t%d\t[%d %d]\n", running->fd[i]->offset, running->fd[i]->minodePtr->dev, running->fd[i]->minodePtr->ino);
                break;
            case 3:
                printf("APPEND\t%d\t[%d %d]\n", running->fd[i]->offset, running->fd[i]->minodePtr->dev, running->fd[i]->minodePtr->ino);
                break;
            default:
                printf("(unknown)\t%d\t[%d %d]\n", running->fd[i]->offset, running->fd[i]->minodePtr->dev, running->fd[i]->minodePtr->ino);
                break;
        } 
    }
}

int mydup(int fd)
{
    OFT *oftp;
    oftp = running->fd[fd];

    if(running->fd[fd] == 0)
    {
        printf("FD is not an opened desctiptor\n");
        return -1;
    }

    for(int i = 0; i < NFD; i++)
    {
        // Copy file descriptor into first open slot
        if (running->fd[i] == 0)
        {
            running->fd[i] = oftp;
            oftp->refCount += 1;
            return 0;
        }
    }

}

int mydup2(int fd, int gd)
{
    OFT *oftp;
    oftp = running->fd[fd];
  
    if(running->fd[gd] != 0)
    {
        close(gd); //myclose
    }

    running->fd[gd] = oftp;                                                                                                                                                                    
}