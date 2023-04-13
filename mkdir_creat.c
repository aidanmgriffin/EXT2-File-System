#include "functions.h"

int mymkdir(char *path)
{
    char temp[256], dname[128], bname[128];
    strcpy(temp, path);
    strcpy(dname, dirname(temp));
    strcpy(temp, path);
    strcpy(bname, basename(temp));
    // copy pathname to avoid it getting altered, and get basename/dirname

    int pino = getino(dname); // parent inode number
    MINODE* pmip = iget(dev, pino); //parent minode

    if(!S_ISDIR(pmip->INODE.i_mode)) // parent must be directory
    {
        printf("Not a directory. Can not mkdir\n");
        return;
    }
    if(search(pmip, bname) != 0) // basename (child) can not already exist in the parent dir
    {
        printf("Directory already exists.\n");
        return;
    }
    mkdir_creat(pmip, bname, 1);
}

int mycreat(char *path)
{
    char temp[256], dname[128], bname[128];
    strcpy(temp, path);
    strcpy(dname, dirname(temp));
    strcpy(temp, path);
    strcpy(bname, basename(temp));
    // copy pathname to avoid it getting altered, and get basename/dirname

    int pino = getino(dname); // parent inode number
    MINODE* pmip = iget(dev, pino); // parent minode

    if(!S_ISDIR(pmip->INODE.i_mode)) // parent must be a directory
    {
        printf("Not a directory. Can not creat\n");
        return;
    }
    if(search(pmip, bname) != 0) // child can not already exist in parent dir
    {
        printf("File already exists.\n");
        return;
    }
    mkdir_creat(pmip, bname, 0);
}

int mkdir_creat(MINODE *pmip, char *basename, int isDir)
{
    //4.1
    int ino = ialloc(dev); // inode number
    int blk = balloc(dev); // block number

    //4.2
    MINODE *mip = iget(dev, ino);
    INODE *ip = &mip->INODE;
    if (isDir)
    {
        ip->i_mode = 0x41ED;
        ip->i_size = BLKSIZE; // size in bytes
        ip->i_links_count = 2; // links count=2 because of . and ..
    }
    else
    {
        ip->i_mode = 0x81A4;
        ip->i_size = 0;
        ip->i_links_count = 1;
    }
    ip->i_uid = running->uid; // owner uid
    ip->i_gid = running->gid; // group Id        
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
    ip->i_block[0] = blk;
    for (int i = 1; i < 15; i++)
    {
        mip->INODE.i_block[i] = 0;
    }
    mip->dirty = 1;
    iput(mip);

    //4.3
    char buf[BLKSIZE];
    bzero(buf, BLKSIZE); // optional: clear buf[ ] to 0
    DIR *dp = (DIR *)buf;
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name_len = 1;
    dp->name[0] = '.';

    dp = (char *)dp + 12;
    dp->inode = pmip->ino;
    dp->rec_len = BLKSIZE-12; // rec_len spans block
    dp->name_len = 2;
    strcpy(dp->name, "..");
    put_block(dev, blk, buf);
    enter_name(pmip, ino, basename); //enter the child name into parent node
}

int enter_name(MINODE *pmip, int ino, char *name)
{
    char buf[BLKSIZE];
    INODE *pip = &pmip->INODE;
    char *cp;
    int ideal_length = 0, remain = 0, need_length = 4 * ((8 + strlen(name) + 3) / 4);
    int bno;

    for (int i = 0; i < 12; i++) // for each block in the parent
    {
        if (pip->i_block[i] == 0)
            break;
        
        bno = pip->i_block[i];
        get_block(pmip->dev, pip->i_block[i], buf);

        DIR* dp = (DIR*) buf;
        cp = buf;

        // step to last entry in the data block
        while(cp + dp->rec_len < buf + BLKSIZE)
        {
            cp += dp->rec_len;
            dp = (DIR*) cp;
        }

        ideal_length = 4 * ((8 + dp->name_len + 3)/4 );
        remain = dp->rec_len - ideal_length;

        if (remain >= need_length)
        {
            // enter new entry as the lat entry in the block
            dp->rec_len = ideal_length;

            cp += dp->rec_len;
            dp = (DIR*) cp;

            dp->inode = ino;
            dp->rec_len = remain;
            dp->name_len = strlen(name);
            strncpy(dp->name, name, dp->name_len);

            put_block(pmip->dev, bno, buf);
            return 0;
        }
        else
        {
            pip->i_size = BLKSIZE; // size is new block
            bno = balloc(dev);    // allocate new block
            pip->i_block[i] = bno; // add the block to the list
            pmip->dirty = 1;       // ino is changed so make dirty

            get_block(pmip->dev, bno, buf); // get the blcok from memory
            dp = (DIR *)buf;
            cp = buf;

            dp->name_len = strlen(name); // add name len
            strcpy(dp->name, name);      // name
            dp->inode = ino;             // inode
            dp->rec_len = BLKSIZE;         // only entry so full size

            put_block(dev, bno, buf); //save
            return 1;
        }
    }
}
