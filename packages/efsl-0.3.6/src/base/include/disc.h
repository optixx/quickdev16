/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : disc.h                                                           *
* Release  : 0.3 - devel                                                      *
* Description : This file contains the functions regarding the whole disc     *
*               such as loading the MBR and performing read/write tests.      *
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

#ifndef __DISC_H_
#define __DISC_H_

/*****************************************************************************/
#include "config.h"
#include "types.h"
#include "extract.h"
#include "debug.h"
#include "error.h"
#include "interface.h"
#include "ioman.h"

/*****************************************************************************/

#define LBA_ADDR_MBR 0
#define PARTITION_TABLE_OFFSET 0x1BE

/**********************************************************\
           PartitionField
            ------------
* uchar 	type		Type of partition
* ulong		LBA_begin 	LBA address of first sector.
* ulong		numSectors	Number of 512byte sectors
This structure is a literal representation of a 16 byte
partitionfield. Direct I/O is possible.
\**********************************************************/
struct _PartitionField{
	euint8 bootFlag;
	euint8 CHS_begin[3];
	euint8 type;
	euint8 CHS_end[3];
	euint32 LBA_begin;
	euint32 numSectors;
};
typedef struct _PartitionField  PartitionField;

#define SIZE_PARTITION_FIELD 16

/***************************************************************************************\
              Disc     
               --      
* CompactFlash*	sourcedisc		Pointer to the hardwareobject that this disc is on.
* PartitionField* partitions	Array of PartitionFields, containing the partition info
\***************************************************************************************/
struct _Disc{
	IOManager *ioman;
	DISC_ERR_EUINT8
};
typedef struct _Disc Disc;

void disc_initDisc(Disc *disc,IOManager *ioman);

euint8 disc_findPartition(Disc *disc, euint8 partitionType, euint8 partitionIndex, euint32* startSector, euint32* sectorCount);

#include "sextract.h"

#endif
