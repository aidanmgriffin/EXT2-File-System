/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
// C LIBS:
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>


// USR LIBS:
#include "globals.h"
#include "type.h"
#include "functions.h"

// forward declarations of funcs in main.c
int init();
int quit();
int mount_root();

char *disk = "diskimage";

int main(int argc, char *argv[])
{
    int ino;
    char buf[BLKSIZE];

    printf("checking EXT2 FS ....");
    if ((fd = open(disk, O_RDWR)) < 0)
    {
        printf("open %s failed\n", disk);
        exit(1);
    }

    dev = fd; // global dev same as this fd

    /********** read super block  ****************/
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53)
    {
        printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
        exit(1);
    }
    printf("EXT2 FS OK\n");
    ninodes = sp->s_inodes_count;
    nblocks = sp->s_blocks_count;

    get_block(dev, 2, buf);
    gp = (GD *)buf;

    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    iblk = gp->bg_inode_table;
    printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

    init();
    mount_root();
    printf("root refCount = %d\n", root->refCount);

    printf("creating P0 as running process\n");
    running = &proc[0];
    running->cwd = iget(dev, 2);
    printf("root refCount = %d\n", root->refCount);

    // WRTIE code here to create P1 as a USER process

    while (1)
    {
        printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink|open|cat|cp|mount|quit] ");
        fgets(line, 128, stdin);
        line[strlen(line) - 1] = 0;

        if (line[0] == 0)
            continue;
        pathname[0] = 0;
        pathname2[0] = 0;

        sscanf(line, "%s %s %s", cmd, pathname, pathname2);
        printf("cmd=%s pathname=%s %s\n", cmd, pathname, pathname2);

        if (strcmp(cmd, "ls") == 0)
            ls();
        else if (strcmp(cmd, "cd") == 0)
            cd();
        else if (strcmp(cmd, "pwd") == 0)
            pwd(running->cwd);
        else if (strcmp(cmd, "link") == 0)
            mylink();
        else if (strcmp(cmd, "unlink") == 0)
            myunlink();
        else if (strcmp(cmd, "symlink") == 0)
            mysymlink();
        else if (strcmp(cmd, "readlink") == 0)
            myreadlink();
        else if (strcmp(cmd, "mkdir") == 0)
            mymkdir(pathname);
        else if (strcmp(cmd, "creat") == 0)
            mycreat(pathname);
        else if (strcmp(cmd, "rmdir") == 0)
            myrmdir(pathname);
        else if (strcmp(cmd, "mount") == 0)
            mymount();
        else if (strcmp(cmd, "cat") == 0)
            mycat(pathname);
        else if (strcmp(cmd, "cp") == 0)
            mycp(pathname, pathname2);
        else if (strcmp(cmd, "open") == 0)
        {
            int mode = atoi(pathname2);
            myopen(pathname, mode);
        }
        else if (strcmp(cmd, "quit") == 0)
            quit();

    }
}

int quit()
{
    int i;
    MINODE *mip;
    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        if (mip->refCount > 0)
            iput(mip);
    }
    exit(0);
}


int init()
{
    int i, j;
    MINODE *mip;
    PROC *p;

    printf("init()\n");

    for (i = 0; i < NMINODE; i++)
    {
        mip = &minode[i];
        mip->dev = mip->ino = 0;
        mip->refCount = 0;
        mip->mounted = 0;
        mip->mptr = 0;
    }
    for (i = 0; i < NOFT; i++)
    {
        oft[i].refCount = 0;
    }
    for (i = 0; i < NPROC; i++)
    {
        p = &proc[i];
        p->pid = i;
        p->uid = p->gid = 0;
        p->cwd = 0;
        for (j = 0; j < NFD; j++)
        {
            p->fd[j] = NULL;
        }
    }
    for (i = 0; i < NMOUNT; i++)
    {
        MOUNT* mptr = &mountTable[i];
        mptr->dev = 0;
    }
}

// load root INODE and set root pointer to it
int mount_root()
{
    printf("mount_root()\n");
    root = iget(dev, 2);

    mountTable[0].dev = root->dev;
    mountTable[0].ninodes = ninodes;
    mountTable[0].nblocks = nblocks;
    mountTable[0].bmap = bmap;
    mountTable[0].imap = imap;
    mountTable[0].iblk = iblk;
}
