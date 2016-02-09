/*
ngp.c - NeoGeo Pocket support for uCON64

Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2001        Gulliver


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
#include "console/ngp.h"
#include "backup/backup.h"
#include "backup/pl.h"


static st_ucon64_obj_t ngp_obj[] =
  {
    {UCON64_NGP, WF_SWITCH}
  };

const st_getopt2_t ngp_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Neo Geo Pocket/Neo Geo Pocket Color"/*"1998/1999 SNK http://www.neogeo.co.jp"*/,
      NULL
    },
    {
      UCON64_NGP_S, 0, 0, UCON64_NGP,
      NULL, "force recognition",
      &ngp_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


typedef struct st_ngp_header
{
  char pad[48];
} st_ngp_header_t;

#define NGP_HEADER_START 0
#define NGP_HEADER_LEN (sizeof (st_ngp_header_t))


st_ngp_header_t ngp_header;


int
ngp_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1;
  char *snk_code = "COPYRIGHT BY SNK CORPORATION",
       *third_code = " LICENSED BY SNK CORPORATION", buf[MAXBUFSIZE];

  rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ? ucon64.backup_header_len : 0;

  ucon64_fread (&ngp_header, NGP_HEADER_START + rominfo->backup_header_len,
    NGP_HEADER_LEN, ucon64.fname);

  if (!strncmp ((const char *) &OFFSET (ngp_header, 0), snk_code, strlen (snk_code)) ||
      !strncmp ((const char *) &OFFSET (ngp_header, 0), third_code, strlen (third_code)))
    result = 0;
  else
    result = -1;
  if (ucon64.console == UCON64_NGP)
    result = 0;

  rominfo->header_start = NGP_HEADER_START;
  rominfo->header_len = NGP_HEADER_LEN;
  rominfo->header = &ngp_header;

  // internal ROM name
  strncpy (rominfo->name, (const char *) &OFFSET (ngp_header, 0x24), 12);
  rominfo->name[12] = 0;

  // ROM maker
  rominfo->maker =
    !strncmp ((const char *) &OFFSET (ngp_header, 0), snk_code, strlen (snk_code)) ?
      "SNK" : "Third party";

  // misc stuff
  sprintf (buf, "Mode: %s",
           (OFFSET (ngp_header, 0x23) == 0x00) ? "Mono" :
           (OFFSET (ngp_header, 0x23) == 0x10) ? "Color" :
           "Unknown");
  strcat (rominfo->misc, buf);

  rominfo->console_usage = ngp_usage[0].help;
  rominfo->backup_usage = !rominfo->backup_header_len ? pl_usage[0].help : unknown_backup_usage[0].help;

  return result;
}
