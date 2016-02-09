/*
spsc.c - (Starpath) Supercharger support for uCON64

Copyright (c) 2004 NoisyB (noisyb@gmx.net)


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
#include <stdio.h>
#include "backup/spsc.h"


const st_getopt2_t spsc_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "(Starpath) Supercharger",
      /*"19XX Bridgestone http://www.bridgestonemultimedia.com"*/
      NULL
    },
#if 0
    {
      "xspsc", 0, 0, UCON64_XSPSC,
      NULL, "send/receive ROM to/from (Starpath) Supercharger\n"
      "actually (Starpath) Supercharger backup units use\n"
      "audio input/output to transfer ROMs",
      &ucon64_wf[WF_OBJ_ATA_STOP_NO_ROM]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


/*
Supercharger Tape Format Specification
Tone Specifications:
The Supercharger uses 2 distinct frequencies to store its data: A low frequency for a 1 bit and a high frequency for a 0 bit. To clarify this, let me state that the wavelength of the 0 bit is shorter than that of a 1 bit. The specific frequencies of the tones are not important as long as they are distinct enough to be distinguished by the Supercharger. To the Supercharger, each sinewave is a data bit. In the remaining text, I will refer to the tones as simply "zero bit" and "one bit" and any "bytes" mentioned represent the 8 bits or tones that correspond to the byte's value. Now getting on with the actual format of the tape:
Tape Format:

Section: Header Tone

Contents: Series of alternating 1 bits and
          0 bits.  The end of the header tone
          is marked by two 0 bits.  The header
          tone should last for a minimum of about
          1 second.
Example:  1010101010....10100


Section: Data Header

Contents: 
   byte # Description
   ------ -----------
   1      Start Address Lo-Byte
   2      Start Address Hi-Byte
   3      Control Byte
            This is the control byte,
            it determines the bankswitching
            mode of the SC.

            Bits        Function
            ------      --------
            D7-D5       Write Pulse Delay (Set to 0)
            D4-D2       RAM/ROM Configuration

                        Value     $f000     $f800
                        -----     -----     -----
                         000        3        ROM
                         001        1        ROM
                         010        3         1
                         011        1         3
                         100        3        ROM
                         101        2        ROM
                         110        3         2
                         111        2         3

   4      # of pages to load
          (1 page = 256 bytes)

   5      Data Header Checksum
            The 8 bytes in the Data Header must
            add up to #$55 (hex).

   6      Multi-load byte
            Zero for single-load games.  For Multi-Load
          games you need to know what value the game is
          looking for.  For Supercharger 8448 byte files
          this value is contained in the file.

   7      Lo-byte Bar Speed byte
            This is the lo-byte of the speed at which
          the bars move across the screen upon loading
          the game.  Supercharger Fastload uses #$42.

   8      Hi-byte Bar Speed byte
            This is the hi-byte of the speed at which
          the bars move across the screen upon loading
          the game.  Supercharger Fastload uses #$02.


Section: Page Header

Contents:

   byte # Description
   ------ -----------
   1      Bank Byte - Tells Supercharger where to load
          the current page (256 bytes) into. This is
          determined as follows:

            Bank Byte Bits:  7 6 5 4 3 2 1 0
                              \_______/   \/
                               Page #    Bank #

          For example, a page loaded into bank 3, page 4
          would have a page # of 000100 and a bank # of
          11, combining these together you get 00010010
          for a Bank Byte of #$13.  The simple formula to
          calculate the Bank Byte is:

                 (Page # * 4) + Bank #

   2      Page Checksum - This byte is equal to:
                #$55 - (sum of all 256 bytes in page)

Section: Data Page
Contents: 256 bytes of data . . . The Page Header and Data Page sections are repeated for each page in the file to be loaded.
Section: Footer Tone

Contents: Series of alternating 1 bits and
          0 bits.  The footer tone should be
          about 1 second long.
Example:  1010101010....10100
*/
