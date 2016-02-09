/*
md-pro.c - MD-PRO flash card programmer support for uCON64

Copyright (c) 2003 - 2005 dbjh
Copyright (c) 2003        NoisyB

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
#include "backup/md-pro.h"


#ifdef  USE_PARALLEL
static st_ucon64_obj_t mdpro_obj[] =
  {
    {UCON64_GEN, WF_DEFAULT | WF_STOP | WF_NO_SPLIT | WF_NO_ROM},
    {UCON64_GEN, WF_STOP | WF_NO_ROM}
  };
#endif

const st_getopt2_t mdpro_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "MD-PRO flash card programmer"/*"2003 ToToTEK Multi Media http://www.tototek.com"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xmd", 0, 0, UCON64_XMD,
      NULL, "send/receive ROM to/from MD-PRO flash card programmer\n" OPTION_LONG_S "port=PORT\n"
      "receives automatically (32/64 Mbits) when ROM does not exist",
      &mdpro_obj[0]
    },
    {
      "xmds", 0, 0, UCON64_XMDS,
      NULL, "send/receive SRAM to/from MD-PRO flash card programmer\n" OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &mdpro_obj[1]
    },
    {
      "xmdb", 1, 0, UCON64_XMDB,
      "BANK", "send/receive SRAM to/from MD-PRO BANK\n"
      "BANK can be a number from 1 to 4; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &mdpro_obj[1]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

static void eep_reset (void);
static unsigned short int check_card (void);
static void write_rom_by_byte (int *addr, unsigned char *buf);
static void write_rom_by_page (int *addr, unsigned char *buf);
static void write_ram_by_byte (int *addr, unsigned char *buf);
static void write_ram_by_page (int *addr, unsigned char *buf);

static unsigned short int md_id;


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


unsigned short int
check_card (void)
{
  unsigned short int id;

  eep_reset ();
  id = ttt_get_id ();
  if ((id != 0xb0d0) && (id != 0x8916) && (id != 0x8917)) // Sharp 32M, Intel 64J3
    {
      fprintf (stderr, "ERROR: MD-PRO flash card (programmer) not detected (ID: 0x%02hx)\n", id);
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
      if (md_id == 0xb0d0)
        ttt_write_byte_sharp (*addr, buf[*addr & 0x3fff]);
      else if (md_id == 0x8916 || md_id == 0x8917)
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
      // Send the same byte again => SRAM files needn't store redundant data
      ttt_write_byte_ram (*addr, buf[i]);
      (*addr)++;
    }
}


void
write_ram_by_page (int *addr, unsigned char *buf)
{
  int x;

  for (x = 0; x < 0x80; x++)
    {
      ttt_write_page_ram2 (*addr, buf);
      (*addr) += 0x80;
    }
}


int
md_read_rom (const char *filename, unsigned short parport, int size)
{
  FILE *file;
  unsigned short int id;
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

  id = check_card ();
  if (id == 0)
    {
      fclose (file);
      remove (filename);
      exit (1);
    }

  if ((id == 0xb0d0 || id == 0x8916) && size > 32 * MBIT)
    size = 32 * MBIT;                           // Sharp or Intel 32 Mbit flash card
#if 0
  // size is set to 64 * MBIT "by default" (in ucon64_opts.c)
  else if (id == 0x8917 && size > 64 * MBIT)
    size = 64 * MBIT;                           // Intel 64 Mbit flash card
#endif

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
  // original code doesn't call ttt_rom_disable() when byte-size function is
  //  used (ttt_read_rom_b() calls it)
  if (read_block == ttt_read_rom_w)
    ttt_rom_disable ();

  fclose (file);

  return 0;
}


int
md_write_rom (const char *filename, unsigned short parport)
{
  FILE *file;
  unsigned char buffer[0x4000], game_table[32 * 0x20];
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

  fseek (file, 0x83f4, SEEK_SET);
  buffer[0] = 0;
  fread (buffer, 1, 12, file);                  // it's OK to not verify if we can read
  // currently we ignore the version string (full string is "uCON64 2.0.1")
  multi_game = strncmp ((char *) buffer, "uCON64", 6) ? 0 : 1;

  if (multi_game)
    {
      fseek (file, 0x8000, SEEK_SET);
      bytesread = fread (game_table, 1, 32 * 0x20, file);
      if (bytesread != 32 * 0x20)
        {
          fputs ("ERROR: Could not read game table from file\n", stderr);
          fclose (file);
          return -1;
        }
    }

  size = ucon64.file_size;
  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  md_id = check_card ();
  if (md_id == 0)
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
        bytesleft = game_table[game_no * 0x20 + 0x1d] * MBIT;
      else if (multi_game)
        bytesleft = MD_PRO_LOADER_SIZE;         // the loader

      while (bytesleft > 0 && (bytesread = fread (buffer, 1, 0x4000, file)) != 0)
        {
          ucon64_bswap16_n (buffer, 0x4000);
          if ((((address & 0xffff) == 0) && (md_id == 0xb0d0)) ||
              (((address & 0x1ffff) == 0) && (md_id == 0x8916 || md_id == 0x8917)))
            ttt_erase_block (address);
          write_block (&address, buffer);

          bytessent += bytesread;
          ucon64_gauge (starttime, bytessent, size);
          bytesleft -= bytesread;
        }
      // Games have to be aligned to (start at) a 2 Mbit boundary.
      address = (address + 2 * MBIT - 1) & ~(2 * MBIT - 1);
      game_no++;
    }
  while (multi_game ? (game_table[game_no * 0x20] && game_no < 31) : 0);

  fclose (file);

  return 0;
}


int
md_read_sram (const char *filename, unsigned short parport, int start_bank)
/*
  The MD-PRO has 256 kB of SRAM. However, the SRAM dumps of all games that have
  been tested had each byte doubled. In order to make it possible to easily
  obtain the SRAM data for use in an emulator, or to send an emulator SRAM file
  to the MD-PRO, we remove the redundant data when receiving/dumping and double
  the data when sending.
  It could be that this approach causes trouble for some games. However, when
  looking at ToToTEK's own code in ttt_write_page_ram2() this seems unlikely
  (data is doubled in that function). Note that write_sram_by_byte() is a
  function written by us, and so does the doubling of data, but it doesn't mean
  it should work for all games.
*/
{
  FILE *file;
  unsigned char buffer[0x100];
  int blocksleft, address, bytesreceived = 0, size, i;
  time_t starttime;
  void (*read_block) (int, unsigned char *) = ttt_read_ram_b; // ttt_read_ram_w
  // This function does not seem to work if ttt_read_ram_w() is used, but see
  //  note below

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
//      address *= 2;
      ttt_ram_enable ();
#if 0
      // According to JohnDie, disabling this statement should make it possible
      //  to use ttt_read_ram_w().
      ttt_set_ai_data (6, 0x98);        // rst=1, wei=0(dis.), rdi=0(dis.), inc mode, rom_CS
#endif
    }
//  else
//    ttt_set_ai_data (6, 0x94);          // rst=1, wei=0(dis.), rdi=0(dis.), inc mode, rom_CS

  blocksleft = size >> 7;
  starttime = time (NULL);
  while (blocksleft-- > 0)
    {
      read_block (address, buffer);             // 0x100 bytes read
      for (i = 0; i < 0x80; i++)
        buffer[i] = buffer[2 * i];              // data is doubled => no problems with endianess
      fwrite (buffer, 1, 0x80, file);
      address += 0x100;
      bytesreceived += 0x80;
      if ((address & 0x3fff) == 0)
        ucon64_gauge (starttime, bytesreceived, size);
    }
  if (read_block == ttt_read_ram_w)
    ttt_ram_disable ();

  fclose (file);

  return 0;
}


int
md_write_sram (const char *filename, unsigned short parport, int start_bank)
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
