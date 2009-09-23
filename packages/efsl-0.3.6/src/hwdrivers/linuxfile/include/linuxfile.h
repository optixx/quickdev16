/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : linuxfile.h                                                      *
* Release  : 0.3 - devel                                                      *
* Description : This file contains the functions needed to use efs for        *
*               accessing files under linux. This interface is meant          *
*               to be used for debugging purposes.                            *
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

#ifndef __LINUXFILE_H__
#define __LINUXFILE_H__

/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "helper.h"
#include "debug.h"
#include "types.h"
#include "ioctl.h"
#include "config.h"
/*****************************************************************************/

#define _LARGEFILE_SOURCE 
#define _GNU_SOURCE 
#define _FILE_OFFSET_BITS 64

/*************************************************************\
              hwInterface
               ----------
* FILE* 	imagefile		File emulation of hw interface.
* long		sectorCount		Number of sectors on the file.
\*************************************************************/
struct _linuxFileInterface{
	FILE 	*imageFile;
	euint8* fileName;
	eint32  sectorCount;
};
typedef struct _linuxFileInterface linuxFileInterface;

esint8 lf_init(void* LFI);
esint8 lf_readBuf(void* LFI,euint32 address,euint8* buf);
esint8 lf_writeBuf(void* LFI,euint32 address,euint8* buf);
esint8 lf_ioctl(void* LFI,euint16 ctl,void* data);

esint8 lf_setPos(void* LFI,euint32 address);

#endif
