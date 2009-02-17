/*
fig.h - Super PRO Fighter support for uCON64

Copyright (c) 1999 - 2002 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2003 dbjh
Copyright (c) 2003        JohnDie


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
#ifndef FIG_H
#define FIG_H

extern const st_getopt2_t fig_usage[];

#ifdef USE_PARALLEL
#endif // USE_PARALLEL

/*
Super Pro Fighter (FIG) Header Format
Last edited: 19.06.2002

Offset       |  Content
-------------+------------------------------------
$0000        |  Lo-Byte of 8K-Block#
-------------+------------------------------------
$0001        |  Hi-Byte of 8K-Block#
-------------+------------------------------------
$0002        |  $00 = Last File
             |  $40 = More Files Present
-------------+------------------------------------
$0003        |  $00 = LoROM
             |  $80 = HiROM
-------------+------------------------------------
$0004-$0005  |  $77 $83 = No SRAM  (LoROM)
             |  $00 $80 = 16 KBit  (LoROM)
             |  $00 $80 = 64 KBit  (LoROM)
             |  $00 $00 = 256 KBit (LoROM)
             |  $47 $83 = No SRAM  (LoROM) (DSP)
             |  $11 $02 = 256 KBit (LoROM) (SFX)
             |  $77 $83 = No SRAM  (HiROM)
             |  $DD $82 = 16 KBit  (HiROM)
             |  $DD $82 = 64 KBit  (HiROM)
             |  $DD $02 = 256 KBit (HiROM)
             |  $F7 $83 = No SRAM  (HiROM) (DSP)
             |  $FD $82 = 16 KBit  (HiROM) (DSP)
-------------+------------------------------------
$0006-$01FF  |  Reserved (=$00)



NOTE 1: The Super Pro Fighter does not distinguish between 16 KBit SRAM
        and 64 KBit SRAM.

NOTE 2: When splitting files, the SPF writes all relevant header fields
        to all files. So each file has the same header with exception of
        the last one, because it has $0002 set to $00 to indicate that it
        is the last file.
*/
typedef struct st_fig_header
{
/*
  Don't create fields that are larger than one byte! For example size_low and size_high
  could be combined in one unsigned short int. However, this gives problems with little
  endian vs. big endian machines (e.g. writing the header to disk).
*/
  unsigned char size_low;
  unsigned char size_high;
  unsigned char multi;
  unsigned char hirom;
  unsigned char emulation1;
  unsigned char emulation2;
  unsigned char pad[506];
} st_fig_header_t;

#define FIG_HEADER_START 0
#define FIG_HEADER_LEN (sizeof (st_fig_header_t))

#ifdef USE_PARALLEL
extern int fig_read_rom (const char *filename, unsigned int parport);
extern int fig_write_rom (const char *filename, unsigned int parport);
extern int fig_read_sram (const char *filename, unsigned int parport);
extern int fig_write_sram (const char *filename, unsigned int parport);
extern int fig_read_cart_sram (const char *filename, unsigned int parport);
extern int fig_write_cart_sram (const char *filename, unsigned int parport);
#endif

#endif // FIG_H
