/*
aps.h - Advanced Patch System support for uCON64

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
#ifndef APS_H
#define APS_H

#include "misc/getopt2.h"                       // st_getopt2_t


extern const st_getopt2_t aps_usage[];

extern int aps_apply (const char *modname, const char *apsname);
extern int aps_create (const char *orgname, const char *modname);
extern int aps_set_desc (const char *apsname, const char *description);

#endif
