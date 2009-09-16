/*
cmc.h - Cyan's Megadrive ROM copier support for uCON64

Copyright (c) 1999-2004 Cyan Helkaraxe


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
// See cmc.c for important information
#ifndef CMC_H
#define CMC_H

extern const st_getopt2_t cmc_usage[];

#ifdef USE_PARALLEL
extern int cmc_read_rom (const char *filename, unsigned int parport, int speed);
extern int cmc_test (int test, unsigned int parport, int speed);
#endif

#endif
