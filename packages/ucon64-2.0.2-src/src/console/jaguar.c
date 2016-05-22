/*
jaguar.c - Atari Jaguar support for uCON64

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
#include "misc/archive.h"
#include "misc/file.h"
#include "ucon64_misc.h"
#include "console/jaguar.h"
#include "backup/backup.h"


static st_ucon64_obj_t jaguar_obj[] =
  {
    {UCON64_JAG, WF_SWITCH}
  };

const st_getopt2_t jaguar_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Panther(32-bit prototype)/Jaguar64/Jaguar64 CD"/*"1989 Flare2/1993 Atari/1995 Atari"*/,
      NULL
    },
    {
      UCON64_JAG_S, 0, 0, UCON64_JAG,
      NULL, "force recognition",
      &jaguar_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
};


typedef struct st_jaguar
{
  char pad[16];
} st_jaguar_t;
#define JAGUAR_HEADER_START 0x400
#define JAGUAR_HEADER_LEN (sizeof (st_jaguar_t))

static st_jaguar_t jaguar_header;


int
jaguar_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1, x, value;

  rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ?
    ucon64.backup_header_len : 0;

  ucon64_fread (&jaguar_header, JAGUAR_HEADER_START +
                  rominfo->backup_header_len, JAGUAR_HEADER_LEN, ucon64.fname);
  value = 0;
  for (x = 0; x < 12; x++)
    value += OFFSET (jaguar_header, x);
  if (value == 0xb0)
    result = 0;
  else
    {
      rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ?
        ucon64.backup_header_len : (int) UNKNOWN_BACKUP_HEADER_LEN;

      ucon64_fread (&jaguar_header, JAGUAR_HEADER_START +
                      rominfo->backup_header_len, JAGUAR_HEADER_LEN, ucon64.fname);
      value = 0;
      for (x = 0; x < 12; x++)
        value += OFFSET (jaguar_header, x);

      if (value == 0xb0)
        result = 0;
      else
        result = -1;
    }
  if (ucon64.console == UCON64_JAG)
    result = 0;

  rominfo->header_start = JAGUAR_HEADER_START;
  rominfo->header_len = JAGUAR_HEADER_LEN;
  rominfo->header = &jaguar_header;

  rominfo->console_usage = jaguar_usage[0].help;
  rominfo->backup_usage = unknown_backup_usage[0].help;

  return result;
}
