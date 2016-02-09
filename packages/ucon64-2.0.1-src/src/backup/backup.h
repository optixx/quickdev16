/*
backup.h - backup support for uCON64 

Copyright (c) 2003 NoisyB


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
#ifndef BACKUP_H
#define BACKUP_H

#include "misc/getopt2.h"                       // st_getopt2_t


/*
  usage and init function for all unknown backup units/emulators
*/
extern const st_getopt2_t unknown_backup_usage[];


/*
  default header for unknown backup units
*/
typedef struct // st_unknown_header
{
  /*
    Don't create fields that are larger than one byte! For example size_low and
    size_high could be combined in one unsigned short int. However, this gives
    problems with little endian vs. big endian machines (e.g. writing the header
    to disk).
  */
  unsigned char size_low;
  unsigned char size_high;
  unsigned char emulation;
  unsigned char hirom;
  unsigned char emulation1;
  unsigned char emulation2;
  unsigned char pad[2];
  unsigned char id1;
  unsigned char id2;
  unsigned char type;
  unsigned char pad2[501];
} st_unknown_backup_header_t;

#define UNKNOWN_BACKUP_HEADER_START 0
#define UNKNOWN_BACKUP_HEADER_LEN (sizeof (st_unknown_backup_header_t))

#endif
