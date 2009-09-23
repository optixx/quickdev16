/*#######################################################################################
Connect ARM to MMC/SD 

Copyright (C) 2004 Ulrich Radig
#######################################################################################*/

#ifndef _FAT_H_
 #define _FAT_H_

#include <string.h>
#include "mmc.h"
#include "usart.h"

#define FAT_DEBUG	usart_write 
//#define FAT_DEBUG(...)	


//Prototypes
extern unsigned int fat_root_dir_addr (unsigned char *);
extern unsigned int fat_read_dir_ent (unsigned int,unsigned char,unsigned long*,unsigned char *,unsigned char *);
extern void fat_load (unsigned int,unsigned long *,unsigned char *);
extern void fat_read_file (unsigned int,unsigned char *,unsigned long);
extern void fat_write_file (unsigned int,unsigned char *,unsigned long);
extern void fat_init (void);
extern unsigned char fat_search_file (unsigned char *,unsigned int *,unsigned long *,unsigned char *,unsigned char *);

//Block Size in Bytes
#define BlockSize			512

//Master Boot Record
#define MASTER_BOOT_RECORD	0

//Volume Boot Record location in Master Boot Record
#define VBR_ADDR 			0x1C6

//define ASCII
#define SPACE 				0x20
#define DIR_ENTRY_IS_FREE   0xE5
#define FIRST_LONG_ENTRY	0x01
#define SECOND_LONG_ENTRY	0x42

//define DIR_Attr
#define ATTR_LONG_NAME		0x0F
#define ATTR_READ_ONLY		0x01
#define ATTR_HIDDEN			0x02
#define ATTR_SYSTEM			0x04
#define ATTR_VOLUME_ID		0x08
#define ATTR_DIRECTORY		0x10
#define ATTR_ARCHIVE		0x20

struct BootSec 
{
	unsigned char BS_jmpBoot[3];
	unsigned char BS_OEMName[8];
	unsigned int BPB_BytesPerSec; //2 bytes
	unsigned char	BPB_SecPerClus;
	unsigned int	BPB_RsvdSecCnt; //2 bytes
	unsigned char	BPB_NumFATs;
	unsigned int	BPB_RootEntCnt; //2 bytes
	unsigned int	BPB_TotSec16; //2 bytes
	unsigned char	BPB_Media;
	unsigned int	BPB_FATSz16; //2 bytes
	unsigned int	BPB_SecPerTrk; //2 bytes
	unsigned int	BPB_NumHeads; //2 bytes
	unsigned long	BPB_HiddSec; //4 bytes
	unsigned long	BPB_TotSec32; //4 bytes
};

//FAT12 and FAT16 Structure Starting at Offset 36
#define BS_DRVNUM			36
#define BS_RESERVED1		37
#define BS_BOOTSIG			38
#define BS_VOLID			39
#define BS_VOLLAB			43
#define BS_FILSYSTYPE		54

//FAT32 Structure Starting at Offset 36
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
//End of Boot Sctor and BPB Structure

struct DirEntry {
	unsigned char	DIR_Name[11];     //8 chars filename
	unsigned char	DIR_Attr;         //file attributes RSHA, Longname, Drive Label, Directory
	unsigned char	DIR_NTRes;        //set to zero
	unsigned char	DIR_CrtTimeTenth; //creation time part in milliseconds
	unsigned int	DIR_CrtTime;      //creation time
	unsigned int	DIR_CrtDate;      //creation date
	unsigned int	DIR_LastAccDate;  //last access date
	unsigned int	DIR_FstClusHI;    //first cluster high word                 
	unsigned int	DIR_WrtTime;      //last write time
	unsigned int	DIR_WrtDate;      //last write date
	unsigned int	DIR_FstClusLO;    //first cluster low word                 
	unsigned long	DIR_FileSize;     
	};

#endif //_FAT_H_
