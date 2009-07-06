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

#define BUFFERSIZE 8192



int
snesram_write_rom (const char *filename)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend, size, offset, n_blocks1, n_blocks2, n_blocks3, n;
  time_t starttime;


  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  fread (buffer, 1, SMC_HEADER_LEN, file);


  size = (n_blocks1 + n_blocks2 + n_blocks3) * 8 * 1024 + 8 +
         (buffer[0] & SMC_TRAINER ? 512 : 0);
  printf ("Send: %d Bytes (%.4f Mb)\n", size, (float) size / MBIT);

  ffe_send_block (0x5020, buffer, 8);           // send "file control block"
  bytessend = 8;

  if (buffer[1] >> 5 > 4)
    offset = 12;
  else
    offset = 0;

  if (buffer[0] & SMC_TRAINER)                  // send trainer if present
    {
      fread (buffer, 1, 512, file);
      ffe_send_block (0x600, buffer, 512);
      bytessend += 512;
    }

  printf ("Press q to abort\n\n");
  starttime = time (NULL);

  for (n = 0; n < n_blocks1; n++)
    {
      ffe_send_command0 (0x4507, (unsigned char) (n + offset));
      if ((bytesread = fread (buffer, 1, BUFFERSIZE, file)) == 0)
        break;
      ffe_send_block (0x6000, buffer, bytesread);

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }

  for (n = 0; n < n_blocks2; n++)
    {
      ffe_send_command0 (0x4507, (unsigned char) (n + 32));
      if ((bytesread = fread (buffer, 1, BUFFERSIZE, file)) == 0)
        break;
      ffe_send_block (0x6000, buffer, bytesread);

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }

  ffe_send_command0 (0x2001, 0);

  for (n = 0; n < n_blocks3; n++)
    {
      if (n == 0)
        {
          ffe_send_command0 (0x4500, 0x22);
          ffe_send_command0 (0x42ff, 0x30);
          if ((bytesread = fread (buffer, 1, BUFFERSIZE, file)) == 0)
            break;
          ffe_send_block (0x6000, buffer, bytesread);
        }
      else
        {
          int m;

          ffe_send_command0 (0x4500, 7);
          for (m = 0; m < 8; m++)
            ffe_send_command0 ((unsigned short) (0x4510 + m), (unsigned char) (n * 8 + m));
          if ((bytesread = fread (buffer, 1, BUFFERSIZE, file)) == 0)
            break;
          ffe_send_block2 (0, buffer, bytesread);
        }

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }

  for (n = 0x4504; n < 0x4508; n++)
    ffe_send_command0 ((unsigned short) n, 0);
  for (n = 0x4510; n < 0x451c; n++)
    ffe_send_command0 ((unsigned short) n, 0);

  ffe_send_command (5, 1, 0);

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}

#endif // USE_USB
