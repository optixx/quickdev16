/*
fal.h - Flash Linker Advance support for uCON64

Copyright (c) 2001        Jeff Frohwein
Copyright (c) 2001        NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2002 dbjh


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
#ifndef FAL_H
#define FAL_H

extern const st_getopt2_t fal_usage[];

#ifdef USE_PARALLEL
extern int fal_read_rom (const char *filename, unsigned int parport, int size);
extern int fal_write_rom (const char *filename, unsigned int parport);
extern int fal_read_sram (const char *filename, unsigned int parport, int bank);
extern int fal_write_sram (const char *filename, unsigned int parport, int bank);
#endif

#endif
