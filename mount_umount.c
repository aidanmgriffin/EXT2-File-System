#include "functions.h"

int mymount()
{
    char *filesys, *mount_point;
    int mountNumber = -1;
    char buf[BLKSIZE];

    strcpy(filesys, pathname);
    strcpy(mount_point, pathname2);

    if (strcmp(filesys, "") && strcmp(mount_point, ""))
    {
        //display current mounted filesystems
    }

    for (int i = 0; i < NMOUNT; i++)
	{
        if (strcmp(mountTable[i].name, filesys)==0)
        {
            printf("REJECTED: file already mounted\n");
            return -1;
        }
    }
    for (int i = 0; i < NMOUNT; i++)
	{
        if (mountTable[i].dev == 0)
        {
            mountNumber = i;
            break;
        }
    }

    if (mountNumber == -1)
    {
        printf("ERROR: Device mount limit reached\n");
        return -1;
    }

    // open for RW
    int fd = open(filesys, O_RDWR);

    /********** read super block  ****************/
    get_block(fd, 1, buf);
    sp = (SUPER *)buf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53)
    {
        printf("filesys %s is not an ext2 filesystem\n", filesys);
        return -1;
    }

    int ino = getino(mount_point);
    MINODE *mip = iget(running->cwd->dev, ino);

    if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("REJECTED: mount point is not a directory\n");
        return -1;
    }
    if(mip->refCount > 2)
    {
        printf("REJECTED: mount point directory is busy\n");
        return -1;
    }
    
    MOUNT* mptr = &mountTable[mountNumber];
    mptr->dev = fd;
    strcpy(mptr->name, filesys);
    strcpy(mptr->mount_name, mount_point);

    mptr->ninodes = sp->s_inodes_count;
    mptr->nblocks = sp->s_blocks_count;
    get_block(dev, 2, buf);
    gp = (GD *)buf;

    mptr->bmap = gp->bg_block_bitmap;
    mptr->imap = gp->bg_inode_bitmap;
    mptr->iblk = gp->bg_inode_table;
    
    mip->mounted = 1;
    mip->mptr = mptr;
    mptr->mounted_inode = mip;

    printf("mymount: mounted filesys %s onto mount_point %s\n", filesys, mount_point);
    return 0;
}

int myumount(char *filesys);

MOUNT *getmptr(int dev)
{
	for (int i = 0; i < NMOUNT; i++)
	{
		if(mountTable[i].dev == dev)
			return &mountTable[i];
	}
	return -1;
}