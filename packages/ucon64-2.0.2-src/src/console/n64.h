/*
n64.h - Nintendo 64 support for uCON64

Copyright (c) 1999 - 2001 NoisyB


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
#ifndef N64_H
#define N64_H

#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"


extern const st_getopt2_t n64_usage[];

extern int n64_bot (st_ucon64_nfo_t *rominfo, const char *bootfile);
extern int n64_chk (st_ucon64_nfo_t *rominfo);
extern int n64_f (st_ucon64_nfo_t *rominfo);
extern int n64_init (st_ucon64_nfo_t *rominfo);
extern int n64_n (st_ucon64_nfo_t *rominfo, const char *name);
extern int n64_sram (st_ucon64_nfo_t *rominfo, const char *sramfile);
extern int n64_usms (st_ucon64_nfo_t *rominfo, const char *smsrom);
extern int n64_v64 (st_ucon64_nfo_t *rominfo);
extern int n64_z64 (st_ucon64_nfo_t *rominfo);

#endif
