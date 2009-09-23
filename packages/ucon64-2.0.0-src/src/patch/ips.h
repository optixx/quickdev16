/*
ips.h - IPS support for uCON64

Copyright (c) ???? - ???? madman
Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2002        dbjh


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
#ifndef IPS_H
#define IPS_H
extern const st_getopt2_t ips_usage[];

extern int ips_apply (const char *destname, const char *ipsname);
extern int ips_create (const char *orgname, const char *modname);
#endif
