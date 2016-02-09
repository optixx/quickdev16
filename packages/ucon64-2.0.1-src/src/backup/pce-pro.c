/*
pce-pro.c - PCE-PRO flash card programmer support for uCON64

Copyright (c) 2004 - 2005 dbjh

Based on Delphi source code by ToToTEK Multi Media. Information in that source
code has been used with permission. However, ToToTEK Multi Media explicitly
stated that the information in that source code may be freely distributed.


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
#include <string.h>
#include "misc/archive.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "backup/tototek.h"
#include "backup/pce-pro.h"


#ifdef  USE_PARALLEL
static st_ucon64_obj_t pcepro_obj[] =
  {
    {UCON64_PCE, WF_DEFAULT | WF_STOP | WF_NO_SPLIT | WF_NO_ROM}
  };
#endif

const st_getopt2_t pcepro_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "PCE-PRO flash card programmer"/*"2004 ToToTEK Multi Media http://www.tototek.com"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xpce", 0, 0, UCON64_XPCE,
      NULL, "send/receive ROM to/from PCE-PRO flash card programmer\n" OPTION_LONG_S "port=PORT\n"
      "receives automatically (32 Mbits) when ROM does not exist",
      &pcepro_obj[0]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

static void eep_reset (void);
static unsigned short int check_card (void);
static void write_rom_by_byte (int *addr, unsigned char *buf);
static void write_rom_by_page (int *addr, unsigned char *buf);


static void
eep_reset (void)
{
  ttt_rom_enable ();
  ttt_write_mem (0x000000, 0xff);               // reset EEP
  ttt_write_mem (0x200000, 0xff);               // reset EEP
  ttt_rom_disable ();
}


static unsigned short int
check_card (void)
{
  unsigned short int id;

  eep_reset ();
  id = ttt_get_id ();
  if (id != 0xb0d0)
    {
      fprintf (stderr, "ERROR: PCE-PRO flash card (programmer) not detected (ID: 0x%02hx)\n", id);
      return 0;
    }
  else
    return id;
}


static void
write_rom_by_byte (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x4000; x++)
    {
      ttt_write_byte_sharp (*addr, buf[*addr & 0x3fff]);
      (*addr)++;
    }
}


static void
write_rom_by_page (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x200; x++)
    {
      ttt_write_page_rom (*addr, buf);
      (*addr) += 0x20;
    }
}


int
pce_read_rom (const char *filename, unsigned short parport, int size)
{
  FILE *file;
  unsigned char buffer[0x100];
  int blocksleft, address = 0;
  time_t starttime;
  void (*read_block) (int, unsigned char *) = ttt_read_rom_w; // ttt_read_rom_b

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  ttt_init_io (parport);

  printf ("Receive: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  if (check_card () == 0)
    {
      fclose (file);
      remove (filename);
      exit (1);
    }

  blocksleft = size >> 8;
  eep_reset ();
  ttt_rom_enable ();
  if (read_block == ttt_read_rom_w)
    ttt_set_ai_data (6, 0x94);          // rst=1, wei=0(dis.), rdi=0(dis.), inc mode, rom_CS
  starttime = time (NULL);
  while (blocksleft-- > 0)
    {
      read_block (address, buffer);             // 0x100 bytes read
      fwrite (buffer, 1, 0x100, file);
      address += 0x100;
      if ((address & 0x3fff) == 0)
        ucon64_gauge (starttime, address, size);
    }
  // original code doesn't call ttt_rom_disable() when byte-size function is
  //  used (ttt_read_rom_b() calls it)
  if (read_block == ttt_read_rom_w)
    ttt_rom_disable ();

  fclose (file);

  return 0;
}


int
pce_write_rom (const char *filename, unsigned short parport)
{
  FILE *file;
  unsigned char buffer[0x4000], game_table[32 * 0x20];
  int game_no, size, romsize = 0, startaddress, address = 0, bytesread,
      bytessent = 0, bytesleft, multi_game;
  time_t starttime;
  void (*write_block) (int *, unsigned char *) = write_rom_by_page; // write_rom_by_byte
  (void) write_rom_by_byte;

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  ttt_init_io (parport);

  fseek (file, 0xb3f4, SEEK_SET);
  buffer[0] = 0;
  fread (buffer, 1, 12, file);                  // it's OK to not verify if we can read
  // currently we ignore the version string (full string is "uCON64 2.0.1")
  multi_game = strncmp ((char *) buffer, "uCON64", 6) ? 0 : 1;

  if (multi_game)
    {
      fseek (file, 0xb000, SEEK_SET);
      bytesread = fread (game_table, 1, 32 * 0x20, file);
      if (bytesread != 32 * 0x20)
        {
          fputs ("ERROR: Could not read game table from file\n", stderr);
          fclose (file);
          return -1;
        }
    }

  size = ucon64.file_size;
  // 4 Mbit games need the last 2 Mbit to be mirrored (so, they need 6 Mbit)
  if (multi_game)
    {
      game_no = 0;
      while (game_table[game_no * 0x20] && game_no < 31)
        {
          if (game_table[game_no * 0x20 + 0x1e] == 4)
            size += 2 * MBIT;
          game_no++;
        }
    }
  else
    {
      romsize = size;                           // one file (no multi-game)
      if (size == 4 * MBIT)
        size += 2 * MBIT;
    }
  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  if (check_card () == 0)
    {
      fclose (file);
      exit (1);
    }

  fseek (file, 0, SEEK_SET);

  starttime = time (NULL);
  eep_reset ();
  game_no = -1;
  do
    {
      if (game_no >= 0)                         // a game of a multi-game file
        romsize = game_table[game_no * 0x20 + 0x1e] * MBIT;
      else if (multi_game)
        romsize = PCE_PRO_LOADER_SIZE;          // the loader

      bytesleft = romsize;
      if (bytesleft == 4 * MBIT)
        bytesleft += 2 * MBIT;
      startaddress = address;

      while (bytesleft > 0 && (bytesread = fread (buffer, 1, 0x4000, file)) != 0)
        {
          if ((address & 0xffff) == 0)
            ttt_erase_block (address);
          write_block (&address, buffer);
          if ((romsize == 3 * MBIT) && (address - startaddress == 2 * MBIT))
            address += 2 * MBIT;
          else if ((romsize == 4 * MBIT) && (address - startaddress == 4 * MBIT))
            fseek (file, -2 * MBIT, SEEK_CUR);

          bytessent += bytesread;
          ucon64_gauge (starttime, bytessent, size);
          bytesleft -= bytesread;
        }
      // Games have to be aligned to a Mbit boundary.
      address = (address + MBIT - 1) & ~(MBIT - 1);
      game_no++;
    }
  while (multi_game ? (game_table[game_no * 0x20] && game_no < 31) : 0);

  fclose (file);

  return 0;
}

#endif // USE_PARALLEL
