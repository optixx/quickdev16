/*
gbx.c - Game Boy Xchanger/GBDoctor support for uCON64

Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2004 dbjh
Based on gbt15.c - Copyright (c) Bung Enterprises


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

/********************************************************************
*  use parallel EPP/SPP port to r/w game boy cartridge              *
*                                                                   *
* ai[]=0 w a[7..0]                                                  *
* ai[]=1 w a[15..8]                                                 *
* ai[]=2 w control d7=rs,d6=spp,d1=xwe_en,d0=cs_en                  *
* ai[]=3 r/w data                                                   *
*                                                                   *
* MBC1                                                              *
*   R/W A000~BFFF   RAM SWITCHING BANK(256Kbit) 4 BANKS OF 8Kbyte   *
*   R 4000~7FFF     ROM SWITCHING BANK(4Mbit) 32 BANKS OF 128Kbit   *
*   W 2000~3FFF     SET ROM BANK (5 BIT)                            *
*   R 0000~3FFF     FIX ROM BANK 0                                  *
*   W 4000~5FFF     SET RAM BANK (2 BIT)                            *
*   W 0000~1FFF     SET 0A ENABLE RAM BANK                          *
*                                                                   *
* MBC2                                                              *
*   R/W A000~BFFF   512 X 4 BIT RAM                                 *
*   R 4000~7FFF     ROM SWITCHING BANK(2Mbit) 16 BANKS OF 128Kbit   *
*   W 2100          SET ROM BANK (4 BIT)                            *
*   R 0000~3FFF     FIX ROM BANK 0                                  *
*   W 0000          SET 0A ENABLE RAM BANK                          *
*                                                                   *
* MBC5                                                              *
*   R/W A000~BFFF   RAM SWITCHING BANK(1Mbit) 16 BANKS OF 64 Kbit   *
*   R 4000~7FFF     ROM SWITCHING BANK(64Mbit) 512 BANKS OF 128Kbit *
*   W 3000~3FFF     SET ROM BANK1(BANK Q8)  TOTAL 9 BIT             *
*   W 2000~2FFF     SET ROM BANK0(BANK Q7~Q0)                       *
*   R 0000~3FFF     FIX ROM BANK 0                                  *
*   W 4000~7FFF     SET RAM BANK (4 BIT)                            *
*   W 0000~1FFF     SET 0A ENABLE RAM BANK                          *
*                                                                   *
********************************************************************/

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
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
#include "gbx.h"
#include "console/gb.h"                         // gb_logodata, rocket_logodata


const st_getopt2_t gbx_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Game Boy Xchanger/GBDoctor"/*"19XX Bung Enterprises Ltd http://www.bung.com.hk"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xgbx", 0, 0, UCON64_XGBX,
      NULL, "send/receive ROM to/from GB Xchanger; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when ROM does not exist",
      &ucon64_wf[WF_OBJ_GB_DEFAULT_STOP_NO_ROM]
    },
    {
      "xgbxs", 0, 0, UCON64_XGBXS,
      NULL, "send/receive SRAM to/from GB Xchanger; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_GB_STOP_NO_ROM]
    },
    {
      "xgbxb", 1, 0, UCON64_XGBXB,
      "BANK", "send/receive 64 kbits SRAM to/from GB Xchanger BANK\n"
      "BANK can be a number from 0 to 15; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when ROM does not exist",
      &ucon64_wf[WF_OBJ_GB_STOP_NO_ROM]
    },
    {
      "xgbxm", 0, 0, UCON64_XGBXM,
      NULL, "try to enable EPP mode, default is SPP mode",
      &ucon64_wf[WF_OBJ_GB_SWITCH]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef USE_PARALLEL

//#define set_ai_write outportb (port_a, 5);    // ninit=1, nwrite=0
#define set_data_read outportb (port_a, 0);     // ninit=0, nastb=1, nib_sel=0, ndstb=1, nwrite=1
#define set_data_write outportb (port_a, 1);    // ninit=0, nastb=1, nib_sel=0, ndstb=1, nwrite=0
//#define set_normal outportb (port_a, 4);      // ninit=1, nwrite=1

typedef enum { UNKNOWN_MBC, BUNG, ROM, MBC1, MBC2, MBC3, MBC5, CAMERA, ROCKET } mbc_t;
typedef enum { UNKNOWN_EEPROM, WINBOND, MX, INTEL } eeprom_t;

static unsigned short int port_8, port_9, port_a, port_b, port_c, rocket_game_no;
static unsigned char buffer[32768];
static mbc_t mbc_type;
static eeprom_t eeprom_type;
static parport_mode_t port_mode;


static volatile void
delay_us (int n_us)
// Hmmm, usleep() can be used under UNIX, but default under DOS is delay() which
//  waits for a number of milliseconds... Better use the same code on all platforms.
{
  volatile int n;
  int n_max;

  // it's probably best to use the original strange loop values
  if (n_us == 10)
    n_max = 0x2000;
  else if (n_us == 100)
    n_max = 0x10000;
  else if (n_us == 20000)
    n_max = 0xfffff;
  else
    n_max = n_us * 1000;

  for (n = 0; n < n_max; n++)
    ;
}


static void
spp_set_ai (unsigned char ai)
{
  set_data_write
//  outportb (port_a, 1);                         // nastb=1,nib_sel=0,ndstb=1,nwrite=0
  outportb (port_8, ai);                        // put ai at data bus
  outportb (port_a, 9);                         // nastb=0,nib_sel=0,ndstb=1,nwrite=0
  outportb (port_a, 1);                         // nastb=1,nib_sel=0,ndstb=1,nwrite=0
  // nastb ~~~~|___|~~~~
}


static void
spp_write_data (unsigned char data)
{
//  outportb (port_a, 1);                         // nastb=1,nib_sel=0,ndstb=1,nwrite=0
  outportb (port_8, data);                      // put data at data bus
  outportb (port_a, 3);                         // nastb=1,nib_sel=0,ndstb=0,nwrite=0
  outportb (port_a, 1);                         // nastb=1,nib_sel=0,ndstb=1,nwrite=0
  // ndstb ~~~~|___|~~~~
}


static void
spp_set_ai_data (unsigned char ai, unsigned char data)
{
  spp_set_ai (ai);
  spp_write_data (data);
}


static unsigned char
spp_read_data (void)
{
  unsigned char byte;

  set_data_read
  outportb (port_a, 2);                         // nastb=1,nib_sel=0,ndstb=0,nwrite=1
  byte = (inportb (port_9) >> 3) & 0x0f;
  outportb (port_a, 6);                         // nastb=1,nib_sel=1,ndstb=0,nwrite=1
  byte |= (inportb (port_9) << 1) & 0xf0;
  outportb (port_a, 0);                         // nastb=1,nib_sel=0,ndstb=1,nwrite=1
  // nibble_sel ___|~~~ and ndstb ~~~~|___|~~~~

  return byte;
}


static void
epp_set_ai (unsigned char ai)
{
  set_data_write
  outportb (port_b, ai);
}


static void
epp_set_ai_data (unsigned char ai, unsigned char data)
{
  epp_set_ai (ai);
  set_data_write
  outportb (port_c, data);
}


static void
set_ai (unsigned char ai)
{
  set_data_write
  if (port_mode == UCON64_SPP)
    spp_set_ai (ai);
  else
    epp_set_ai (ai);
}


static void
set_ai_data (unsigned char ai, unsigned char data)
{
  if (port_mode == UCON64_SPP)
    spp_set_ai_data (ai, data);                 // SPP mode
  else
    epp_set_ai_data (ai, data);                 // EPP mode
}


static void
write_data (unsigned char data)
{
  if (port_mode == UCON64_SPP)
    spp_write_data (data);                      // SPP write data
  else
    outportb (port_c, data);                    // EPP write data
}


static unsigned char
read_data (void)
{
  if (port_mode == UCON64_SPP)
    return spp_read_data ();                    // SPP read data
  else
    return inportb (port_c);                    // EPP read data
}


static void
init_port (void)
{
#ifndef USE_PPDEV
  outportb (port_9, 1);                         // clear EPP time flag
#endif
  set_ai_data ((unsigned char) 2, 0);           // rst=0, wei=0(dis.), rdi=0(dis.)
  set_ai_data ((unsigned char) 2, 0x80);        // rst=1, wei=0(dis.), rdi=0(dis.)
}


static void
end_port (void)
{
  set_ai_data ((unsigned char) 2, 0);           // rst=0, wei=0(dis.), rdi=0(dis.)
  outportb (port_a, 4);                         // ninit=1, nwrite=1
}


static void
set_adr (unsigned int adr)
{
  set_ai_data ((unsigned char)  0, (unsigned char) adr); // a[7..0]
  set_ai_data ((unsigned char)  1, (unsigned char) (adr >> 8)); // a[15..8]
  set_ai (3);
  set_data_read                                 // ninit=0, nwrite=1
}


static void
set_adr_long (unsigned int adr, int ignore_xh)  // real address
{
  unsigned char xh, h, m, l;

  set_ai_data ((unsigned char) 2, 0x80);        // disable wr/rd inc.
  l = (unsigned char) adr;                      // a7-a0
  m = (unsigned char) (adr >> 8) & 0x3f;        // a13-a8
  h = (unsigned char) (adr >> 14) & 0xff;       // a21-a13
  if (h)
    m |= 0x40;                                  // > bank 0

  if (!ignore_xh)
    {
      xh = (unsigned char) (adr >> 22) & 0x7;   // max. 256Mbit
      if (xh)
        m |= 0x40;                              // > bank 0
      set_adr (0x3000);                         // write 3000:xh
      set_data_write
      write_data (xh);                          // set ROM bank extend value
    }

  set_adr (0x2000);                             // write 2000:h
  set_data_write
  write_data (h);                               // set ROM bank value
  set_ai_data ((unsigned char) 1, m);           // a[15..8]
  set_ai_data ((unsigned char) 0, l);           // a[7..0]
}


static void
set_bank (unsigned int adr, unsigned char bank)
{
  set_ai_data ((unsigned char) 2, 0x80);        // disable inc
  set_ai_data ((unsigned char) 0, (unsigned char) adr); // a[7..0]
  set_ai_data ((unsigned char) 1, (unsigned char) ((adr >> 8) & 0x7f)); // a[15..8]
  set_ai_data ((unsigned char) 3, bank);        // write bank no
  set_data_read                                 // ninit=0, nwrite=1
}


static void
out_byte_eeprom (unsigned char data)
{
  set_ai_data ((unsigned char) 2, 0x82);        // wei enable
  set_ai (3);                                   // default write mode
//  set_data_read                                 // ninit=0, nwrite=1
  set_data_write
  write_data (data);                            // out data
  set_ai_data ((unsigned char) 2, 0x80);        // wei disable
  set_ai (3);                                   // default write mode
}


static void
out_byte (unsigned char data)
{
  set_ai (3);
//  set_data_read                                 // ninit=0, nwrite=1
  set_data_write
  write_data (data);                            // out data
}


static void
out_data (unsigned char h, unsigned char m, unsigned char l, unsigned char data)
{
  // ai[]=2 w control d7=rs,d1=xwe_en,d0=cs_en
  h = ((h << 2) | (m >> 6)) & 0x1f;             // maximum bank is 1f
  if (h)
    m = (m & 0x3f) | 0x40;                      // > bank 0
  else
    m = m & 0x3f;                               // bank 0

  set_adr (0x2000);                             // write 2000:h
  set_data_write
  write_data (h);                               // set ROM bank value
  set_ai_data ((unsigned char) 1, m);           // a[15..8]
  set_ai_data ((unsigned char) 0, l);           // a[7..0]
  out_byte_eeprom (data);                       // write data to EEPROM
}


static void
out_adr2_data (unsigned int adr, unsigned char data) // address shift 1 bit
{
  set_adr_long (adr << 1, 1);                   // adr x 2
  out_byte_eeprom (data);                       // write data to EEPROM
}


static void
out_adr_data (unsigned int adr, unsigned char data) // real address
{
  set_adr_long (adr, 0);
  out_byte_eeprom (data);                       // write data to EEPROM
}


static void
out_adr_data_32k (unsigned int adr, unsigned char data)
{
  set_adr (adr);
  out_byte_eeprom (data);                       // write data to EEPROM
}


static unsigned char
read_byte (void)
{
  set_ai (3);                                   // default write mode
  set_data_read                                 // ninit=0, nwrite=1
  return read_data ();
}


static void
enable_protection (void)
{
//  set_bank (0x2000,0);                          // set bank 0
  out_data (0, 0x55, 0x55, 0xaa);               // adr2,adr1,adr0,data 05555:aa
  out_data (0, 0x2a, 0xaa, 0x55);
  out_data (0, 0x55, 0x55, 0xa0);
}


/*
static void
disable_protection (void)
{
  out_data (0, 0x55, 0x55, 0xaa);               // adr2,adr1,adr0,data 05555:aa
  out_data (0, 0x2a, 0xaa, 0x55);
  out_data (0, 0x55, 0x55, 0x80);
  out_data (0, 0x55, 0x55, 0xaa);
  out_data (0, 0x2a, 0xaa, 0x55);
  out_data (0, 0x55, 0x55, 0x20);
  delay_us (20000);
}
*/


#if 0
static int
data_polling_data (unsigned char last_data)
{
  unsigned int timeout = 0;

  while (timeout++ < 0x07ffffff)
    if (((read_byte () ^ last_data) & 0x80) == 0)
      return 0;
  return 1;                                     // ready to exit the while loop
}
#endif


static int
data_polling (void)
{
  unsigned char predata, currdata;
  unsigned int timeout = 0;

  predata = read_byte () & 0x40;
  while (timeout++ < 0x07ffffff)
    {
      currdata = read_byte () & 0x40;
      if (predata == currdata)
        return 0;
      predata = currdata;
    }
  return 1;
}


static void
reset_to_read (void)                            // return to read mode
{
  out_adr2_data (0x5555, 0xaa);                 // 5555:aa adr2,adr1,adr0,data
  out_adr2_data (0x2aaa, 0x55);                 // 2aaa:55
  out_adr2_data (0x5555, 0xf0);                 // 5555:f0
}


/*
static void
read_status_reg_cmd (void)
{
  out_adr2_data (0x5555, 0xaa);                 // 5555:aa adr2,adr1,adr0,data
  out_adr2_data (0x2aaa, 0x55);                 // 2aaa:55
  out_adr2_data (0x5555, 0x70);                 // 5555:70
}
*/


static int
wait_status (void)
{
  unsigned temp = read_byte ();                 // read first status byte

  while ((temp & 0xfc) != 0x80)
    {
      if ((temp & 0x20) == 0x20 && port_mode == UCON64_SPP)
        {
          fputs ("\nERROR: Erase failed\n", stderr);
          return -1;
        }
      if ((temp & 0x10) == 0x10)
        {
          fputs ("\nERROR: Programming failed\n", stderr);
          return -2;
        }
      temp = read_data ();
    }
//  reset_to_read ();
  return 0;
}


static int
mx_erase (void)
{
  int retval;

  out_adr2_data (0x5555, 0xaa);                 // 5555:aa adr2,adr1,adr0,data
  out_adr2_data (0x2aaa, 0x55);                 // 2aaa:55
  out_adr2_data (0x5555, 0x80);                 // 5555:80
  out_adr2_data (0x5555, 0xaa);                 // 5555:aa
  out_adr2_data (0x2aaa, 0x55);                 // 2aaa:55
  out_adr2_data (0x5555, 0x10);                 // 5555:10

  delay_us (100);
//  read_status_reg_cmd ();                       // send read status reg. cmd
  retval = wait_status () ? -1 : 0;
  reset_to_read ();
  return retval;
}


/*
static int
win_erase (void)
{
  out_data (0, 0x55, 0x55, 0xaa);               // adr2,adr1,adr0,data 05555:aa
  out_data (0, 0x2a, 0xaa, 0x55);
  out_data (0, 0x55, 0x55, 0x80);
  out_data (0, 0x55, 0x55, 0xaa);
  out_data (0, 0x2a, 0xaa, 0x55);
  out_data (0, 0x55, 0x55, 0x10);

  delay_us (20000);
  if (data_polling ())
    {
      fputs ("ERROR: Erase failed\n", stderr);
      return -1;
    }
  else
    return 0;
}
*/


static unsigned char
intel_read_status (void)
{
  out_adr_data (0, 0x70);                       // read status command
  return read_byte ();
}


static int
intel_check_status (void)
{
  unsigned int time_out = 0x8000;

  while (!(intel_read_status () & 0x80))
    {
      time_out--;
      if (time_out == 0)
        {
          fprintf (stderr, "\nERROR: Intel read status time out\n"
                             "       Status = 0x%02x\n", intel_read_status ());
          out_adr_data (0, 0x50);               // clear status register
          return -1;
        }
    }
  return 0;
}


static int
intel_block_erase (unsigned int block)
{
  unsigned int time_out = 0x8000;

  while ((intel_read_status ()) != 0x80)
    {
      time_out--;
      if (time_out == 0)
        {
          fprintf (stderr, "\nERROR: Intel block erase time out\n"
                             "       Status = 0x%02x\n", intel_read_status ());
          return -1;
        }
    }
  out_adr_data (block, 0x20);                   // block erase
  out_adr_data (block, 0xd0);                   // write confirm
  time_out = 0x8000;
  while (!(intel_read_status () & 0x80))
    {
      time_out--;
      if (time_out == 0)
        {
          fprintf (stderr, "\nERROR: Intel block erase time out at 0x%x\n"
                             "       Status = 0x%02x\n", block, intel_read_status ());
          out_adr_data (block, 0x50);           // clear status register
          fprintf (stderr, "       Status = 0x%02x\n", intel_read_status ());
          return -1;
        }
    }

  if ((intel_read_status ()) == 0x80)
    return 0;
  else
    {
      fprintf (stderr, "\nERROR: Intel block erase error at 0x%x\n"
                         "       Status = 0x%02x\n", block, intel_read_status ());
      out_adr_data (block, 0x50);               // clear status register
      fprintf (stderr, "       Status = 0x%02x\n", intel_read_status ());
      out_adr_data (0x0000, 0xff);              // read array
      return -1;
    }
}


/*
static int
intel_erase (void)
{
  unsigned int block;
  for (block = 0; block < 64; block++)
    if (intel_block_erase (block * 0x20000))
      return -1;
  return 0;
}


static int
erase (void)
{
  if (eeprom_type == WINBOND)
    return win_erase ();
  else if (eeprom_type == MX)
    return mx_erase ();
  else if (eeprom_type == INTEL)
    return intel_erase ();

  fputs ("ERROR: Unknown EEPROM type\n", stderr);
  return -1;
}
*/


static int
check_eeprom (void)
{
  int i;

  // check 4M flash
  out_adr_data_32k (0x5555, 0xaa);              // software product ID entry
  out_adr_data_32k (0x2aaa, 0x55);
  out_adr_data_32k (0x5555, 0x80);
  out_adr_data_32k (0x5555, 0xaa);
  out_adr_data_32k (0x2aaa, 0x55);
  out_adr_data_32k (0x5555, 0x60);

  delay_us (10);
  set_adr (0);                                  // adr2,adr1,adr0
  if (read_byte () == 0xda)                     // manufacturer code
    {
      set_adr (1);                              // adr2,adr1,adr0
      if (read_byte () == 0x46)                 // device code
        {
          out_adr_data_32k (0x5555, 0xaa);      // software product ID exit
          out_adr_data_32k (0x2aaa, 0x55);      // adr2,adr1,adr0,data
          out_adr_data_32k (0x5555, 0xf0);      // adr2,adr1,adr0,data
          eeprom_type = WINBOND;                // Winbond 4 Mbit flash
          return 0;
        }
    }

  // check 16M flash
  out_adr2_data (0x5555, 0xaa);                 // 5555:aa software product ID entry
  out_adr2_data (0x2aaa, 0x55);                 // 2aaa:55 adr2,adr1,adr0,data
  out_adr2_data (0x5555, 0x90);                 // 5555:90 adr2,adr1,adr0,data

  set_adr (0);                                  // adr2,adr1,adr0
  if (read_byte () == 0xc2)                     // manufacturer code
    {
      set_adr (2);                              // adr2,adr1,adr0
      if (read_byte () == 0xf1)                 // device code
        {
          reset_to_read ();                     // reset to read mode
          eeprom_type = MX;                     // MX 16 Mbit flash
          return 0;
        }
    }

  // check 64M flash
  reset_to_read ();
  init_port ();
  out_adr_data (0x0000, 0x98);                  // read query
  for (i = 0; i < 128; i += 2)
    {
      set_adr (i);                              // adr2,adr1,adr0
      buffer[i / 2] = read_byte ();
    }
//  dumper (stdout, buffer, 64, 0, DUMPER_HEX);

  if (buffer[0] == 0x89 && buffer[1] == 0x15
      && buffer[0x10] == 'Q' && buffer[0x11] == 'R' && buffer[0x12] == 'Y')
    {
      out_adr_data (0x0000, 0xff);              // read array
      eeprom_type = INTEL;                      // Intel 64 Mbit flash
      return 0;
    }

//  eeprom_type = UNKNOWN_EEPROM;
  return 1;
}


static void
check_mbc (void)
// determine memory bank controller type
{
  if (eeprom_type != UNKNOWN_EEPROM)
    mbc_type = BUNG;
  else
    {
      unsigned char rom_type = buffer[0x47];

      if (memcmp (buffer + 4, rocket_logodata, GB_LOGODATA_LEN) == 0)
        mbc_type = ROCKET;                      // rom_type == 0x97 || rom_type == 0x99
      else if (rom_type == 0)
        mbc_type = ROM;
      else if ((rom_type >= 1 && rom_type <= 3) || rom_type == 0xff)
        mbc_type = MBC1;
      else if (rom_type == 5 || rom_type == 6)
        mbc_type = MBC2;
      else if (rom_type >= 0x0f && rom_type <= 0x13)
        mbc_type = MBC3;
      else if (rom_type >= 0x19 && rom_type <= 0x1e)
        mbc_type = MBC5;
      else if (rom_type == 0x1f)
        mbc_type = CAMERA;
      else
        mbc_type = UNKNOWN_MBC;
    }
}


static int
check_card (void)
/*
  16 kB ROM = 1 bank
  8 kB RAM = 1 bank
*/
{
  unsigned char sum = 0;
  char game_name[16];
  int i;

  puts ("Checking ROM data...");

  if (mbc_type == ROCKET)
    puts ("NOTE: Rocket Games cartridge detected");
  else if (memcmp (buffer + 4, gb_logodata, GB_LOGODATA_LEN) != 0)
    {
      puts ("WARNING: Cartridge does not contain official Nintendo logo data");
      dumper (stdout, buffer, 0x50, 0x100, DUMPER_HEX);
    }

  memcpy (game_name, buffer + 0x34, 15);
  game_name[15] = 0;
  for (i = 0; i < 15; i++)
    if (!isprint ((int) game_name[i]) && game_name[i] != 0)
      game_name[i] = '.';
  printf ("Game name: \"%s\"\n", game_name);

  if (buffer[0x48] > 8)                         // ROM size
    printf ("NOTE: Strange ROM size byte value in header (0x%02x)\n", buffer[0x48]);

  if (buffer[0x49] > 5)                         // SRAM size
    printf ("NOTE: Strange RAM size byte value in header (0x%02x)\n", buffer[0x49]);

/*
  // [47] = ROM type
  if (buffer[0x47] > 0 && buffer[0x47] < 4 && buffer[0x48] > 4)
    mbc1_exp = 1;                               // MBC1 8 Mbit/16 Mbit
*/

  for (i = 0x34; i < 0x4d; i++)
    sum += ~buffer[i];
  if (buffer[0x4d] != sum)
    printf ("NOTE: Incorrect header checksum (0x%02x), should be 0x%02x\n",
            buffer[0x4d], sum);

  return (1 << (buffer[0x48] > 8 ? 9 : buffer[0x48])) * 32 * 1024;
}


static void
set_sram_bank (unsigned char bank)
// original code only did "set_adr (0x4000); out_byte (bank);"
{
  if (eeprom_type != UNKNOWN_EEPROM)
    {                                               // flash card
      set_adr (0x4000);                             // set SRAM adr
      out_byte (bank);                              // SRAM bank 0
      // this should be equivalent to the code above: set_bank (0x4000, bank)
    }
  else
    {                                               // game cartridge
      switch (mbc_type)
        {
        case MBC1:
          set_bank (0x6000, 1);
          set_bank (0x4000, (unsigned char) (bank & 3));
          break;
        case MBC3:
        case MBC5:
          set_bank (0x4000, bank);
          break;
        default:
          break;
        }
    }
}


static void
set_bank2 (unsigned int bank)
{
  switch (mbc_type)
    {
    case BUNG:
    case UNKNOWN_MBC:
    case MBC5:
      set_bank (0x2000, (unsigned char) bank);
      if (eeprom_type != WINBOND)
        set_bank (0x3000, (unsigned char) (bank >> 8));
      break;
    case MBC1:
      set_bank (0x6000, 0);
      set_bank (0x2000, (unsigned char) (bank & 0x1f));
      set_bank (0x4000, (unsigned char) ((bank >> 5) & 3)); // (bank & 0x60) >> 5
      break;
    case MBC2:
      set_bank (0x2100, (unsigned char) (bank & 0x0f));
      break;
    case MBC3:
      set_bank (0x2000, (unsigned char) bank);
      break;
    case CAMERA:
      set_bank (0x4000, (unsigned char) bank);
      break;
    case ROCKET:
      set_bank (0x3f00, (unsigned char) bank);
      break;
    default:
      break;
    }
}


static void
read_rom_16k (unsigned int bank)                // ROM or EEPROM
{
  int idx = 0, i, j;
  char game_name[16];

  set_bank2 (bank);
  for (j = 0; j < 64; j++)
    {                                           // 16k bytes = 64 x 256 bytes
      if (bank)
        set_ai_data ((unsigned char) 1, (unsigned char) (j | 0x40)); // set adr[15..8]
      else
        set_ai_data ((unsigned char) 1, (unsigned char) j); // a[15..0]
      set_ai_data ((unsigned char) 0, 0);       // a[7..0]
      set_ai_data ((unsigned char) 2, 0x81);    // enable read inc.
      set_ai (3);                               // read/write data
      set_data_read
      for (i = 0; i < 256; i++)                 // page = 256
        buffer[idx + i] = read_data ();
      idx += 256;

      /*
        One can select the game in 2-in-1 cartridges by writing the game number
        to 0x3fc0. This has been verified for 2 cartridges with ROM type byte
        value 0x99. Maybe there exist other Rocket Games n-in-1 games with a
        different ROM type byte value. That's why we don't check for that
        specific ROM type byte value.
      */
      if (mbc_type == ROCKET && j == 1)
        if (memcmp (buffer + 0x104, rocket_logodata, GB_LOGODATA_LEN) == 0)
          {
            set_adr (0x3fc0);
            out_byte ((unsigned char) rocket_game_no++);
            if (bank)
              {
                // Reread the last two pages, because the data came from the
                //  previously selected game (data is "mirrored"). This does not
                //  apply to the first game.
                int k;

                idx = 0;
                for (k = 0; k < 2; k++)
                  {
                    set_ai_data ((unsigned char) 1, (unsigned char) (k | 0x40));
                    set_ai_data ((unsigned char) 0, 0);
                    set_ai_data ((unsigned char) 2, 0x81);
                    set_ai (3);
                    set_data_read
                    for (i = 0; i < 256; i++)
                      buffer[idx + i] = read_data ();
                    idx += 256;
                  }

                clear_line ();                  // remove last gauge
                memcpy (game_name, buffer + 0x134, 15);
                game_name[15] = 0;
                for (i = 0; i < 15; i++)
                  if (!isprint ((int) game_name[i]) && game_name[i] != 0)
                    game_name[i] = '.';
                printf ("Found another game: \"%s\"\n\n", game_name);
              }
          }
    }
}


static int
verify_rom_16k (unsigned int bank)              // ROM or EEPROM
{
  int idx = 0, i, j;

  set_bank2 (bank);
  for (j = 0; j < 64; j++)
    {                                           // 16k bytes = 64 x 256 bytes
      if (bank)
        set_ai_data ((unsigned char) 1, (unsigned char) (j | 0x40)); // set adr[15..8]
      else
        set_ai_data ((unsigned char) 1, (unsigned char) j);
      set_ai_data ((unsigned char) 0, 0);       // a[7..0]
      set_ai_data ((unsigned char) 2, 0x81);    // enable read inc.
      set_ai (3);                               // read/write data
      set_data_read
      for (i = 0; i < 256; i++)
        if (read_data () != buffer[idx + i])
          {
            printf ("\nWARNING: Verify error at 0x%x\n",
                    (bank * 16384) + (j * 256) + i);
            return -1;
          }
      idx += 256;
    }

  return 0;
}


static int
win_write_eeprom_16k (unsigned int bank)
{
  int wr_done, err_cnt, idx = 0, i, j;

//  disable_protection ();

  for (j = 0; j < 64; j++)
    {                                           // 16k bytes = 64 x 256 bytes
      err_cnt = 16;                             // retry write counter
      wr_done = 1;
      while (wr_done)
        {
          enable_protection ();
          // write 256 bytes
          set_bank (0x2000, (unsigned char) bank); // for MCB1 16k bank
          if (bank)
            set_ai_data ((unsigned char) 1, (unsigned char) (j | 0x40)); // set adr[15..8]
          else
            set_ai_data ((unsigned char) 1, (unsigned char) j);

          set_ai_data ((unsigned char) 0, 0);   // a[7..0]
//          set_ai_data ((unsigned char) 2, 0x82); // enable flash write
          set_ai_data ((unsigned char) 2, 0x83); // enable flash write inc.
          set_ai (3);                           // read/write data
//          set_ai_data ((unsigned char) 2, 0x80);

          for (i = 0; i < 256; i++)
            write_data (buffer[idx + i]);       // write data to EEPROM
          set_ai_data ((unsigned char) 2, 0x80); // disable wr/rd inc.
          set_ai_data ((unsigned char) 0, 0xff); // point to xxff
          if (data_polling ())
            puts ("\nWARNING: Write error");    // was: "Write error check (d6)"

          wr_done = 0;

          // verify 256 bytes
          set_ai_data ((unsigned char) 0, 0);   // a[7..0]
          set_ai_data ((unsigned char) 2, 0x81); // enable read inc.
          set_ai (3);                           // read/write data
          set_data_read
          for (i = 0; i < 256; i++)
            if (read_data () != buffer[idx + i])
              {
                err_cnt--;
                wr_done = 1;
                break;
              }
          if (err_cnt == 0)
            {
              fputs ("\nERROR: Programming failed after retry\n", stderr);
              return -1;
            }
        }
      idx += 256;
    }
/*
  enable_protection();
  disable_protection();
  delay_us (20000);
*/
  return verify_rom_16k (bank);
}


static void
set_page_write (void)                           // start page write command
{
  out_adr2_data (0x5555, 0xaa);                 // 5555:aa adr2,adr1,adr0,data
  out_adr2_data (0x2aaa, 0x55);                 // 2aaa:55
  out_adr2_data (0x5555, 0xa0);                 // 5555:a0
}


static int
page_write_128 (unsigned int bank, unsigned char hi_lo, int j, int idx)
{
  int retry = 3, verify_ok, i;

  while (retry)
    {
      set_page_write ();                        // each page is 128 bytes
      set_bank (0x2000, (unsigned char) bank);  // for MCB1 16k bank
      if (bank)
        set_ai_data ((unsigned char) 1, (unsigned char) (j | 0x40)); // set adr[15..8]
      else
        set_ai_data ((unsigned char) 1, (unsigned char) j);

      set_ai_data ((unsigned char) 0, hi_lo);   // a[7..0]
      set_ai_data ((unsigned char) 2, 0x83);    // enable flash write inc.
      set_ai (3);                               // read/write data
      for (i = 0; i < 128; i++)
        write_data (buffer[idx + i]);           // write data to EEPROM
      set_ai_data ((unsigned char) 2, 0x80);    // disable wr/rd inc.
      delay_us (10);                            // delay is large enough? - dbjh
      if (wait_status ())
        return -1;                              // wait_status() prints error message

      // verify data
      reset_to_read ();                         // return to read mode
      verify_ok = 1;                            // verify ok
      set_bank (0x2000, (unsigned char) bank);  // for MCB1 16k bank
      if (bank)
        set_ai_data ((unsigned char) 1, (unsigned char) (j | 0x40)); // set adr[15..8]
      else
        set_ai_data ((unsigned char) 1, (unsigned char) j);

      set_ai_data ((unsigned char) 0, hi_lo);   // a[7..0]
      set_ai_data ((unsigned char) 2, 0x81);    // enable inc.
      set_ai (3);                               // read/write data
      set_data_read
      for (i = 0; i < 128; i++)                 // page = 128
        if (read_data () != buffer[idx + i])
          {
            verify_ok = 0;                      // verify error
            break;
          }
      if (verify_ok)
        break;
      else
        {
          retry--;
          if (retry == 0)
            {
              fputs ("\nERROR: Programming failed after retry\n", stderr);
              return -1;
            }
        }
    }
  return 0;
}


static int
mx_write_eeprom_16k (unsigned int bank)
{
  int idx = 0, j;

  for (j = 0; j < 64; j++)
    {                                           // 16k bytes = 64 x 256 bytes
      if (page_write_128 (bank, 0, j, idx))     // write first 128 bytes
        return -1;
      idx += 128;
      if (page_write_128 (bank, 0x80, j, idx))  // write second 128 bytes
        return -1;
      idx += 128;
    }
  reset_to_read ();                             // return to read mode
  return verify_rom_16k (bank);
}


/*
static void
dump_intel_data (void)
{
  int i;

  out_adr_data (0, 0xff);                       // read array command
  for (i = 0; i < 64; i++)
    {                                           // read 0x100-0x150 to buffer
      set_adr (i);
      buffer[i] = read_data ();
    }
  dumper (stdout, buffer, 64, 0, DUMPER_HEX);
}


static int
intel_byte_write_32 (unsigned int block_adr, int idx)
{
  int time_out, i;

  for (i = 0; i < 32; i++)
    {
      out_adr_data (block_adr + idx + i, 0x40); // Write byte command
      out_adr_data (block_adr + idx + i, buffer[idx + i]); // Write data
      time_out = 0x8000;

      if (intel_check_status ())
        {
          fprintf (stderr, "\nERROR: Intel byte write command time out\n"
                             "       Status = 0x%02x\n", intel_read_status ());
//          dump_intel_data ();
          return -1;
        }
    }
  if (intel_read_status () == 0x80)
    return 0;
  else
    return -1;                                  // error
}
*/


static int
intel_buffer_write_32 (unsigned int block_adr, int idx)
{
  int time_out = 0x80000, i;

  out_adr_data (block_adr + idx, 0xe8);         // Write buffer command
  set_ai_data ((unsigned char) 2, 0x82);        // wei enable
  set_ai (3);                                   // default write mode
//  write_data (0xe8);                            // Write buffer command
  set_data_read                                 // ninit=0, nwrite=1
  while (!(read_data () & 0x80))
    {
      time_out--;
      if (time_out == 0)
        {
          fprintf (stderr, "\nERROR: Intel buffer write command time out\n"
                             "       Status = 0x%02x\n", intel_read_status ());
//          dump_intel_data ();
          return -1;
        }
      set_data_write
      write_data (0xe8);                        // out data
      set_data_read
//      out_byte_eeprom (0xe8);                   // write buffer command
    }

//  out_byte_eeprom (0x1f);                       // set write byte count command
  write_data (0x1f);                            // out data

  set_ai_data ((unsigned char) 2, 0x83);        // enable flash write inc.
  set_ai (3);
  for (i = 0; i < 32; i++)
    write_data (buffer[idx + i]);               // write data to EEPROM
  write_data (0xd0);                            // write confirm command

  return 0;
}


static int
intel_write_eeprom_16k (unsigned int bank)
{
  unsigned int block_adr = bank << 14;          // convert to real address
  int idx, j;

  if ((bank & 0x07) == 0)
    if (intel_block_erase (block_adr))
      return -1;

//  set_adr_long (block_adr, 0);                  // set real address
  for (j = 0; j < 512; j++)
    {                                           // 16k bytes = 512 x 32 bytes
      idx = j * 32;
//      if (intel_byte_write_32 (block_adr, idx)) return -1;
      if (intel_buffer_write_32 (block_adr, idx))
        {
          fprintf (stderr, "\nERROR: Write error\n"
                             "       Status = 0x%02x\n", intel_read_status ());
          return -1;
        }
    }

  if (intel_check_status ())
    {
      fprintf (stderr, "\nERROR: Intel buffer write command error\n"
                         "       Status = 0x%02x\n", intel_read_status ());
//      dump_intel_data();
      return -1;
    }
  if (intel_read_status () != 0x80)
    return -1;                                  // error

  out_adr_data (0, 0xff);                       // read array
  set_data_read
  return verify_rom_16k (bank);
}


static int
write_eeprom_16k (unsigned int bank)
{
  if (eeprom_type == WINBOND)                   // Winbond 4 Mbits EEPROM
    return win_write_eeprom_16k (bank);
  if (eeprom_type == MX)                        // MX 16 Mbits EEPROM
    return mx_write_eeprom_16k (bank);
  if (eeprom_type == INTEL)                     // Intel 64 Mbits EEPROM
    return intel_write_eeprom_16k (bank);
  return -1;
}


static void
enable_sram_bank (void)
{
  init_port ();
  set_adr (0x0000);                             // write 0x0000:0x0a default read mode
  out_byte (0x0a);                              // enable SRAM
  out_byte (0xc0);                              // disable SRAM
  set_adr (0xa000);
  out_byte (0xa0);                              // ctr index
  set_adr (0xa100);
//  out_byte(0x00);                               // ram_off,ram_bank_disable,MBC1
  out_byte (0xc0);                              // ram_on,ram_bank_enable,MBC1

  set_adr (0x0000);                             // write 0x0000:0x0a
  out_byte (0x0a);                              // enable SRAM
}


/*
static void
usage (char *progname)
{
  fprintf (stderr, "Usage: %s [-option] <Filename>\n", progname);
  fprintf (stderr, "-l   : load ROM file to GB Card.\n");
  fprintf (stderr, "-lsa : load 256k/1Mbits sram from PC to GB Card.\n");
  fprintf (stderr,
           "-lsn : load 64kbits sram file from PC to specific sram bank(-n) in GB card.\n");
  fprintf (stderr, "-lsc : load 1Mbits sram file from PC to Pocket Camera.\n");
  fprintf (stderr, "-b   : auto-detect size and backup entire GB Card to PC.\n");
  fprintf (stderr, "-ba  : backup full 4Mbits/16Mbits GB Card to PC.\n");
  fprintf (stderr, "-bsa : retrieve all sram data (256k/1Mbits) from GB Card to PC.\n");
  fprintf (stderr,
           "-bsn : retrieve specific bank(-n) sram data(64kbits) from GB Card to PC.\n");
  fprintf (stderr, "-bsc : retrieve 1Mbits sram from Pocket Camera to PC.\n");
  fprintf (stderr, "-v   : verify file in PC with GB Card.\n");
  fprintf (stderr, "-e   : erase Flash rom.\n");
  fprintf (stderr, "-c   : check ROM file header.\n");
  exit (2);
}
*/


static int
check_port_mode (void)
{
  init_port ();
  set_ai_data ((unsigned char) 1, 0x12);
  set_ai_data ((unsigned char) 0, 0x34);
  set_ai (1);
  set_data_read                                 // ninit=0, nwrite=1
  if (read_data () != 0x12)
    return 1;
  set_ai (0);
  set_data_read                                 // ninit=0, nwrite=1
  if (read_data () != 0x34)
    return 1;
  end_port ();
  return 0;
}


static int
check_port (void)
{
  if (ucon64.parport_mode == UCON64_EPP && port_8 != 0x3bc)
    port_mode = UCON64_EPP;                     // if port == 0x3bc => no EPP available
  else
    port_mode = UCON64_SPP;

  if (check_port_mode ())
    {
      port_mode = UCON64_SPP;
      if (check_port_mode ())
        return 1;
      else
        end_port ();
    }

  // If we get here, a GBX was detected
  if (port_mode == UCON64_EPP)
    puts ("GBX found. EPP found");
  else
    puts ("GBX found. EPP not found or not enabled - SPP used");

  return 0;
}


#if 0 // not used
static void
win_id (void)
{
  out_data (0, 0x55, 0x55, 0xaa);               // software product ID entry
  out_data (0, 0x2a, 0xaa, 0x55);               // adr2,adr1,adr0,data
  out_data (0, 0x55, 0x55, 0x80);               // adr2,adr1,adr0,data
  out_data (0, 0x55, 0x55, 0xaa);               // adr2,adr1,adr0,data
  out_data (0, 0x2a, 0xaa, 0x55);               // adr2,adr1,adr0,data
  out_data (0, 0x55, 0x55, 0x60);               // adr2,adr1,adr0,data

  delay_us (10);
  set_adr (0);                                  // adr2,adr1,adr0
  printf ("Manufacturer code: 0x%02x\n", read_byte ());
  set_adr (1);                                  // adr2,adr1,adr0
  printf ("Device code: 0x%02x\n", read_byte ());
/*
  set_adr (2);                                  // adr2,adr1,adr0
  printf ("First 16 k protection code: 0x%02x\n", read_byte ());
  set_bank (0x2000, 0x1f);
  set_adr (0x7ff2);                             // adr2,adr1,adr0=0x7fff2
  printf("Last 16 k protection code: 0x%02x\n", read_byte ());
*/

  out_data (0, 0x55, 0x55, 0xaa);               // software product ID exit
  out_data (0, 0x2a, 0xaa, 0x55);               // adr2,adr1,adr0,data
  out_data (0, 0x55, 0x55, 0xf0);               // adr2,adr1,adr0,data
}


static void
mx_id (void)
{
  out_adr2_data (0x5555, 0xaa);                 // software product ID entry
  out_adr2_data (0x2aaa, 0x55);                 // adr2,adr1,adr0,data
  out_adr2_data (0x5555, 0x90);                 // adr2,adr1,adr0,data

  set_adr (0);                                  // adr2,adr1,adr0
  printf ("Manufacturer code: 0x%02x\n", read_byte ());
  set_adr (2);                                  // adr2,adr1,adr0
  printf ("Device code: 0x%02x\n", read_byte ());
  set_adr (4);                                  // adr2,adr1,adr0
  printf ("First 16 k protection code: 0x%02x\n", read_byte ());
  reset_to_read ();                             // reset to read mode
}


static void
intel_id (void)
{
  int i;

  out_adr_data (0, 0x98);                       // read query
  for (i = 0; i < 128; i += 2)
    {
      set_adr (i);                              // adr2,adr1,adr0
      buffer[i / 2] = read_byte ();
    }
//  dumper (stdout, buffer, 64, 0, DUMPER_HEX);

  printf ("Manufacture code = 0x%02x\n", buffer[0]);
  printf ("Device code = 0x%02x\n", buffer[1]);
}


static void
disp_id (void)
{
  if (eeprom_type == WINBOND)
    win_id ();
  if (eeprom_type == MX)
    mx_id ();
  if (eeprom_type == INTEL)
    intel_id ();
}


static void
gen_pat (unsigned int offset)
{
  int i;

  for (i = 0; i < 0x2000; i++)                  // 8 k words = 16 k bytes
    ((unsigned short int *) buffer)[i] = i + offset;
}


static int
test_sram_v (int n_banks)
{
  int idx, i, j, bank;

  enable_sram_bank ();
  for (bank = 0; bank < n_banks; bank++)
    {
      idx = 0;
      set_sram_bank (bank);
      gen_pat (bank);
      for (j = 0; j < 0x20; j++)
        {                                       // 32 x 256 = 8192 (8 kbytes)
          set_ai_data ((unsigned char) 1, (unsigned char) (0xa0 + j)); // SRAM at 0xa000-0xbfff
          set_ai_data ((unsigned char) 0, 0);   // a[7..0]=0
          set_ai_data ((unsigned char) 2, 0x81); // enable inc
          set_ai (3);                           // point to data r/w port
          set_data_read
          for (i = 0; i < 256; i++)
            if (read_data () != buffer[i + idx])
              {
                fputs ("ERROR: SRAM verify error\n", stderr);
                return -1;
              }
          set_ai_data ((unsigned char) 2, 0x80); // disable inc
          idx += 256;
        }
    }
  return 0;
}


static int
test_sram_wv (int n_banks)
{
  int idx, i, j, bank;

  enable_sram_bank ();
  for (bank = 0; bank < n_banks; bank++)
    {
      idx = 0;
      set_sram_bank (bank);
      gen_pat (bank);
//      dumper (stdout, buffer, 0x10, 0, DUMPER_HEX);
      for (j = 0; j < 0x20; j++)
        {                                       // 32 x 256 = 8192(8kbytes)
          set_ai_data ((unsigned char) 1, (unsigned char) (0xa0 + j)); // SRAM at 0xa000-0xbfff
          set_ai_data ((unsigned char) 0, 0);   // a[7..0]=0
          set_ai_data ((unsigned char) 2, 0x81); // enable inc
          set_ai (3);                           // point to data r/w port
          set_data_write
          for (i = 0; i < 256; i++)
            write_data (buffer[i + idx]);
          set_ai_data ((unsigned char) 2, 0x80); // disable inc
          idx += 256;
        }
    }
  return test_sram_v (n_banks);
}


int cart_type = 0;                              // should be set to value of ROM[0x147]

static void
set_rom_bank (unsigned char bank)
{
  // cart_type < 4 is MCB1, other is MCB2
  if (cart_type < 4)
    set_bank (0x2000, bank);                    // for MCB1
  else
    set_bank (0x2100, bank);                    // for MCB2
}


static void
try_read (void)
{
  int i;

  set_ai_data ((unsigned char) 0, 0);           // a[7..0]=0
  set_ai_data ((unsigned char) 1, 0x40);
  set_ai_data ((unsigned char) 2, 0x81);        // enable inc
  set_ai (3);                                   // point to data r/w port
  for (i = 0; i < 16; i++)
    buffer[i] = read_data ();
  dumper (stdout, buffer, 16, 0, DUMPER_HEX);
}


static void
try_read0 (void)
{
  int j;

  set_rom_bank (1);
  for (j = 0; j < 4; j++)
    {
      set_sram_bank ((unsigned char) j);
      try_read ();
    }
  set_bank (0x6000, (unsigned char) 1);
  puts ("6000:1");
  for (j = 0; j < 4; j++)
    {
      set_sram_bank ((unsigned char) j);
      try_read ();
    }
  set_bank (0x6000, (unsigned char) 0);
  puts ("6000:0");
  for (j = 0; j < 4; j++)
    {
      set_sram_bank ((unsigned char) j);
      try_read ();
    }
}


static void
test_intel (void)
{
  int i;

  out_adr2_data (0x0000, 0x90);                 // software product ID entry
  for (i = 0; i < 128; i += 2)
    {
      set_adr (i);                              // adr2,adr1,adr0
      buffer[i / 2] = read_byte ();
    }
  dumper (stdout, buffer, 64, 0, DUMPER_HEX);

  out_adr2_data (0x0000, 0x70);                 // read status register
  printf ("Status register = 0x%02x\n", read_byte ());
}


static void gbx_init (unsigned int parport, int read_header);

static int
verify_card_from_file (const char *filename, unsigned int parport)
{
  unsigned int bank, n_banks, filesize;
  FILE *file;
  time_t starttime;

  gbx_init (parport, 1);
  filesize = fsizeof (filename);
  if ((filesize < 0x8000) || (filesize & 0x7fff) || (filesize > 64 * MBIT))
    {
      fputs ("ERROR: File size error\n", stderr);
      exit (1);
    }
  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  n_banks = (filesize / 0x8000) * 2;            // how many 16k banks (rounded
  starttime = time (NULL);                      //  down to 32k boundary)
  for (bank = 0; bank < n_banks; bank++)
    {
      if (fread (buffer, 1, 0x4000, file) != 0x4000)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], filename);
          fclose (file);
          exit (1);
        }
/*
      if (bank == 0)
        {
          // [0x147] = ROM type; [0x148] = ROM size
          if (buffer[0x147] > 0 && buffer[0x147] < 4 && buffer[0x148] > 4)
            mbc1_exp = 1;                       // MBC1 8 Mbit/16 Mbit
        }
*/
      if (verify_eeprom_16k (bank))
        {
          printf ("Verify card error at bank 0x%x\n", bank);
          fclose (file);
          exit (1);
        }
      ucon64_gauge (starttime, (bank + 1) * 0x4000, filesize);
    }
  fclose (file);
  puts ("\nVerify card OK");

  return 0;
}
#endif


static void
gbx_init (unsigned int parport, int read_header)
{
  int i;

  eeprom_type = UNKNOWN_EEPROM;
  rocket_game_no = 0;

  port_8 = parport;
  port_9 = parport + 1;
  port_a = parport + 2;
  port_b = parport + 3;
  port_c = parport + 4;

  parport_print_info ();

  if (check_port () != 0)
    {
      fputs ("ERROR: GBX not found or not turned on\n", stderr);
      exit (1);
    }
  init_port ();
  check_eeprom ();

  if (read_header)
    for (i = 0x100; i < 0x150; i++)
      {                                         // read 0x100-0x150 to buffer
        set_adr (i);
        buffer[i - 0x100] = read_data ();
      }
  /*
    buffer is undefined if read_header == 0. This is not a problem as
    read_header is only 0 if a flash card should be programmed
    (gbx_write_rom()). In that case check_mbc() won't use buffer.
  */
  check_mbc ();
}


int
gbx_read_rom (const char *filename, unsigned int parport)
{
  unsigned int bank, n_banks, rom_size, n_bytes = 0, totalbytes;
  time_t starttime;
  FILE *file;

  gbx_init (parport, 1);
  rom_size = check_card ();

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      end_port ();
      exit (1);
    }

  /*
    0  256 kbit = 32 kB  = 2 banks
    1  512 kbit = 64 kB  = 4 banks
    2  1 Mbit   = 128 kB = 8 banks
    3  2 Mbit   = 256 kB = 16 banks
    4  4 Mbit   = 512 kB = 32 banks
    5  8 Mbit   = 1 MB   = 64 banks
    6  16 Mbit  = 2 MB   = 128 banks
  */
  n_banks = rom_size / (16 * 1024);
  if (eeprom_type == WINBOND)
    n_banks = 32;                               // backup 4 Mbit
  if (eeprom_type == MX)
    n_banks = 128;                              // backup 16 Mbit
  if (eeprom_type == INTEL)
    n_banks = 512;                              // backup 64 Mbit

  totalbytes = n_banks * 16 * 1024;
  printf ("Receive: %d Bytes (%.4f Mb)\n\n", totalbytes, (float) totalbytes / MBIT);

  starttime = time (NULL);
  for (bank = 0; bank < n_banks; bank++)
    {
      read_rom_16k (bank);
      if (verify_rom_16k (bank))
        printf ("Verify card error at bank 0x%x\n", bank);

      fwrite (buffer, 1, 0x4000, file);
      n_bytes += 16 * 1024;
      ucon64_gauge (starttime, n_bytes, totalbytes);
    }

  fclose (file);
  end_port ();

  return 0;
}


int
gbx_write_rom (const char *filename, unsigned int parport)
{
  int bank, n_banks, n_bytes = 0, filesize;
  time_t starttime;
  FILE *file;

  gbx_init (parport, 0);
  if (eeprom_type == UNKNOWN_EEPROM)
    {
      fputs ("ERROR: Unknown EEPROM type\n", stderr);
      end_port ();
      exit (1);
    }

  filesize = fsizeof (filename);
  if ((filesize < 0x8000) || (filesize & 0x7fff) || (filesize > 64 * MBIT))
    {
      fputs ("ERROR: File size error\n", stderr);
      exit (1);
    }
  n_banks = (filesize / 0x8000) * 2;            // how many 16k banks (rounded
                                                //  down to 32k boundary)
  if (eeprom_type == MX)
    if (mx_erase ())                            // erase 16M flash
      {
        // wait_status() prints error message
        end_port ();
        exit (1);
      }

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      end_port ();
      exit (1);
    }

  printf ("Send: %d Bytes (%.4f Mb)\n\n",
          n_banks * 0x4000, (float) (n_banks * 0x4000) / MBIT);

  starttime = time (NULL);
  for (bank = 0; bank < n_banks; bank++)
    {
      if (fread (buffer, 1, 0x4000, file) != 0x4000)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], filename);
          fclose (file);
          end_port ();
          exit (1);
        }
      if (write_eeprom_16k (bank))
        {
          fprintf (stderr, "ERROR: Write card error at bank 0x%x\n", bank);
          fclose (file);
          end_port ();
          exit (1);
        }
      n_bytes += 0x4000;
      ucon64_gauge (starttime, n_bytes, filesize);
    }

#if 0 // write_eeprom_16k() already calls verify_rom_16k() (indirectly)...
  clear_line ();                                // remove last gauge
  puts ("Verifying card...\n");
  fseek (file, 0, SEEK_SET);
  n_bytes = 0;
  starttime = time (NULL);
  for (bank = 0; bank < n_banks; bank++)
    {
      if (fread (buffer, 1, 0x4000, file) != 0x4000)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], filename);
          fclose (file);
          end_port ();
          exit (1);
        }
      if (verify_rom_16k (bank))
        {
          fprintf (stderr, "ERROR: Verify card error at bank 0x%x\n", bank);
          fclose (file);
          end_port ();
          exit (1);
        }
      n_bytes += 0x4000;
      ucon64_gauge (starttime, n_bytes, filesize);
    }
#endif

  fclose (file);
  end_port ();

  return 0;
}


static int
sram_size_banks (int pocket_camera, unsigned char sram_size_byte)
{
  int n_banks;

  if (eeprom_type == UNKNOWN_EEPROM)
    {
      // it seems reasonable that eeprom_type == UNKNOWN_EEPROM when a pocket
      //  camera (1 Mbit SRAM) is plugged in the GBX
      if (pocket_camera)
        n_banks = 16;
      else // no flash card, must be game cartridge
        {
          int x = (sram_size_byte & 7) << 1;
          if (x > 5)
            x = 6;
          if (x)
            x = 1 << (x - 1);                   // SRAM size in kB
          n_banks = (x + 8192 - 1) / 8192;      // round up to 1 bank if it's 2 kB
        }
    }
  else
    {
      // if eeprom_type != UNKNOWN_EEPROM, it has to be WINBOND, MX or INTEL (no
      //  need to set n_banks to a default value)
      if (eeprom_type == WINBOND)
        n_banks = 4;                            // 4 x 8 kB = 32 kB
      else // if (eeprom_type == MX || eeprom_type == INTEL)
        n_banks = 16;                           // 16 x 8 kB = 128 kB
    }

  return n_banks;
}


int
gbx_read_sram (const char *filename, unsigned int parport, int start_bank)
{
  int bank, n_banks, n_bytes = 0, totalbytes, idx, i, j;
  time_t starttime;
  FILE *file;

  gbx_init (parport, 1);

  n_banks = sram_size_banks (buffer[0x47] == 0x1f, buffer[0x49]);
  if (!n_banks)
    {
      fputs ("ERROR: No SRAM available\n", stderr);
      end_port ();
      exit (1);
    }

  if (start_bank == -1)
    start_bank = 0;
  else
    {
      if (start_bank >= n_banks)
        {
          fprintf (stderr, "ERROR: Bank must be a value 0 - %d (for this card)\n",
                   n_banks - 1);
          end_port ();
          exit (1);
        }
      n_banks = 1;
    }

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      end_port ();
      exit (1);
    }

  memset (buffer, 0, 8192);
  totalbytes = n_banks * 8192;
  printf ("Receive: %d Bytes (%.4f Mb)\n\n", totalbytes, (float) totalbytes / MBIT);

  enable_sram_bank ();
  starttime = time (NULL);
  for (bank = start_bank; bank < start_bank + n_banks; bank++)
    {
      idx = 0;
      set_sram_bank ((unsigned char) bank);
      for (j = 0; j < 32; j++)
        {                                       // 32 x 256 = 8192 (8 kbytes)
          set_ai_data ((unsigned char) 1, (unsigned char) (0xa0 + j)); // SRAM at 0xa000-0xbfff
          set_ai_data ((unsigned char) 0, 0);   // a[7..0]=0
          set_ai_data ((unsigned char) 2, 0x81); // enable inc
          set_ai (3);                           // point to data r/w port
          set_data_read
          for (i = 0; i < 256; i++)
            buffer[idx + i] = read_data ();
          set_ai_data ((unsigned char) 2, 0x80); // disable inc
          idx += 256;
        }
      fwrite (buffer, 1, 8192, file);
      n_bytes += 8192;
      ucon64_gauge (starttime, n_bytes, totalbytes);
    }
  fclose (file);
  end_port ();

  return 0;
}


int
gbx_write_sram (const char *filename, unsigned int parport, int start_bank)
{
  int bank, n_banks, n_bytes = 0, totalbytes, idx, i, j;
  time_t starttime;
  FILE *file;

  gbx_init (parport, 1);

  n_banks = sram_size_banks (buffer[0x47] == 0x1f, buffer[0x49]);
  if (!n_banks)
    {
      fputs ("ERROR: No SRAM to write to\n", stderr);
      end_port ();
      exit (1);
    }

  if (start_bank == -1)
    {
      start_bank = 0;
      i = fsizeof (filename);
      n_banks = MIN (n_banks, (i + 8192 - 1) / 8192); // "+ 8192 - 1" to round up
    }
  else
    {
      if (start_bank >= n_banks)
        {
          fprintf (stderr, "ERROR: Bank must be a value 0 - %d (for this card)\n",
                   n_banks - 1);
          end_port ();
          exit (1);
        }
      n_banks = 1;
    }

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      end_port ();
      exit (1);
    }

  memset (buffer, 0, 8192);
  totalbytes = n_banks * 8192;                  // yes, we _send_ totalbytes bytes
  printf ("Send: %d Bytes (%.4f Mb)\n\n", totalbytes, (float) totalbytes / MBIT);

  enable_sram_bank ();
  starttime = time (NULL);
  for (bank = start_bank; bank < start_bank + n_banks; bank++)
    {
      idx = 0;
      if (!fread (buffer, 1, 8192, file))
        { // handle/allow files that are not an exact multiple of 8 kB
          fprintf (stderr, ucon64_msg[READ_ERROR], filename);
          fclose (file);
          end_port ();
          exit (1);
        }
      set_sram_bank ((unsigned char) bank);
      for (j = 0; j < 32; j++)
        {                                       // 32 x 256 = 8192 (8 kbytes)
          set_ai_data ((unsigned char) 1, (unsigned char) (0xa0 + j)); // SRAM at 0xa000-0xbfff
          set_ai_data ((unsigned char) 0, 0);   // a[7..0]=0
          set_ai_data ((unsigned char) 2, 0x81); // enable inc
          set_ai (3);                           // point to data r/w port
          set_data_write
          for (i = 0; i < 256; i++)
            write_data (buffer[idx + i]);
          set_ai_data ((unsigned char) 2, 0x80); // disable inc
          idx += 256;
        }
      n_bytes += 8192;
      ucon64_gauge (starttime, n_bytes, totalbytes);
    }
  fclose (file);
  end_port ();

  return 0;
}

#endif // USE_PARALLEL
