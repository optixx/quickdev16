
#include "integer.h"
#include "diskio.h"
#include "config.h"
#include "data.h"

static 
DSTATUS Stat = STA_NOINIT;  /* Disk status */


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

#include <string.h>

#define IMAGE_NAME "disk00.vfat"

BYTE  *image_addr;

DSTATUS disk_initialize (BYTE drv) {
    
    byte retval;
    if (drv) return STA_NOINIT;             /* Supports only single drive */


    Stat |= STA_NOINIT;
    *(byte*) MMIO_RETVAL = STA_VOID;
    *(byte*) MMIO_CMD = CMD_INIT;    
    while(*(byte*) MMIO_RETVAL == STA_VOID);
    retval = *(byte*) MMIO_RETVAL;
    Stat &= ~retval;                    /* When device goes ready, clear STA_NOINIT */
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
    DWORD offset;
    INT size;
    byte retval;
    word i;
    for (i=0;i<(count*512);i++)
        *(byte*)(SHARED_ADDR+i) = buff[i];
    
    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    *(byte*) MMIO_RETVAL = STA_VOID;
    *(byte*) MMIO_CMD = CMD_READ;    
    
    *(byte*) MMIO_SECTOR01 = (sector >> 24) & 0xff;    
    *(byte*) MMIO_SECTOR02 = (sector >> 16) & 0xff;    
    *(byte*) MMIO_SECTOR03 = (sector >> 8) & 0xff;
    *(byte*) MMIO_SECTOR04 = (sector) & 0xff;
    
    *(byte*) MMIO_COUNT = count;    
    
    while(*(byte*) MMIO_RETVAL == STA_VOID);
    retval = *(byte*) MMIO_RETVAL;
    

    return retval;
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
    DWORD offset;
    INT size;

    if (drv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    offset = sector  * 512;
    size = count * 512;
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
    BYTE n, w, ofs, dl, dh, *ptr = (BYTE*)buff;


    if (drv) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (ctrl) {
        case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
            ofs = 60; w = 2; n = 0;
            break;

        case GET_SECTOR_SIZE :  /* Get sectors on the disk (WORD) */
            *(WORD*)buff = 512;
            return RES_OK;

        case GET_BLOCK_SIZE :   /* Get erase block size in sectors (DWORD) */
            *(DWORD*)buff = 32;
            return RES_OK;

        default:
            return RES_PARERR;
    }
    return RES_OK;
}
#endif /* _USE_IOCTL != 0 */




