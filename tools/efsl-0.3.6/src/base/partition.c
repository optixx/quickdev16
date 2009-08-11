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

/*****************************************************************************/
#include "partition.h"
/*****************************************************************************/

/* ****************************************************************************  
 * void part_initPartition(Partition *part,Disc* refDisc)
 * Description: This function searches the 4 partitions for a FAT class partition
 * and marks the first one found as the active to be used partition.
*/
void part_initPartition(Partition *part,Disc* refDisc)
{
	eint16 c;
	
	part->disc=refDisc;
	part->LBA_offset=0;
	part->LBA_sectorcount=0;
	part_setError(part,PART_NOERROR);
}
/*****************************************************************************/ 

euint8 part_openPartitionType(Partition *part,euint8 type)
{
	return(
		disc_findPartition(part->disc,type,0,&(part->LBA_offset),&(part->LBA_sectorcount))
	);
}

/* ****************************************************************************  
 * euint8* part_getSect(Partition *part, euint32 address, euint8 mode)
 * Description: This function calls ioman_getSector, but recalculates the sector
 * address to be partition relative.
 * Return value: Whatever getSector returns. (pointer or 0)
*/
euint8* part_getSect(Partition *part, euint32 address, euint8 mode)
{
	return(ioman_getSector(part->disc->ioman,part_getRealLBA(part,address),mode));
}

/* ****************************************************************************  
 * esint8 part_relSect(Partition *part, euint8* buf)
 * Description: This function calls ioman_releaseSector.
 * Return value: Whatever releaseSector returns.
*/
esint8 part_relSect(Partition *part, euint8* buf)
{
	return(ioman_releaseSector(part->disc->ioman,buf));
}

esint8 part_flushPart(Partition *part,euint32 addr_l, euint32 addr_h)
{
	return( 
		ioman_flushRange(part->disc->ioman,part_getRealLBA(part,addr_l),part_getRealLBA(part,addr_h)) 
	);	
}

esint8 part_directSectorRead(Partition *part,euint32 address, euint8* buf)
{
	return(
		ioman_directSectorRead(part->disc->ioman,part_getRealLBA(part,address),buf)
	);
}

esint8 part_directSectorWrite(Partition *part,euint32 address, euint8* buf)
{
	return(
		ioman_directSectorWrite(part->disc->ioman,part_getRealLBA(part,address),buf)
	);
}


