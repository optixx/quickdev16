/*
gd.h - Game Doctor support for uCON64

Copyright (c) 2002 John Weidman
Copyright (c) 2002 dbjh


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
#ifndef GD_H
#define GD_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t


#define GD_HEADER_START 0
#define GD_HEADER_LEN 512
#define GD3_MAX_UNITS 16                        // Maximum that the hardware supports
// Each logical memory unit is 8 Mbit in size (internally it's 2*4 Mbit)

extern const st_getopt2_t gd_usage[];

#ifdef  USE_PARALLEL
extern int gd3_read_rom (const char *filename, unsigned short parport);
extern int gd3_write_rom (const char *filename, unsigned short parport,
                          st_ucon64_nfo_t *rominfo);
extern int gd6_read_rom (const char *filename, unsigned short parport);
extern int gd6_write_rom (const char *filename, unsigned short parport,
                          st_ucon64_nfo_t *rominfo);
extern int gd3_read_sram (const char *filename, unsigned short parport);
extern int gd3_write_sram (const char *filename, unsigned short parport);
extern int gd6_read_sram (const char *filename, unsigned short parport);
extern int gd6_write_sram (const char *filename, unsigned short parport);
extern int gd3_read_saver (const char *filename, unsigned short parport);
extern int gd3_write_saver (const char *filename, unsigned short parport);
extern int gd6_read_saver (const char *filename, unsigned short parport);
extern int gd6_write_saver (const char *filename, unsigned short parport);
#endif

#endif
