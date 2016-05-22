/*
vboy.c - Virtual Boy support for uCON64

Copyright (c) 2005 NoisyB


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
#include <string.h>
#include "misc/archive.h"
#include "misc/file.h"
#include "ucon64_misc.h"
#include "console/console.h"
#include "console/vboy.h"
#include "backup/backup.h"


#define VBOY_NAME_LEN 20


static st_ucon64_obj_t vboy_obj[] =
  {
    {UCON64_VBOY, WF_SWITCH}
  };

const st_getopt2_t vboy_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Nintendo Virtual Boy"/*"19XX Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      UCON64_VBOY_S, 0, 0, UCON64_VBOY,
      NULL, "force recognition",
      &vboy_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

// the game header comes before the last 512 bytes at the end of the ROM
typedef struct st_vboy_header
{
  char name[VBOY_NAME_LEN];
  char reserved[5];
  char maker_high;
  char maker_low;
  char id[3];
  char game_id_country;
  unsigned char version;
} st_vboy_header_t;

#define VBOY_HEADER_LEN (sizeof (st_vboy_header_t))
#define VBOY_HEADER_START (ucon64.file_size - (VBOY_HEADER_LEN + 512))


int
vboy_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1, value = 0;
  static st_vboy_header_t vboy_header;

  rominfo->console_usage = vboy_usage[0].help;
  rominfo->backup_usage = unknown_backup_usage[0].help;

  // It's correct to use VBOY_HEADER_START (a macro, not a constant), because
  //  the header is located at a constant offset relative to the end of the file
  //  (no need to use ucon64.backup_header_len).
  ucon64_fread (&vboy_header, VBOY_HEADER_START, VBOY_HEADER_LEN, ucon64.fname);

  if (ucon64.console == UCON64_VBOY)
    {
      result = 0;

      rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ?
        ucon64.backup_header_len : 0;

      rominfo->header_start = VBOY_HEADER_START - rominfo->backup_header_len;
      if (rominfo->header_start < 0)
        rominfo->header_start = 0;
      rominfo->header_len = VBOY_HEADER_LEN;
      rominfo->header = &vboy_header;

      // internal ROM name
      strncpy (rominfo->name, (const char *) vboy_header.name, VBOY_NAME_LEN);
      rominfo->name[VBOY_NAME_LEN] = 0;

      // ROM maker
      {
        int ih = vboy_header.maker_high <= '9' ?
                   vboy_header.maker_high - '0' : vboy_header.maker_high - 'A' + 10,
            il = vboy_header.maker_low <= '9' ?
                   vboy_header.maker_low - '0' : vboy_header.maker_low - 'A' + 10;
        value = ih * 36 + il;
      }
      if (value < 0 || value >= NINTENDO_MAKER_LEN)
        value = 0;
      rominfo->maker = NULL_TO_UNKNOWN_S (nintendo_maker[value]);

      // ROM country
      rominfo->country =
        (vboy_header.game_id_country == 'J') ? "Japan/Asia" :
        (vboy_header.game_id_country == 'E') ? "U.S.A." :
        (vboy_header.game_id_country == 'P') ? "Europe, Australia and Africa" :
        "Unknown country";

      // misc stuff
      sprintf (rominfo->misc,
        "ID: %c%c%c%c\n"
        "Version: v1.%d",
        vboy_header.id[0], vboy_header.id[1], vboy_header.id[2], vboy_header.game_id_country,
        vboy_header.version);
    }

  return result;
}
