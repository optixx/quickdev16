/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : sextract.c                                                       *
* Release  : 0.3 - devel                                                      *
* Description : The function in this file are to load and set the structures  *
*               as found on the disc, only those structures for the entire    *
*               library are located here, filesystem specific ones are in     *
*               that filesystems directory                                    *
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

#include "sextract.h"

void ex_setPartitionField(euint8* buf,PartitionField* pf)
{
	*(buf) = pf->bootFlag;
	*(buf+1) = pf->CHS_begin[0];
	*(buf+2) = pf->CHS_begin[1];
	*(buf+3) = pf->CHS_begin[2];
	*(buf+4) = pf->type;
	*(buf+5) = pf->CHS_end[0];
	*(buf+6) = pf->CHS_end[1]; 
	*(buf+7) = pf->CHS_end[2];
	ex_setb32(buf+8,pf->LBA_begin); 
	ex_setb32(buf+12,pf->numSectors); 
}
/*****************************************************************************/

void ex_getPartitionField(euint8* buf,PartitionField* pf)
{
	pf->bootFlag       = *(buf);
	pf->CHS_begin[0]   = *(buf + 1);
	pf->CHS_begin[1]   = *(buf + 2);
	pf->CHS_begin[2]   = *(buf + 3);
	pf->type           = *(buf + 4);
	pf->CHS_end[0]     = *(buf + 5);
	pf->CHS_end[1]     = *(buf + 6);
	pf->CHS_end[2]     = *(buf + 7);
	pf->LBA_begin      = ex_getb32(buf + 8);
	pf->numSectors     = ex_getb32(buf + 12);
}
/*****************************************************************************/



