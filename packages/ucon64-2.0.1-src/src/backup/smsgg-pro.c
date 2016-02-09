/*
smsgg-pro.c - SMS-PRO/GG-PRO flash card programmer support for uCON64

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
#include "backup/smsgg-pro.h"


#ifdef  USE_PARALLEL
static st_ucon64_obj_t smsggpro_obj[] =
  {
    {UCON64_SMS, WF_DEFAULT | WF_STOP | WF_NO_SPLIT | WF_NO_ROM},
    {UCON64_SMS, WF_STOP | WF_NO_ROM}
  };
#endif

const st_getopt2_t smsggpro_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "SMS-PRO/GG-PRO flash card programmer"/*"2004 ToToTEK Multi Media http://www.tototek.com"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xgg", 0, 0, UCON64_XGG,
      NULL, "send/receive ROM to/from SMS-PRO/GG-PRO flash card programmer\n" OPTION_LONG_S "port=PORT\n"
      "receives automatically (32 Mbits) when ROM does not exist",
      &smsggpro_obj[0]
    },
    {
      "xggs", 0, 0, UCON64_XGGS,
      NULL, "send/receive SRAM to/from SMS-PRO/GG-PRO flash card programmer\n" OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &smsggpro_obj[1]
    },
    {
      "xggb", 1, 0, UCON64_XGGB,
      "BANK", "send/receive SRAM to/from SMS-PRO/GG-PRO BANK\n"
      "BANK can be a number from 1 to 4; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &smsggpro_obj[1]
    },
#endif // USE_PARALLEL
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

static void eep_reset (void);
static unsigned short int check_card (void);
static void write_rom_by_byte (int *addr, unsigned char *buf);
static void write_rom_by_page (int *addr, unsigned char *buf);
static void write_ram_by_byte (int *addr, unsigned char *buf);
static void write_ram_by_page (int *addr, unsigned char *buf);


void
eep_reset (void)
{
  ttt_rom_enable ();
  ttt_write_mem (0x000000, 0xff);               // reset EEP
  ttt_write_mem (0x200000, 0xff);               // reset EEP
  ttt_rom_disable ();
}


unsigned short int
check_card (void)
{
  unsigned short int id;

  eep_reset ();
  id = ttt_get_id ();
  if (id != 0xb0d0)
    {
      fprintf (stderr, "ERROR: SMS-PRO/GG-PRO flash card (programmer) not detected (ID: 0x%02hx)\n", id);
      return 0;
    }
  else
    return id;
}


void
write_rom_by_byte (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x4000; x++)
    {
      ttt_write_byte_sharp (*addr, buf[*addr & 0x3fff]);
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
  int x;

  for (x = 0; x < 0x4000; x++)
    {
      ttt_write_byte_ram (*addr, buf[*addr & 0x3fff]);
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
smsgg_read_rom (const char *filename, unsigned short parport, int size)
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
smsgg_write_rom (const char *filename, unsigned short parport)
{
  FILE *file;
  unsigned char buffer[0x4000], game_table[32 * 0x10];
  int game_no, size, address = 0, bytesread, bytessent = 0, bytesleft = 0,
      multi_game;
  time_t starttime;
  void (*write_block) (int *, unsigned char *) = write_rom_by_page; // write_rom_by_byte
  (void) write_rom_by_byte;

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  ttt_init_io (parport);

  fseek (file, 0x21f4, SEEK_SET);
  buffer[0] = 0;
  fread (buffer, 1, 12, file);                  // it's OK to not verify if we can read
  // currently we ignore the version string (full string is "uCON64 2.0.1")
  multi_game = strncmp ((char *) buffer, "uCON64", 6) ? 0 : 1;

  if (multi_game)
    {
      fseek (file, 0x2000, SEEK_SET);
      bytesread = fread (game_table, 1, 32 * 0x10, file);
      if (bytesread != 32 * 0x10)
        {
          fputs ("ERROR: Could not read game table from file\n", stderr);
          fclose (file);
          return -1;
        }
    }

  size = ucon64.file_size;
  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  if (check_card () == 0)
    {
      fclose (file);
      exit (1);
    }

  fseek (file, 0, SEEK_SET);

  starttime = time (NULL);
  if (!multi_game)
    bytesleft = size;                           // one file (no multi-game)
  eep_reset ();
  game_no = -1;
  do
    {
      if (game_no >= 0)                         // a game of a multi-game file
        bytesleft = game_table[game_no * 0x10 + 0x0d] * 16384;
      else if (multi_game)
        bytesleft = SMSGG_PRO_LOADER_SIZE;      // the loader

      while (bytesleft > 0 && (bytesread = fread (buffer, 1, 0x4000, file)) != 0)
        {
          if ((address & 0xffff) == 0)
            ttt_erase_block (address);
          write_block (&address, buffer);

          bytessent += bytesread;
          ucon64_gauge (starttime, bytessent, size);
          bytesleft -= bytesread;
        }
      // Games have to be aligned to a 16 kB boundary.
      address = (address + 16384 - 1) & ~(16384 - 1);
      game_no++;
    }
  while (multi_game ? (game_table[game_no * 0x10] && game_no < 31) : 0);

  fclose (file);

  return 0;
}


int
smsgg_read_sram (const char *filename, unsigned short parport, int start_bank)
{
  FILE *file;
  unsigned char buffer[0x100];
  int blocksleft, address, size, bytesreceived = 0;
  time_t starttime;
  void (*read_block) (int, unsigned char *) = ttt_read_ram_b; // ttt_read_ram_w

  if (start_bank == -1)
    {
      address = 0;
      size = 128 * 1024;
    }
  else
    {
      if (start_bank < 1 || start_bank > 4)
        {
          fputs ("ERROR: Bank must be a value 1 - 4\n", stderr);
          exit (1);
        }
      address = (start_bank - 1) * 32 * 1024;
      size = 32 * 1024;
    }

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

  if (read_block == ttt_read_ram_w)
    {
      ttt_ram_enable ();
      ttt_set_ai_data (6, 0x98);        // rst=1, wei=0(dis.), rdi=0(dis.), inc mode, rom_CS
    }
//  else
//    ttt_set_ai_data (6, 0x94);          // rst=1, wei=0(dis.), rdi=0(dis.), inc mode, rom_CS

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
  if (read_block == ttt_read_ram_w)
    ttt_ram_disable ();

  fclose (file);

  return 0;
}


int
smsgg_write_sram (const char *filename, unsigned short parport, int start_bank)
{
  FILE *file;
  unsigned char buffer[0x4000];
  int size, bytesread, bytessent = 0, address;
  time_t starttime;
  void (*write_block) (int *, unsigned char *) = write_ram_by_byte; // write_ram_by_page
  (void) write_ram_by_page;

  size = ucon64.file_size;
  if (start_bank == -1)
    address = 0;
  else
    {
      if (start_bank < 1 || start_bank > 4)
        {
          fputs ("ERROR: Bank must be a value 1 - 4\n", stderr);
          exit (1);
        }
      address = (start_bank - 1) * 32 * 1024;
    }

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  ttt_init_io (parport);
  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  if (check_card () == 0)
    {
      fclose (file);
      exit (1);
    }

  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, 0x4000, file)) != 0)
    {
      write_block (&address, buffer);
      bytessent += bytesread;
      ucon64_gauge (starttime, bytessent, size);
    }

  fclose (file);

  return 0;
}

#endif // USE_PARALLEL
