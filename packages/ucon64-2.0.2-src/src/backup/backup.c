/*
backup.c - backup support for uCON64 

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
#include <stdio.h>
#include "backup/backup.h"


const st_getopt2_t unknown_backup_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Unknown backup unit/emulator",
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };
