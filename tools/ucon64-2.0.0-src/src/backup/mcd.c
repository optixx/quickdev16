/*
mcd.c - Mike Pavone's Genesis/Sega CD transfer cable support for uCON64

Copyright (c) 2004 dbjh


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
#include "misc/misc.h"
#include "misc/parallel.h"
#include "misc/itypes.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "mcd.h"


const st_getopt2_t mcd_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Mike Pavone's Genesis/Sega CD transfer cable",
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xmcd", 0, 0, UCON64_XMCD,
      NULL, "receive ROM from Genesis/Sega CD; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_GEN_DEFAULT_STOP_NO_SPLIT_NO_ROM]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

#define BUFFERSIZE 8192


static void
checkabort (int status)
{
  if (((!ucon64.frontend) ? kbhit () : 0) && getch () == 'q')
    {
      puts ("\nProgram aborted");
      exit (status);
    }
}


static void
read_block (unsigned char *buffer, int size, unsigned short parport)
{
  int i;

  for (i = 0; i < size; i++)
    {
      while (inportb ((unsigned short) (parport + PARPORT_STATUS)) & 0x08)
        ;
      outportb ((unsigned short) (parport + PARPORT_CONTROL), 0xa2);

      while (!(inportb ((unsigned short) (parport + PARPORT_STATUS)) & 0x08))
        ;
      buffer[i] = inportb ((unsigned short) (parport + PARPORT_DATA)) & 0x0f;
      outportb ((unsigned short) (parport + PARPORT_CONTROL), 0xa0);

      while (inportb ((unsigned short) (parport + PARPORT_STATUS)) & 0x08)
        ;
      outportb ((unsigned short) (parport + PARPORT_CONTROL), 0xa2);

      while (!(inportb ((unsigned short) (parport + PARPORT_STATUS)) & 0x08))
        ;
      buffer[i] |=
        (inportb ((unsigned short) (parport + PARPORT_DATA)) & 0x0f) << 4;
      outportb ((unsigned short) (parport + PARPORT_CONTROL), 0xa0);
    }
}


int
mcd_read_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char buffer[BUFFERSIZE];
  int n_bytes = 0, size;
  time_t starttime;

#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  init_conio ();
#endif
  parport_print_info ();

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }

  read_block (buffer, 1, (unsigned short) parport);
  size = (buffer[0] + 1) * 64 * 1024;
  printf ("Receive: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);
  puts ("Press q to abort\n");

  starttime = time (NULL);
  while (n_bytes < size)
    {
      read_block (buffer, BUFFERSIZE, (unsigned short) parport);
      fwrite (buffer, 1, BUFFERSIZE, file);
      n_bytes += BUFFERSIZE;
      ucon64_gauge (starttime, n_bytes, size);
      checkabort (2);
    }

  fclose (file);
#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  deinit_conio ();
#endif

  return 0;
}

#endif // USE_PARALLEL
