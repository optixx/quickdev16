/*
neogeo.h - NeoGeo support for uCON64

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
#ifndef NEOGEO_H
#define NEOGEO_H

extern int neogeo_bios (const char *fname);
extern int neogeo_init (st_rominfo_t *rominfo);
extern int neogeo_mgd (void);
extern int neogeo_mvs (void);
extern int neogeo_s (void);
extern int neogeo_sam (const char *fname);

extern const st_getopt2_t neogeo_usage[];

#endif
