/*
msg.c - Magic Super Griffin support for uCON64

Copyright (c) 2003 dbjh


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
#include <stdlib.h>
#include "misc/archive.h"
#include "misc/misc.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "backup/ffe.h"
#include "backup/msg.h"


#ifdef  USE_PARALLEL
static st_ucon64_obj_t msg_obj[] =
  {
    {UCON64_PCE, WF_DEFAULT | WF_STOP | WF_NO_SPLIT | WF_NO_ROM}
  };
#endif

const st_getopt2_t msg_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Magic Super Griffin/MSG"/*"1993/1994/1995/19XX Front Far East/FFE http://www.front.com.tw"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xmsg", 0, 0, UCON64_XMSG,
      NULL, "send/receive ROM to/from Magic Super Griffin/MSG; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when ROM does not exist",
      &msg_obj[0]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

#define BUFFERSIZE 8192                         // don't change, only 8192 works!


static void set_header (unsigned char *buffer);
static int check (unsigned char *info_block, int index1, int index2, int size);


#if BUFFERSIZE < 512
#error receive_rom_info() expects BUFFERSIZE to be at least 512 bytes.
#endif
static void
set_header (unsigned char *buffer)
{
  unsigned short n;
  unsigned char m = 0, sizes[] = "\x10\x20\x30\x30\x40\x40\x60\x80";

  for (n = 0; n < 128; n++)
    {
      ffe_send_command (5, n, 0);
      buffer[n] = ffe_send_command1 (0xa0a0);
      wait2 (1);
      if (buffer[n] != 0xff)
        m = 1;
    }
  if (m == 0)
    {                                           // no cartridge in Magic Super Griffin
      buffer[0] = 0;
      return;
    }

  m = 0;
  if (!check (buffer, 0, 0x40, 0x20))
    m |= 2;
  if (check (buffer, 0, 0x20, 0x20))
    {
      if (!check (buffer, 0, 0x10, 0x10))
        m |= 1;
    }
  else
    {
      m |= 4;
      if (!check (buffer, 0x40, 0x60, 0x20))
        m |= 1;
    }

  memset (buffer, 0, MSG_HEADER_LEN);
  buffer[0] = sizes[m];
  if (buffer[0] == 0x30)
    buffer[1] = 1;
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 2;
}


static int
check (unsigned char *info_block, int index1, int index2, int size)
{
  int n;

  for (n = 0; n < size; n++)
    if (info_block[n + index1] != info_block[n + index2])
      return 0;

  return 1;
}


int
msg_read_rom (const char *filename, unsigned short parport)
{
  FILE *file;
  unsigned char *buffer, blocksleft, emu_mode_select;
  int size, bytesreceived = 0;
  time_t starttime;
  unsigned short blocksdone = 0;

  ffe_init_io (parport);

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  set_header (buffer);
  if (buffer[0] == 0)
    {
      fprintf (stderr, "ERROR: There is no cartridge present in the Magic Super Griffin\n");
      fclose (file);
      remove (filename);
      exit (1);
    }
  blocksleft = buffer[0];
  size = buffer[0] * 8192;
  printf ("Receive: %d Bytes (%.4f Mb)\n", size, (float) size / MBIT);

  fwrite (buffer, 1, MSG_HEADER_LEN, file);     // write header
  emu_mode_select = buffer[1];

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xbff0, 0);

  printf ("Press q to abort\n\n");

  starttime = time (NULL);
  while (blocksleft > 0)
    {
      ffe_send_command (5, blocksdone, 0);
      if (emu_mode_select && blocksdone >= 32)
        ffe_send_command (5, blocksdone + 32, 0);
      ffe_receive_block (0xa000, buffer, BUFFERSIZE);
      // vgs aborts if the checksum doesn't match the data, we let the user decide
      blocksleft--;
      blocksdone++;
      fwrite (buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, size);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);

  return 0;
}


int
msg_write_rom (const char *filename, unsigned short parport)
{
  FILE *file;
  unsigned char *buffer, emu_mode_select;
  int bytesread, bytessent = 0, size;
  time_t starttime;
  unsigned short blocksdone = 0;

  ffe_init_io (parport);

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

  size = ucon64.file_size - MSG_HEADER_LEN;
  printf ("Send: %d Bytes (%.4f Mb)\n", size, (float) size / MBIT);

  fread (buffer, 1, MSG_HEADER_LEN, file);
  emu_mode_select = buffer[1];                  // this byte is needed later

  ffe_send_command0 (0xe008, 0);
  printf ("Press q to abort\n\n");

  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)) != 0)
    {
      ffe_send_command (5, blocksdone, 0);
      ffe_send_block (0x8000, buffer, (unsigned short) bytesread);
      blocksdone++;

      bytessent += bytesread;
      ucon64_gauge (starttime, bytessent, size);
      ffe_checkabort (2);
    }

  if (emu_mode_select & 1)
    ffe_send_command (4, 0xff00, 0);
  else
    ffe_send_command (4, 0xff03, 0);

  free (buffer);
  fclose (file);

  return 0;
}

#endif // USE_PARALLEL
