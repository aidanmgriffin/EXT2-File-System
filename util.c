/*********** util.c file ****************/

#include "functions.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <ext2fs/ext2_fs.h>
// #include <string.h>
// #include <libgen.h>
// #include <sys/stat.h>
// #include <time.h>

// #include "type.h"

/**** globals defined in main.c file ****/
// extern MINODE minode[NMINODE];
// extern MINODE *root;
// extern PROC   proc[NPROC], *running;

// extern char gpath[128];
// extern char *name[64];
// extern int n;

// extern int fd, dev;
// extern int nblocks, ninodes, bmap, imap, iblk;

// extern char line[128], cmd[32], pathname[128];

int get_block(int dev, int blk, char *buf)
{
	lseek(dev, (long)blk*BLKSIZE, 0);
  	read(dev, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
	lseek(dev, (long)blk*BLKSIZE, 0);
	write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
	int i;
	char *s;
	printf("tokenize %s\n", pathname);

	strcpy(gpath, pathname);   // tokens are in global gpath[ ]
	n = 0;

	s = strtok(gpath, "/");
	while(s)
	{
		name[n] = s;
		n++;
		s = strtok(0, "/");
	}
	name[n] = 0;
	
	for (i= 0; i<n; i++)
		printf("%s  ", name[i]);
	printf("\n");
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
	int i;
	MINODE *mip;
	char buf[BLKSIZE];
	int blk, offset;
	INODE *ip;

	for (i=0; i<NMINODE; i++)
	{
		mip = &minode[i];
		if (mip->refCount && (mip->dev == dev) && (mip->ino == ino))
		{
			mip->refCount++;
			//printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
			return mip;
		}
	}
    
  	for (i=0; i<NMINODE; i++)
  	{
		MINODE* temp = &minode[i];
		if (temp->refCount == 0)
		{
			//printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
			mip = temp;
			break;
		}
  	}  

	mip->dev = dev;
	mip->ino = ino; 

	blk = (ino - 1) / 8 + iblk;
    offset = (ino - 1) % 8;
    get_block(dev, blk, buf); // read the block
    ip = (INODE *)buf + offset;
    mip->INODE = *ip; // set pointer to block

	mip->refCount = 1;
    mip->mounted = 0;
    mip->dirty = 0;
    mip->mptr = 0;
  	return mip;
}

void iput(MINODE *mip)
{
	int i, block, offset;
	char buf[BLKSIZE];
	INODE *ip;

	if (mip==0) 
		return;

	mip->refCount--;
	
	if (mip->refCount > 0) return;
	if (!mip->dirty)       return;
 
	block = (mip->ino - 1) / 8 + iblk;
	offset = (mip->ino - 1) % 8;
	// get block containing this inode
	get_block(mip->dev, block, buf);
	ip = (INODE *)buf + offset; // ip points at INODE
	*ip = mip->INODE; // copy INODE to inode in block
	put_block(mip->dev, block, buf); // write back to disk
	mip->refCount = 0;
} 

int search(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   printf("  ino   rlen  nlen  name\n");

   while (cp < sbuf + BLKSIZE)
   {
		strncpy(temp, dp->name, dp->name_len);
		temp[dp->name_len] = 0;
		printf("%4d  %4d  %4d    %s\n", 
			dp->inode, dp->rec_len, dp->name_len, dp->name);
		if (strcmp(temp, name)==0){
			printf("found %s : ino = %d\n", temp, dp->inode);
			return dp->inode;
		}
		cp += dp->rec_len;
		dp = (DIR *)cp;
   }
   return 0;
}

int getino(char *pathname)
{
	int i, ino, blk, offset;
	char buf[BLKSIZE];
	INODE *ip;
	MINODE *mip;

	printf("getino: pathname=%s\n", pathname);
	if (strcmp(pathname, "/")==0)
		return 2;
	
	// starting mip = root OR CWD
	if (pathname[0]=='/')
	{
		dev = root->dev;
		ino = root->ino;
	}
	else
	{
		dev = running->cwd->dev;
		ino = running->cwd->ino;
	}

	mip = iget(dev, ino);
	
	tokenize(pathname);

	for (i=0; i<n; i++)
	{
		printf("===========================================\n");
		printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);

		ino = search(mip, name[i]);

		if (ino==0)
		{
			mip->dirty = 1;
			iput(mip);
			printf("name %s does not exist\n", name[i]);
			return 0;
		}
		iput(mip);
		mip = iget(dev, ino);
	}

	mip->dirty = 1;
	iput(mip);
	return ino;
}

// These 2 functions are needed for pwd()
int findmyname(MINODE *parent, u32 myino, char myname[ ]) 
{
	char *cp, sbuf[BLKSIZE], temp[256];
	DIR *dp;
	INODE *ip;

	ip = &(parent->INODE);
	get_block(dev, ip->i_block[0], sbuf);
	dp = (DIR*)sbuf;
	cp = sbuf;

	while(cp < sbuf+BLKSIZE)
	{
	   if (dp->inode == myino)
		{
		   strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			strcpy(myname, temp);
			return 1;
		}
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}
	return 0;
}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
	char *cp, sbuf[BLKSIZE], temp[256];
	DIR *dp;
	INODE *ip;

	ip = &(mip->INODE);
	get_block(dev, ip->i_block[0], sbuf);
	dp = (DIR*)sbuf;
	cp = sbuf;
	*myino = dp->inode;
	cp += dp->rec_len;
	dp = (DIR*)cp;
	return dp->inode;
}
