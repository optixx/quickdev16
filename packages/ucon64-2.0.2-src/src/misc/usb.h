/*
usb.h - USB support

Copyright (c) 2003 Ulrich Hecht <uli@emulinks.de>
Copyright (c) 2004 NoisyB
Copyright (c) 2015 dbjh


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef MISC_USB_H
#define MISC_USB_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  USE_USB
#if     defined _WIN32 || defined __CYGWIN__
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4255) // 'function' : no function prototype given: converting '()' to '(void)'
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <lusb0_usb.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#else
#include <usb.h>
#endif

#define USBOPEN_SUCCESS      0          // no error
#define USBOPEN_ERR_ACCESS   1          // not enough permissions to open device
#define USBOPEN_ERR_NOTFOUND 2          // device not found

extern int usbport_open (usb_dev_handle **result_handle, int vendor_id,
                         char *vendor_name, int product_id, char *product_name);
extern int usbport_close (usb_dev_handle *handle);
extern int usbport_read (usb_dev_handle *handle, char *buffer, int buffer_size);
extern int usbport_write (usb_dev_handle *handle, char *buffer, int buffer_size);
extern struct usb_device *usbport_probe (int vendor_id, int product_id);

#endif // USE_USB

#endif // MISC_USB_H
