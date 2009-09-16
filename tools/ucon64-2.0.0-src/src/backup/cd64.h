/*
cd64.h - CD64 support for uCON64

Copyright (c) 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2004 dbjh


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
#ifndef CD64_H
#define CD64_H

extern const st_getopt2_t cd64_usage[];

#if     defined USE_PARALLEL && defined USE_LIBCD64
extern int cd64_read_rom(const char *filename, int size);
extern int cd64_write_rom(const char *filename);
extern int cd64_write_bootemu (const char *filename);
extern int cd64_read_sram(const char *filename);
extern int cd64_write_sram(const char *filename);
extern int cd64_read_flashram(const char *filename);
extern int cd64_write_flashram(const char *filename);
extern int cd64_read_eeprom(const char *filename);
extern int cd64_write_eeprom(const char *filename);
extern int cd64_read_mempack(const char *filename, int index);
extern int cd64_write_mempack(const char *filename, int index);
#endif // USE_PARALLEL && USE_LIBCD64

#endif // CD64_H
