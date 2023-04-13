# Final project for CPTS360 at Washington State University

A. OBJECTIVE:
   Design and implement a Linux-compatible EXT2 file system.

B. SPECIFICATIONS:
   Implementation of EXT2 File System: Chapter 11.6

1. Files:
   Files are exactly the same as they are in the Linux file system, i.e.
   we shall use the same EXT2 file system data structures, except:
      Only DIR and REG file types; no SPECIAL files (I/O devices).
   
2. Disks:
    Disks are "virtual disks" simulated by Linux files.  
    Disk I/O are by Linux read()/write() on a BLKSIZE basis.

3. File names:
   As in Unix/Linux, each file is identified by a pathname, /a/b/c or x/y/z.

   If a pathname begins with "/",  it's relative to the / directory.
   Otherwise, it's relative to the Current Working Directory (CWD) of the 
   running process.

4. File System Commands and Operations:
		    
   File operations will be executed as commands. The required commands are 
   listed below. LEVEL 1 is the MINIMUM requirements for passing the course.
    
              ------------  LEVEL 1 ------------ 
	       showblock mountroot, ls, cd, pwd
               mkdir, creat, rmdir, 
	       link,  unlink, symlink, readlink

	      ------------  LEVEl 2 -------------
	       open, close, read, write, cat, dd
	              permission checking
              -----------------------------------
      
All commands work exactly the same as they do in Unix/Linux.

Completed with Zach Griswold. To note: commit history is weird, since we combined our work at the end of the project. Files uploaded by 'maudan567' are written by Aidan Griffin. 
