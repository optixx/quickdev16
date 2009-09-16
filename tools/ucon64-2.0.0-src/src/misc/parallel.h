/*
parallel.h - miscellaneous parallel port functions

Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2004 dbjh


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
#ifndef MISC_PARALLEL_H
#define MISC_PARALLEL_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  USE_PARALLEL

#define PARPORT_DATA     0                      // output
#define PARPORT_STATUS   1                      // input
#define PARPORT_CONTROL  2
#define PARPORT_EADDRESS 3                      // EPP/ECP address
#define PARPORT_EDATA    4                      // EPP/ECP output/input

#define PARPORT_INPUT_MASK 0x78

#define PARPORT_IBUSY  0x80
#define PARPORT_STROBE 1

#define PARPORT_UNKNOWN (-1)

// DJGPP (DOS) has these, but it's better that all code uses the same functions.
extern unsigned char inportb (unsigned short port);
extern unsigned short inportw (unsigned short port);
extern void outportb (unsigned short port, unsigned char byte);
extern void outportw (unsigned short port, unsigned short word);

extern int parport_open (int parport);
extern int parport_close (int parport);
extern void parport_print_info (void);
#endif // USE_PARALLEL
#endif // MISC_PARALLEL_H
