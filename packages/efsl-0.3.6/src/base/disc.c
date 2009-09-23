/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : disc.c                                                           *
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

/*****************************************************************************/
#include "disc.h"
/*****************************************************************************/

/* ****************************************************************************  
 * void disc_initDisc(Disc *disc,hcInterface* source)
 * Description: This initialises the disc by loading the MBR and setting the
 * pointer to the hardware object.
*/
void disc_initDisc(Disc *disc,IOManager* ioman)
{
	disc->ioman=ioman;
	disc_setError(disc,DISC_NOERROR);

}
/*****************************************************************************/ 


/* ****************************************************************************  
 * euint8 disc_findPartition(Disc *disc, euint8 partitionType, euint8 partitionIndex, euint32* startSector, euint32* sectorCount)
 * Description: Locates partition of type partitionType w/ index partitionIndex on the disc and returns
 * the startsector and the size to the caller.
*/
euint8 disc_findPartition(Disc *disc, euint8 partitionType, euint8 partitionIndex, euint32* startSector, euint32* sectorCount)
{
	/* For now we ignore the partitionIndex */
	/* Partition buffering is also not here for memory saving reasons.
	   Add it later with compile time option */

	euint8 c, *buf, currentIndex=0;
	PartitionField pf;

	/* Get the MBR */
	
	buf = ioman_getSector(disc->ioman,LBA_ADDR_MBR,IOM_MODE_READONLY|IOM_MODE_EXP_REQ);

	for(c=0;c<4;c++){
		ex_getPartitionField(buf+(c*SIZE_PARTITION_FIELD),&pf);
		if(pf.type==partitionType){
			if(partitionIndex==currentIndex){
				ioman_releaseSector(disc->ioman,buf);
				*startSector = pf.LBA_begin;
				*sectorCount = pf.numSectors;
				return(0);
			}else{
				currentIndex++;
			}
		}
	}	
	
	ioman_releaseSector(disc->ioman,buf);	
	return(1); /* Nothing found */
}



