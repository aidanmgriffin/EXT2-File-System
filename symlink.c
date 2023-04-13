#include "functions.h"
/*SYMLINK command

**symlink creates a symbolic link from a filenew to a fileold. 

**unlink hard links, symlinks can link to ANYthing. (including DIRS, files not on same dev)

** ALGORITHM **
 (1) Check if old and new files exist. Old file MUST exist. New file MUST NOT exist.
 (2) change filenew to LNK type
 (3) Assume fileold name len <= 60 chars

*/

int mysymlink(){

// printf("%s %s", root->dev, running->cwd->dev); //to delete
 
printf("symlink %s %s", pathname, pathname2);

// (1) Check if old and new files exist.

if(!getino(pathname)) //old file must exist
{
    printf("Error- old file must exist");
    return 0;
}

if(getino(pathname2)) //new file must not exist
{
    printf("Error - new file must not exist");
    return 0;
}

mycreat(pathname2); //change new file to LNK type

// (2) change filenew to LNK type

int ino = getino(pathname2);
MINODE * mip = iget(dev, ino);
mip->INODE.i_mode = 0xA1FF; //LNK type.

//(3) Assume fileold name len <= 60 chars

strcpy(mip->INODE.i_block, pathname); //cpy fileold name over to filenew iblock
mip->INODE.i_size = strlen(pathname); //set new file size to length of old file
mip->dirty = 1; // mark new_file’s minode dirty;
iput(mip);

}
/* 
Readlink

** Reads target file name of symbolic file and returns the length of the target file name

(1) Get file's INODE in memory; verify it's a LNK file.
(2) Copy target filename from INODE.i_block[] into buffer
(3) return file size 
*/

int myreadlink()
{

    char buf[BLKSIZE];

    printf("readlink %s\n", pathname);

    int ino = getino(pathname); // Get file's INODE in memory; 
    MINODE *mip = iget(dev, ino);

    if(!S_ISLNK(mip->INODE.i_mode)) //verify it's a LNK file.
    {
        printf("Can't readlink. Selected item is not a LNK file.\n");
        return 0;
    }

    get_block(dev, mip->INODE.i_block[0], buf); //Copy target filename from INODE.i_block[] into buffer

    printf("\nFile Size: %d\n\n", mip->INODE.i_size); //print file size
    return mip->INODE.i_size; //return file size
}