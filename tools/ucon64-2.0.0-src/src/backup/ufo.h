/*
ufo.h - Super UFO for uCON64

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
#ifndef UFO_H
#define UFO_H

extern const st_getopt2_t ufo_usage[];

/*
Super UFO Pro 8 Header Format (researched by John Weidman)

Byte-Offset  Function
-----------  ---------------------------------------------------
00-01        Chunk size: 04 00 == 4 Mbit (same format as the DX2)
02           LoROM games: 0x40 == continue loading another chunk after this one.
                          0x00 == This is the last chunk.
             HiROM games: 0x40 == more work to figure this out -- maybe interleave info
                          0x10 == more work to figure this out -- maybe interleave info
                          0x00 == This is the last chunk.
03-07        0x00
08-0F        53 55 50 45 52 55 46 4F  (SUPERUFO)
10           0x01 == This file is a ROM image file
11           ROM size: 04 == 4 Mb, 0x18 == 24 Mb, 0x20 == 32 Mb, etc.
12           ROM format:  00 == HiROM,  01 == LoROM

====  Start SRAM address mapping config ===============

13          SRAM Size:
               00:  0Kb
               01:  16Kb
               02:  64Kb
               03:  256Kb
               04-07: Not used
               08: XXXKb (Used for sram sizes above 256Kb, like 1024Kb)
14          SRAM A15 control:
               00: A15 not used for SRAM control?
                   Use this for HiROM games
                   LoROM: Use this if SRAM size = 0Kb (no SRAM)
               01: A15=X selects SRAM
               02: A15=0 selects SRAM
               03: A15=1 selects SRAM
15          SRAM A20 and A21 control:
             Bits 3:2
               00: A21=x selects SRAM
               01: Not used?
               10: A21=0 selects SRAM
               11: A21=1 selects SRAM
             Bits 1:0
               00: A20=x selects SRAM
               01: Not used?
               10: A20=0 selects SRAM
               11: A20=1 selects SRAM
16          SRAM A22 and A23 control:
             Bits 3:2
               00: A23=x selects SRAM
               01: Not used?
               10: A23=0 selects SRAM
               11: A23=1 selects SRAM
             Bits 1:0
               00: A22=x selects SRAM
               01: Not used?
               10: A22=0 selects SRAM
               11: A22=1 selects SRAM
17          SRAM type
               0x00: Linear (HiROM)
               0x03: Skip (LoROM)

====  End SRAM address mapping config  ================

18-1FF      00

LoROM SRAM header
=========================================================

The SRAM mapping I would try first for LoROM games is:

0Kb SRAM
0012-0017       01 00 00 00 02 00
0Kb LoROM DSP
0012-0017       01 00 01 0C 00 03

Note: LoROM DSPs with SRAM don't seem to work on the Super UFO
      (For reference, no LoROM DSP carts work on the SWC DX2)

Non 0 SRAM - default map (map low halves of banks 7x to SRAM)
0012-0017       01 ss 02 0F 03 03
Non 0 SRAM - alternate map (map all of banks 7x to SRAM -- will not work for > 28 Mbit
                            games )
0012-0017       01 ss 01 0F 03 03

HiROM SRAM header
==========================================================

0Kb SRAM
0012-0017       00 00 00 00 02 00
Non 0 SRAM
0012-0017       00 ss 00 0C 02 00   (Hopefully this will work for everything?)

If you find an SRAM protected game that doesn't work with the above mapping try:
0012-0017       00 ss 00 03 02 00   (seen in a couple of games but should work with above
                                     mapping too)
--
For Tales of Phantasia or Dai Kaijyu Monogatari II

0012-0017       00 ss 00 00 0E 00   (Unverified)
*/
typedef struct st_ufo_header
{
  unsigned char size_low;
  unsigned char size_high;
  unsigned char multi;
  unsigned char pad[5];
  unsigned char id[8];                          // "SUPERUFO"
  unsigned char isrom;
  unsigned char size;
  unsigned char banktype;
  unsigned char sram_size;
  unsigned char sram_a15;
  unsigned char sram_a20_a21;
  unsigned char sram_a22_a23;
  unsigned char sram_type;
  unsigned char pad2[488];
} st_ufo_header_t;

#define UFO_HEADER_LEN (sizeof (st_ufo_header_t))

#endif // UFO_H
