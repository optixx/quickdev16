/*
gba.h - Game Boy Advance support for uCON64

Copyright (c) 2001 NoisyB <noisyb@gmx.net>


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
#ifndef GBA_H
#define GBA_H

#define GBA_LOGODATA_LEN 156

extern const st_getopt2_t gba_usage[];
extern const unsigned char gba_logodata[];

extern int gba_chk (st_rominfo_t *rominfo);
extern int gba_crp (st_rominfo_t *rominfo, const char *value);
extern int gba_init (st_rominfo_t *rominfo);
extern int gba_logo (st_rominfo_t *rominfo);
extern int gba_n (st_rominfo_t *rominfo, const char *name);
extern int gba_sram (void);
extern int gba_multi (int truncate_size, char *fname);

#endif
