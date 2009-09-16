/*
sflash.h - Super Flash flash card programmer support for uCON64

Copyright (c) 2004 JohnDie


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
#include "misc/parallel.h"
#include "misc/itypes.h"
#include "misc/misc.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/file.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "tototek.h"
#include "sflash.h"


const st_getopt2_t sflash_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Super Flash flash card programmer"/*"2004 ToToTEK Multi Media http://www.tototek.com"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xsf", 0, 0, UCON64_XSF,
      NULL, "send/receive ROM to/from Super Flash flash card programmer\n" OPTION_LONG_S "port=PORT\n"
      "receives automatically (64 Mbits) when ROM does not exist",
      &ucon64_wf[WF_OBJ_SNES_DEFAULT_STOP_NO_SPLIT_NO_ROM]
    },
    {
      "xsfs", 0, 0, UCON64_XSFS,
      NULL, "send/receive SRAM to/from Super Flash flash card programmer\n" OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_SNES_STOP_NO_ROM]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

static void eep_reset (void);
static void write_rom_by_byte (int *addr, unsigned char *buf);
static void write_rom_by_page (int *addr, unsigned char *buf);
static void write_ram_by_byte (int *addr, unsigned char *buf);
static void write_ram_by_page (int *addr, unsigned char *buf);


void
eep_reset (void)
{
  ttt_rom_enable ();
  ttt_write_mem (0x400000, 0xff);               // reset EEP chip 2
  ttt_write_mem (0, 0xff);                      // reset EEP chip 1
  ttt_write_mem (0x600000, 0xff);               // reset EEP chip 2
  ttt_write_mem (0x200000, 0xff);               // reset EEP chip 1
  ttt_rom_disable ();
}


void
write_rom_by_byte (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x4000; x++)
    {
      ttt_write_byte_intel (*addr, buf[*addr & 0x3fff]);
      (*addr)++;
    }
}


void
write_rom_by_page (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x200; x++)
    {
      ttt_write_page_rom (*addr, buf);
      (*addr) += 0x20;
    }
}


void
write_ram_by_byte (int *addr, unsigned char *buf)
{
  int x, i = *addr & 0x3fff;

  for (x = 0; x < 0x4000; x++, i = (i + 1) & 0x3fff)
    {
      ttt_write_byte_ram (*addr, buf[i]);
      (*addr)++;
    }
}


void
write_ram_by_page (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x40; x++)
    {
      ttt_write_page_ram (*addr, buf);
      (*addr) += 0x100;
    }
}


int
sf_read_rom (const char *filename, unsigned int parport, int size)
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

  blocksleft = size >> 8;
  eep_reset ();
  ttt_rom_enable ();
  if (read_block == ttt_read_rom_w)
    ttt_set_ai_data (6, 0x94);          // rst=1, wei=0(dis.), rdi=0(dis.), inc mode, rom_CS
  starttime = time (NULL);
  while (blocksleft-- > 0)
    {
      read_block (address, buffer);             // 0x100 bytes read
      if (read_block == ttt_read_rom_b)
        ucon64_bswap16_n (buffer, 0x100);
      fwrite (buffer, 1, 0x100, file);
      address += 0x100;
      if ((address & 0x3fff) == 0)
        ucon64_gauge (starttime, address, size);
    }
  // ttt_read_rom_b() calls ttt_rom_disable()
  if (read_block == ttt_read_rom_w)
    ttt_rom_disable ();

  fclose (file);
  ttt_deinit_io ();

  return 0;
}


int
sf_write_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char buffer[0x4000], game_table[0x80];
  int game_no, romsize, size, address = 0, bytesread, bytessend = 0;
  time_t starttime;
  void (*write_block) (int *, unsigned char *) = write_rom_by_page; // write_rom_by_byte
  (void) write_rom_by_byte;

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  ttt_init_io (parport);

  size = fsizeof (filename);
  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  eep_reset ();
  if (ttt_get_id () != 0x8917)                  // Intel 64J3
    {
      fputs ("ERROR: Super Flash flash card (programmer) not detected\n", stderr);
      fclose (file);
      ttt_deinit_io ();
      exit (1);
    }

  starttime = time (NULL);

  // Erase last block now, because we have to write to it anyway later. Erasing
  //  it later could erase part of a game.
  ttt_erase_block (0x7e0000);

  fseek (file, 0x4000, SEEK_SET);
  bytesread = fread (game_table, 1, 0x80, file);
  if (bytesread != 0x80)
    {
      fputs ("ERROR: Could not read game table from file\n", stderr);
      fclose (file);
      ttt_deinit_io ();
      return 0;
    }

  fseek (file, 0x8000, SEEK_SET);

  for (game_no = 0; game_no < 4; game_no++)
    {
      if (game_table[game_no * 0x20] == 0)
        continue;

      romsize = game_table[game_no * 0x20 + 0x1f] * 0x8000;

      switch (game_table[game_no * 0x20 + 0x1d] & 0x60)
        {
        case 0x00:
          address = 0x000000;
          break;
        case 0x40:
          address = 0x200000;
          break;
        case 0x20:
          address = 0x400000;
          break;
        case 0x60:
          address = 0x600000;
          break;
        // no default case because we handled all possible cases
        }

      eep_reset ();

      while (romsize && (bytesread = fread (buffer, 1, 0x4000, file)))
        {
          if ((address & 0x1ffff) == 0)
            ttt_erase_block (address);
          if (address < 0x7f8000)               // We mustn't write to the loader space
            write_block (&address, buffer);
          bytessend += bytesread;
          ucon64_gauge (starttime, bytessend, size);
          romsize -= 0x4000;
        }
    }

  fseek (file, 0, SEEK_SET);
  romsize = 0x8000;
  address = 0x7f8000;
  while (romsize && (bytesread = fread (buffer, 1, 0x4000, file)))
    {
      write_block (&address, buffer);
      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      romsize -= 0x4000;
    }

  fclose (file);
  ttt_deinit_io ();

  return 0;
}


int
sf_read_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char buffer[0x100];
  int blocksleft, address, bytesreceived = 0, size;
  time_t starttime;
  void (*read_block) (int, unsigned char *) = ttt_read_ram_w; // ttt_read_ram_w

  address = 0xfe0000;                           // SRAM is stored at 0xfe0000
  size = 0x020000;                              // SRAM size is 1024 kbit

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }

  ttt_init_io (parport);
  printf ("Receive: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

//  ttt_ram_enable ();          // The next ttt_set_ai_data also enables ram access
  ttt_set_ai_data (6, 0x98);  // Enable cartridge access, auto address increment

  blocksleft = size >> 8;
  starttime = time (NULL);
  while (blocksleft-- > 0)
    {
      read_block (address, buffer);             // 0x100 bytes read
      fwrite (buffer, 1, 0x100, file);
      address += 0x100;
      bytesreceived += 0x100;
      if ((address & 0x3fff) == 0)
        ucon64_gauge (starttime, bytesreceived, size);
    }

  ttt_ram_disable ();

  fclose (file);
  ttt_deinit_io ();

  return 0;
}


int
sf_write_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char buffer[0x4000];
  int size, bytesread, bytessend = 0, address;
  time_t starttime;
  void (*write_block) (int *, unsigned char *) = write_ram_by_byte; // write_ram_by_page
  (void) write_ram_by_page;

  size = fsizeof (filename);
  address = 0xfe0000;

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  ttt_init_io (parport);
  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  ttt_ram_enable ();

  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, 0x4000, file)))
    {
      write_block (&address, buffer);             // 0x4000 bytes write
      bytessend += bytesread;
      if ((address & 0x3fff) == 0)
        ucon64_gauge (starttime, bytessend, size);
    }

  fclose (file);
  ttt_deinit_io ();

  return 0;
}

#endif // USE_PARALLEL
