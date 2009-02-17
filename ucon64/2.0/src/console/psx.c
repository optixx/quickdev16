/*
psx.c - Playstation support for uCON64

Copyright (c) 2001 NoisyB <noisyb@gmx.net>


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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/misc.h"
#include "misc/itypes.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "psx.h"


const st_getopt2_t psx_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Playstation (One)/Playstation 2 (CD only)"/*"1994/(2000) Sony http://www.playstation.com"*/,
      NULL
    },
    {
      "psx", 0, 0, UCON64_PSX,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_PSX_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
};


int
psx_init (st_rominfo_t *rominfo)
{
  int result = -1;

  rominfo->console_usage = psx_usage[0].help;
//  rominfo->copier_usage = cdrw_usage[0].help;

  return result;
}
