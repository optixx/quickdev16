/*
f2a.h - Flash 2 Advance support for uCON64

Copyright (c) 2003 Ulrich Hecht <uli@emulinks.de>
Copyright (c) 2004 NoisyB <noisyb@gmx.net>


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
#ifndef F2A_H
#define F2A_H

extern const st_getopt2_t f2a_usage[];

#if     defined USE_PARALLEL || defined USE_USB
extern int f2a_read_rom (const char *filename, int size);
extern int f2a_write_rom (const char *filename, int size);
extern int f2a_read_sram (const char *filename, int bank);
extern int f2a_write_sram (const char *filename, int bank);
#endif

#endif
