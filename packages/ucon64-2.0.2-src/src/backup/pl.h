/*
pl.h - Pocket Linker support for uCON64

Copyright (c) 2004 Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>
Partly based on PokeLink - Copyright (c) Dark Fader / BlackThunder


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
#ifndef PL_H
#define PL_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t


extern const st_getopt2_t pl_usage[];

#ifdef  USE_PARALLEL
extern int pl_info (unsigned short parport);
extern int pl_read_rom (const char *filename, unsigned short parport);
extern int pl_write_rom (const char *filename, unsigned short parport);
#endif

#endif
