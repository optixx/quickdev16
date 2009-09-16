/*
swc.c - Super Wild Card support for uCON64

Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2004 dbjh
Copyright (c) 2001        Caz
Copyright (c) 2003        John Weidman
Copyright (c) 2004        JohnDie


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
#include "misc/parallel.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ffe.h"
#include "swc.h"
#include "console/snes.h"                       // for snes_get_file_type ()


const st_getopt2_t swc_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Super Com Pro/Super Magicom/SMC/Super Wild Card (1.6XC/2.7CC/2.8CC/DX/DX2)/SWC"
      /*"1993/1994/1995/19XX Front Far East/FFE http://www.front.com.tw"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xswc", 0, 0, UCON64_XSWC,
      NULL, "send/receive ROM to/from Super Wild Card*/SWC; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when ROM does not exist",
      &ucon64_wf[WF_OBJ_SNES_DEFAULT_STOP_NO_SPLIT_NO_ROM]
    },
    {
      "xswc2", 0, 0, UCON64_XSWC2,
      NULL, "same as " OPTION_LONG_S "xswc, but enables Real Time Save mode (SWC only)",
      &ucon64_wf[WF_OBJ_SNES_DEFAULT_STOP_NO_SPLIT_NO_ROM]
    },
#if 1
/*
  The following help text used to be hidden, because we wanted to avoid people
  to "accidentally" create overdumps, bad dumps or report bugs that aren't bugs
  (SA-1). However, now that ucon64.io_mode is useful for -xswcc I guess the
  help should be complete. - dbjh
*/
    {
      "xswc-io", 1, 0, UCON64_XSWC_IO,
      "MODE", "specify SWC I/O mode; use with -xswc or -xswcc\n"
      "MODE=0x001 force 32 Mbit dump\n"
      "MODE=0x002 use alternative method for determining ROM size\n"
      "MODE=0x004 Super FX\n"
      "MODE=0x008 S-DD1\n"
      "MODE=0x010 SA-1\n"
      "MODE=0x020 SPC7110\n"
      "MODE=0x040 DX2 trick (might work with other SWC models)\n"
      "MODE=0x080 Mega Man X 2\n"
      "MODE=0x100 dump BIOS\n"
      "It is possible to combine flags. MODE=0x44 makes it possible\n"
      "to dump for example Yoshi's Island",
      &ucon64_wf[WF_OBJ_SNES_SWITCH]
    },
#endif
    {
      "xswcs", 0, 0, UCON64_XSWCS,
      NULL,
      "send/receive SRAM to/from Super Wild Card*/SWC; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_SNES_STOP_NO_ROM]
    },
    {
      "xswcc", 0, 0, UCON64_XSWCC,
      NULL, "send/receive SRAM to/from cartridge in Super Wild Card*/SWC;\n"
      OPTION_LONG_S "port=PORT\n" "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_SNES_STOP_NO_ROM]
    },
    {
      "xswcr", 0, 0, UCON64_XSWCR,
      NULL, "send/receive RTS data to/from Super Wild Card*/SWC; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when RTS file does not exist",
      &ucon64_wf[WF_OBJ_SNES_STOP_NO_ROM]
    },
#endif // USE_PARALLEL
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

#ifdef  USE_PARALLEL

#define BUFFERSIZE 8192                         // don't change, only 8192 works!

/*
  Some notes about dumping special chip cartridges (JohnDie):
  The following defines enable code to dump cartridges containing special chips
  like the SA-1, the S-DD1, the SPC7110 and the C4. However, enabling these
  options is not all there is to successfully dump these cartridges.
  The SPC7110 and SA-1 need several attempts to "boot up" in the copier. This
  involves powering off and on until the chip comes out of reset and allows
  access to the cartridge. And you need to make sure that pin 1 of the SNES
  adapter of the SWC is not cut! The standard SWC DX2 adapter has this trace
  cut, so you have to reconnect it somehow.
  Dumping Super FX 2 cartridges is only possible when enabling the DX2 trick.
  Otherwise uCON64 will not detect the cartridge. This is because of the way the
  Super FX 2 cartridges have their ROM mapped into the SNES address space.
*/
#define DUMP_MMX2
#define DUMP_SA1
#define DUMP_SDD1
#define DUMP_SPC7110

static int receive_rom_info (unsigned char *buffer, int io_mode);
static int get_rom_size (unsigned char *info_block);
static int check1 (unsigned char *info_block, int index);
static int check2 (unsigned char *info_block, int index, unsigned char value);
static int check3 (unsigned char *info_block, int index1, int index2, int size);
static unsigned char get_emu_mode_select (unsigned char byte, int size);
static void handle_fig_header (unsigned char *header);
static void set_bank_and_page (unsigned char bank, unsigned char page);
static void read_cartridge (unsigned int address, unsigned char *buffer,
                            unsigned int length);
static unsigned char read_cartridge1 (unsigned int address);
static void write_cartridge (unsigned int address, unsigned char *buffer,
                             unsigned int length);
static void write_cartridge1 (unsigned int address, unsigned char byte);
static void dump_rom (FILE *file, int size, int numblocks, unsigned int mask1,
                      unsigned int mask2, unsigned int address);
static void dump_bios (FILE *file);
static int sub (void);
static int mram_helper (int x);
static int mram (void);

static int hirom;                               // `hirom' was `special'
static int dx2_trick = 0;

#ifdef  DUMP_SA1
static void set_sa1_map (unsigned short chunk);
static int snes_sa1 = 0;
#endif
#ifdef  DUMP_SDD1
static void set_sdd1_map (unsigned short chunk);
static int snes_sdd1 = 0;
#endif
#ifdef  DUMP_SPC7110
static void set_spc7110_map (unsigned short chunk);
static int snes_spc7110 = 0;
#endif


#if BUFFERSIZE < 512
#error receive_rom_info() and swc_read_sram() expect BUFFERSIZE to be at least \
       512 bytes.
#endif
int
receive_rom_info (unsigned char *buffer, int io_mode)
/*
  - returns size of ROM in Mb (128 kB) units
  - returns ROM header in buffer (index 2 (emulation mode select) is not yet
    filled in)
  - sets global `hirom'
*/
{
  int n, size;
  volatile int m;
  unsigned char byte;

#ifdef  DUMP_MMX2
  if (io_mode & SWC_IO_MMX2)
    {
      /*
        MMX2 can be dumped after writing a 0 to SNES register 0x7f52. Before we can
        write to that register we have to enable cartridge page mapping. That is
        done by writing to SWC register 0xe00c. When cartridge page mapping is
        enabled we can access SNES registers by reading or writing to the SWC
        address range 0x2000-0x3fff. Before reading or writing to an address in that
        range we have to "announce" the address to the SWC (via command 5). Because
        we access a SNES register we only set the page number bits (0-1).
      */
      unsigned short address = 0x7f52;
      ffe_send_command0 (0xe00c, 0);

      ffe_send_command (5, (unsigned short) (address / 0x2000), 0);
      ffe_receive_block ((unsigned short) ((address & 0x1fff) + 0x2000), buffer, 8);
      dumper (stdout, buffer, 8, address, DUMPER_HEX);

      ffe_send_command (5, (unsigned short) (address / 0x2000), 0);
      ffe_send_command0 ((unsigned short) ((address & 0x1fff) + 0x2000), 0);

      ffe_send_command (5, (unsigned short) (address / 0x2000), 0);
      ffe_receive_block ((unsigned short) ((address & 0x1fff) + 0x2000), buffer, 8);
      dumper (stdout, buffer, 8, address, DUMPER_HEX);
    }
#endif

  ffe_send_command0 (0xe00c, 0);

  if (UCON64_ISSET (ucon64.snes_hirom))
    hirom = ucon64.snes_hirom ? 1 : 0;
  else
    {
      byte = read_cartridge1 (0x00ffd5);
      hirom = ((byte & 1 && byte != 0x23) || byte == 0x3a) ? 1 : 0; // & 1 => 0x21, 0x31, 0x35
    }

  for (n = 0; n < (int) SWC_HEADER_LEN; n++)
    {
      for (m = 0; m < 65536; m++)               // a delay is necessary here
        ;
      ffe_send_command (5, (unsigned short) (0x200 + n), 0);
      buffer[n] = ffe_send_command1 (0xa0a0);
    }

  if (io_mode & SWC_IO_FORCE_32MBIT)
    {
      if (!UCON64_ISSET (ucon64.snes_hirom))
        hirom = 1;                              // default to super HiROM dump
      size = 32;                                // dump 32 Mbit
    }
  else
    {
      size = get_rom_size (buffer);
#ifdef  DUMP_SA1
      if (!snes_sa1)
#endif
      if (hirom)
        size <<= 1;
    }

  // Fix up ROM size for Super FX 2 cartridge, because get_rom_size() fails for
  //  Super FX 2 cartridges and returns 0.
  if (io_mode & SWC_IO_SUPER_FX)
    // 00:303b returns the GSU revision and is non-zero if there is a GSU
    if (size == 0 && read_cartridge1 (0x00303b) != 0)
      size = 16;

#ifdef  DUMP_SDD1
  // Adjust size to 48 Mbit for Star Ocean
  if (snes_sdd1 && size == 32)
    {
      byte = read_cartridge1 (0x00ffd7);
      if (byte == 0x0d)
        size = 48;
    }
#endif

#ifdef  DUMP_SA1
  // Fix up size for SA-1 chips
  if (snes_sa1)
    {
      byte = read_cartridge1 (0x00ffd7);
      switch (byte)
        {
        case 0x09:
          size = 4;
          break;
        case 0x0a:
          size = 8;
          break;
        case 0x0b:
          size = 16;
          break;
        case 0x0c:
          size = 32;
          break;
        default:
          break;
        }
    }
#endif

#ifdef  DUMP_SPC7110
  // Fix up size for SPC7110 chips
  if (snes_spc7110)
    {
      byte = read_cartridge1 (0x00ffd7);
      switch (byte)
        {
        case 0x0c:
          size = 24;
          break;
        case 0x0d:
          size = 40;
          break;
        default:
          break;
        }
    }
#endif

  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[0] = size << 4;                        // *16 for 8 kB units; low byte
  buffer[1] = size >> 4;                        // *16 for 8 kB units /256 for high byte
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 4;

  return size;
}


int
get_rom_size (unsigned char *info_block)
// returns size of ROM in Mb units
{
  if (check1 (info_block, 0))
    return 0;
  if (check2 (info_block, 0x10, 0x84))
    return 0;
  if (check3 (info_block, 0, 0x20, 0x20))
    return 2;
  if (check3 (info_block, 0, 0x40, 0x20))
    return 4;
  if (check3 (info_block, 0x40, 0x60, 0x20))
    return 6;
  if (check3 (info_block, 0, 0x80, 0x10))
    return 8;
  if (check1 (info_block, 0x80))
    return 8;
  if (check3 (info_block, 0x80, 0x90, 0x10))
    return 8;
  if (check2 (info_block, 0x80, 0xa0))
    return 8;
  if (check3 (info_block, 0x80, 0xa0, 0x20))
    return 10;
  if (check1 (info_block, 0xc0))
    return 12;
  if (check2 (info_block, 0xc0, 0xb0))
    return 12;
  if (check3 (info_block, 0x80, 0xc0, 0x20))
    return 12;
  if (check3 (info_block, 0x100, 0, 0x10))
    return 16;
  if (check2 (info_block, 0x100, 0xc0))
    return 16;
  if (check3 (info_block, 0x100, 0x120, 0x10))
    return 18;
  if (check3 (info_block, 0x100, 0x140, 0x10))
    return 20;
  if (check2 (info_block, 0x140, 0xd0))
    return 20;
  if (check3 (info_block, 0x100, 0x180, 0x10))
    return 24;
  if (check2 (info_block, 0x180, 0xe0))
    return 24;
  if (check3 (info_block, 0x180, 0x1c0, 0x10))
    return 28;
  if (check3 (info_block, 0x1f0, 0x1f0, 0x10))
    return 32;

  return 0;
}


int
check1 (unsigned char *info_block, int index)
{
  int n;

  for (n = 0; n < 16; n++)
    if (info_block[n + index] != info_block[index])
      return 0;

  return 1;
}


int
check2 (unsigned char *info_block, int index, unsigned char value)
{
  int n;

  for (n = 0; n < 4; n++)
    if (info_block[n + index] != value)
      return 0;

  return 1;
}


int
check3 (unsigned char *info_block, int index1, int index2, int size)
{
  int n;

  for (n = 0; n < size; n++)
    if (info_block[n + index1] != info_block[n + index2])
      return 0;

  return 1;
}


unsigned char
get_emu_mode_select (unsigned char byte, int size)
{
  int x;
  unsigned char ems;

  if (byte == 0)
    x = 0xc;
  else if (byte == 1)
    x = 8;
  else if (byte == 3)
    x = 4;
  else
    x = 0;

  if (hirom)
    {
      if (x == 0xc && size <= 0x1c)
        ems = 0x1c;
      else
        ems = x + 0x30;
    }
  else
    {
      if (x == 0xc)
        ems = 0x2c;
      else
        ems = x;

//      if (size <= 8)                          // This bit should always be 0 - JohnDie
//        ems++;
    }

  return ems;
}


void
handle_fig_header (unsigned char *header)
{
  if ((header[4] == 0x77 && header[5] == 0x83) ||
      (header[4] == 0xf7 && header[5] == 0x83) ||
      (header[4] == 0x47 && header[5] == 0x83))
    header[2] = 0x0c;                           // 0 kB
  else if (header[4] == 0xfd && header[5] == 0x82)
    header[2] = 0x08;                           // 2 kB
  else if ((header[4] == 0xdd && header[5] == 0x82) ||
           (header[4] == 0x00 && header[5] == 0x80))
    /*
      8 kB *or* 2 kB (shortcoming of FIG header format). We give the emu mode
      select byte a value as if the game uses 8 kB. At least this makes games
      that use 8 kB work.
      Users should not complain if the game doesn't work because of a SRAM
      protection, because they should have converted the ROM to SWC format in
      the first place.
    */
    header[2] = 0x04;
  else // if ((header[4] == 0xdd && header[5] == 0x02) ||
       //     (header[4] == 0x00 && header[5] == 0x00) ||
       //     (header[4] == 0x11 && header[5] == 0x02))
    header[2] = 0;                              // 32 kB

  if (header[3] & 0x80)                         // Pro Fighter (FIG) HiROM dump
    header[2] |= 0x30;                          // set bit 5&4 (SRAM & DRAM mem map mode 21)
}


void
swc_unlock (unsigned int parport)
/*
  "Unlock" the SWC. However, just starting to send, then stopping with ^C,
  gives the same result.
*/
{
  ffe_init_io (parport);
  ffe_send_command (6, 0, 0);
  ffe_deinit_io ();
}


#ifdef  DUMP_SA1
void
set_sa1_map (unsigned short chunk)
{
  volatile int m;

  // map the 8 Mbit ROM chunk specified by chunk into the F0 bank
  write_cartridge1 (0x002223, (unsigned char) ((chunk & 0x07) | 0x80));
  for (m = 0; m < 65536; m++)
    ;
}
#endif


#ifdef  DUMP_SDD1
void
set_sdd1_map (unsigned short chunk)
{
  volatile int m;

  // map the 8 Mbit ROM chunk specified by chunk into the F0 bank
  write_cartridge1 (0x004807, (unsigned char) (chunk & 0x07));
  for (m = 0; m < 65536; m++)
    ;
}
#endif


#ifdef  DUMP_SPC7110
void
set_spc7110_map (unsigned short chunk)
{
  volatile int m;

  // map the 8 Mbit ROM chunk specified by chunk into the F0 bank
  write_cartridge1 (0x004834, 0xff);
  write_cartridge1 (0x004833, (unsigned char) (chunk & 0x07));
  for (m = 0; m < 65536; m++)
    ;
}
#endif


void
set_bank_and_page (unsigned char bank, unsigned char page)
{
  static unsigned char currentbank = 0, currentpage = 4; // Force update on first call

  page &= 3;

#if 0
  // We only send a command to the SWC if the bank or page differs.
  /*
    In order to avoid problems with that no other code should change the bank or
    page number. So, no calls to ffe_send_command(5, ...). I prefer to be able
    to call ffe_send_command(5, ...) without breaking this function. Besides,
    the benefit of this optimisation is rather small. - dbjh
  */
  if (bank != currentbank || page != currentpage)
#endif
    {
      currentbank = bank;
      currentpage = page;

      if (dx2_trick)
        {
          /*
            The SWC DX2 does not allow to access banks 00-7f. But this is needed
            to dump some cartridges (ToP, DKJM2 and Super FX 2). This trick
            avoids using ffe_send_command(5, ...) to set the bank value and
            writes the bank value directly into the SNES RAM where the DX2 BIOS
            would store it. Note that this hack is specific to the SWC DX2 and
            will probably not work with other copiers. - JohnDie
          */
          ffe_send_command (5, currentpage, 0);
          ffe_send_command0 (0x0007, currentbank);
        }
      else
        ffe_send_command (5, (unsigned short) ((currentbank << 2) | currentpage), 0);
    }
}


void
read_cartridge (unsigned int address, unsigned char *buffer, unsigned int length)
{
  address &= 0xffffff;
  set_bank_and_page ((unsigned char) (address >> 16),
                     (unsigned char) ((address & 0x7fff) / 0x2000));

  if ((address & 0x00ffff) < 0x8000)
    ffe_receive_block ((unsigned short) (((address & 0x7fffff) < 0x400000 ?
                         0x6000 : 0x2000) + (address & 0x001fff)), buffer, length);
  else
    ffe_receive_block ((unsigned short) (0xa000 + (address & 0x001fff)),
                       buffer, length);
}


unsigned char
read_cartridge1 (unsigned int address)
{
  unsigned char byte;

  read_cartridge (address, &byte, 1);

  return byte;
}


void
write_cartridge (unsigned int address, unsigned char *buffer, unsigned int length)
{
  address &= 0xffffff;
  set_bank_and_page ((unsigned char) (address >> 16),
                     (unsigned char) ((address & 0x7fff) / 0x2000));

  if ((address & 0x00ffff) < 0x8000)
    ffe_send_block ((unsigned short) (((address & 0x7fffff) < 0x400000 ?
                      0x6000 : 0x2000) + (address & 0x001fff)), buffer, length);
  else
    ffe_send_block ((unsigned short) (0xa000 + (address & 0x001fff)),
                    buffer, length);
}


void
write_cartridge1 (unsigned int address, unsigned char byte)
{
  write_cartridge (address, &byte, 1);
}


void
dump_rom (FILE *file, int size, int numblocks, unsigned int mask1,
          unsigned int mask2, unsigned int address)
{
  int i, bytesreceived = 0;
  unsigned char *buffer;
  time_t starttime;
#if     defined DUMP_SA1 || defined DUMP_SDD1 || defined DUMP_SPC7110
  unsigned short chunk_num = 0;         // 0 = 1st 8 Mb ROM chunk, 1 = 2nd 8 Mb, ...
#endif

  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  starttime = time (NULL);

  for (i = 0; i < numblocks; i++)
    {
      address |= mask1;                         // make sure to stay in ROM areas
      address &= mask2;

#ifdef  DUMP_SA1
      if (snes_sa1 && address == 0xf00000)
        set_sa1_map (chunk_num++);
#endif
#ifdef  DUMP_SDD1
      if (snes_sdd1 && address == 0xf00000)
        set_sdd1_map (chunk_num++);
#endif
#ifdef  DUMP_SPC7110
      if (snes_spc7110 && address == 0xf00000)
        set_spc7110_map (chunk_num++);
#endif

      read_cartridge (address, buffer, BUFFERSIZE);

      fwrite (buffer, 1, BUFFERSIZE, file);
      address += BUFFERSIZE;

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, size);
      ffe_checkabort (2);
    }

  ffe_send_command (5, 0, 0);

  free (buffer);
}


void
dump_bios (FILE *file)
{
  unsigned short int address;
  int bytesreceived = 0;
  unsigned char *buffer;
  time_t starttime;

  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  starttime = time (NULL);
  for (address = 0; address < 0x080; address += 4) // banks 00-1f
    {
      ffe_send_command (5, address, 0);
      ffe_receive_block (0xe000, buffer, BUFFERSIZE);
      fwrite (buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, 0x20 * 0x2000);
      ffe_checkabort (2);
    }

  free (buffer);
}


int
swc_read_rom (const char *filename, unsigned int parport, int io_mode)
{
  FILE *file;
  unsigned char buffer[SWC_HEADER_LEN], byte;
  int size, blocksleft;

  ffe_init_io (parport);

#ifdef  DUMP_SA1
  if (io_mode & SWC_IO_SA1)
    snes_sa1 = 1;
#endif
#ifdef  DUMP_SDD1
  if (io_mode & SWC_IO_SDD1)
    snes_sdd1 = 1;
#endif
#ifdef  DUMP_SPC7110
  if (io_mode & SWC_IO_SPC7110)
    snes_spc7110 = 1;
#endif
  if (io_mode & SWC_IO_DX2_TRICK)
    dx2_trick = 1;

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }

  if (io_mode & SWC_IO_DUMP_BIOS)
    {
      puts ("Press q to abort\n");

      dump_bios (file);

      ffe_deinit_io ();
      fclose (file);
      return 0;                                 // skip the other code in this function
    }

  size = receive_rom_info (buffer, io_mode);
  if (size == 0)
    {
      fputs ("ERROR: There is no cartridge present in the Super Wild Card\n", stderr);
      fclose (file);
      remove (filename);
      exit (1);
    }
  blocksleft = size * 16;                       // 1 Mb (128 kB) unit == 16 8 kB units
  printf ("Receive: %d Bytes (%.4f Mb)\n", size * MBIT, (float) size);
#ifdef  DUMP_SA1
  if (snes_sa1)
    puts ("NOTE: Dumping SA-1 cartridge");
#endif
#ifdef  DUMP_SDD1
  if (snes_sdd1)
    puts ("NOTE: Dumping S-DD1 cartridge");
#endif
#ifdef  DUMP_SPC7110
  if (snes_spc7110)
    puts ("NOTE: Dumping SPC7110 cartridge");
#endif
  size *= MBIT;                                 // size in bytes for ucon64_gauge() below

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00c, 0);
  byte = ffe_send_command1 (0xbfd8);
  buffer[2] = get_emu_mode_select (byte, blocksleft / 16);
  fwrite (buffer, 1, SWC_HEADER_LEN, file);     // write header (other necessary fields are
                                                //  filled in by receive_rom_info())

  puts ("Press q to abort\n");                  // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
#ifdef  DUMP_SA1
  if (snes_sa1)
    dump_rom (file, size, blocksleft, 0xf00000, 0xffffff, 0xf00000);
  else
#endif
#ifdef  DUMP_SDD1
  if (snes_sdd1)
    dump_rom (file, size, blocksleft, 0xf00000, 0xffffff, 0xf00000);
  else
#endif
#ifdef  DUMP_SPC7110
  if (snes_spc7110)
    {
      // First dump the 8 MBit P-ROM (program ROM)
      dump_rom (file, 8 * MBIT, 8 * 16, 0xc00000, 0xcfffff, 0xc00000);
      // Then dump the remaining amount of D-ROM (data ROM)
      dump_rom (file, size - 8 * MBIT, blocksleft - 8 * 16, 0xf00000, 0xffffff,
                0xf00000);
    }
  else
#endif
  if (io_mode & SWC_IO_SUPER_FX)
    dump_rom (file, size, blocksleft, 0x008000, 0x7fffff, 0x008000);
  else if (hirom)
    dump_rom (file, size, blocksleft, 0x400000, 0xffffff, 0xc00000);
  else
    dump_rom (file, size, blocksleft, 0x008000, 0xffffff, 0x808000);

#ifdef  DUMP_SA1
  if (snes_sa1)
    set_sa1_map (3);
#endif
#ifdef  DUMP_SDD1
  if (snes_sdd1)
    set_sdd1_map (3);
#endif
  ffe_send_command (5, 0, 0);

  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_write_rom (const char *filename, unsigned int parport, int enableRTS)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend, totalblocks, blocksdone = 0, emu_mode_select, fsize;
  unsigned short address;
  time_t starttime;

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

  fsize = fsizeof (filename);
  printf ("Send: %d Bytes (%.4f Mb)\n", fsize, (float) fsize / MBIT);

  ffe_send_command0 (0xc008, 0);
  fread (buffer, 1, SWC_HEADER_LEN, file);

  if (snes_get_file_type () == FIG)
    handle_fig_header (buffer);
#if 1
  /*
    0x0c == no SRAM & LoROM; we use the header, so that the user can override this
    bit 4 == 0 => DRAM mode 20 (LoROM); disable SRAM by setting SRAM mem map mode 2
  */
  if ((buffer[2] & 0x1c) == 0x0c)
    buffer[2] |= 0x20;
#else
  // The code below doesn't work for some HiROM games that don't use SRAM.
  if ((buffer[2] & 0x0c) == 0x0c)               // 0x0c == no SRAM; we use the header, so
    {                                           //  that the user can override this
      if (buffer[2] & 0x10)                     // bit 4 == 1 => DRAM mode 21 (HiROM)
        buffer[2] &= ~0x20;                     // disable SRAM by setting SRAM mem map mode 1
      else                                      // bit 4 == 0 => DRAM mode 20 (LoROM)
        buffer[2] |= 0x20;                      // disable SRAM by setting SRAM mem map mode 2
    }
#endif
  emu_mode_select = buffer[2];                  // this byte is needed later

#if 1                                           // sending the header is not required
  ffe_send_command (5, 0, 0);
  ffe_send_block (0x400, buffer, SWC_HEADER_LEN); // send header
#endif
  bytessend = SWC_HEADER_LEN;

  puts ("Press q to abort\n");                  // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = 0x200;                              // VGS '00 uses 0x200, VGS '96 uses 0,
  starttime = time (NULL);                      //  but then some ROMs don't work
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      ffe_send_command0 ((unsigned short) 0xc010, (unsigned char) (blocksdone >> 9));
      ffe_send_command (5, address, 0);
      ffe_send_block (0x8000, buffer, bytesread);
      address++;
      blocksdone++;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, fsize);
      ffe_checkabort (2);
    }

  if (blocksdone > 0x200)                       // ROM dump > 512 8 kB blocks (=32 Mb (=4 MB))
    ffe_send_command0 (0xc010, 2);

  ffe_send_command (5, 0, 0);
  totalblocks = (fsize - SWC_HEADER_LEN + BUFFERSIZE - 1) / BUFFERSIZE; // round up
  ffe_send_command (6, (unsigned short) (5 | (totalblocks << 8)), (unsigned short) (totalblocks >> 8)); // bytes: 6, 5, #8 K L, #8 K H, 0
  ffe_send_command (6, (unsigned short) (1 | (emu_mode_select << 8)), (unsigned short) enableRTS); // last arg = 1 enables RTS
                                                               //  mode, 0 disables it
  ffe_wait_for_ready ();
  outportb ((unsigned short) (parport + PARPORT_DATA), 0);
  outportb ((unsigned short) (parport + PARPORT_CONTROL),
            (unsigned char) (inportb ((unsigned short) // invert strobe
                                      (parport + PARPORT_CONTROL)) ^ PARPORT_STROBE));

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_read_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int blocksleft, bytesreceived = 0;
  unsigned short address;
  time_t starttime;

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

  printf ("Receive: %d Bytes\n", 32 * 1024);
  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 5;
  fwrite (buffer, 1, SWC_HEADER_LEN, file);

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00d, 0);
  ffe_send_command0 (0xc008, 0);

  puts ("Press q to abort\n");                  // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  blocksleft = 4;                               // SRAM is 4*8 kB
  address = 0x100;
  starttime = time (NULL);
  while (blocksleft > 0)
    {
      ffe_send_command (5, address, 0);
      ffe_receive_block (0x2000, buffer, BUFFERSIZE);
      blocksleft--;
      address++;
      fwrite (buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, 32 * 1024);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_write_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend = 0, size;
  unsigned short address;
  time_t starttime;

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

  size = fsizeof (filename) - SWC_HEADER_LEN;   // SWC SRAM is 4*8 kB, emu SRAM often not
  printf ("Send: %d Bytes\n", size);
  fseek (file, SWC_HEADER_LEN, SEEK_SET);       // skip the header

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00d, 0);
  ffe_send_command0 (0xc008, 0);

  puts ("Press q to abort\n");                  // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = 0x100;
  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      ffe_send_command (5, address, 0);
      ffe_send_block (0x2000, buffer, bytesread);
      address++;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
sub (void)
{
  ffe_send_command (5, 7 * 4, 0);
  ffe_send_command0 (0xe00d, 0);
  ffe_send_command0 (0xe003, 0);

  if (ffe_send_command1 (0xb080) != 'S')
    return 0;
  if (ffe_send_command1 (0xb081) != 'U')
    return 0;
  if (ffe_send_command1 (0xb082) != 'B')
    return 0;

  return 1;
}


int
mram_helper (int x)
{
  ffe_send_command (5, (unsigned short) x, 0);
  x = ffe_send_command1 (0x8000);
  ffe_send_command0 (0x8000, (unsigned char) (x ^ 0xff));
  if (ffe_send_command1 (0x8000) != (unsigned char) (x ^ 0xff))
    return 0;

  ffe_send_command0 (0x8000, (unsigned char) x);
  return 1;
}


int
mram (void)
{
  if (mram_helper (0x76 * 4))
    return 0x76 * 4;
  if (mram_helper (0x56 * 4))
    return 0x56 * 4;
  if (mram_helper (0x36 * 4))
    return 0x36 * 4;
  return 0x16 * 4;
}


int
swc_read_rts (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int blocksleft, bytesreceived = 0;
  unsigned short address1, address2;
  time_t starttime;

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

  printf ("Receive: %d Bytes\n", 256 * 1024);
  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 8;
  fwrite (buffer, 1, SWC_HEADER_LEN, file);

  puts ("Press q to abort\n");
  blocksleft = 32;                              // RTS data is 32*8 kB

  if (sub ())
    {
      address1 = 0;
      address2 = 0xa000;
    }
  else
    {
      address1 = mram ();
      address2 = 0x8000;
    }

  starttime = time (NULL);
  while (blocksleft > 0)
    {
      ffe_send_command (5, address1, 0);
      if (address2 == 0x8000)
        ffe_send_command0 (0xc010, 1);
      ffe_receive_block (address2, buffer, BUFFERSIZE);

      blocksleft--;
      address1++;
      fwrite (buffer, 1, BUFFERSIZE, file);

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, 256 * 1024);
      ffe_checkabort (2);
    }
  ffe_send_command (6, 3, 0);

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_write_rts (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer;
  int bytesread, bytessend = 0, size;
  unsigned short address1, address2;
  time_t starttime;

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

  size = fsizeof (filename) - SWC_HEADER_LEN;
  printf ("Send: %d Bytes\n", size);
  fseek (file, SWC_HEADER_LEN, SEEK_SET);       // skip the header

  puts ("Press q to abort\n");
  if (sub ())
    {
      address1 = 0;
      address2 = 0xa000;
    }
  else
    {
      address1 = mram ();
      address2 = 0x8000;
    }

  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      ffe_send_command (5, address1, 0);
      if (address2 == 0x8000)
        ffe_send_command0 (0xc010, 1);
      ffe_send_block (address2, buffer, bytesread);
      address1++;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }
  ffe_send_command (6, 3, 0);

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_read_cart_sram (const char *filename, unsigned int parport, int io_mode)
{
  FILE *file;
  unsigned char *buffer, byte;
  int bytesreceived = 0, size;
  unsigned short address;
  time_t starttime;

  ffe_init_io (parport);

  if (io_mode & SWC_IO_DX2_TRICK)
    dx2_trick = 1;

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

  size = receive_rom_info (buffer, io_mode);
  if (size == 0)
    {
      fputs ("ERROR: There is no cartridge present in the Super Wild Card\n", stderr);
      fclose (file);
      remove (filename);
      exit (1);
    }

  ffe_send_command (5, 3, 0);                   // detect cartridge SRAM size because
  ffe_send_command0 (0xe00c, 0);                //  we don't want to read too few data
  byte = read_cartridge1 (io_mode & SWC_IO_SUPER_FX ? 0x00ffbd : 0x00ffd8);
  size = MAX ((byte ? 1 << (byte + 10) : 0), 32 * 1024);
  printf ("Receive: %d Bytes\n", size);

  memset (buffer, 0, SWC_HEADER_LEN);
  buffer[8] = 0xaa;
  buffer[9] = 0xbb;
  buffer[10] = 5;
  fwrite (buffer, 1, SWC_HEADER_LEN, file);

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00c, 0);
//  ffe_send_command0 (0xc008, 0);

  puts ("Press q to abort\n");                  // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = hirom ? 0x2c3 : 0x1c0;

  starttime = time (NULL);
  while (bytesreceived < size)
    {
      set_bank_and_page ((unsigned char) (address >> 2), (unsigned char) (address & 3));
      ffe_receive_block ((unsigned short) (hirom ? 0x6000 : 0x2000), buffer, BUFFERSIZE);
      fwrite (buffer, 1, BUFFERSIZE, file);
      address += hirom ? 4 : 1;

      bytesreceived += BUFFERSIZE;
      ucon64_gauge (starttime, bytesreceived, size);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}


int
swc_write_cart_sram (const char *filename, unsigned int parport, int io_mode)
{
  FILE *file;
  unsigned char *buffer, byte;
  int bytesread, bytessend = 0, size;
  unsigned short address;
  time_t starttime;

  ffe_init_io (parport);

  if (io_mode & SWC_IO_DX2_TRICK)
    dx2_trick = 1;

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

  size = receive_rom_info (buffer, io_mode);
  if (size == 0)
    {
      fputs ("ERROR: There is no cartridge present in the Super Wild Card\n", stderr);
      fclose (file);
      exit (1);
    }

  ffe_send_command (5, 3, 0);                   // detect cartridge SRAM size because we don't
  ffe_send_command0 (0xe00c, 0);                //  want to write more data than necessary
  byte = read_cartridge1 (io_mode & SWC_IO_SUPER_FX ? 0x00ffbd : 0x00ffd8);
  size = fsizeof (filename) - SWC_HEADER_LEN;   // SWC SRAM is 4*8 kB, emu SRAM often not
  size = MIN ((byte ? 1 << (byte + 10) : 0), size);

  printf ("Send: %d Bytes\n", size);
  fseek (file, SWC_HEADER_LEN, SEEK_SET);       // skip the header

  ffe_send_command (5, 0, 0);
  ffe_send_command0 (0xe00c, 0);
//  ffe_send_command0 (0xc008, 0);

  puts ("Press q to abort\n");                  // print here, NOT before first SWC I/O,
                                                //  because if we get here q works ;-)
  address = hirom ? 0x2c3 : 0x1c0;

  starttime = time (NULL);
  while ((bytessend < size) && (bytesread = fread (buffer, 1, MIN (size, BUFFERSIZE), file)))
    {
      set_bank_and_page ((unsigned char) (address >> 2), (unsigned char) (address & 3));
      ffe_send_block ((unsigned short) (hirom ? 0x6000 : 0x2000), buffer, bytesread);
      address += hirom ? 4 : 1;

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      ffe_checkabort (2);
    }

  free (buffer);
  fclose (file);
  ffe_deinit_io ();

  return 0;
}

#endif // USE_PARALLEL
