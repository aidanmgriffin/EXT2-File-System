#include "functions.h"

int mystat(char *filename)
{
    struct stat myst;
    int ino = getino(filename);
    MINODE *mip = iget(dev, ino);
    INODE *ip = &mip->INODE;
    printf("  File: %s\n", filename);
	printf("  Size: %d\tBlocks: %12d ", ip->i_size, ip->i_blocks);
	if(S_ISDIR(ip->i_mode))
		printf("  Directory\n");
	else
		printf("  File\n");
	printf("Inode: %d Links:%d \n", ino, ip->i_links_count);

	char *my_atime = ctime( (time_t*)&ip->i_atime);
	char *my_mtime = ctime( (time_t*)&ip->i_mtime);
	char *my_ctime = ctime( (time_t*)&ip->i_ctime);

	printf("Access: %26s", my_atime);
	printf("Modify: %26s", my_mtime);
	printf("Change: %26s", my_ctime);
}

int mychmod(char *filename, __u16 mode)
{
    int ino = getino(filename);
    MINODE *mip = iget(dev, ino);
    mip->INODE.i_mode = mode;
    mip->dirty = 1;
    iput(mip);
}

int myutime(char * filename)
{
    int ino = getino(filename);
    MINODE *mip = iget(dev, ino);
    mip->INODE.i_atime = time(0L);
    mip->dirty = 1;
    iput(mip);
}