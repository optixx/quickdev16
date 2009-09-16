/*
dex.c - DexDrive support for uCON64

Copyright (c) 2002 NoisyB <noisyb@gmx.net>
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
#include "misc/misc.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "dex.h"
#include "psxpblib.h"
#include "misc/parallel.h"


const st_getopt2_t dex_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "DexDrive"/*"19XX InterAct http://www.dexdrive.de"*/,
      NULL
    },
    {
      "xdex", 1, 0, UCON64_XDEX,
      "N", "send/receive Block N to/from DexDrive; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT_STOP_NO_ROM]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef USE_PARALLEL

#define CONPORT 1
#define TAP 1
#define DELAY 4
#define FRAME_SIZE 128
#define BLOCK_SIZE (64*FRAME_SIZE)

static int print_data;


static unsigned char *
read_block (int block_num)
{
  return psx_memcard_read_block (print_data, CONPORT, TAP, DELAY, block_num);
}


static int
write_block (int block_num, unsigned char *data)
{
  return psx_memcard_write_block (print_data, CONPORT, TAP, DELAY, block_num,
                                  data);
}


#if 0
char *
read_frame (int frame, char *data)
{
  return psx_memcard_read_frame (print_data, CONPORT, TAP, DELAY, frame);
}


int
write_frame (int frame, char *data)
{
  return psx_memcard_write_frame (print_data, CONPORT, TAP, DELAY, frame,
                                  data);
}
#endif


int
dex_read_block (const char *filename, int block_num, unsigned int parport)
{
  unsigned char *data;

  print_data = parport;
  parport_print_info ();

  if ((data = read_block (block_num)) == NULL)
    {
      fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
      exit (1);
    }

  ucon64_fwrite (data, 0, BLOCK_SIZE, filename, "wb");

  return 0;
}


int
dex_write_block (const char *filename, int block_num, unsigned int parport)
{
  unsigned char data[BLOCK_SIZE];

  print_data = parport;
  parport_print_info ();

  ucon64_fread (data, 0, BLOCK_SIZE, filename);

  if (write_block (block_num, data) == -1)
    {
      fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
      exit (1);
    }

  return 0;
}

#endif // USE_PARALLEL
