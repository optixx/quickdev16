/*
mccl.c - Mad Catz Camera Link (Game Boy Camera) support for uCON64

Copyright (c) 2002 NoisyB <noisyb@gmx.net>


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

/*
This cable is made by Mad Catz, Inc. and has a Game Boy link connector on one
end and a parallel port connector on the other. It is designed to interface
with the Game Boy Camera cart and comes with included software for this. It
works by simulating the Game Boy Printer with a PIC chip inside the parallel
connector shell. It doesn't do a particularly good job at that so it pretty
much only works with the Game Boy Camera.

Mad Catz Camera Link Communications Protocol

Printer IO Ports:
Base+0: Data Port
Base+1: Status Port
Base+2: Control

Reset Procedure:
1. Output 0x24 to control (tristate data and set control to 0100)
2. Wait for bit 5 of status port to become 1
3. Read lower 4 bits of data port
4. If read data != 4, then go to step 1.
5. (Useless read of control port?)
6. Output 0x22 to control (tristate data and set control to 0010)
7. Wait for bit 5 of status port to become 0
8. Output 0x26 to control (tristate data and set control to 0110)

Data Read Procedure:
1. Output 0x26 to control (tristate data and set control to 0110)
2. Wait for bit 5 of status port to become 1
3. Read lower 4 bits of data port, store to lower 4 bits of received byte
4. (Useless read of control port?)
5. Output 0x22 to control (tristate data and set control to 0010)
6. Wait for bit 5 of status port to become 0
7. Output 0x26 to control (tristate data and set control to 0110)
8. Wait for bit 5 of status port to become 1
9. Read lower 4 bits of data port, store to upper 4 bits of received byte
10. (Useless read of control port?)
11. Output 0x22 to control (tristate data and set control to 0010)
12. Wait for bit 5 of status port to become 0
13. Go to step 1
*/
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "misc/misc.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "mccl.h"
#include "misc/parallel.h"


const st_getopt2_t mccl_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Mad Catz Camera Link (Game Boy Camera)"/*"XXXX Mad Catz Inc. http://www.madcatz.com"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xmccl", 0, 0, UCON64_XMCCL,
      NULL, "receives from Mad Catz Camera Link; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_GB_DEFAULT_STOP_NO_ROM]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef USE_PARALLEL

#define DATA ((unsigned short) (parport + PARPORT_DATA))
#define STATUS ((unsigned short) (parport + PARPORT_STATUS))
#define CONTROL ((unsigned short) (parport + PARPORT_CONTROL))


int
mccl_read (const char *filename, unsigned int parport)
{
  unsigned char buffer[0x1760];
  char dest_name[FILENAME_MAX];
  int inbyte, count = 0;
  time_t starttime;

  parport_print_info ();
  puts ("Resetting device");
  do
    {
      outportb (CONTROL, 0x24);
      while ((inportb (STATUS) & 0x20) == 0)
        ;
    }
  while ((inportw (DATA) & 0xf) != 4);
  outportb (CONTROL, 0x22);
  while ((inportb (STATUS) & 0x20) != 0)
    ;
  outportb (CONTROL, 0x26);

  printf ("Receive: %d Bytes (%.4f Mb)\n\n", 0x1760, (float) 0x1760 / MBIT);
  starttime = time (NULL);
  do
    {
      outportb (CONTROL, 0x26);
      while ((inportb (STATUS) & 0x20) == 0)
        ;
      inbyte = inportw (DATA) & 0xf;
      outportb (CONTROL, 0x22);
      while ((inportb (STATUS) & 0x20) != 0)
        ;
      outportb (CONTROL, 0x26);
      while ((inportb (STATUS) & 0x20) == 0)
        ;
      inbyte |= (inportw (DATA) & 0xf) << 4;
      outportb (CONTROL, 0x22);
      while ((inportb (STATUS) & 0x20) != 0)
        ;
      buffer[count++] = inbyte;
      if ((count & 0x1f) == 0)
        ucon64_gauge (starttime, count, 0x1760);
    }
  while (count < 0x1760);

  strcpy (dest_name, filename);
  ucon64_file_handler (dest_name, NULL, 0);
  ucon64_fwrite (buffer, 0, count, dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}

#endif // USE_PARALLEL
