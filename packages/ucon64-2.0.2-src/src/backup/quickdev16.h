/*
quickdev16.h - Quickdev16 support for uCON64

Copyright (c) 2009 david@optixx.org
Copyright (c) 2015 dbjh


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
#ifndef QUICKDEV16_H
#define QUICKDEV16_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t


extern const st_getopt2_t quickdev16_usage[];

#ifdef USE_USB
extern int quickdev16_write_rom (const char *filename);
#endif

#endif
