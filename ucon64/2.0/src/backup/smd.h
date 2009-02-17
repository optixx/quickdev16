/*
smd.h - Super Magic Drive support for uCON64

Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2002 - 2003 dbjh


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
#ifndef SMD_H
#define SMD_H

extern const st_getopt2_t smd_usage[];

// For the header format, see ffe.h
typedef struct st_smd_header
{
  unsigned char size;
  unsigned char id0;
  unsigned char split;
  char pad[5];
  unsigned char id1;
  unsigned char id2;
  unsigned char type;
  char pad2[501];
} st_smd_header_t;


#ifdef USE_PARALLEL
extern int smd_read_rom (const char *filename, unsigned int parport);
extern int smd_write_rom (const char *filename, unsigned int parport);
extern int smd_read_sram (const char *filename, unsigned int parport);
extern int smd_write_sram (const char *filename, unsigned int parport);
#endif
// the following two functions are used by non-transfer code in genesis.c and sms.c
extern void smd_interleave (unsigned char *buffer, int size);
extern void smd_deinterleave (unsigned char *buffer, int size);

#define SMD_HEADER_LEN (sizeof (st_smd_header_t))

#endif // SMD_H
