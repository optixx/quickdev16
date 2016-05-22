/*
gg.h - GameGenie support for uCON64

Copyright (c) 2001 WyrmCorp <http://wyrmcorp.com>.
Copyright (c) 2002 NoisyB


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
#ifndef GG_H
#define GG_H

#include "misc/getopt2.h"                       // st_getopt2_t


extern const st_getopt2_t gg_usage[];

extern int gg_apply (st_ucon64_nfo_t *rominfo, const char *code);
extern int gg_display (st_ucon64_nfo_t *rominfo, const char *code);

#endif
