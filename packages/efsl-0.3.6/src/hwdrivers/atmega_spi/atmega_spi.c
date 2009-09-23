/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : atmega_spi.c                                                     *
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

/*****************************************************************************/
#include "include/atmega_spi.h"
/*****************************************************************************/

euint8 atmega_spi_init(atmegaSpiInterface *iface)
{
	euint8 i;
	
	/* Unselect card */
	PORTB |= iface->pinSelect;
	
	/* Set as master, clock and chip select output */
	DDR_SPI = (1<<DD_MOSI) | (1<<DD_SCK) | 1;

	/* Enable SPI, master, set clock rate to fck/2 */
	SPCR = (1<<SPE) | (1<<MSTR); /* fsck / 4 */
	SPSR = 1; /* fsck / 2 */

	/* Send 10 spi commands with card not selected */
	for(i=0;i<10;i++)
		atmega_spi_send(iface,0xff);

	/* Select card */
	PORTB &= ~(iface->pinSelect);

	return(0);
}
/*****************************************************************************/

euint8 atmega_spi_send(atmegaSpiInterface *iface, euint8 data)
{
	euint8 incoming=0;
	
	PORTB &= ~(iface->pinSelect);
	
	SPDR = data;
	while(!(SPSR & (1<<SPIF)))
		incoming = SPDR;

	PORTB |= iface->pinSelect;

	return(incoming);
}
/*****************************************************************************/

