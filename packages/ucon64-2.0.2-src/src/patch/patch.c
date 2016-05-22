/*
patch.c - patch support for uCON64

Copyright (c) 2006 NoisyB


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
#include "ucon64.h"
#include "ucon64_misc.h"
#include "patch/patch.h"


static st_ucon64_obj_t patch_obj[] =
  {
    {0, WF_SWITCH},
    {0, WF_INIT | WF_PROBE}
  };

const st_getopt2_t patch_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Patching",
      NULL
    },
    {
      "poke", 1, 0, UCON64_POKE,
      "OFF:V", "change byte at file offset OFF to value V (both in hexadecimal)",
      NULL
    },
    {
      "pattern", 1, 0, UCON64_PATTERN,
      "FILE", "change ROM based on patterns specified in FILE",
      &patch_obj[1]
    },
    {
      "patch", 1, 0, UCON64_PATCH,
      "PATCH", "specify the PATCH for the following options\n"
      "use this option or uCON64 expects the last commandline\n"
      "argument to be the name of the PATCH file",
      &patch_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };
