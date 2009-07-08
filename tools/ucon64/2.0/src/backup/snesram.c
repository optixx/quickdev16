/*
snesram.c - Snesram support for uCON64

Copyright (c) 2009 david@optixx.org


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
#include <time.h>
#include <string.h>
#include <usb.h>                
#include "misc/misc.h"
#include "misc/itypes.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/file.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ffe.h"
#include "smc.h"
#include "snesram.h"

#include "../misc/opendevice.h"

const st_getopt2_t snesram_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Snesram"/* http://www.optixx.org */,
      NULL
    },
#ifdef  USE_USB
    {
      "xsnesram", 0, 0, UCON64_XSNESRAM, // send only
      NULL, "send ROM (in FFE format) to Snesram",
      &ucon64_wf[WF_OBJ_NES_DEFAULT_STOP_NO_SPLIT]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef USE_USB

#define READ_BUFFER_SIZE    8192
#define SEND_BUFFER_SIZE    128
#define SNES_HIROM_SHIFT    16
#define SNES_LOROM_SHIFT    15

int
snesram_write_rom (const char *filename)
{
  FILE *file;
  int bytesread, bytessend, size;
  time_t starttime;
  usb_dev_handle *handle = NULL;
  const unsigned char rawVid[2] = { USB_CFG_VENDOR_ID }, rawPid[2] = { USB_CFG_DEVICE_ID};
  char vendor[] = { USB_CFG_VENDOR_NAME, 0 }, product[] = { USB_CFG_DEVICE_NAME, 0};
  int cnt, vid, pid;
  uint8_t *read_buffer;
  uint32_t addr = 0;
  uint16_t addr_lo = 0;
  uint16_t addr_hi = 0;
  uint16_t step = 0;
  uint8_t bank = 0;
  uint8_t bank_cnt = 0;
  uint16_t bank_shift;
  uint32_t bank_size;
  uint32_t hirom;
  uint8_t byte = 0;

  
  usb_init();
  vid = rawVid[1] * 256 + rawVid[0];
  pid = rawPid[1] * 256 + rawPid[0];
  if (usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0) {
        fprintf(stderr,
                "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n",
                product, vid, pid);
        exit(1);
    }
    printf("Open USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid,pid);

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  if ((read_buffer = (unsigned char *) malloc (READ_BUFFER_SIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], READ_BUFFER_SIZE);
      exit (1);
    }

    if (UCON64_ISSET (ucon64.snes_hirom)) { 
        hirom = ucon64.snes_hirom ? 1 : 0;
    } else {
        fseek (file, 0x00ffd5, SEEK_SET);
        fread(&byte, 1, 1, file);
        hirom = ((byte & 1 && byte != 0x23) || byte == 0x3a) ? 1 : 0; // & 1 => 0x21, 0x31, 0x35
    }
    printf("Hirom: %i\n",hirom);
    if (hirom) {
        bank_shift = SNES_HIROM_SHIFT;
        bank_size = 1 << SNES_HIROM_SHIFT;
    } else {
        bank_shift = SNES_LOROM_SHIFT;
        bank_size = 1 << SNES_LOROM_SHIFT;
    }



  fseek (file, 0, SEEK_END);
  size = ftell (file);
  fseek (file, SMC_HEADER_LEN, SEEK_SET);
  size -= SMC_HEADER_LEN;
  bank_cnt = size / bank_size;

  printf ("Send: %d Bytes (%.4f Mb) Hirom: %s, Banks: %i\n", size, (float) size / MBIT, hirom ? "Yes" : "No", bank_cnt);
  bytessend = 0;
  printf ("Press q to abort\n\n");
  starttime = time (NULL);
  
  cnt = usb_control_msg(handle,
                      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                      USB_ENDPOINT_OUT, USB_MODE_AVR, 0, 0, NULL,
                      0, 5000);
  
 
  cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
        USB_BULK_UPLOAD_INIT, bank_shift , bank_cnt, NULL, 0, 5000);


   while ((bytesread = fread(read_buffer, READ_BUFFER_SIZE, 1, file)) > 0) {
      for (step = 0; step < READ_BUFFER_SIZE; step += SEND_BUFFER_SIZE) {

        addr_lo = addr & 0xffff;
        addr_hi = (addr >> 16) & 0x00ff;

        if (addr == 0){
            cnt = usb_control_msg(handle,
                                USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                                USB_ENDPOINT_OUT, USB_BULK_UPLOAD_ADDR, addr_hi,
                                addr_lo, (char *) read_buffer + step,
                                SEND_BUFFER_SIZE, 5000);
        } else {
            cnt = usb_control_msg(handle,
                                USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                                USB_ENDPOINT_OUT, USB_BULK_UPLOAD_NEXT, addr_hi,
                                addr_lo, (char *) read_buffer + step,
                                SEND_BUFFER_SIZE, 5000);
        }
        if (cnt < 0) {
            fprintf(stderr, "USB error: %s\n", usb_strerror());
            usb_close(handle);
            exit(-1);
        }

        bytessend += SEND_BUFFER_SIZE;
        ucon64_gauge (starttime, bytessend, size);

        addr += SEND_BUFFER_SIZE;
        if ( addr % bank_size == 0) {
            bank++;
        }
      }
  }
  cnt = usb_control_msg(handle,
                        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                        USB_ENDPOINT_OUT, USB_BULK_UPLOAD_END, 0, 0, NULL,
                        0, 5000);

#if 0
  bank = 0;
  fseek(fp, 0, SEEK_SET);
  while ((cnt = fread(read_buffer, READ_BUFFER_SIZE, 1, fp)) > 0) {
      printf ("bank=0x%02x crc=0x%04x\n", bank++,
          do_crc(read_buffer, READ_BUFFER_SIZE));
  }
  fclose(fp);
#endif
  cnt = usb_control_msg(handle,
                      USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                      USB_ENDPOINT_OUT, USB_MODE_SNES, 0, 0, NULL,
                      0, 5000);


  free (read_buffer);
  fclose (file);
  return 0;
}


#endif // USE_USB
