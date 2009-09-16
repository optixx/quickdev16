/*
doctor64jr.c - Bung Doctor V64 Junior support for uCON64

Copyright (c) 1999 - 2002 NoisyB <noisyb@gmx.net>
Copyright (c) 2004        dbjh


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

/*
drjr transfer protocol


DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX  ai  XXXXX  data  XXXX
p1    nwrite   ~~~~~~~~~|_____________________|~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~~~~~~~~|_|~~~~~~
p17   nastb    ~~~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~


ai[]=0	r/w a[7..0]
ai[]=1	r/w a[15..8]
ai[]=2	r/w a[23..16]
ai[]=3	w a[28..24]
ai[]=3  r (rst,wdf,wcf,a[28..24])
ai[]=4	r/w data
ai[]=5	w mode
ai[]=6	w en_1
ai[]=7	w en_0
*remark
        a[8..1] support page count up

        ai[3]d7:0=N64 power off, 1=N64 power on
             d6:0=no dram data written, 1=dram data written
             d5:0=no data write in b4000000~b7ffffff, 1=some data written in b4000000~b7ffffff

        mode d0:0=dram read only and clear wdf, 1=dram write enable
             d1:0=disable cartridge read and clear wcf flag,
                1=enable cartridge read(write b4000000~b7ffffff will switch off dram and cartridge will present at b0000000~b3ffffff)

        en_0=05 and en_1=0a is enable port control


mode:q0              0                   1                  0                    1
mode:q1              0                   0                  1                    1
b7ff ffff
b400 0000      dram read only         dram r/w        cartridge read     cartridge read(* write this area will switch off dram)

b3ff ffff
b000 0000      dram read only         dram r/w        dram read only        dram r/w


eg:enable port control

DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX 07 XX 05 XXXX 06 XX 0a XXXXXXXXXXXX
p1    nwrite   ~~~~~~~~~|_____________________________|~~~~~~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~
p17   nastb    ~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~
                            en_0=05       en_1=0a


eg:write adr $b0123456, data $a55a,$1234..

DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX 00 XX 56 XXXX 01 XX 34 XXXX 02 XX 12 XXXX 03 XX b0 XXXXXX 04 XX 5a XX a5 XX 34 XX 12 XXXXXXXXXXX
p1    nwrite   ~~~~~~~~~|_______________________________________________________________________________________|~~~~~~~~~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~|_|~~~|_|~~~|_|~~~|_|~~~~~~~~~~~
p17   nastb    ~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                              set adr word low            set adr word high                wdata a55a  wdata 1234 (after write adr=b012345a)


eg:read adr $b0123400~$b01235ff, 512 data

DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX 00 XX 00 XXXX 01 XX 34 XXXX 02 XX 12 XXXX 03 XX b0 XXXXXX 04 XX data0 XX data1 X ... X data510 XX data511 XXXXX
p1    nwrite   ~~~~~~~~~|________________________________________________________________~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~~|_|~~~~~~|_|~~~ ~~~ ~~~~|_|~~~~~~~~|_|~~~~~~~~
p17   nastb    ~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                              set adr word low            set adr word high                (after 512 read adr=b0123400)


eg:dram write protect, disable N64 access to cartridge and disable port control

DB25  pin name
p2~p9 pd[7:0]  XXXXXXXXXXX 05 XX 00 XXXX 07 XX 00 XXXX 06 XX 00 XXXXXXXXXXXX
p1    nwrite   ~~~~~~~~~|________________________________________|~~~~~~~~~~
p14   ndstb    ~~~~~~~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~
p17   nastb    ~~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~|_|~~~~~~~~~~~~~~~~~~
                            mode=00       en_0=00       en_1=00


simple backup rountine for N64

void writePI(unsigned long addr, unsigned long value)
{
  do {} while (*(volatile unsigned long *) (0xa4600010) & 3);     // check parallel interface not busy
  addr &=0xbffffffc;
  *(unsigned long *)(addr)=value;
}

unsigned long readPI(unsigned long addr)
{
  do {} while (*(volatile unsigned long *) (0xa4600010) & 3);     // check parallel interface not busy
  addr &=0xbffffffc;
  return *(unsigned long *)(addr);
}

// MAIN -- START OF USER CODE
void mainproc(void *arg) {
    u32 base_adr;
    for (base_adr=0;base_adr<0x1000000;base_adr++){                     // backup 128Mbits
       writePI(0xb0000000+base_adr,readPI(0xb4000000 + base_adr));      // write data
    }
}
*/

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "misc/misc.h"
#include "misc/itypes.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/parallel.h"
#include "misc/file.h"
#include "ucon64.h"
#include "ucon64_misc.h"
#include "doctor64jr.h"


const st_getopt2_t doctor64jr_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Doctor V64 Junior"/*"19XX Bung Enterprises Ltd http://www.bung.com.hk"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xdjr", 0, 0, UCON64_XDJR,
      NULL, "send ROM to Doctor V64 Junior; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_N64_DEFAULT_STOP_NO_ROM]
    },
#if 0
    {
      "xdjrs", 0, 0, UCON64_XDJRS,
      NULL, "send/receive SRAM to/from Doctor V64 Junior; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_N64_DEFAULT_STOP_NO_ROM]
    },
#endif
#endif // USE_PARALLEL
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef USE_PARALLEL

#define BUFFERSIZE 32768
//#define set_ai_write outportb (port_a, 5);    // ninit=1, nwrite=0
#define set_data_write outportb (port_a, 1);    // ninit=0, nwrite=0
#define set_data_read outportb (port_a, 0);     // ninit=0, nwrite=1
//#define set_normal outportb (port_a, 4);      // ninit=1, nwrite=1

static unsigned short int port_8, port_9, port_a, port_b, port_c,
                          *buffer;
static int wv_mode;


static void
set_ai (unsigned char ai)
{
  outportb (port_a, 5);                         // ninit=1, nwrite=0
  outportb (port_b, ai);
}


static void
set_ai_data (unsigned char ai, unsigned char data)
{
  set_ai (ai);
  set_data_write                                // ninit=0, nwrite=0
  outportb (port_c, data);
}


static void
init_port (int enable_write)
{
#ifndef USE_PPDEV
  outportb (port_9, 1);                         // clear EPP time flag
#endif
  set_ai_data (6, 0x0a);
  set_ai_data (7, 0x05);                        // 6==0x0a, 7==0x05 is pc_control mode
//  set_ai (5);
//  set_data_read
//  enable_write = inportb (port_c);
  set_ai_data (5, (unsigned char) enable_write); // d0=0 is write protect mode
}


static void
end_port (int enable_write)
{
  set_ai_data (5, (unsigned char) enable_write); // d0=0 is write protect mode
  set_ai_data (7, 0);                           // release pc mode
  set_ai_data (6, 0);                           // 6==0x0a, 7==0x05 is pc_control mode
  outportb (port_a, 4);                         // ninit=1, nwrite=1
}


static int
check_card (void)
{
  set_ai_data (3, 0x12);
  set_ai_data (2, 0x34);
  set_ai_data (1, 0x56);
  set_ai_data (0, 0x78);

  set_ai (3);
  set_data_read                                 // ninit=0, nwrite=1
  if ((inportb (port_c) & 0x1f) != 0x12)
    return 1;

  set_ai (2);
  set_data_read
  if (inportb (port_c) != 0x34)
    return 1;

  set_ai (1);
  set_data_read
  if (inportb (port_c) != 0x56)
    return 1;

  set_ai (0);
  set_data_read
  if (inportb (port_c) != 0x78)
    return 1;

  return 0;
}


static int
write_32k (unsigned short int hi_word, unsigned short int lo_word)
{
  unsigned char unpass, pass1;
  unsigned short int i, j, fix;

  set_ai_data (3, (unsigned char) (0x10 | (hi_word >> 8)));
  set_ai_data (2, (unsigned char) hi_word);
  for (i = 0; i < 0x40; i++)
    {
      unpass = 3;
      while (unpass)
        {
          set_ai_data (1, (unsigned char) ((i << 1) | lo_word));
          set_ai_data (0, 0);
          set_ai (4);                           // set address index=4
          set_data_write                        // ninit=0, nwrite=0
          fix = i << 8;
          for (j = 0; j < 256; j++)
            outportw (port_c, buffer[j + fix]);
          set_data_read                         // ninit=0, nwrite=1
          if (wv_mode)
            {
              for (j = 0; j < 256; j++)
                if (inportw (port_c) != buffer[j + fix])
                  break;
            }
          else
            {
              pass1 = 1;
              for (j = 0; j < 4; j++)
                if (inportw (port_c) != buffer[j + fix])
                  {
                    pass1 = 0;
                    break;
                  }
              if (pass1)
                {
                  set_ai_data (1, (unsigned char) ((i << 1) | lo_word | 1));
                  set_ai_data (0, 0xf8);
                  set_ai (4);
                  set_data_read                 // ninit=0, nwrite=1
                  for (j = 252; j < 256; j++)
                    if (inportw (port_c) != buffer[j + fix])
                      break;
                }
            }
          set_ai (0);
          set_data_read                         // ninit=0, nwrite=1
          if (inportb (port_c) != 0)
            {
              unpass--;
              outportb (port_a, 0x0b);          // set all pin=0 for debug
              set_ai_data (3, (unsigned char) (0x10 | (hi_word >> 8)));
              set_ai_data (2, (unsigned char) hi_word);
              if (unpass == 0)
                return 1;
            }
          else
            unpass = 0;
        }
    }

/*
  outportb (ai, 0);
  printf ("\na[7..0]=%02x\n", inportb (data));
  outportb (ai, 1);
  printf ("a[15..8]=%02x\n", inportb (data));
*/
  return 0;
}


#if 0 // not used
static int
verify_32k (unsigned short int hi_word, unsigned short int lo_word)
{
  char unpass;
  unsigned short int i, j, fix;

  set_ai_data (3, (unsigned char) (0x10 | (hi_word >> 8)));
  set_ai_data (2, (unsigned char) hi_word);
  for (i = 0; i < 0x40; i++)
    {
      unpass = 3;
      while (unpass)
        {
          set_ai_data (1, (unsigned char) ((i << 1) | lo_word));
          set_ai_data (0, 0);
          set_ai (4);
          set_data_read                         // ninit=0, nwrite=1
          fix = i << 8;
          for (j = 0; j < 256; j++)
            {
              if (inportw (port_c) != buffer[j + fix])
                {
                  outportb (port_a, 0x0b);      // all pin=0 for debug
                  set_ai_data (3, (unsigned char) (0x10 | (hi_word >> 8)));
                  set_ai_data (2, (unsigned char) hi_word);
                  unpass--;
                  if (unpass == 0)
                    return 1;
                  else
                    break;
                }
            }
          if (j == 256)
            break;
        }
    }

/*
  outportb (ai,0);
  printf ("\na[7..0]=%02x\n", inportb (data));
  outportb (ai, 1);
  printf ("a[15..8]=%02x\n", inportb (data));
*/
  return 0;
}


static void
gen_pat_32k (unsigned short int offset)
{
  int i;

  for (i = 0; i < 0x4000; i++)
    buffer[i] = i + offset;
}


static unsigned short int
test_dram (void)
{
  int n_pages = 0, page;

  gen_pat_32k (0x0000);
  write_32k (0, 0);

  gen_pat_32k (0x8000);
  write_32k (0x100, 0);

  gen_pat_32k (0x0000);
  if (verify_32k (0, 0) == 0)                   // find lower 128 Mbits
    n_pages = 0x100;
  gen_pat_32k (0x8000);
  if (verify_32k (0x100, 0) == 0)               // find upper 128 Mbits
    n_pages = 0x200;

  printf ("Testing DRAM...\n");

  for (page = 0; page < n_pages; page++)
    {
      gen_pat_32k ((unsigned short int) (page * 2));
      if (write_32k (page, 0))
        return 0;
      else
        {
          fputc ('w', stdout);
          fflush (stdout);
        }

      gen_pat_32k ((unsigned short int) (page * 2 + 1));
      if (write_32k (page, 0x80))
        return 0;
      else
        {
          fputc ('w', stdout);
          fflush (stdout);
        }
    }

  fputc ('\n', stdout);
  for (page = 0; page < n_pages; page++)
    {
      gen_pat_32k ((unsigned short int) (page * 2));
      if (verify_32k (page, 0))
        return 0;
      else
        {
          fputc ('v', stdout);
          fflush (stdout);
        }
      gen_pat_32k ((unsigned short int) (page * 2 + 1));
      if (verify_32k (page, 0x80))
        return 0;
      else
        {
          fputc ('v', stdout);
          fflush (stdout);
        }
    }

  return n_pages;
}
#endif


static unsigned long int
get_address (void)
{
  unsigned long int address;

  set_ai_data (6, 0x0a);                        // enable pc mode
  set_ai_data (7, 0x05);                        // enable pc mode

  set_ai (3);
  set_data_read                                 // ninit=0, nwrite=1
  address = inportb (port_c) << 24;

  set_ai (2);
  set_data_read
  address |= inportb (port_c) << 16;

  set_ai (1);
  set_data_read
  address |= inportb (port_c) << 8;

  set_ai (0);
  set_data_read
  address |= inportb (port_c);

  return address;
}


int
doctor64jr_read (const char *filename, unsigned int parport)
{
  (void) filename;
  (void) parport;
  return fprintf (stderr, "ERROR: The function for dumping a cartridge is not yet implemented for the\n"
                          "       Doctor V64 Junior\n");
}


int
doctor64jr_write (const char *filename, unsigned int parport)
{
  unsigned int enable_write = 0, init_time, size, bytesread, bytessend = 0,
               n_pages;
  unsigned short int page;
  FILE *file;

  parport_print_info ();

  port_8 = parport;
  port_9 = parport + 1;
  port_a = parport + 2;
  port_b = parport + 3;
  port_c = parport + 4;

  init_port (enable_write);

  if (check_card () != 0)
    {
      fprintf (stderr, "ERROR: No Doctor V64 Junior card present\n");
      end_port (enable_write);
      exit (1);
    }

  wv_mode = 0;

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned short int *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  size = fsizeof (filename);
  printf ("Send: %d Bytes (%.4f Mb)\n\n", size, (float) size / MBIT);

#if 0
  if (dram_test)
    {
      dram_size = test_dram ();
      if (dram_size)
        printf ("\nDRAM size=%dMbits\n", (dram_size / 2));
      else
        fprintf (stderr, "\nERROR: DRAM test failed\n");
      return 0;
    }
#endif

  n_pages = (size + (64 * 1024 - 1)) / (64 * 1024); // "+ (64 * 1024 - 1)" to round up
  init_time = time (0);
  for (page = 0; page < n_pages; page++)
    {
      bytesread = fread ((unsigned char *) buffer, 1, BUFFERSIZE, file);
      if (write_32k (page, 0))
        {
          fprintf (stderr, "ERROR: Transfer failed at address 0x%8lx", get_address ());
          break;
        }

      bytesread += fread ((unsigned char *) buffer, 1, BUFFERSIZE, file);
      if (write_32k (page, 0x80))
        {
          fprintf (stderr, "ERROR: Transfer failed at address 0x%8lx", get_address ());
          break;
        }

      bytessend += bytesread;
      ucon64_gauge (init_time, bytessend, size);
    }
  fputc ('\n', stdout);

  if (enable_write)                             // 1 or 3
    printf ("DRAM write protect disabled\n");
  if (enable_write & 2)                         // 3
    printf ("Run cartridge enabled\n");

//  set_ai_data(5, enable_write);                 // d0=0 is write protect mode
  end_port (enable_write);

  return 0;
}

#endif // USE_PARALLEL
