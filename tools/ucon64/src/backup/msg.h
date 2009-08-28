/*
msg.h - Magic Super Griffin support for uCON64

Copyright (c) 2003 dbjh


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
#ifndef MSG_H
#define MSG_H

extern const st_getopt2_t msg_usage[];

// For the header format, see ffe.h
typedef struct st_msg_header
{
  unsigned char size;
  unsigned char emulation;
  unsigned char pad[6];
  unsigned char id1;
  unsigned char id2;
  unsigned char type;
  unsigned char pad2[501];
} st_msg_header_t;

#define MSG_HEADER_START 0
#define MSG_HEADER_LEN (sizeof (st_msg_header_t))

#ifdef USE_PARALLEL
extern int msg_read_rom (const char *filename, unsigned int parport);
extern int msg_write_rom (const char *filename, unsigned int parport);
#endif

#endif // MSG_H
