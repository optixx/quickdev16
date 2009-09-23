/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : partition.c                                                      *
* Release  : 0.3 - devel                                                      *
* Description : These functions are partition specific. Searching FAT type    *
*               partitions and read/write functions to partitions.            *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2006 Lennart Yseboodt *
*                                                    (c)2006 Michael De Nil   *
\*****************************************************************************/

#ifndef __PARTITION_H__
#define __PARTITION_H__

/*****************************************************************************/
#include "config.h"
#include "error.h"
#include "interface.h"
#include "disc.h"
#include "types.h"
/*****************************************************************************/

#define PT_EMPTY  		0x00
#define PT_FAT12  		0x01
#define PT_FAT16A 		0x04
#define PT_EXTENDED		0x05
#define PT_FAT16  		0x06
#define PT_FAT32  		0x0B
#define PT_FAT32A 		0x5C
#define PT_FAT16B 		0x5E

/*************************************************************************************\
              Partition
               -------
* Disc*		disc				Pointer to disc containing this partition.
* eint8		activePartition	 	Array subscript for disc->partitions[activePartition]
\*************************************************************************************/
struct _Partition{
	Disc *disc;
 	euint32 LBA_offset;
 	euint32 LBA_sectorcount;
};
typedef struct _Partition Partition;

void part_initPartition(Partition *part,Disc* refDisc);

euint8* part_getSect(Partition *part, euint32 address,euint8 mode);
esint8 part_relSect(Partition *part, euint8* buf);

esint8 part_flushPart(Partition *part,euint32 addr_l, euint32 addr_h);

esint8 part_directSectorRead(Partition *part, euint32 address, euint8* buf);
esint8 part_directSectorWrite(Partition *part, euint32 address, euint8* buf);

#include "extract.h"

#endif
