/*
pl.c - Pocket Linker support for uCON64

Copyright (c) 2004 Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>
Partly based on PokeLink - Copyright (c) Dark Fader / BlackThunder


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
#include "misc/itypes.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/misc.h"
#include "misc/parallel.h"
#include "misc/file.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "pl.h"


const st_getopt2_t pl_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Pocket Linker"/*"19XX Bung Enterprises Ltd"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xpl", 0, 0, UCON64_XPL,
      NULL, "send/receive ROM to/from Pocket Linker; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when ROM does not exist",
      &ucon64_wf[WF_OBJ_NGP_DEFAULT_STOP_NO_ROM]
    },
    {
      "xpli", 0, 0, UCON64_XPLI,
      NULL, "show information about inserted cartridge; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_NGP_STOP_NO_ROM]
    },
    {
      "xplm", 0, 0, UCON64_XPLM,
      NULL, "try to enable EPP mode, default is SPP mode",
      &ucon64_wf[WF_OBJ_NGP_SWITCH]
    },
#endif // USE_PARALLEL
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

// flash unlock addresses with chip select
#define fx0002aa(A) (((A) & 0x200000) | 0x0002aa)
#define fx000555(A) (((A) & 0x200000) | 0x000555)
#define fx000aaa(A) (((A) & 0x200000) | 0x000aaa)
#define fx002aaa(A) (((A) & 0x200000) | 0x002aaa)
#define fx005555(A) (((A) & 0x200000) | 0x005555)

#define set_data_read outportb (port_a, 0);
#define set_data_write outportb (port_a, 1);
#define reset_port outportb (port_a, 4);
#define clear_timeout outportb (port_9, 1);

#define CMD_READ       0xf0
#define CMD_INFO       0x90
#define CMD_ERASE      0x80

#define TYPE_BW        0x00
#define TYPE_COLOR     0x01
#define TYPE_MULTI     0x02

static short int port_8, port_9, port_a, port_b, port_c;
static parport_mode_t port_mode;
static int current_ai;
static unsigned char ai_value[4];


static void
epp_write_data (unsigned char data)
{
  outportb (port_c, data);
}


static void
spp_write_data (unsigned char data)
{
  outportb (port_8, data);
  outportb (port_a, 3);
  outportb (port_a, 1);
}


static void
write_data (unsigned char data)
{
  ai_value[current_ai] = data;
  if (port_mode == UCON64_SPP)
    spp_write_data (data);
  else
    epp_write_data (data);
}


static unsigned char
epp_read_data (void)
{
  return inportb (port_c);
}


static unsigned char
spp_read_data (void)
{
  unsigned char byte;

  outportb (port_a, 2);
  byte = (inportb (port_9) >> 3) & 0x0f;
  outportb (port_a, 6);
  byte |= (inportb (port_9) << 1) & 0xf0;
  outportb (port_a, 0);

  return byte;
}


static unsigned char
read_data (void)
{
  if (port_mode == UCON64_SPP)
    return spp_read_data ();
  else
    return epp_read_data ();
}


static void
epp_set_ai (unsigned char ai)
{
  current_ai = ai;
  outportb (port_b, ai);
}


static void
spp_set_ai (unsigned char ai)
{
  current_ai = ai;
  outportb (port_8, ai);
  outportb (port_a, 9);
  outportb (port_a, 1);
}


static void
set_ai (unsigned char ai)
{
  if (port_mode == UCON64_SPP)
    spp_set_ai (ai);
  else
    epp_set_ai (ai);
}


static void
epp_set_ai_data (unsigned char ai, unsigned char data)
{
  epp_set_ai (ai);
  epp_write_data (data);
}


static void
spp_set_ai_data (unsigned char ai, unsigned char data)
{
  spp_set_ai (ai);
  spp_write_data (data);
}


static void
set_ai_data (unsigned char ai, unsigned char data)
{
  if (port_mode == UCON64_SPP)
    spp_set_ai_data (ai, data);
  else
    epp_set_ai_data (ai, data);
}


static void
init_port (void)
{
#ifndef USE_PPDEV
  clear_timeout
#endif
  set_data_write
  set_ai_data (2, 0);
}


static void
end_port (void)
{
  set_data_write
  set_ai_data (2, 0);
  reset_port
}


static int
detect_linker (void)
{
  init_port ();
  set_data_write
  set_ai_data (1, 0x12);
  set_ai_data (0, 0x34);
  set_ai (1);
  set_data_read
  if (read_data () != 0x12)
    return 0;
  set_data_write
  set_ai (0);
  set_data_read
  if (read_data () != 0x34)
    return 0;
  end_port ();
  return 1;
}


static void
select_address (unsigned int addr, int inc)
{
  unsigned char data = (((addr >> 14) & 0x3c) | ((addr >> 13) & 0x80) |
                        (inc ? 0x01 : 0x00));   // a[20..16], auto-increment
  if (data != ai_value[2])
    set_ai_data (2, data);
  set_ai_data (1, (unsigned char) ((addr >> 8) & 0xff)); // a[15..8]
  set_ai_data (0, (unsigned char) (addr & 0xff)); // a[7..0]
}


static void
write_address_data (unsigned int addr, unsigned char data)
{
  select_address (addr, 0);
  set_ai_data (3, data);
}


static void
send_command (unsigned char cmd)
{
  set_data_write
  write_address_data (0x5555, 0xaa);
  write_address_data (0x2aaa, 0x55);
  write_address_data (0x5555, cmd);
}


static void
reset_read (void)
{
  send_command (CMD_READ);
}


static unsigned char
read_ai3_data (void)
{
  set_ai (3);
  set_data_read
  return read_data ();
}


static int
detect_chip (void)
{
  int ai;
  unsigned char info[4];

  reset_read ();
  reset_read ();
  reset_read ();
  send_command (CMD_INFO);
  for (ai = 0; ai < 4; ai++)
    {
      set_data_write
      select_address (ai, 0);
      info[ai] = read_ai3_data ();
    }
  reset_read ();
  if (((info[0] & 0x90) == 0x90) && (info[2] == 0x01) && (info[3] & 0x80))
    return 1;
  else
    return 0;
}


static void
select_chip (unsigned int addr)
{
  set_data_write
  set_ai_data (2, 2);
  set_ai_data (3, (unsigned char) ((addr & 0x200000) ? 1 : 2));
  set_ai_data (2, 0);
}


static unsigned int
cart_size (void)
{
  select_chip (0x000000);
  if (!detect_chip ())
    return 0;                                   // no cartridge found
  select_chip (0x200000);
  if (!detect_chip ())
    return 0x200000;                            // 16 megabit
  return 0x400000;                              // 32 megabit
}


static void
read_blocks (unsigned int addr, unsigned char *buf, int blocks)
{
  int block, i, offset = 0;

  select_chip (addr);
  for (block = 0; block < blocks; block++)
    {
      set_data_write
      select_address (addr | (block << 8), 1);
      set_ai (3);
      set_data_read
      for (i = 0; i < 0x100; i++)
        buf[offset++] = read_data ();
    }
}


static int
is_erased (unsigned char *buf, unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
    if (buf[i] != 0xff)
      return 0;

  return 1;
}


static int
is_header (unsigned char *buf)
{
  char msg[0x1d];

  strncpy (msg, (char *) buf, 0x1c);
  msg[0x1c] = '\0';

  if (strstr (msg, "COPYRIGHT") || strstr (msg, "SNK") ||
      strstr (msg, "LICENSED") || strstr (msg, "CORPORATION"))
    return 1;                                   // header found

  return 0;                                     // other data found
}


static int
same_header (unsigned char *header, unsigned char *buf)
{
  return (!memcmp (header, buf, 0x100));
}


static unsigned int
game_info (unsigned int cart_size, char name[13], int *type)
{
  unsigned char header[0x100];
  unsigned char buf[0x8000];

  select_chip (0x000000);
  read_blocks (0x000000, header, 1);

  if (!is_header (header))
    return 0;                                   // no game found

  if (name)
    {
      strncpy (name, (char *) (header + 0x24), 12);
      name[12] = '\0';
    }

  if (type)
    {
      if (strstr (name, "Multi"))
        *type = TYPE_MULTI;
      else if (header[0x23] == 0x10)
        *type = TYPE_COLOR;
      else
        *type = TYPE_BW;
    }

  read_blocks (0x080000, buf, 128);
  if ((same_header (buf, header)) || is_erased (buf, 0x8000))
    return 0x080000;                            // 4 megabit

  read_blocks (0x100000, buf, 128);
  if ((same_header (buf, header)) || is_erased (buf, 0x8000))
    return 0x100000;                            // 8 megabit

  if (cart_size == 0x400000)
    {
      read_blocks (0x200000, buf, 128);
      if (is_erased (buf, 0x8000))
        return 0x200000;                        // 16 megabit
      return 0x400000;                          // 32 megabit
    }
  return 0x200000;                              // 16 megabit
}


static void
deinit_io (void)
{
  end_port ();
}


static void
init_io (unsigned int port)
{
#ifndef USE_PPDEV
  outportb ((unsigned short) (port_8 + 0x402), 0); // Set EPP mode for ECP chipsets
#endif

  port_8 = port;
  port_9 = port + 1;
  port_a = port + 2;
  port_b = port + 3;
  port_c = port + 4;

  parport_print_info ();

  if (ucon64.parport_mode == UCON64_EPP && port_8 != 0x3bc)
    port_mode = UCON64_EPP;                     // if port == 0x3bc => no EPP available
  else
    port_mode = UCON64_SPP;

  if (!detect_linker ())
    {
      port_mode = UCON64_SPP;
      if (!detect_linker ())
        {
          fputs ("ERROR: Pocket Linker not found or not turned on\n", stderr);
          deinit_io ();
          exit (1);
        }
    }

  // If we get here, a Pocket Linker was detected
  if (port_mode == UCON64_EPP)
    puts ("Pocket Linker found. EPP found");
  else
    puts ("Pocket Linker found. EPP not found or not enabled - SPP used");
}


static void
set_address (unsigned int addr, int inc)
{
  set_ai_data (0, (unsigned char) (addr & 0xff)); // a[7..0]
  set_ai_data (1, (unsigned char) ((addr >> 8) & 0xff)); // a[15..8]
  set_ai_data (2, 0x02);                        // latch chip select
  set_ai_data (3, (unsigned char) ~(1 << (addr >> 21))); // a[23..21]
  set_ai_data (2, (unsigned char) ((((addr >> 16) & 0x0f) << 2) | // a[20..16], auto-increment
                                   (((addr >> 20) & 0x01) << 7) | 
                                   (inc ? 0x01 : 0x00)));
  set_ai (3);
}


static int
program (unsigned int addr, unsigned char data, int retries)
{
  int to = 10000;

  set_data_write
  set_address (fx005555 (addr), 0);             // program byte
  write_data (0xaa);
  set_address (fx002aaa (addr), 0);
  write_data (0x55);
  set_address (fx005555 (addr), 0);
  write_data (0xa0);
  set_address (addr, 0);
  write_data (data);

  set_data_read
  while (to--)
    {
      unsigned char s = read_data ();
      if ((s & 128) == 0)
        return 0;                               // ok
      if (s & 32)
        {
          int s = read_data ();
          if ((s & 128) == 0)
            return 0;                           // ok
          if (data == read_data ())
            return 0;                           // ok
        }
    }

  set_data_write
  set_address (addr, 0);
  set_data_read
  if (data == read_data ())
    return 0;                                   // ok
  if (retries == 0)
    return 1;                                   // error
  return program (addr, data, retries - 1);
}


static int
write_block (unsigned int addr, unsigned char *buf)
{
  int count;
  select_address (addr, 1);
  for (count = 0; count < 0x100; count++)
    {
      if (buf[count] == 0xff)
        continue;                               // nothing to write
      if (program (addr + count, buf[count], 3))
        {
          select_address (addr + count, 0);
          set_data_read
          fprintf (stderr, "\nERROR: Programming failed at 0x%06x (w:0x%02x, "
                           "r:0x%02x)\n", addr + count, buf[count], read_data ());
          return 1;
        }
    }
  return 0;
}


#if 0
static int
wait_erased (void)
{
  int i = 0;
  unsigned char cur_byte, prev_byte;

  prev_byte = read_ai3_data () & 0x40;
  while (++i < 0x7ffffff)
    {
      cur_byte = read_data () & 0x40;
      if (cur_byte == prev_byte)
        return 0;                               // ok
      prev_byte = cur_byte;
    }
  return 1;                                     // erase error
}


static int
erase_chip (void)
{
  reset_read ();
  send_command (CMD_ERASE);
  write_address_data (0x5555, 0xaa);
  write_address_data (0x2aaa, 0x55);
  write_address_data (0x5555, 0x10);
  return wait_erased ();
}


static int
erase_cart (unsigned int size)
{
  unsigned int addr;
  unsigned char buf[0x8000];

  for (addr = 0x000000; addr < size; addr += 0x200000)
    {
      select_chip (addr);
      if (erase_chip ())
        {
          fprintf (stderr, "ERROR: Erase chip %d failed\n", addr >> 21);
          return 1;
        }

      read_blocks (0x000000, buf, 128);
      if (!is_erased (buf, 0x8000))
        {
          fprintf (stderr, "ERROR: Erase chip %d verify failed\n", addr >> 21);
          return 1;
        }
    }
}
#endif


static int
erase (void)
{
  unsigned int addr;

  for (addr = 0; addr < 0x400000; addr += 0x200000)
    {
      set_data_write
      set_address (fx005555 (addr), 0);
      write_data (0xaa);
      set_address (fx002aaa (addr), 0);
      write_data (0x55);
      set_address (fx005555 (addr), 0);
      write_data (0x80);
      set_address (fx005555 (addr), 0);
      write_data (0xaa);
      set_address (fx002aaa (addr), 0);
      write_data (0x55);
      set_address (fx005555 (addr), 0);
      write_data (0x10);

      set_data_read
      while (~read_data () & 0x80)              // wait for completion
        ;
    }
  return 0;
}


int
pl_read_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned int blocksleft, c_size, size, address = 0;
  unsigned char buffer[0x8000];
  time_t starttime;

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  init_io (parport);

  c_size = cart_size ();

  if (c_size == 0x00)                           // Check for cartridge
    {
      fputs ("ERROR: No cartridge detected\n", stderr);
      deinit_io ();
      exit (1);
    }

  size = game_info (c_size, NULL, NULL);

  if (size == 0x00)
    {
      fputs ("ERROR: No game detected\n", stderr);
      deinit_io ();
      exit (1);
    }

  reset_read ();

  printf ("Receive: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  blocksleft = size >> 15;
  starttime = time (NULL);

  while (blocksleft-- > 0)
    {
      read_blocks (address, buffer, 128);
      fwrite (buffer, 1, 0x8000, file);
      address += 0x8000;
      ucon64_gauge (starttime, address, size);
    }

  fclose (file);
  deinit_io ();

  return 0;
}


int
pl_write_rom (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned int size, bytesread, bytessent = 0, address = 0;
  unsigned char buffer[0x100];
  time_t starttime;

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  init_io (parport);
  size = fsizeof (filename);

  erase ();
  reset_read ();

  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

  starttime = time (NULL);

  while ((bytesread = fread (buffer, 1, 0x100, file)))
    {
      if (write_block (address, buffer))
        break;                                  // write failed
      bytessent += bytesread;
      address += 0x100;
      ucon64_gauge (starttime, bytessent, size);
    }

  fclose (file);
  deinit_io ();

  return 0;
}


int
pl_info (unsigned int parport)
{
  unsigned int c_size, g_size;
  int g_type;
  char g_name[13];

  init_io (parport);

  c_size = cart_size ();
  if (c_size == 0)
    {
      printf ("No cartridge found\n");
      deinit_io ();
      return 0;
    }

  g_size = game_info (c_size, g_name, &g_type);

  if (g_size == 0)
    printf ("No game found\n");
  else
    {
      printf ("Game name: \"%s\"\n", g_name);
      printf ("Game type: %s\n", (g_type == TYPE_COLOR) ? "Color" : "B&W");
      printf ("Game size: %d Bytes (%.4f Mb)\n", g_size, (float) g_size / MBIT);
    }
  deinit_io ();

  return 0;
}

#endif // USE_PARALLEL
