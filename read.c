#include "functions.h"

int myread(int fd, char *buf, int nbytes)
{
    OFT* oftp = running->fd[fd];
    MINODE *mip = oftp->minodePtr;

    int count = 0, offset = oftp->offset;
    int lbk, startByte, blk;
    int avil = mip->INODE.i_size - oftp->offset;
    
    int ibuf[BLKSIZE] = {0};
    int dibuf[BLKSIZE] = {0};
    char *cq = buf;

    if(nbytes > avil)
        nbytes = avil;

    while (nbytes && avil)
    {
        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;
        //direct blocks
        if (lbk < 12)
        {
            blk = mip->INODE.i_block[lbk]; // map logical block to physical block
        }
        else if (lbk >= 12 && lbk < 268)
        {
            //indirect block
            get_block(mip->dev, mip->INODE.i_block[12], ibuf);
            blk = ibuf[lbk-12]; // get physical indirect block
        }
        else
        {
            //double indirect
            int indirectBlk = (lbk - 268) / 256;
            int doubleIndBlk = (lbk - 268) % 256;
            get_block(mip->dev, mip->INODE.i_block[13], (char *)ibuf); // access indirect blocks
            blk = ibuf[indirectBlk];

            get_block(mip->dev, blk, dibuf); // access double indirect block
            blk = dibuf[doubleIndBlk]; // get physical double indirect block
        }

        char kbuf[BLKSIZE];
        get_block(mip->dev, blk, kbuf);
        char *cp = kbuf + startByte;
        int remain = BLKSIZE - startByte; // number of remaining bytes 

        if (nbytes <= remain) 
        {
            memcpy(cq, cp, nbytes);
            cq += nbytes;
            cp += nbytes;
            count += nbytes;
            oftp->offset += nbytes;
            avil -= nbytes; 
            remain -= nbytes;
            nbytes = 0;
        }
        else 
        {
            memcpy(cq, cp, remain);
            cq += remain;
            cp += remain;
            count += remain;
            oftp->offset += remain;
            avil -= remain;
            nbytes -= remain; 
            remain = 0;
        }
    }
    return count;   // count is the actual number of bytes read
}

int read_file(int fd, int nbytes)
{
    char buf[BLKSIZE];
    if (running->fd[fd] == NULL || (running->fd[fd]->mode != 0 && running->fd[fd]->mode != 2))
    {
        printf("File not opened for Read\n");
        return -1;
    }
    
    int count = myread(fd, buf, nbytes);
    printf("\n read %d bytes from file fd: %d\n", count, fd);
    return count;
}