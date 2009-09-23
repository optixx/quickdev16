/*
other.c - other/proprietary image support for libdiscmage

Copyright (c) 2003 NoisyB (noisyb@gmx.net)
Gamecube support is based on specs from GC-NFO.COM - [w] / GC-NFO


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifdef  HAVE_CONFIG_H
#include "../config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../misc.h"
#include "../libdiscmage.h"
#include "../libdm_misc.h"
#include "format.h"
#ifdef  DJGPP
#include "../dxedll_priv.h"
#endif

// header magic
#define GC_MAGIC   (0x424e5231)
#define GC_MAGIC_S ("BNR1")
//if (0 != (rc = read_raw_frame(cdrom, 16, buf))) {       /* Safe PSX signature is at 0x9200 */
#if 0
static unsigned char psx_sign[] = {
  0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x09, 0x00,
  0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0x00,
  0x50, 0x4C, 0x41, 0x59, 0x53, 0x54, 0x41, 0x54,
  0x49, 0x4F, 0x4E, 0x20, 0x20, 0x20, 0x20, 0x20
};
#endif
              
static uint32_t header_start = 0; //, version = 0, position = 0;

#if 0
//TODO: replace with dm_sheet_track_init()
int
other_track_init (dm_track_t *track, FILE *fh)
{
  (void) fh;
  return 0;
}
#endif


static dm_image_t *
dm_other_gc_init (dm_image_t *image)
{
  typedef struct
    {
      char serial[4];
      char maker[2]; // currently identical to the gameboy advance makers
      char pad[102];
      char desc[944];
      char pad2[3];
      char pad3;
      char pad4[32];
    } st_boot_bin_t;

  typedef struct
    {
      char magic[4]; // "BNR1"
      char pad[6172];
      char name[32];
      char company[32];
      char name_long[64];
      char company_long[64];
      char desc[128];
    } st_opening_bnr_t;
  
  st_boot_bin_t boot_bin;
  st_opening_bnr_t opening_bnr;
  dm_track_t *track = (dm_track_t *) &image->track[0];
  FILE *fh;

  image->sessions =
  image->tracks = 1;
  image->session[0] = 1;

  image->header_start = header_start;
  image->header_len = sizeof (st_opening_bnr_t);
  
  track->track_start = 0;
  track->track_len =
  track->total_len = q_fsize (image->fname);

  track->sector_size = 1;

  memset (&boot_bin, 0, sizeof (st_boot_bin_t));
  memset (&opening_bnr, 0, sizeof (st_opening_bnr_t));

  if (!(fh = fopen (image->fname, "rb")))
    return NULL;

  fread (&boot_bin, sizeof (st_boot_bin_t), 1, fh);
  fseek (fh, header_start, SEEK_SET);
  fread (&opening_bnr, sizeof (st_opening_bnr_t), 1, fh);

  fclose (fh);

#ifdef  DEBUG
  mem_hexdump (&boot_bin, sizeof (st_boot_bin_t), sizeof (st_boot_bin_t));
  mem_hexdump (&opening_bnr, sizeof (st_opening_bnr_t), sizeof (st_opening_bnr_t));

  printf ("%d\n%d\n\n", sizeof (st_boot_bin_t), sizeof (st_opening_bnr_t));
  fflush (stdout);
#endif

  opening_bnr.name[sizeof (opening_bnr.name) - 1] = 
  opening_bnr.company[sizeof (opening_bnr.company) - 1] = 
  opening_bnr.name_long[sizeof (opening_bnr.name_long) - 1] = 
  opening_bnr.company_long[sizeof (opening_bnr.company_long) - 1] = 
  opening_bnr.desc[sizeof (opening_bnr.desc) - 1] = 0;

  sprintf (image->misc, "Misc: %s\n      %s\n      %s\n      %s\n      %s",
           opening_bnr.name,
           opening_bnr.company,
           opening_bnr.name_long,
           opening_bnr.company_long,
           opening_bnr.desc);

  return image;
}
    

int
other_init (dm_image_t *image)
{
  typedef struct
  {
    uint32_t version;
    char *version_s;
    uint32_t start;
    uint32_t len;
    char *version_long;
    dm_image_t *(*func) (dm_image_t *);
  } st_probe_t;

  static const st_probe_t probe[] = {
      {GC_MAGIC, GC_MAGIC_S, 0, 0x20000, "proprietary GameCube image", dm_other_gc_init},
      {0, NULL, 0, 0, NULL, NULL}
    };
  int x = 0; //, s = 0, t = 0, size = q_fsize (image->fname);
//  FILE *fh;
//  uint16_t value_16;
//  uint32_t value_32;

//  if (!(fh = fopen (image->fname, "rb")))
//    return -1;

  for (x = 0; probe[x].version; x++)
    if ((header_start = q_fncmp (image->fname,
                                 probe[x].start,
                                 probe[x].len,
                                 probe[x].version_s, 
                                 strlen (probe[x].version_s),
                                 0)))
      {
         image->desc = probe[x].version_long;
         probe[x].func (image);
         break;
      }

//  fclose (fh);

  return 0;
}
