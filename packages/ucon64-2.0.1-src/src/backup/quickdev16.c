/*
quickdev16.c - Quickdev16 support for uCON64

Copyright (c) 2009 david@optixx.org
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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <time.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "misc/archive.h"
#include "misc/itypes.h"
#include "misc/misc.h"
#include "misc/term.h"
#include "misc/usb.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "console/snes.h"
#include "backup/quickdev16.h"
#include "backup/swc.h"


#ifdef  USE_USB
static st_ucon64_obj_t quickdev16_obj[] =
  {
    {UCON64_SNES, WF_DEFAULT | WF_STOP | WF_NO_SPLIT}
  };
#endif

const st_getopt2_t quickdev16_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Quickdev16"/* http://www.optixx.org */,
      NULL
    },
#ifdef  USE_USB
    {
      "xqd16", 0, 0, UCON64_XQD16,              // send only
      NULL, "send ROM to Quickdev16",
      &quickdev16_obj[0]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_USB

#if 0 // unused
#define USB_UPLOAD_INIT      0
#define USB_UPLOAD_ADDR      1
#define USB_DOWNLOAD_INIT    2
#define USB_DOWNLOAD_ADDR    3
#define USB_CRC              4
#define USB_CRC_ADDR         5
#endif

#define USB_BULK_UPLOAD_INIT 6
#define USB_BULK_UPLOAD_ADDR 7
#define USB_BULK_UPLOAD_NEXT 8
#define USB_BULK_UPLOAD_END  9
#define USB_MODE_SNES        10
#define USB_MODE_AVR         11

#if 0 // unused
#define USB_AVR_RESET        12
#endif

#define READ_BUFFER_SIZE 8192
#define SEND_BUFFER_SIZE 128
#define SNES_HIROM_SHIFT 16
#define SNES_LOROM_SHIFT 15


static int check_quit (void)
{
  return (!ucon64.frontend ? kbhit () : 0) && getch () == 'q';
}


int
quickdev16_write_rom (const char *filename)
{
  int vendor_id = 0x16c0, product_id = 0x05dd, size, bytesread, quit = 0,
      numbytes, bytessent = 0, offset = 0;
  char vendor[] = "optixx.org", product[] = "QUICKDEV16", *buffer;
  usb_dev_handle *handle = NULL;
  FILE *file;
  time_t starttime;
  uint32_t bank_size, address = 0;
  uint16_t bank_shift;

#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  init_conio ();
  if (register_func (deinit_conio) == -1)
    {
      fputs ("ERROR: Could not register function with register_func()\n", stderr);
      exit (1);
    }
#endif

  usb_init ();
  switch (usbport_open (&handle, vendor_id, vendor, product_id, product))
    {
    case USBOPEN_SUCCESS:
      printf ("Opened USB device \"%s\" with vendor ID 0x%04x and product ID 0x%04x\n",
              product, vendor_id, product_id);
      break;
    case USBOPEN_ERR_ACCESS:
      fprintf (stderr,
               "ERROR: Could not access USB device \"%s\" with vendor ID 0x%04x and\n"
               "       product ID 0x%04x\n",
               product, vendor_id, product_id);
      exit (1);
      break;
    case USBOPEN_ERR_NOTFOUND:
      fprintf (stderr,
               "ERROR: Could not find USB device \"%s\" with vendor ID 0x%04x and\n"
               "       product ID 0x%04x\n",
               product, vendor_id, product_id);
      exit (1);
      break;
    }

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  if ((buffer = (char *) malloc (READ_BUFFER_SIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], READ_BUFFER_SIZE);
      exit (1);
    }

  if (snes_get_snes_hirom ())
    {
      bank_shift = SNES_HIROM_SHIFT;
      bank_size = 1 << SNES_HIROM_SHIFT;
    }
  else
    {
      bank_shift = SNES_LOROM_SHIFT;
      bank_size = 1 << SNES_LOROM_SHIFT;
    }

  size = ucon64.file_size - SWC_HEADER_LEN;
  fseek (file, SWC_HEADER_LEN, SEEK_SET);

  printf ("Send: %d Bytes\n", size);
  puts ("Press q to abort\n");
  starttime = time (NULL);

  usb_control_msg (handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                   USB_MODE_AVR, 0, 0, NULL, 0, 5000);
  // wait for the loader to depack
  wait2 (500);
  usb_control_msg (handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                   USB_BULK_UPLOAD_INIT, bank_shift, size / bank_size, NULL, 0, 5000);

  while ((bytesread = fread (buffer, 1, READ_BUFFER_SIZE, file)) > 0)
    {
      offset = 0;
      while (offset < bytesread && (quit = check_quit ()) == 0)
        {
          numbytes = usb_control_msg (handle,
                                      USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                                      address ? USB_BULK_UPLOAD_NEXT : USB_BULK_UPLOAD_ADDR,
                                      (address >> 16) & 0x00ff, address & 0xffff,
                                      buffer + offset,
                                      bytesread - offset >= SEND_BUFFER_SIZE ?
                                        SEND_BUFFER_SIZE : bytesread - offset, 5000);
          if (numbytes < 0)
            {
              fprintf (stderr, "\nERROR: USB error: %s\n", usb_strerror ());
              usb_close (handle);
              exit (1);
            }

          bytessent += numbytes;
          ucon64_gauge (starttime, bytessent, size);

          address += numbytes;
          offset += numbytes;
        }
      if (quit)
        {
          fputs ("\nTransfer aborted", stdout);
          break;
        }
    }

  usb_control_msg (handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                   USB_BULK_UPLOAD_END, 0, 0, NULL, 0, 5000);
  usb_control_msg (handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
                   USB_MODE_SNES, 0, 0, NULL, 0, 5000);

  free (buffer);
  fclose (file);

  return 0;
}

#endif // USE_USB
