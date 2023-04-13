#include "functions.h"

int write_file(int fd)
{
    char buf[BLKSIZE];

    if(running->fd[fd] == NULL || running->fd[fd]->mode == 0)
    {
        printf("Error, file not opened for write!\n");
        return -1;
    }

    printf("Enter string to write > ");
    fgets(buf, BLKSIZE, stdin);
    int nbytes = strlen(buf) - 1;
    buf[nbytes] = 0;

    return(mywrite(fd, buf, nbytes));
}

int mywrite(int fd, char *buf, int nbytes)
{
    OFT *oftp = running->fd[fd];
    MINODE *mip = oftp->minodePtr;
    INODE *ip = &mip->INODE;
    int count = 0, lbk, start, remain, blk, indBlock, indOffset;
    char kbuf[BLKSIZE] = {0}, ibuf[BLKSIZE] = {0}, dibuf2[BLKSIZE] = {0};
    char *cq = buf; // copy buf for writing
    int *bp;

    while (nbytes > 0)
    {
        lbk = oftp->offset / BLKSIZE;
        start = oftp->offset % BLKSIZE;

        if (lbk < 12) // direct block
        {
            if (ip->i_block[lbk] == 0) // if block doesnt exist, allocate block first
                ip->i_block[lbk] = balloc(mip->dev);
            
            blk = ip->i_block[lbk]; // block number = new block
        }
        else if (lbk >= 12 && lbk < 268) // indirect block
        {
            // if block doesn't exist
            if (ip->i_block[12] == 0)
            {
                // create new block
                int temp = ip->i_block[12] = balloc(mip->dev);
                if (temp == 0)
                    return 0;
                // get block
                get_block(mip->dev, ip->i_block[12], ibuf);
                // zero out block
                for(int i = 0; i < 256; i++)
                {
                    ibuf[i] = 0; 
                }
                // put empty block in position
                put_block(mip->dev, ip->i_block[12], ibuf);
                // increase block count
                ip->i_blocks++;
            }

            int ibuf2[256] = {0};
            // get free block from memory
            get_block(mip->dev, ip->i_block[12], (char *)ibuf2);

            //set blk to new block
            blk = ibuf2[lbk-12];

            // if block number invalid
            if(blk == 0)
			{
                // allocate memory for block
				blk = ibuf2[lbk-12] = balloc(mip->dev);
                // increase block count
                ip->i_blocks++;
                // place block into memory
                put_block(mip->dev, ip->i_block[12], (char*)ibuf2);
			}
        }
        else // double indirect
        {
            // if block doesnt exist, allocate new block and fill with zeroes
            if (ip->i_block[13] == 0)
            {
                // allocate memory for block
                int temp = ip->i_block[13] = balloc(mip->dev);

                if (temp == 0)
                    return 0;

                // get block
                get_block(mip->dev, ip->i_block[13], ibuf);

                // zero out the block
                for(int i = 0; i < 256; i++)
                {
                    ibuf[i] = 0; 
                }
                // put empty block in position
				put_block(mip->dev, ip->i_block[13], ibuf);
                // increase block count
                ip->i_blocks++;
            }

            indBlock = (lbk - 268) / 256;
            indOffset = (lbk - 268) % 256;

            int dibuf[256] = {0};
            // get the double indirect block
            get_block(mip->dev, ip->i_block[13], (char*) dibuf);
            // get the double indirect block number
            int dblk = dibuf[indBlock];

            //if block doesnt exist, allocate it and fill with 0
            if (dblk == 0)
            {
                // alocate memory for block
                dblk = dibuf[indBlock] = balloc(mip->dev);

                if (dblk == 0)
                    return 0;

                // get double indirect block
                get_block(mip->dev, dblk, dibuf2);

                // zero out the block
                for(int i = 0; i < 256; i++)
                {
                    dibuf2[i] = 0;
                }
                
                // put double indirect block into 14th block
                put_block(mip->dev, dblk, dibuf2);
                // increase block count
                mip->INODE.i_blocks++;
                // put 14th block into memory
                put_block(mip->dev, ip->i_block[13], (char *)dibuf);
            }


            memset(dibuf, 0, 256);
            get_block(mip->dev, dblk, (char *)dibuf);
            if (dibuf[indOffset] == 0) 
            {
                // allocate memory for double indirect block offset
                blk = dibuf[indOffset] = balloc(mip->dev);
                if (blk == 0)
                    return 0;
                // increase block count
                mip->INODE.i_blocks++;
                // place block into memory
                put_block(mip->dev, dblk, (char *)dibuf);
            }
        }

        // get block to write to
        get_block(mip->dev, blk, kbuf);
        char *cp = kbuf + start;
        remain = BLKSIZE - start;

        // write in an optimized way
        if(remain <= nbytes)
        {
            memcpy(cp, cq, remain);
            cq += remain;
            cp += remain;
            oftp->offset += remain;
            nbytes -= remain;
        }
        else 
        {
            memcpy(cp, cq, nbytes); 
            cq += nbytes;
            cp += nbytes;
            oftp->offset += nbytes;
            nbytes -= nbytes;
        }
        if(oftp->offset > mip->INODE.i_size)
            mip->INODE.i_size = oftp->offset;

        put_block(mip->dev, blk, kbuf); // write the buffer to the disk
        mip->INODE.i_blocks++;
    }

    // mark node as dirty
    mip->dirty = 1;
    //printf("mywrite: wrote %d chars into fd = %d\n", count, fd);
    return nbytes;
}