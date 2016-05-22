/*
coleco.c - Colecovision support for uCON64

Copyright (c) 2005 NoisyB

Header information was written by Jeff Frohwein


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
#include "console/coleco.h"
#include "backup/backup.h"


static st_ucon64_obj_t coleco_obj[] =
  {
    {UCON64_COLECO, WF_SWITCH}
  };

const st_getopt2_t coleco_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Coleco/ColecoVision"/*"1982"*/,
      NULL
    },
    {
      UCON64_COLECO_S, 0, 0, UCON64_COLECO,
      NULL, "force recognition",
      &coleco_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


/*
  The First thing the Coleco does, when you turn it on, is check
  memory location 8000H & 8001H. If there is a 55H & AAH located at
  these locations then the Coleco goes to the memory address pointed
  to by 800AH & 800BH, and begins immediate execution of cartridge.
  If there is a AAH & 55H(just the opposite) found at 8000H & 8001H,
  then the Coleco prints COLECOVISION on the screen. It then prints
  the name of the cartridge, found at location 8024H, on the screen,
  followed by who the cartridge presents, and the year of cartidge.
  (Each are seperated by a "/"). This is displayed on the screen for
  about 12 seconds. After the delay, the Coleco then goes to the
  memory location pointed to by 800AH & 800BH and begins execution of
  cartridge.


  8000  AA 55 2B 70 00 40 AB 70 D3 70 41 80 C3 32 89 C9  ..+..@....A..2..
  8000  TYPE |  -  |  -  |  -  |  -  |START|  RST 1 | R

  8010  00 00 C9 00 00 C9 00 00 C9 00 00 C9 00 00 ED 4D  ................
  8010  ST 2 |  RST 3 |  RST 4 |  RST 5 |  RST 6 |  RST

  8020  00 C3 4F 89 5A 41 58 58 4F 4E 1E 1F 2F 50 52 45  ..O.ZAXXON../PRE
  8020  7 |   NMI  | NAME / PRESENTED BY / YEAR

  8030  53 45 4E 54 53 20 53 45 47 41 27 53 2F 31 39 38  SENTS SEGA'S/198
  8040  32 CD 7C 1F 21 FF 8F 39 4D 44 21 00 70 11 01 70  2...!..9MD!.....
  8050  AF 77 ED B0 CD 41 C1 21 82 71 11 8E 71 CD C7 1F  .....A.!........
*/
typedef struct st_coleco_header
{
  uint16_t type;
  uint16_t pad;
  uint16_t pad2;
  uint16_t pad3;
  uint16_t pad4;
  uint16_t start;
  uint8_t rst1[3];
  uint8_t rst2[3];
  uint8_t rst3[3];
  uint8_t rst4[3];
  uint8_t rst5[3];
  uint8_t rst6[3];
  uint8_t rst7[3];
  uint8_t nmi[3];
  char name[44];
} st_coleco_header_t;


#define COLECO_HEADER_START 0
#define COLECO_HEADER_LEN (sizeof (st_coleco_header_t))


int
coleco_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1;
  static st_coleco_header_t coleco_header;

  coleco_header.type = 0;
  rominfo->console_usage = coleco_usage[0].help;
  rominfo->backup_usage = unknown_backup_usage[0].help;

  rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ?
    ucon64.backup_header_len : 0;

  ucon64_fread (&coleco_header, ucon64.backup_header_len, COLECO_HEADER_LEN, ucon64.fname);

  if (coleco_header.type == 0xaa55 ||  // Coleco
      coleco_header.type == 0x55aa)    // ColecoVision
    result = 0;
  else
    result = -1;
  if (ucon64.file_size > 1024 * 1024)
    result = -1;

  if (ucon64.console == UCON64_COLECO)
    result = 0;

  if (coleco_header.type == 0xaa55 || coleco_header.type == 0x55aa)
    {
      rominfo->header_start = COLECO_HEADER_START;
      rominfo->header_len = COLECO_HEADER_LEN;
      rominfo->header = &coleco_header;

      // internal ROM name
      strcpy (rominfo->name, coleco_header.name);

      // misc stuff
      sprintf (rominfo->misc,
        "Start address: %04x\n"
        "Type: %s",
        coleco_header.start,
        coleco_header.type == 0xaa55 ? "Coleco" : "ColecoVision");
    }

  return result;
}
