/*
console.h - console support for uCON64

Copyright (c) 2003 - 2006 NoisyB


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
#ifndef CONSOLE_H
#define CONSOLE_H

#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"


/*
  usage and init function for all unknown consoles
*/
extern const st_getopt2_t unknown_console_usage[];

extern int unknown_console_init (st_ucon64_nfo_t *rominfo);


/*
  usages for unsupported consoles
*/
extern const st_getopt2_t cd32_usage[];
extern const st_getopt2_t cdi_usage[];
extern const st_getopt2_t channelf_usage[];
extern const st_getopt2_t gamecom_usage[];
extern const st_getopt2_t gc_usage[];
extern const st_getopt2_t gp32_usage[];
extern const st_getopt2_t intelli_usage[];
extern const st_getopt2_t arcade_usage[];
extern const st_getopt2_t odyssey2_usage[];
extern const st_getopt2_t odyssey_usage[];
extern const st_getopt2_t ps2_usage[];
extern const st_getopt2_t real3do_usage[];
extern const st_getopt2_t s16_usage[];
extern const st_getopt2_t sat_usage[];
extern const st_getopt2_t vc4000_usage[];
extern const st_getopt2_t vectrex_usage[];
extern const st_getopt2_t xbox_usage[];


#define NINTENDO_MAKER_LEN 684
extern const char *nintendo_maker[];

#endif
