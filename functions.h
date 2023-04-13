#ifndef FUNCTIONS
#define FUNCTIONS

// C Libs:
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>


// USR
#include "type.h"
#include "globals.h"


/* util.c */
int get_block(int dev, int blk, char *buf);

int put_block(int dev, int blk, char *buf);

int tokenize(char *pathname);

MINODE *iget(int dev, int ino);

void iput(MINODE *mip);

int getino(char *pathname);

int findmyname(MINODE *parent, u32 myino, char myname[]);

int findino(MINODE *mip, u32 *myino);

/* cd_ls_pwd.c */
int cd();

int ls_file(MINODE *mip, char *name);

int ls_dir(MINODE *mip);

int ls();

void rpwd(MINODE *wd);

void pwd(MINODE *wd);

 /* link_unlink.c */
int mylink();

int myunlink();

/* symlink.c */
int mysymlink();

int myreadlink();

/* alloc_dalloc.c */
int tst_bit(char *buf, int bit);

int set_bit(char *buf, int bit);

int decFreeInodes(int dev);

int ialloc(int dev);

int balloc(int dev);

int clr_bit(char *buf, int bit);

int idalloc(int dev, int ino);

int bdalloc(int dev, int bno);

int incFreeInodes(int dev);

/* open_close_lseek.c */
int myopen(char* filename, int mode);

int mytruncate(MINODE *mip);

int mylseek(int fd, int position);

int myclose(int fd);

int mypfd();

int mydup(int fd);

int mydup2(int fd, int gd);


/* mkdir_creat.c */
int mymkdir(char *path);

int mycreat(char *path);

int mkdir_creat(MINODE *pmip, char *basename, int isDir);

int enter_name(MINODE *pmip, int ino, char *name);

/* rmdir.c */
int myrmdir(char* path);

int rm_child(MINODE* pmip, char *name);

/* level1misc.c */
int mystat(char *filename);

int mychmod(char *filename, __u16 mode);

int myutime(char * filename);

/* read.c */
int myread(int fd, char *buf, int nbytes);

int read_file(int fd, int nbytes);

/* cat_cp.c */
int mycat(char * filename);

int mycp(char *source, char *dest);

/* write.c */
int write_file(int fd);

int mywrite(int fd, char *buf, int nbytes);

/* permission.c */
int myaccess(char *filename, char mode);

/* mount_umount.c */
int mymount();

int myumount(char *filesys);

MOUNT *getmptr(int dev);

#endif