/*
dc.h - Dreamcast support for uCON64

Copyright (c) 2004 NoisyB


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
#ifndef DC_H
#define DC_H

#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"


extern const st_getopt2_t dc_usage[];

extern int dc_init (st_ucon64_nfo_t *rominfo);
extern int dc_parse (const char *template_file);
extern int dc_mkip (void);
extern int dc_scramble (void);
extern int dc_unscramble (void);

#endif
