/*
usb.c - USB support

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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef  USE_USB
#include <stdio.h>
#include <usb.h>
#include "usb.h"
#include "misc.h"


usb_dev_handle *
usbport_open (struct usb_device *device)
{
  usb_dev_handle *handle = usb_open (device);
  return handle;
}


int
usbport_close (usb_dev_handle *handle)
{
  usb_release_interface (handle, 0);
  return usb_close (handle);
}


int
usbport_read (usb_dev_handle *handle, char *buffer, int buffer_size)
{
  int result;

  result = usb_bulk_read (handle, 0x83, buffer, buffer_size, 20000);
  if (result == -1)
    {
      fprintf (stderr, "ERROR: Could not read requested number of bytes read from USB\n"
                       "       %s\n", usb_strerror ());
      return -1;
    }
  return result;
}


int
usbport_write (usb_dev_handle *handle, char *buffer, int buffer_size)
{
  int result;

  result = usb_bulk_write (handle, 0x4, buffer, buffer_size, 20000);
  if (result == -1)
    {
      fprintf (stderr, "ERROR: Could not write requested number of bytes read to USB\n"
                       "       %s\n", usb_strerror ());
      return -1;
    }
  return result;
}


struct usb_device *
usbport_probe (int vendor_id, int product_id)
{
  struct usb_bus *bus;
  struct usb_device *dev;

  usb_init ();
  usb_find_busses ();
  usb_find_devices ();

  for (bus = usb_busses; bus; bus = bus->next)  // usb_busses is present in libusb
    for (dev = bus->devices; dev; dev = dev->next)
      if ((dev->descriptor.idVendor == vendor_id) &&
          (dev->descriptor.idProduct == product_id))
        return dev;

  return NULL;
}

#endif // USE_USB
