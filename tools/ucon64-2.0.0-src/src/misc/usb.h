/*
usb.h - USB support

Copyright (c) 2003 Ulrich Hecht <uli@emulinks.de>
Copyright (c) 2004 NoisyB <noisyb@gmx.net>


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
#include <usb.h>

extern struct usb_device *usbport_probe (int vendor_id, int product_id);
extern usb_dev_handle *usbport_open (struct usb_device *device);
extern int usbport_close (usb_dev_handle *handle);
extern int usbport_read (usb_dev_handle *handle, char *buffer, int buffer_size);
extern int usbport_write (usb_dev_handle *handle, char *buffer, int buffer_size);

#endif // USE_USB
#endif // MISC_USB_H
