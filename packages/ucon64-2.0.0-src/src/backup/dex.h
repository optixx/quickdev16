/*
dex.h - DexDrive support for uCON64

Copyright (c) 2002 NoisyB <noisyb@gmx.net>


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
#ifndef DEX_H
#define DEX_H

extern const st_getopt2_t dex_usage[];

#define DEX_HEADER_START 0
#define DEX_HEADER_LEN 0

#ifdef USE_PARALLEL
extern int dex_read_block (const char *filename, int block_num, unsigned int parport);
extern int dex_write_block (const char *filename, int block_num, unsigned int parport);
#endif // USE_PARALLEL

#endif
