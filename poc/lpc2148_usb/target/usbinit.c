/*
	LPCUSB, an USB device driver for LPC microcontrollers	
	Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/** @file
	USB stack initialisation
 */

#include "type.h"
#include "usbdebug.h"
#include "usbapi.h"


/** data storage area for standard requests */
static U8	abStdReqData[8];


/**
	USB reset handler
	
	@param [in] bDevStatus	Device status
 */
static void HandleUsbReset(U8 bDevStatus)
{
	if (bDevStatus & DEV_STATUS_RESET) {
		DBG("\n!");
	}
}


/**
	Initialises the USB hardware and sets up the USB stack by
	installing default callbacks.
	
	@return TRUE if initialisation was successful
 */
BOOL USBInit(void)
{
	// init hardware
	USBHwInit();
	
	// register bus reset handler
	USBHwRegisterDevIntHandler(HandleUsbReset);
	
	// register control transfer handler on EP0
	USBHwRegisterEPIntHandler(0x00, USBHandleControlTransfer);
	USBHwRegisterEPIntHandler(0x80, USBHandleControlTransfer);
	
	// register standard request handler
	USBRegisterRequestHandler(REQTYPE_TYPE_STANDARD, USBHandleStandardRequest, abStdReqData);

	return TRUE;
}

