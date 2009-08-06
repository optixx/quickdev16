/*
 */

#ifndef _FAT_H_
#define _FAT_H_
#include <string.h>
#include "mmc.h"
#include "uart.h"

#define FAT_DEBUG	uart_puts
    // #define FAT_DEBUG(...) 

    // Prototypes
extern uint16_t fat_root_dir_addr(uint8_t *);
extern uint16_t fat_read_dir_ent(uint16_t, uint8_t,
                                     uint32_t *, uint8_t *,
                                     uint8_t *);
extern void fat_load(uint16_t, uint32_t *, uint8_t *);
extern void fat_read_file(uint16_t, uint8_t *, uint32_t);
extern void fat_write_file(uint16_t, uint8_t *, uint32_t);
extern void fat_init(uint8_t * Buffer);
extern uint8_t fat_search_file(uint8_t *, uint16_t *,
                                     uint32_t *, uint8_t *,
                                     uint8_t *);

    // Block Size in Bytes
#define BlockSize			512

    // Master Boot Record
#define MASTER_BOOT_RECORD	0

    // Volume Boot Record location in Master Boot Record
#define VBR_ADDR 			0x1C6

    // define ASCII
#define SPACE 				0x20
#define DIR_ENTRY_IS_FREE   0xE5
#define FIRST_LONG_ENTRY	0x01
#define SECOND_LONG_ENTRY	0x42

    // define DIR_Attr
#define ATTR_LONG_NAME		0x0F
#define ATTR_READ_ONLY		0x01
#define ATTR_HIDDEN			0x02
#define ATTR_SYSTEM			0x04
#define ATTR_VOLUME_ID		0x08
#define ATTR_DIRECTORY		0x10
#define ATTR_ARCHIVE		0x20
struct BootSec {
    uint8_t BS_jmpBoot[3];
    uint8_t BS_OEMName[8];
    uint16_t BPB_BytesPerSec;       // 2 bytes
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;        // 2 bytes
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;        // 2 bytes
    uint16_t BPB_TotSec16;  // 2 bytes
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;   // 2 bytes
    uint16_t BPB_SecPerTrk; // 2 bytes
    uint16_t BPB_NumHeads;  // 2 bytes
    uint32_t BPB_HiddSec;  // 4 bytes
    uint32_t BPB_TotSec32; // 4 bytes
};

    // FAT12 and FAT16 Structure Starting at Offset 36
#define BS_DRVNUM			36
#define BS_RESERVED1		37
#define BS_BOOTSIG			38
#define BS_VOLID			39
#define BS_VOLLAB			43
#define BS_FILSYSTYPE		54

    // FAT32 Structure Starting at Offset 36
#define BPB_FATSZ32			36
#define BPB_EXTFLAGS		40
#define BPB_FSVER			42
#define BPB_ROOTCLUS		44
#define BPB_FSINFO			48
#define BPB_BKBOOTSEC		50
#define BPB_RESERVED		52

#define FAT32_BS_DRVNUM		64
#define FAT32_BS_RESERVED1	65
#define FAT32_BS_BOOTSIG	66
#define FAT32_BS_VOLID		67
#define FAT32_BS_VOLLAB		71
#define FAT32_BS_FILSYSTYPE	82
    // End of Boot Sctor and BPB Structure
struct DirEntry {
    uint8_t DIR_Name[11]; // 8 chars filename
    uint8_t DIR_Attr;     // file attributes RSHA, Longname, Drive Label, Directory
    uint8_t DIR_NTRes;    // set to zero
    uint8_t DIR_CrtTimeTenth;     // creation time part in milliseconds
    uint16_t DIR_CrtTime;   // creation time
    uint16_t DIR_CrtDate;   // creation date
    uint16_t DIR_LastAccDate;       // last access date
    uint16_t DIR_FstClusHI; // first cluster high word 
    uint16_t DIR_WrtTime;   // last write time
    uint16_t DIR_WrtDate;   // last write date
    uint16_t DIR_FstClusLO; // first cluster low word 
    uint32_t DIR_FileSize;
};

#endif                          // _FAT_H_
