/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : sd.h                                                             *
* Release  : 0.3 - devel                                                      *
* Description : This file contains the functions needed to use efs for        *
*               accessing files on an SD-card.                                *
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

#ifndef __SD_H__ 
#define __SD_H__
/*****************************************************************************/

#include "config.h"
#include "types.h"
#include "debug.h"
/*****************************************************************************/

#define CMDREAD     17
#define CMDWRITE    24
/*****************************************************************************/

struct _SdSpiProtocol
{
	void *spiHwInterface;
	euint8 (*spiHwInit)(void* spiHwInterface);
	euint8 (*spiSendByte)(void* spiHwInterface,euint8 data);
};
typedef struct _SdSpiProtocol SdSpiProtocol;


esint8  sd_Init(SdSpiProtocol *ssp);
void sd_Command(SdSpiProtocol *ssp,euint8 cmd, euint16 paramx, euint16 paramy);
euint8 sd_Resp8b(SdSpiProtocol *ssp);
void sd_Resp8bError(SdSpiProtocol *ssp,euint8 value);
euint16 sd_Resp16b(SdSpiProtocol *ssp);
esint8 sd_State(SdSpiProtocol *ssp);

esint8 sd_readSector(SdSpiProtocol *ssp,euint32 address,euint8* buf);
esint8 sd_writeSector(SdSpiProtocol *ssp,euint32 address, euint8* buf);
esint8 sd_ioctl(SdSpiProtocol* ssp,euint16 ctl,void* data);

#endif
