#include "functions.h"

/*LINK command

** link file_old file_new creates a HARD link from file_new --> file_old

** Hard Links can only be applied to regular files, because DIRS may create LOOPS in the file system name space

** Hard link files share the SAME inode. They are on the SAME device. 

** ALGORITHM **

(1) verify an old file exists and it is a regular file (NOT a dir)
(2) Check if new file exists yet. NEW file must not exist yet 
(3) Create new_file with the SAME ino as old_file 
(4) Create an entry in new parent directory with SAME ino as old_file

*/

int mylink(){

int oino = getino(pathname); //it is error if ino is equal to 0.
MINODE* omip = iget(dev, oino);

printf("%s", omip->INODE);

int i;

/* (1) verify an old file exists and it is a regular file (NOT a dir) */

if(S_ISDIR(omip->INODE.i_mode)) // POSIX macro is defined to check the file type.  0x4000 also stands for DIR in HEX so that can be used to check.
{ 
    printf("Link Error - Old file is not a directory.\n");
    return 0; //old file must be regular file not directory.
}

/* (2) Check if new file exists yet. NEW file must not exist yet */ 

if(getino(pathname2))
{
    printf("Link Error - New file already exists.\n");
    return 0;
}

/* (3) Create new_file with the SAME ino as old_file */

char* parent = dirname(pathname2);
char* child = basename(pathname2);
int pino = getino(parent);
MINODE * pmip = iget(dev, pino);

// (4) Create an entry in new parent directory with SAME ino as old_file

enter_name(pmip, oino, child); //We probably need enter_name here instead

omip->INODE.i_links_count++; //add to INODE's links_count. This shows the number of HARD links to the same INODE
omip->dirty = 1; //Omip has been interfered with

iput(omip);
iput(pmip);

} 

/*UNLINK command

**in the i_mode fields of the inode structure. 

** i_mode = |tttt|ugs|rwxrwxrwx|

**The leading 4 bits specify the file type. For example tttt = 1000 for REG file and 0100 for DIR. Next 3 bits are the file's special usage and the last 9 bits are for the rwx permission bits for file prediction.

** ALGORITHM **
 (1) Get filename's minode
 (2) Remove name entry from parent DIR's data block
 (3) Decrement INODE's link count by 1
 (4) If there is remaining link, indicate that mip has been modified. ** The INODE is written back to the disk if it is modified **
 (5) Otherwise, deallocate data blocks and inode to remove file.
 (6) Release mip

*/

int myunlink()
{
    printf("unlink %s\n\n\n\n\n", pathname);
    //  (1) Get filename's minode

    int ino = getino(pathname);
    MINODE * mip = iget(dev, ino);

    // ensure file is a REG or LNK file. Can not be a DIR
    if(S_ISDIR(mip->INODE.i_mode)){
        printf("Link Error - File is a directory.\n");
        return 0;
    }

    // (2) Remove name entry from parent DIR's data block

    char* parent = dirname(pathname);
    char* child = basename(pathname);
    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);
    rm_child(pmip, child);

    pmip->dirty = 1; //pmip has been interfered with

    iput(pmip);

     // (3) Take one away from the link_count of INODE

     mip->INODE.i_links_count -= 1;

    // (4) If there is remaining link, indicate that mip has been modified. ** The INODE is written back to the disk if it is modified **

     if(mip->INODE.i_links_count > 0)
     {
         mip->dirty = 1; //mip has been edited. for writing back to disk node.
     }

    //  (5) Otherwise, deallocate data blocks and inode to remove file.

     else //links count is equal to 0
     {
         bdalloc(mip->dev, mip->INODE.i_block[0]); //deallocate data blocks
         idalloc(mip->dev, mip->ino); //deallocate inode
     }

    //  (6) Release MINODE

     iput(mip);
     
}

