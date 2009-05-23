
#include "integer.h"
#include "diskio.h"

static volatile
DSTATUS Stat = STA_NOINIT;  /* Disk status */

/*
[david@slap]Transfer/ffsample/linux % sudo mkfs.vfat -F 32 -v disk00.vfat                                                                                                                                  [941]
mkfs.vfat 2.11 (12 Mar 2005)
disk00.vfat has 64 heads and 32 sectors per track,
logical sector size is 512,
using 0xf8 media descriptor, with 8192 sectors;
file system has 2 32-bit FATs and 1 sector per cluster.
FAT size is 63 sectors, and provides 8034 clusters.
Volume ID is 4a1424ec, no volume label.

filesize 4194304

*/

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

#define IMAGE_NAME "disk00.vfat"

int image_addr;  

DSTATUS disk_initialize (BYTE drv) {
    if (drv) return STA_NOINIT;             /* Supports only single drive */

    Stat |= STA_NOINIT;
    /* map image */



    int fd = open(IMAGE_NAME,);
    int size = fseek(END);
    fseek(0);
    
    image_addr = mmap(0,fd,)
    

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
    
    printf("disk_read: sector=%i count=%i\n",sector,count);

    DWORD offset = sector * 512;
    DWORD size = count * 512;
    
    printf("disk_read: addr=%p offset=%i size=%i\n",image_addr,offset,size);
    memcpy(buff,image_addr + offset,size);
    
    /* Issue Read Setor(s) command */
    /*
    write_ata(REG_COUNT, count);
    write_ata(REG_SECTOR, (BYTE)sector);
    write_ata(REG_CYLL, (BYTE)(sector >> 8));
    write_ata(REG_CYLH, (BYTE)(sector >> 16));
    write_ata(REG_DEV, ((BYTE)(sector >> 24) & 0x0F) | LBA);
    write_ata(REG_COMMAND, CMD_READ);
    */

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
    BYTE s, c, iowr_l, iowr_h;
    
    

    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    /* Issue Write Setor(s) command */
    /*
    write_ata(REG_COUNT, count);
    write_ata(REG_SECTOR, (BYTE)sector);
    write_ata(REG_CYLL, (BYTE)(sector >> 8));
    write_ata(REG_CYLH, (BYTE)(sector >> 16));
    write_ata(REG_DEV, ((BYTE)(sector >> 24) & 0x0F) | LBA);
    write_ata(REG_COMMAND, CMD_WRITE);
    */

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

        case CTRL_SYNC :        /* Nothing to do */
            printf("disk_ioctl: CTRL_SIZE\n");
            return RES_OK;

        case ATA_GET_REV :      /* Get firmware revision (8 chars) */
            printf("disk_ioctl: ATAL_GET_REV\n");
            ofs = 23; w = 4; n = 4;
            break;

        case ATA_GET_MODEL :    /* Get model name (40 chars) */
            printf("disk_ioctl: ATAL_GET_MODEL\n");
            ofs = 27; w = 20; n = 20;
            break;

        case ATA_GET_SN :       /* Get serial number (20 chars) */
            printf("disk_ioctl: ATAL_GET_SN\n");
            ofs = 10; w = 10; n = 10;
            break;

        default:
            return RES_PARERR;
    }

    /*
    write_ata(REG_COMMAND, CMD_IDENTIFY);
    if (!wait_data()) return RES_ERROR;
    read_part(ptr, ofs, w);
    while (n--) {
        dl = *ptr; dh = *(ptr+1);
        *ptr++ = dh; *ptr++ = dl;
    }
    */
    return RES_OK;
}
#endif /* _USE_IOCTL != 0 */




