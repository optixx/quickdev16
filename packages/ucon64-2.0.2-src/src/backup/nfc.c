/*
nfc.h - support for Neo Flash Card for NDS/SP/GBA

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
#include <stdio.h>
#include "backup/nfc.h"


const st_getopt2_t nfc_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Neo Flash Card"
      /* "2005 NEO Flash Team http://www.neoflash.com" */,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };
