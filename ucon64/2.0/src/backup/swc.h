/*
swc.h - Super Wild Card support for uCON64

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
#ifndef SWC_H
#define SWC_H

#define SWC_IO_FORCE_32MBIT 0x001
#define SWC_IO_ALT_ROM_SIZE 0x002
#define SWC_IO_SUPER_FX     0x004
#define SWC_IO_SDD1         0x008
#define SWC_IO_SA1          0x010
#define SWC_IO_SPC7110      0x020
#define SWC_IO_DX2_TRICK    0x040
#define SWC_IO_MMX2         0x080
#define SWC_IO_DUMP_BIOS    0x100

#define SWC_IO_MAX          0x1ff               // highest valid dumping mode value

extern const st_getopt2_t swc_usage[];

// For the header format, see ffe.h
typedef struct st_swc_header
{
/*
  Don't create fields that are larger than one byte! For example size_low and size_high
  could be combined in one unsigned short int. However, this gives problems with little
  endian vs. big endian machines (e.g. writing the header to disk).
*/
  unsigned char size_low;
  unsigned char size_high;
  unsigned char emulation;
  unsigned char pad[5];
  unsigned char id1;
  unsigned char id2;
  unsigned char type;
  unsigned char pad2[501];
} st_swc_header_t;

#define SWC_HEADER_START 0
#define SWC_HEADER_LEN (sizeof (st_swc_header_t))

#ifdef USE_PARALLEL
extern int swc_read_rom (const char *filename, unsigned int parport, int io_mode);
extern int swc_write_rom (const char *filename, unsigned int parport, int enableRTS);
extern int swc_read_sram (const char *filename, unsigned int parport);
extern int swc_write_sram (const char *filename, unsigned int parport);
extern int swc_read_rts (const char *filename, unsigned int parport);
extern int swc_write_rts (const char *filename, unsigned int parport);
extern int swc_read_cart_sram (const char *filename, unsigned int parport, int io_mode);
extern int swc_write_cart_sram (const char *filename, unsigned int parport, int io_mode);
extern void swc_unlock (unsigned int parport);
#endif

#endif // SWC_H
