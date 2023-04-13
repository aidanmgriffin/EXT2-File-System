#include "functions.h"



/************* cd_ls_pwd.c file **************/
int cd()
{
	int ino = getino(pathname); // return error if ino=0
	MINODE *mip = iget(dev, ino);

	int mode = mip->INODE.i_mode;

	if(!S_ISDIR(mode)) //return error if not DIR
		return -1;

	iput(running->cwd); // release old cwd
	running->cwd = mip;
}

int ls_file(MINODE *mip, char *name)
{
	char *t1 = "xwrxwrxwr-------";
	char *t2 = "----------------";

	INODE *ip = &mip->INODE;
	char ftime[64];
	char linkname[64];

	int mode = mip->INODE.i_mode;

	if ((ip->i_mode & 0xF000) == 0x8000) // if (S_ISREG())
		printf("%c",'-');
	if ((ip->i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
		printf("%c",'d');
	if ((ip->i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
		printf("%c",'l');
	for(int i = 8; i >= 0; i--)
	{
		if( mode & (1 << i )) //print r|w|x
		{
			printf("%c", t1[i]);
		}
		else
		{
			printf("%c", t2[i]); // or print -
		}
	}

	printf("%4d  ", ip->i_links_count); //print file information
	printf("%4d  ", ip->i_gid); //print file information
	printf("%4d  ", ip->i_uid); //print file information

	time_t t = (time_t)mip->INODE.i_mtime;
  	strcpy(ftime, ctime(&t));
	ftime[strlen(ftime) -1] = 0;
	printf("%s ", ftime); //print time

	printf("%8d  ", ip->i_size); //print file information

	printf("%s", basename(name)); //print file basename

	printf("\t\t");
	printf("[%d %2d]", mip->dev, mip->ino);

	if((ip->i_mode & 0xF000) == 0xA000)
	{
		printf(" -> %s", (char *)mip->INODE.i_block);
	}



	printf("\n");

}

int ls_dir(MINODE *mip)
{
	char buf[BLKSIZE], temp[256];
	DIR *dp;
	char *cp;
	int ino;
	MINODE *miptemp;

	get_block(dev, mip->INODE.i_block[0], buf);
	dp = (DIR *)buf;
	cp = buf;

	while (cp < buf + BLKSIZE)
	{
		strncpy(temp, dp->name, dp->name_len);
		temp[dp->name_len] = 0;
		
		ino = dp->inode;
		miptemp = iget(dev, ino);

		ls_file(miptemp, temp);
		//  miptemp->dirty = 1;

		cp += dp->rec_len;
		dp = (DIR *)cp;
		iput(miptemp);

	}
}

int ls()
{
	printf("ls %s\n", pathname);

	if (!pathname[0])
	{
		ls_dir(running->cwd);
	}

	else
	{
		int ino = getino(pathname);

		if (ino > 0)
		{
		MINODE *mip = iget(dev, ino);
		int mode = mip->INODE.i_mode;

		if (S_ISDIR(mode))
		{
			ls_dir(mip);
		}
		else
		{
			ls_file(mip, basename(pathname));
		}

		iput(mip);

		}
		else
		{
		printf("no such file %s\n", basename(pathname));
		}
	}
}

void rpwd(MINODE* wd)
{
	u32 myino;
	char myname[128];
	if (wd == root)
		return;

	int parent_ino = findino(wd, &myino);
	MINODE* pip = iget(dev, parent_ino);
	findmyname(pip, myino, &myname);
	rpwd(pip);
	printf("/%s", myname);	
}

void pwd(MINODE *wd)
{
	printf("CWD = ");
	if (wd == root){
		printf("/\n");
		return;
	}
	else
	{
		rpwd(wd);
		printf("\n");
	}
}



