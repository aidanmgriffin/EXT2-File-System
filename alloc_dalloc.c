#include "functions.h"

int tst_bit(char *buf, int bit)
{
	return buf[bit / 8] & (1 << (bit % 8));
}

int set_bit(char *buf, int bit)
{
    int bitnum, byte;
    byte = bit / 8;
    bitnum = bit % 8;
    if (buf[byte] |= (1 << bitnum))
    {
        return 1;
    }
    return 0;
}

int decFreeInodes(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}

int decFreeBlocks(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count--;
    put_block(dev, 2, buf);
}

int ialloc(int dev)
{
    int  i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(dev, imap, buf);

    for (i=0; i < ninodes; i++)
    {
        // use ninodes from SUPER block
        if (tst_bit(buf, i)==0)
        {
            set_bit(buf, i);
            put_block(dev, imap, buf);

            decFreeInodes(dev);

            printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
            return i+1;
        }
    }
    return 0;
}

int balloc(int dev)
{
    int  i;
    char buf[BLKSIZE];

    // read block_bitmap 
    get_block(dev, bmap, buf);

    for (i=0; i < nblocks; i++)
    {
        // use nblocks from SUPER block
        if (tst_bit(buf, i)==0)
        {
            set_bit(buf, i);
            decFreeBlocks(dev);
            put_block(dev, bmap, buf);
            printf("allocated disk block = %d\n", i+1);
            return i+1;
        }
    }
    return 0;
}

int clr_bit(char *buf, int bit)
{
    int bitnum, byte;
    byte = bit / 8;
    bitnum = bit % 8;
    if (buf[byte] &= ~(1 << bitnum))
    {
        return 1;
    }
    return 0;
}

int incFreeInodes(int dev)
{
    char buf[BLKSIZE];

    // inc free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);
}

int incFreeBlocks(int dev)
{
    char buf[BLKSIZE];

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);
}
		  
int idalloc(int dev, int ino)
{
    int i;  
    char buf[BLKSIZE];

    if (ino > ninodes)
    {
        printf("inumber %d out of range\n", ino);
        return -1;
    }

    // get inode bitmap block
    get_block(dev, imap, buf);
    clr_bit(buf, ino-1);

    // write buf back
    put_block(dev, imap, buf);

    // update free inode count in SUPER and GD
    incFreeInodes(dev);
}

int bdalloc(int dev, int bno)
{
    int i;  
    char buf[BLKSIZE];

    if (bno > nblocks)
    {
        printf("inumber %d out of range\n", bno);
        return -1;
    }

    // get bnode bitmap block
    get_block(dev, bmap, buf);
    clr_bit(buf, bno-1);

    // write buf back
    put_block(dev, bmap, buf);

    // update free block count in SUPER and GD
    incFreeBlocks(dev);
}