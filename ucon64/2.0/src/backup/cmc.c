/*
cmc.c - Cyan's Megadrive ROM copier support for uCON64

Copyright (c) 1999 - 2004 Cyan Helkaraxe

Special thanks to dbjh for helping with the uCON64 integration
of this software, and providing the wrapping code.

CMC version: 2.5
For hardware version 1.x

Copies Sega Megadrive/Genesis cartridges into .BIN format ROM files.


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
Additional information:
This software is distributed in accordance with the GNU General
Public License. The author of the hardware design/documentation and
software, Cyan Helkaraxe, retains the copyright over the 'cmc' code
('software'), the hardware design and construction documentation itself.
Cyan grants you the rights of the GNU General Public License over
the software, as stated above. The copyright does not affect your
rights in relation to the 'cmc' code under the GNU General Public
License in any way.
Cyan Helkaraxe does NOT grant you any rights relating to the hardware
design/documentation, over and above the right to build the device for
personal, not-for-profit use. Likewise, the same applies to the ROM
copier construction documentation, available at
http://www.emulationzone.org/projects/cyan/docs/ for not-for-profit
personal use.

Obviously, feel free to make changes to this software, but if you do so,
*please* clearly mark the fact it is modified, to avoid confusion.
A define is provided below this commented area, which should be edited
as described. This is to inform users which version of the code they are
using at runtime, and if they encounter a problem, it makes it far easier
for me to help debug it. I have no evil nefarious intentions. =P

Obviously, your changes must adhere to the GNU GPL, and please keep the
copyright, Cyan's name, e-mail and web site address in the comments
somewhere.

If you wish to do anything with the hardware design or the documentation
that goes beyond personal use, get in touch with Cyan first;
cyan@emulationzone.org

Disclaimer / Warranty:
This is to emphasise, not replace, any warranty information provided in
the GNU GPL or uCON64.
There is no warranty whatsoever, for either the software or accompanying
hardware/design/documentation. This software is provided free of charge,
AS-IS, in the hope that it may be useful.
Use it at your own risk. The author does not make any guarantee that you
will be able to use this software or accompanying
hardware/design/documentation, or that it will perform smoothly. There is
a possibility that damage or loss (including but not limited to financial
loss, hardware or software damage, data corruption, privacy violation,
or personal injury, suffering, legal action, imprisonment or death) may
arise through the use of this software or the accompanying
hardware/design/documentation, in addition to many other possible outcomes.
You take sole responsibility for any outcomes; by using this software or
accompanying hardware/design/ documentation, you agree that the author will
not be held responsible for anything that may occur. If your jurisdiction
does not allow the author to be isolated from responsibility, then you
must *not* use this software or accompanying hardware/design/documentation.
The author does not condone software piracy. You may not use this software
to engage in piracy. The author is not responsible for anything you choose
to use this software for, although the author strongly recommends that you
use the software for the purpose for which it was intended to be used --
primarily as an educational tool, and secondarily to make backup copies of
your expensive/rare game cartridges, or a similarly harmless and legal
purpose.
Note that although the author isn't aware of any patent violations caused
by this software/hardware/design/documentation, it is possible that
there may be patents covering parts of this device. It is your
responsibility to check this before building or using the hardware or
software, and Cyan may not be held responsible in the event of an
infringement.

That being said, if you do encounter any problems with the hardware or
software, then feel free to get in touch with the author; use the subject
line 'ROM Copier Technical Support'. No promises or guarantees are made,
however.

Also note that the author is not affiliated with anyone involved with uCON64;
therefore, only correspondence relating to this particular file (the 'cmc'
code) or the accompanying hardware design should be directed to Cyan.
If you have more general uCON64 questions, Cyan is *not* the person to ask.
Likewise, the terms "the author" and "software" in this file (cmc.c, and
additionally cmc.h), along with similar terms, apply only to Cyan, and the
CMC software you see in this file. The disclaimer above, for example, relates
exclusively to the CMC code.

All trademarks, indicated or otherwise, are the property of their
respective owners.
*/

/*
  NOTE!
  Please edit the following line, and remove the word "original" from it
  if you have made any modifications to this file. This reduces user
  confusion.
*/
#define INTRO_TEXT "Cyan's Megadrive Copier             (c) 1999-2004 Cyan Helkaraxe\n" \
                   "Software version 2.5 original, designed for hardware version 1.x\n\n"


#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "misc/bswap.h"
#include "misc/misc.h"
#include "misc/parallel.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "cmc.h"

#ifdef  USE_PARALLEL

#define RSLO 0x40                               // reset line 1 d7
#define RSHI 0x01                               // reset line 2 d0
#define CNLO 0x10                               // counter line 1 d4
#define CNHI 0x04                               // counter line 2 d2
#define INLO 0x40                               // input 1 ack (int disabled)
#define INHI 0x10                               // input 2 selectin

#define MBYTE (1024 * 1024)
#define DEFAULT_SPEED 3

#ifdef  _MSC_VER
// Visual C++ doesn't allow inline in C source code
#define inline __inline
#endif


/************************
 *  Internal functions  *
 ************************/

static inline void
cyan_write_copier (unsigned char data, unsigned int parport)
// write a value to the data register of the parallel port
{
  outportb ((unsigned short) (parport + PARPORT_DATA), data);
}


static inline unsigned char
cyan_read_copier (unsigned int parport)
// read a value from the status register of the parallel port
{
  return inportb ((unsigned short) (parport + PARPORT_STATUS));
}


static inline unsigned char
cyan_verify_copier (unsigned int parport)
// read a value back from the data register for verification
{
  return inportb ((unsigned short) (parport + PARPORT_DATA));
}


/**** non-hardware, non-accessing ****/

static unsigned long
cyan_calculate_rom_size (unsigned char *buffer, int test_mode)
/*
  Calculate the ROM size, by looking at the ROM size entry in the ROM 'header',
  and the overall structure.
  This function always returns a value rounded to a power of two between 128
  kbytes and 4 Mbytes. It also inspects the ROM for 0's or ffh's. If test_mode
  is 1 it causes an error on that condition, frees the buffer and exits.
*/
{
  unsigned long i = 0x80000000, reported_size;

  // look at reported size
  reported_size = (buffer[0x1a4] << 24) +
                  (buffer[0x1a5] << 16) +
                  (buffer[0x1a6] << 8) +
                   buffer[0x1a7] + 1;
  // cap
  // there is a minimum valid size for ROMs, according to some sources
  if (reported_size < MBIT)
    reported_size = MBIT;
  if (reported_size > 4 * MBYTE)
    reported_size = 4 * MBYTE;
  // round
  if (reported_size & (reported_size - 1))
    {
      while (!(reported_size & 0x80000000))
        {
          i >>= 1;
          reported_size = (reported_size << 1) | 1;
        }
      reported_size = i << 1;
    }
  // calculate real size
  for (i = 2 * MBYTE; i >= 65536; i >>= 1)
    if (memcmp (buffer, buffer + i, i))
      {
        i >>= 1;
        break;
      }
  i <<= 2;
  if (reported_size < i)
    reported_size = i;                          // pick the safest (largest) of the two
  if (i == MBIT)
    {
      for (i = 0; i < MBIT; i++)
        if ((buffer[i] != 0xff) && (buffer[i] != 0x00))
          break;
      if (i == MBIT)
        {
          FILE *output;

          if (test_mode)
            {
              output = stderr;
              fputs ("\nERROR:   ", stderr);
            }
          else
            {
              output = stdout;
              fputs ("\nWARNING: ", stdout);
            }

          //     "WARNING: "
          fputs (         "The ROM file appears to consist of nothing but 0x00 / 0xff values.\n"
                 "         This usually indicates a serious problem. Perhaps your parallel port\n"
                 "         isn't configured correctly, or there is some problem with the ROM\n"
                 "         copier. Is it getting power? Is a cartridge inserted? Is it properly\n"
                 "         attached to the PC?\n",
                 output);
          if (test_mode)
            {
              free (buffer);
              exit (1);
            }
        }
    }
  return reported_size;
}


static int
cyan_checksum_rom (unsigned char *buffer)
// return 0 on success, -1 on failure
{
  unsigned char *buffer2 = buffer;
  unsigned short reported_sum, running_sum = 0;

  reported_sum = (buffer2[0x18e] << 8) + buffer2[0x18f];
  buffer2 += ((buffer2[0x1a4] << 24) +
              (buffer2[0x1a5] << 16) +
              (buffer2[0x1a6] << 8) +
              (buffer2[0x1a7] & 0xfe)) + 2;
  if (buffer2 > buffer + 4 * MBYTE)
    buffer2 = buffer + 4 * MBYTE;
  buffer += 0x200;
  if (buffer2 < buffer + 2)
    return -1;

  while (buffer2 != buffer)
    {
      running_sum += *--buffer2;
      running_sum += (*--buffer2) << 8;
    }

  return (running_sum & 0xffff) != reported_sum ? -1 : 0;
}


static inline unsigned long
cyan_get_address (unsigned long b)
// return the true address (word -- 0 - 2 M) based on word input
{
  return ((b & 0x000800) >> 11) |               // bit 0
         ((b & 0x002000) >> 12) |               // bit 1
         ((b & 0x004000) >> 12) |               // bit 2
         ((b & 0x000020) >> 2) |                // bit 3
         ((b & 0x100000) >> 16) |               // bit 4
         ((b & 0x020000) >> 12) |               // bit 5
         ((b & 0x000400) >> 4) |                // bit 6
         ((b & 0x000001) << 7) |                // bit 7
         ((b & 0x000002) << 7) |                // bit 8
         ((b & 0x000010) << 5) |                // bit 9
         ((b & 0x000040) << 4) |                // bit 10
         ((b & 0x040000) >> 7) |                // bit 11
         ((b & 0x080000) >> 7) |                // bit 12
         ((b & 0x000080) << 6) |                // bit 13
         ((b & 0x008000) >> 1) |                // bit 14
         ((b & 0x010000) >> 1) |                // bit 15
         ((b & 0x001000) << 4) |                // bit 16
         ((b & 0x000004) << 15) |               // bit 17
         ((b & 0x000008) << 15) |               // bit 18
         ((b & 0x000200) << 10) |               // bit 19
         ((b & 0x000100) << 12);                // bit 20
}


/**** non-hardware, indirectly accessing ****/

static inline void
cyan_delay (int speed, unsigned int parport)
// Delays a certain amount of time depending on speed selected. 0=long delay,
//  used for reset and hi counter.
{
  int i, scritch = 0;

  switch (speed)
    {
    case 0:
      for (i = 0; i < 128; i++)
        scritch += cyan_read_copier (parport);
    case 1:                                     // falling through
      for (i = 0; i < 64; i++)
        scritch += cyan_read_copier (parport);
    case 2:                                     // falling through
      for (i = 0; i < 12; i++)
        scritch += cyan_read_copier (parport);
    case 3:                                     // falling through
      scritch += cyan_read_copier (parport);
      scritch += cyan_read_copier (parport);
    }
}


static void
cyan_reset (unsigned int parport)
// resets the copier
{
  cyan_delay (0, parport);
  // zero all data outputs first, before going into SPP mode
  cyan_write_copier (0, parport);
  // reset the port to SPP, float all control lines high
  cyan_write_copier (0, parport + PARPORT_CONTROL);
  cyan_delay (0, parport);
  cyan_write_copier (RSLO | RSHI, parport);     // both reset lines hi
  cyan_delay (0, parport);
  cyan_write_copier (0, parport);               // both reset lines lo
  cyan_delay (0, parport);
}


static inline unsigned short
cyan_get_word (int speed, unsigned int parport)
// gets a byte pair from the ROM and return two bytes in big endian byte order
{
  unsigned short value = 0;
  unsigned char tempz;

  cyan_write_copier (0, parport);
  cyan_delay (speed, parport);
  tempz = cyan_read_copier (parport);
  value |= tempz & INLO;                        // bit 6
  value |= (tempz & INHI) << 8;                 // bit 12

  cyan_write_copier (CNLO, parport);
  cyan_delay (speed, parport);
  tempz = cyan_read_copier (parport);
  value |= (tempz & INLO) >> 5;                 // bit 1
  value |= (tempz & INHI) << 9;                 // bit 13

  cyan_write_copier (0, parport);
  cyan_delay (speed, parport);
  tempz = cyan_read_copier (parport);
  value |= (tempz & INLO) << 3;                 // bit 9
  value |= (tempz & INHI) << 10;                // bit 14

  cyan_write_copier (CNLO, parport);
  cyan_delay (speed, parport);
  tempz = cyan_read_copier (parport);
  value |= (tempz & INLO) >> 1;                 // bit 5
  value |= (tempz & INHI) << 11;                // bit 15

  cyan_write_copier (0, parport);
  cyan_delay (speed, parport);
  tempz = cyan_read_copier (parport);
  value |= (tempz & INLO) >> 4;                 // bit 2
  value |= (tempz & INHI) << 4;                 // bit 8

  cyan_write_copier (CNLO, parport);
  cyan_delay (speed, parport);
  tempz = cyan_read_copier (parport);
  value |= (tempz & INLO) << 4;                 // bit 10
  value |= (tempz & INHI) >> 4;                 // bit 0

  cyan_write_copier (0, parport);
  cyan_delay (speed, parport);
  tempz = cyan_read_copier (parport);
  value |= (tempz & INLO) >> 2;                 // bit 4
  value |= (tempz & INHI) << 3;                 // bit 7

  cyan_write_copier (CNLO, parport);
  cyan_delay (speed, parport);
  tempz = cyan_read_copier (parport);
  value |= (tempz & INLO) >> 3;                 // bit 3
  value |= (tempz & INHI) << 7;                 // bit 11

  return me2be_16 (value);
}


static inline int
check_exit (void)
// check for user abort
{
  int temp;

  if (ucon64.frontend)
    return 0;
  if (!kbhit ())
    return 0;
  temp = getch ();
  if (temp == 'q' || (temp == 27))
    return 1;
  return 0;
}


static unsigned char *
cyan_read_rom (int speed, unsigned int parport, unsigned char *buffer)
/*
  Read the ROM and return a pointer to a 4 MB area of memory containing all ROM
  data. Designed to be used from inside cyan_copy_rom(), although it can be
  called elsewhere if a raw (but decoded) dump is required.
*/
{
  unsigned long q;
  time_t t;

  // allocate the dump area
  if (!buffer)
    if ((buffer = (unsigned char *) malloc (4 * MBYTE)) == NULL)
      {
        fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], 4 * MBYTE);
        exit (1);
      }

  cyan_reset (parport);                         // reset the copier

  t = time (NULL);
  // copy routine
  for (q = 0; q < 2 * MBYTE; )                  // loop through all words
    {
      // get a (16-bit) word from the ROM
      ((unsigned short *) buffer)[cyan_get_address (q)] = cyan_get_word (speed, parport);

      // periodically update progress bar, without hammering ucon64_gauge()
      if (!(q & (0xfff >> (5 - speed))))
        {
          if (check_exit ())
            {
              free (buffer);
              puts ("\n"
                    "Copy aborted.\n"
                    "Don't forget to turn the ROM copier off and never insert or remove a cartridge\n"
                    "with the power on");
              break;
            }
          ucon64_gauge (t, q * 2, 4 * MBYTE);
        }

      if (!(++q & 0x3ff)) // advance loop counter and carry to hi counter (11 bits)
        {
          cyan_delay (0, parport);
          cyan_write_copier (CNHI, parport);
          cyan_delay (0, parport);
          cyan_write_copier (0, parport);
          cyan_delay (0, parport);
        }
    }

  // make sure it's left in a state where it's safe to remove the cart
  cyan_reset (parport);

  if (q != 2 * MBYTE)
    return NULL;

  ucon64_gauge (t, q * 2, 4 * MBYTE);           // make the progress bar reach 100%

  return buffer;
}


static void
cyan_test_parport (unsigned int parport)
// Test the parallel port to see if it appears to be functioning correctly, and
//  terminate if there's an error.
{
  unsigned short temp;

  cyan_reset (parport);
  fputs ("Basic parallel port test: ", stdout);
  fflush (stdout);
  cyan_write_copier (170, parport);
  cyan_delay (0, parport);
  temp = cyan_verify_copier (parport) & 170;
  cyan_reset (parport);

  // even in unidirectional mode, the parallel port is bidirectional; at
  //  least, for a few short moments before the driver IC smokes
  if ((cyan_verify_copier (parport) & 170) != 0 || temp != 170)
    {
      puts ("FAILED");
      fputs ("ERROR: Parallel port error\n"
             "       Check that your parallel port is configured properly, in the BIOS, OS,\n"
             "       and uCON64, and check for short circuits on the parallel port connector.\n"
             "       Also ensure that the ROM copier is getting power, and a cartridge is\n"
             "       inserted\n",
             stderr);
      exit (1);
    }
  else
    puts ("Passed");

  // discharge caps to see if we've got power
  cyan_reset (parport);
  cyan_reset (parport);
  cyan_write_copier (CNLO + CNHI, parport);
  cyan_delay (0, parport);
  for (temp = 0; temp < 1000; temp++)
    {
      cyan_write_copier (0, parport);
      cyan_delay (3, parport);
      cyan_write_copier (CNLO + CNHI, parport);
      cyan_delay (3, parport);
    }
  cyan_reset (parport);
  cyan_reset (parport);

  fputs ("Parallel port output test: ", stdout);
  fflush (stdout);
  cyan_write_copier (255, parport);
  cyan_delay (0, parport);
  temp = (cyan_verify_copier (parport) != 255);
  cyan_write_copier (0, parport);
  cyan_delay (0, parport);
  temp |= (cyan_verify_copier (parport) != 0);
  cyan_write_copier (CNLO, parport);
  cyan_delay (0, parport);
  temp |= (cyan_verify_copier (parport) != CNLO);
  cyan_write_copier (CNHI, parport);
  cyan_delay (0, parport);
  temp |= (cyan_verify_copier (parport) != CNHI);
  cyan_write_copier (RSLO, parport);
  cyan_delay (0, parport);
  temp |= (cyan_verify_copier (parport) != RSLO);
  cyan_write_copier (RSHI, parport);
  cyan_delay (0, parport);
  temp |= (cyan_verify_copier (parport) != RSHI);
  cyan_reset (parport);

  // if it's still okay after that, then try reading the first set of inputs
  //  with lines high and low
  if (!temp)
    {
      fputs ("Passed\n"
             "Input crosstalk test: ",
             stdout);
      fflush (stdout);
      temp = cyan_read_copier (parport) & (INLO | INHI);
      cyan_write_copier (255 - CNLO, parport);
      cyan_delay (0, parport);
      temp = (temp != (cyan_read_copier (parport) & (INLO | INHI)));
      cyan_reset (parport);
    }

  if (temp)
    {
      puts ("FAILED");
      fputs ("ERROR: Parallel port error\n"
             "Possible causes: ROM copier not getting power (check or replace battery)\n"
             "                 Short circuit or bad connection (on parallel port or board)\n"
             "                 Cartridge not inserted properly (or not inserted at all)\n"
             "                 Parallel port not configured correctly\n"
             "                 Orange, grey or green wire(s) soldered to the wrong locations\n"
             "                 Chips inserted backwards\n"
             "NOTE: Don't forget the ROM copier needs to be turned on before starting!\n",
             stderr);
      exit (1);
    }
  else
    puts ("Passed");
}


static int
cyan_test_copier (int test, int speed, unsigned int parport)
{
  unsigned char *buffer1, *buffer2 = NULL;
  int count = 1;

  fputs (INTRO_TEXT, stdout);
  parport_print_info ();

  switch (test)
    {
    // reliability test -- note: this test may be required to run for 8 hours or more
    case 1:
      printf ("Reliability test mode selected, speed %d\n", speed);
      cyan_test_parport (parport);
      puts ("\n"
            "Entering non-stop reliability test mode (press escape or q to exit, and turn\n"
            "ROM copier off immediately afterwards)\n"
            "\n"
            "Copy process will continue indefinitely until an error is encountered, at\n"
            "which point the program will terminate.\n"
            "A large number of passes suggests that the copier is working reliably at the\n"
            "selected speed\n");
      printf ("                                                                          P %2d",
              count);
      fflush (stdout);
      if (ucon64.frontend)
        fputc ('\n', stdout);
      buffer1 = cyan_read_rom (speed, parport, NULL);
      if (!buffer1)                             // user abort
        exit (0);

      // detect if ROM is all 0x00 or 0xff and print an error if so
      cyan_calculate_rom_size (buffer1, 1);

      while (1)
        {
          clear_line ();                        // remove last gauge
          printf ("   Pass %2d OK\n", count);
          count++;

          // verify checksum of first pass
          if (count == 2)                       // check only in first iteration
            if (cyan_checksum_rom (buffer1))    // verify checksum
              puts ("\n"
                    "WARNING: Checksum of ROM does not appear to be correct.\n"
                    "         This may be normal for this ROM, or it may indicate a bad copy\n");

          printf ("                                                                          P %2d",
                  count);
          fflush (stdout);
          if (ucon64.frontend)
            fputc ('\n', stdout);
          buffer2 = cyan_read_rom (speed, parport, buffer2);
          if (!buffer2)
            {
              free (buffer1);
              exit (0);
            }
          if (memcmp (buffer1, buffer2, 4 * MBYTE))
            {
              // error
              printf ("\n"
                      "\n"
                      "Error detected on pass number %d\n"
                      "\n",
                      count);
              if (count == 2)
                puts ("A failure this early suggests a critical fault, such as a misconfigured or\n"
                      "incompatible parallel port, extremely poor wiring, or power supply problems --\n"
                      "you may wish to replace the battery or try another power supply, and use\n"
                      "shorter cables.\n"
                      "Try lowering the speed and running this test again, as a too high speed can\n"
                      "often cause these symptoms.\n"
                      "Alternatively, it may have been a one-time glitch; re-run the test to be sure.\n"
                      "When (if?) you find a lower speed which works reliably, use that speed for\n"
                      "copying ROMs\n");
              else
                puts ("The first couple of passes were successful. This indicates that you have a\n"
                      "minor intermittent problem; most likely power supply problems, bad wiring, or\n"
                      "some kind of one-time glitch.\n"
                      "You may wish to replace the battery or try another power supply, and use\n"
                      "shorter cables.\n"
                      "Make sure no electrical appliances turn on or off during the copy.\n"
                      "Re-run the test to be sure; it's recommended that you use a lower speed\n");
              free (buffer1);
              free (buffer2);
              exit (1);
            }
        }

      free (buffer1);
      free (buffer2);
      break;
    // manual test
    case 2:
      cyan_reset (parport);
      cyan_write_copier (CNHI, parport);
      cyan_delay (0, parport);
      cyan_write_copier (0, parport);
      cyan_delay (0, parport);

      if (speed != DEFAULT_SPEED)
        puts ("Ignoring specified speed; test bench mode does not require a speed setting");
      // print screen
      puts ("Entering manual test bench mode\n"
            "\n"
            "Probe the board and verify that the counters are being clocked, and are\n"
            "counting correctly. The upper counter should be one count ahead of the lower\n"
            "counter, with both clocked at the same rate.\n"
            "Inject logic levels into the multiplexers to verify that the data bits are\n"
            "being read correctly:\n"
            "*=high .=low, layout: L H L H L H L H (L=low multiplexer, H=high multiplexer)\n"
            "NOTE: The signals in question are the chip native signals; D0 below corresponds\n"
            "to D0 on the multiplexer, NOT D0 on the cartridge port. Likewise with the\n"
            "address lines. The input lines are in counter order, left to right.\n"
            "Press escape or q to exit; be sure to turn the ROM copier off immediately after\n"
            "exiting, to reset the device.\n"
            "\n"
            "If the above didn't make any sense to you, press escape or q and turn the ROM\n"
            "copier off immediately!\n"
            "This test is designed for advanced users only\n");
      while (1)
        {
          const char *status[2] = {"* ", ". "};

          fputc ('\r', stdout);

          cyan_write_copier (0, parport);
          cyan_delay (1, parport);
          count = cyan_read_copier (parport);
          fputs (status[((count ^ INLO) >> 6) & 1], stdout);
          fputs (status[((count ^ INHI) >> 4) & 1], stdout);

          cyan_write_copier (CNLO | CNHI, parport);
          cyan_delay (1, parport);
          count = cyan_read_copier (parport);
          fputs (status[((count ^ INLO) >> 6) & 1], stdout);
          fputs (status[((count ^ INHI) >> 4) & 1], stdout);

          cyan_write_copier (0, parport);
          cyan_delay (1, parport);
          count = cyan_read_copier (parport);
          fputs (status[((count ^ INLO) >> 6) & 1], stdout);
          fputs (status[((count ^ INHI) >> 4) & 1], stdout);

          cyan_write_copier (CNLO | CNHI, parport);
          cyan_delay (1, parport);
          count = cyan_read_copier (parport);
          fputs (status[((count ^ INLO) >> 6) & 1], stdout);
          fputs (status[((count ^ INHI) >> 4) & 1], stdout);

          cyan_write_copier (0, parport);
          cyan_delay (1, parport);
          count = cyan_read_copier (parport);
          fputs (status[((count ^ INLO) >> 6) & 1], stdout);
          fputs (status[((count ^ INHI) >> 4) & 1], stdout);

          cyan_write_copier (CNLO | CNHI, parport);
          cyan_delay (1, parport);
          count = cyan_read_copier (parport);
          fputs (status[((count ^ INLO) >> 6) & 1], stdout);
          fputs (status[((count ^ INHI) >> 4) & 1], stdout);

          cyan_write_copier (0, parport);
          cyan_delay (1, parport);
          count = cyan_read_copier (parport);
          fputs (status[((count ^ INLO) >> 6) & 1], stdout);
          fputs (status[((count ^ INHI) >> 4) & 1], stdout);

          cyan_write_copier (CNLO | CNHI, parport);
          cyan_delay (1, parport);
          count = cyan_read_copier (parport);
          fputs (status[((count ^ INLO) >> 6) & 1], stdout);
          fputs (status[((count ^ INHI) >> 4) & 1], stdout);

          cyan_write_copier (0, parport);
          cyan_delay (1, parport);
          fflush (stdout);

          if (check_exit ())
            {
              cyan_reset (parport);
              puts ("\nUser aborted test");
              exit (0);
            }
        }
      break;
    default: // cmc_test() should only pass a correct speed value
      fputs ("INTERNAL ERROR: Invalid test number passed to cyan_test_copier()\n", stderr);
      exit (1);
    }
  return 0;
}


static int
cyan_copy_rom (const char *filename, int speed, unsigned int parport)
/*
  Copy a ROM file -- this assumes the filename is valid and the file does not
  already exist, since it will blindly try to write (overwrite) the filename you
  give it.
  If the open failed due to an invalid filename or path, it prints an error.
  Speed setting should be between 1-4, 3 is default, and this is verified.
*/
{
  unsigned long romsize;
  unsigned char *buffer;
  FILE *f;

  fputs (INTRO_TEXT, stdout);
  parport_print_info ();

  if (!strlen (filename))
    {
      fputs ("ERROR: Filename not specified\n"
             "       You must specify a filename on the commandline, as follows:\n"
             "       ucon64 " OPTION_LONG_S "xcmc dump.bin\n", stderr);
      exit (1);
    }

  printf ("Speed %d selected\n", speed);
  cyan_test_parport (parport);
  printf ("Destination file: %s\n", filename);

  if ((f = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  fclose (f);

  puts ("NOTE: Dumping copier's full address space (file will be automatically trimmed\n"
        "      after dumping)\n"
        "Press escape or q to abort\n");

  buffer = cyan_read_rom (speed, parport, NULL);
  if (!buffer)
    {
      remove (filename);
      exit(0);
    }

  fputc ('\n', stdout);
  romsize = cyan_calculate_rom_size (buffer, 0);

  fputs ("Writing ROM to disk... ", stdout);
  fflush (stdout);
  if ((f = fopen (filename, "wb")) == NULL)
    {
      puts ("FAILED");
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      free (buffer);
      exit (1);
    }
  if (fwrite (buffer, 1, romsize, f) != romsize)
    {
      puts ("FAILED");
      fprintf (stderr, ucon64_msg[WRITE_ERROR], filename);
      free (buffer);
      fclose (f);
      exit (1);
    }
  fclose (f);
  printf ("%d kBytes OK\n"
          "Verifying checksum... ", (int) (romsize / 1024));
  fflush (stdout);

  if (cyan_checksum_rom (buffer))
    {
      puts ("FAILED\n"
            "WARNING: Checksum of ROM does not appear to be correct.\n"
            "         This may be normal for this ROM, or it may indicate a bad copy.\n"
            "         Please verify the ROM, and consider running a copier test");
    }
  else
    puts ("OK");

  puts ("Copy complete!\n"
        "Don't forget to turn the ROM copier off, and never insert or remove a\n"
        "cartridge with the power on");

  free (buffer);
  return 0;

}

#endif // USE_PARALLEL


/*******************
 * uCON64 wrapping *
 *******************/

const st_getopt2_t cmc_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Cyan's Megadrive ROM copier"/*"1999-2004 Cyan Helkaraxe"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xcmc", 0, 0, UCON64_XCMC,
      NULL, "receive ROM from Cyan's Megadrive ROM copier; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_GEN_STOP_NO_ROM]
    },
    {
      "xcmct", 1, 0, UCON64_XCMCT,
      "TEST", "run test TEST\n"
      "TEST=1 burn-in reliability test (specify speed)\n"
      "TEST=2 testbench mode (experts only)",
      &ucon64_wf[WF_OBJ_GEN_STOP_NO_ROM]
    },
    {
      "xcmcm", 1, 0, UCON64_XCMCM,
      "SPEED", "specify transfer speed\n"
      "SPEED=1 slow (debug)\n"
      "SPEED=2 medium\n"
      "SPEED=3 fast (default)\n"                // verify with value of DEFAULT_SPEED
      "SPEED=4 full speed (risky)",
      &ucon64_wf[WF_OBJ_GEN_SWITCH]
    },
#endif // USE_PARALLEL
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

int
cmc_read_rom (const char *filename, unsigned int parport, int speed)
{
#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  init_conio ();
#endif

  if (speed < 1 || speed > 4)
    speed = DEFAULT_SPEED;
  cyan_copy_rom (filename, speed, parport);

#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  deinit_conio ();
#endif

  return 0;
}


int
cmc_test (int test, unsigned int parport, int speed)
{
#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  init_conio ();
#endif

  if (test < 1 || test > 2)
    {
      fputs ("ERROR: Choose a test between 1 and 2 (inclusive)\n", stderr);
      exit (1);
    }
  if (speed < 1 || speed > 4)
    speed = DEFAULT_SPEED;
  cyan_test_copier (test, speed, parport);

#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  deinit_conio ();
#endif

  return 0;
}

#endif // USE_PARALLEL
