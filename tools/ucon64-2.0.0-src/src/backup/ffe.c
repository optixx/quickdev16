/*
ffe.c - General Front Far East copier routines for uCON64

Copyright (c) 2002 - 2004 dbjh
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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "misc/misc.h"                          // kbhit(), getch()
#include "misc/itypes.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ffe.h"
#include "misc/parallel.h"


#ifdef  USE_PARALLEL

#define N_TRY_MAX          65536                // # times to test if copier ready


static void ffe_sendb (unsigned char byte);
static unsigned char ffe_wait_while_busy (void);

static int ffe_port;


void
ffe_init_io (unsigned int port)
/*
  - sets global `ffe_port'. Then the send/receive functions don't need to pass `port' all
    the way to ffe_sendb()/ffe_receiveb().
  - calls init_conio(). Necessary for kbhit() and DOS-like behaviour of getch().
*/
{
  ffe_port = port;
#if 0 // we want to support non-standard parallel port addresses
  if (ffe_port != 0x3bc && ffe_port != 0x378 && ffe_port != 0x278)
    {
      fprintf (stderr, "ERROR: PORT must be 0x3bc, 0x378 or 0x278\n");
      exit (1);
    }
#endif

#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  init_conio ();
#endif

  parport_print_info ();
}


void
ffe_deinit_io (void)
{
#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  deinit_conio ();
#endif
}


void
ffe_send_block (unsigned short address, unsigned char *buffer, int len)
{
  int n;
  unsigned char checksum = 0x81;

  ffe_send_command (0, address, (unsigned short) len);
  for (n = 0; n < len; n++)
    {
      ffe_sendb (buffer[n]);
      checksum ^= buffer[n];
    }
  ffe_sendb (checksum);
}


void
ffe_send_block2 (unsigned short address, unsigned char *buffer, int len)
{
  int n;
  unsigned char checksum = 0x81;

  ffe_send_command (2, address, (unsigned short) len);
  for (n = 0; n < len; n++)
    {
      ffe_sendb (buffer[n]);
      checksum ^= buffer[n];
    }
  ffe_sendb (checksum);
}


void
ffe_send_command0 (unsigned short address, unsigned char byte)
// command 0 for 1 byte
{
  ffe_send_command (0, address, 1);
  ffe_sendb (byte);
  ffe_sendb ((unsigned char) (0x81 ^ byte));
}


unsigned char
ffe_send_command1 (unsigned short address)
// command 1 for 1 byte
{
  unsigned char byte;

  ffe_send_command (1, address, 1);
  byte = ffe_receiveb ();
  if ((0x81 ^ byte) != ffe_receiveb ())
    puts ("received data is corrupt");

  return byte;
}


void
ffe_send_command (unsigned char command_code, unsigned short a, unsigned short l)
{
  ffe_sendb (0xd5);
  ffe_sendb (0xaa);
  ffe_sendb (0x96);
  ffe_sendb (command_code);
  ffe_sendb ((unsigned char) a);                // low byte
  ffe_sendb ((unsigned char) (a >> 8));         // high byte
  ffe_sendb ((unsigned char) l);                // low byte
  ffe_sendb ((unsigned char) (l >> 8));         // high byte
  ffe_sendb ((unsigned char) (0x81 ^ command_code ^ a ^ (a >> 8) ^ l ^ (l >> 8))); // check sum
}


void
ffe_sendb (unsigned char byte)
{
  ffe_wait_for_ready ();
  outportb ((unsigned short) (ffe_port + PARPORT_DATA), byte);
  outportb ((unsigned short) (ffe_port + PARPORT_CONTROL),
            (unsigned char) (inportb ((unsigned short) // invert strobe
                                      (ffe_port + PARPORT_CONTROL)) ^ PARPORT_STROBE));
  ffe_wait_for_ready ();                        // necessary if followed by ffe_receiveb()
}


void
ffe_receive_block (unsigned short address, unsigned char *buffer, int len)
{
  volatile int n;
  int n_try = 0;
  unsigned char checksum1, checksum2;

  do
    {
      checksum1 = 0x81;
      ffe_send_command (1, address, (unsigned short) len);
      for (n = 0; n < len; n++)
        {
          buffer[n] = ffe_receiveb ();
          checksum1 ^= buffer[n];
        }
      checksum2 = ffe_receiveb ();

      for (n = 0; n < 65536; n++)               // a delay is necessary here
        ;

      n_try++;
    }
  while ((checksum1 != checksum2) && (n_try < N_TRY_MAX));

  if (checksum1 != checksum2)
    puts ("\nreceived data is corrupt");
}


void
ffe_receive_block2 (unsigned short address, unsigned char *buffer, int len)
{
  volatile int n;
  int n_try = 0;
  unsigned char checksum1, checksum2;

  do
    {
      checksum1 = 0x81;
      ffe_send_command (3, address, (unsigned short) len);
      for (n = 0; n < len; n++)
        {
          buffer[n] = ffe_receiveb ();
          checksum1 ^= buffer[n];
        }
      checksum2 = ffe_receiveb ();

      for (n = 0; n < 65536; n++)               // a delay is necessary here
        ;

      n_try++;
    }
  while ((checksum1 != checksum2) && (n_try < N_TRY_MAX));

  if (checksum1 != checksum2)
    puts ("\nreceived data is corrupt");
}


unsigned char
ffe_receiveb (void)
{
  unsigned char byte;

  byte = (unsigned char) ((ffe_wait_while_busy () & PARPORT_INPUT_MASK) >> 3); // receive low nibble
  outportb ((unsigned short) (ffe_port + PARPORT_CONTROL),
            (unsigned char) (inportb ((unsigned short) // invert strobe
                                      (ffe_port + PARPORT_CONTROL)) ^ PARPORT_STROBE));
  byte |= (unsigned char) ((ffe_wait_while_busy () & PARPORT_INPUT_MASK) << 1); // receive high nibble
  outportb ((unsigned short) (ffe_port + PARPORT_CONTROL),
            (unsigned char) (inportb ((unsigned short) // invert strobe
                                      (ffe_port + PARPORT_CONTROL)) ^ PARPORT_STROBE));

  return byte;
}


unsigned char
ffe_wait_while_busy (void)
{
  unsigned char input;
  int n_try = 0;

  do
    {
      input = inportb ((unsigned short) (ffe_port + PARPORT_STATUS));
      n_try++;
    }
  while (input & PARPORT_IBUSY && n_try < N_TRY_MAX);

#if 0
/*
  VGS doesn't check for this, and it seems to happen quite regularly, so it
  is currently commented out
*/
  if (n_try >= N_TRY_MAX)
    {
      fputs ("ERROR: The copier is not ready\n" // yes, "ready" :-)
             "       Turn it off for a few seconds then turn it on and try again\n",
             stderr);
      exit (1);
    }
#endif

  // read port again to let data settle down and to delay a little bit - JohnDie
  return inportb ((unsigned short) (ffe_port + PARPORT_STATUS));
}


void
ffe_wait_for_ready (void)
{
  unsigned char input;
  int n_try = 0;

  do
    {
      input = inportb ((unsigned short) (ffe_port + PARPORT_STATUS));
      n_try++;
    }
  while (!(input & PARPORT_IBUSY) && n_try < N_TRY_MAX);

#if 0
  if (n_try >= N_TRY_MAX)
    {
      fputs ("ERROR: The copier is not ready\n"
             "       Turn it off for a few seconds then turn it on and try again\n",
             stderr);
      exit (1);
    }
#endif
}


void
ffe_checkabort (int status)
{
  if (((!ucon64.frontend) ? kbhit () : 0) && getch () == 'q')
    {
//      ffe_send_command (5, 0, 0);               // VGS: when sending/receiving a SNES ROM
      puts ("\nProgram aborted");
      exit (status);
    }
}

#endif // USE_PARALLEL
