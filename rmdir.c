#include "functions.h"
int myrmdir(char* path)
{
    int ino = getino(path); // inode number
    MINODE* mip = iget(dev, ino); // minode
    char temp[256], parent[128], child[128], copy[128];

    strcpy(copy, path);
    strcpy(parent, dirname(copy));
    strcpy(copy, path);
    strcpy(child, basename(copy));
    //get basename and dirname

    if(!S_ISDIR(mip->INODE.i_mode))
    {
        printf("Not a directory. Can not rmdir\n");
        return;
    }

    char buf[BLKSIZE], *cp;

    INODE* ip = &mip->INODE;

    //test if dir is empty
    if (ip->i_links_count == 2)
    {
        // files do not add to links. have to search for files in the dir
        if (ip->i_block[0] != 0)
        {
            get_block(dev, ip->i_block[0], buf);
            dp = (DIR*) buf;
            cp = buf;

            while(cp < buf + BLKSIZE)
            {
                strncpy(temp, dp->name, dp->name_len);
                temp[dp->name_len] = 0;

                if(strcmp(temp, ".") != 0 && strcmp(temp, "..") != 0)
                {
                    printf("Directory is not empty. Can not rmdir\n");
                    return;
                }
                cp += dp->rec_len;
                dp = (DIR*)cp;
            }
        }    
    }
    if(ip->i_links_count > 2) // 2 links for . and .. , any more means not empty
    {
        printf("Directory is not empty. Can not rmdir.\n");
        return;
    }

    // update the data block free space
    for (int i = 0; i < 12; i++)
    {
        if(ip->i_block[i]==0)
            continue;
        bdalloc(dev, ip->i_block[0]);
    }

    idalloc(mip->dev, mip->ino);

    iput(mip);

    // starting at root
    if(parent[0] == '/')
        mip = root;
    else
        mip = running->cwd;

    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);

    //remove the child from the parent by name
    rm_child(pmip, child);
    pmip->INODE.i_links_count--;
    pmip->dirty = 1;
    iput(pmip);
    return 1;
}

int rm_child(MINODE* pmip, char *name)
{
    MINODE *tempParent = pmip;
    char buf[BLKSIZE], tempBuf[BLKSIZE], tempName[128];
    int recommendedLength, previousRecommended;

    for (int i = 0; i < 12; i++)
    {
        if (tempParent->INODE.i_block[i] == 0)
            return; //block doesnt exist
        
        get_block(dev, tempParent->INODE.i_block[i], buf);

        DIR* dp = (DIR*)buf;
        char *cp = buf;
        char *tempCP = buf;

        while(cp < buf + BLKSIZE)
        {
            strncpy(tempName, dp->name, dp->name_len);
            tempName[dp->name_len] = 0;

            if (!strcmp(tempName, name))
            {
                recommendedLength = dp->rec_len;

                if(cp == buf && cp + recommendedLength == buf + BLKSIZE)
                {
                    // first entry in data block
                    bdalloc(dev, i);
                    tempParent->INODE.i_size -= BLKSIZE;
                    // loop through remaining blocks
                    for(int j = i; j < 12; j++)
                    {
                        get_block(dev, tempParent->INODE.i_block[j], tempBuf);
                        put_block(dev, tempParent->INODE.i_block[j+1], tempBuf);
                    }

                    tempParent->dirty = 1;
                    return 1;
                }

                // check if dir is at the end of the block
                if (cp + recommendedLength >= buf + BLKSIZE)
                {
                    cp -= previousRecommended;
                    dp = (DIR*)cp;
                    dp->rec_len += recommendedLength;

                    put_block(dev, tempParent->INODE.i_block[i], buf);
                    tempParent->dirty = 1;
                    return 1;
                }

                int originalLength = dp->rec_len;
                // dir is in middle of block
                while (cp < buf + BLKSIZE)
                {
                    cp += dp->rec_len;
                    dp = (DIR*)cp;
                    recommendedLength = dp->rec_len;

                    memcpy(tempCP, cp, recommendedLength);
                    
                    if (cp + recommendedLength >= buf + BLKSIZE)
                    {
                        dp = (DIR*)tempCP;
                        dp->rec_len += originalLength;
                        break;
                    }
                    tempCP += dp->rec_len;
                }

                put_block(dev, tempParent->INODE.i_block[i], buf);
                tempParent->dirty = 1;
                return 1;
            }

            previousRecommended = dp->rec_len;
            cp += dp->rec_len;
            tempCP += dp->rec_len;
            dp = (DIR*)cp;
        }
        return;
    }

}