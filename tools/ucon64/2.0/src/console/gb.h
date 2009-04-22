/*
gb.h - Game Boy support for uCON64

Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>


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
#ifndef GB_H
#define GB_H

#define GB_LOGODATA_LEN 48

extern const st_getopt2_t gameboy_usage[];
extern const unsigned char gb_logodata[], rocket_logodata[];

extern int gameboy_chk (st_rominfo_t *rominfo);
extern int gameboy_gbx (st_rominfo_t *rominfo);
extern int gameboy_mgd (st_rominfo_t *rominfo);
extern int gameboy_n (st_rominfo_t *rominfo, const char *name);
extern int gameboy_n2gb (st_rominfo_t *rominfo, const char *emu_rom);
extern int gameboy_sgb (st_rominfo_t *rominfo);
extern int gameboy_ssc (st_rominfo_t *rominfo);
extern int gameboy_init (st_rominfo_t *rominfo);
extern int gameboy_logo (st_rominfo_t *rominfo);

#endif
