/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : atmega_spi.h                                                     *
* Release  : 0.3 - devel                                                      *
* Description : This file contains the functions needed to use efs for        *
*               accessing files on an SD-card connected to an ATMega's.       *
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

#ifndef __ATMEGA_SPI_H__ 
#define __ATMEGA_SPI_H__ 

#define __AVR_ATmega128__ 1
#define FALSE	0X00
#define TRUE	0x01

#define DDR_SPI DDRB
#define DD_MOSI DDB2
#define DD_SCK  DDB1

#include <avr/io.h>
#include <compat/ina90.h>
#include "debug.h"
#include "config.h"


/*************************************************************\
              hwInterface
               ----------
* long		sectorCount		Number of sectors on the file.
* 
* Remark: Configuration of the spi-port should become part of 
* this configuration.
\*************************************************************/
struct _atmegaSpiInterface{
	/*euint8 portSelect;*/ /* TODO */
	euint8 pinSelect;	
};
typedef struct _atmegaSpiInterface atmegaSpiInterface;

euint8 atmega_spi_init(atmegaSpiInterface *iface);
euint8 atmega_spi_send(atmegaSpiInterface *iface, euint8 data);

#endif
