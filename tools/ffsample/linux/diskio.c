
#include "integer.h"
#include "diskio.h"

static volatile
DSTATUS Stat = STA_NOINIT;  /* Disk status */

<<<<<<< HEAD:tools/ffsample/linux/diskio.c
/*

sudo losetup /dev/loop0 disk00.vfat
sudo mkfs.vfat -f 2 -F 16 -v  /dev/loop0
mkfs.vfat 2.11 (12 Mar 2005)
Loop device does not match a floppy size, using default hd params
/dev/loop0 has 64 heads and 32 sectors per track,
logical sector size is 512,
using 0xf8 media descriptor, with 524288 sectors;
file system has 2 16-bit FATs and 8 sectors per cluster.
FAT size is 256 sectors, and provides 65467 clusters.
Root directory contains 512 slots.
Volume ID is 4a1aab3d, no volume label.


FAT type = 2
Bytes/Cluster = 4096
Number of FATs = 2
Root DIR entries = 512
Sectors/FAT = 256
Number of clusters = 65467
FAT start (lba) = 1
DIR start (lba,clustor) = 513
Data start (lba) = 545
Ok
disk_read: sector=513 count=1 addr=0xa8009800  size=512
scan_files ret
0 files, 0 bytes.
0 folders.
261868 KB total disk space.
147456 KB available.




Disk: /dev/disk1        geometry: 993/4/63 [250368 sectors]
Signature: 0xAA55
         Starting       Ending
 #: id  cyl  hd sec -  cyl  hd sec [     start -       size]
------------------------------------------------------------------------
 1: 0B    0   1   1 - 1023 254  63 [        63 -     250299] Win95 FAT-32
 2: 00    0   0   0 -    0   0   0 [         0 -          0] unused      
 3: 00    0   0   0 -    0   0   0 [         0 -          0] unused      
 4: 00    0   0   0 -    0   0   0 [         0 -          0] unused
*/
=======
>>>>>>> 660767c464c3d78d36436123eb2e717c3c85f6bc:tools/ffsample/linux/diskio.c

/* Interface
** Scratch Buffer
addr        3 byte            
size        1 byte

** Call Interface
cmd         1 byte
sector      4 bytes
count       1 byte
return      1 byte

** Commands
    * disk_init
    * disk_read
    * disk_write
*/ 

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#define IMAGE_NAME "disk00.vfat"

char  *image_addr;

DSTATUS disk_initialize (BYTE drv) {
    if (drv) return STA_NOINIT;             /* Supports only single drive */

    Stat |= STA_NOINIT;

    int fd = open(IMAGE_NAME, O_RDWR);
    if (fd == -1) {
   	    perror("Error opening file for writing");
   	    exit(EXIT_FAILURE);
      }

    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    printf("Open Image (size %i)\n",size);
    
    image_addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (image_addr == MAP_FAILED) {
	    close(fd);
	    perror("Error mmapping the file");
	    exit(EXIT_FAILURE);
    }
    
    Stat &= ~STA_NOINIT;                    /* When device goes ready, clear STA_NOINIT */
    return Stat;
}


/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE drv){
    if (drv) return STA_NOINIT;     /* Supports only single drive */
    return Stat;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE drv,       /* Physical drive nmuber (0) */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Sector number (LBA) */
    BYTE count      /* Sector count (1..255) */
)
{
    BYTE c, iord_l, iord_h;
    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    DWORD offset = sector  * 512;
    int size = count * 512;
    printf("disk_read: sector=%li count=%i addr=%p off=%li size=%i\n",sector,count,image_addr + offset,offset,size);
    memcpy(buff,image_addr + offset,size);
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
DRESULT disk_write (
    BYTE drv,           /* Physical drive nmuber (0) */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Sector number (LBA) */
    BYTE count          /* Sector count (1..255) */
)
{

    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    DWORD offset = sector  * 512;
    int size = count * 512;
    printf("disk_write: sector=%li count=%i addr=%p off=%li size=%i\n",sector,count,image_addr + offset,offset,size);
    memcpy(image_addr + offset,buff,size);
    return RES_OK;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL != 0
DRESULT disk_ioctl (
    BYTE drv,       /* Physical drive nmuber (0) */
    BYTE ctrl,      /* Control code */
    void *buff      /* Buffer to send/receive data block */
)
{
    BYTE n, w, ofs, dl, dh, *ptr = buff;


    if (drv) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (ctrl) {
        case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
            printf("disk_ioctl: GET_SECTOR_COUNT\n");
            ofs = 60; w = 2; n = 0;
            break;

        case GET_SECTOR_SIZE :  /* Get sectors on the disk (WORD) */
            printf("disk_ioctl: GET_SECTOR_SIZE\n");
            *(WORD*)buff = 512;
            return RES_OK;

        case GET_BLOCK_SIZE :   /* Get erase block size in sectors (DWORD) */
            printf("disk_ioctl: GET_BLOCK_SIZE\n");
            *(DWORD*)buff = 32;
            return RES_OK;

        default:
            return RES_PARERR;
    }
    return RES_OK;
}
#endif /* _USE_IOCTL != 0 */



