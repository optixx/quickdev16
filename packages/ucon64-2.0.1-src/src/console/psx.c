/*
psx.c - Playstation support for uCON64

Copyright (c) 2001 NoisyB


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
#include "console/psx.h"


static st_ucon64_obj_t psx_obj[] =
  {
    {UCON64_PSX, WF_SWITCH}
  };

const st_getopt2_t psx_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Playstation (One)"/*"1994/(2000) Sony http://www.playstation.com"*/,
      NULL
    },
    {
      UCON64_PSX_S, 0, 0, UCON64_PSX,
      NULL, "force recognition",
      &psx_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
};


int
psx_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1;

  rominfo->console_usage = psx_usage[0].help;
//  rominfo->backup_usage = cdrw_usage[0].help;

  return result;
}
