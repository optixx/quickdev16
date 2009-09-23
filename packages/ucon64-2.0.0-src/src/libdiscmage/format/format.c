/*
format.c - support of different image formats for libdiscmage

Copyright (c) 2004 NoisyB (noisyb@gmx.net)


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
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#ifdef  DEBUG
#ifdef  __GNUC__
#warning DEBUG active
#else
#pragma message ("DEBUG active")
#endif
#endif
#include <sys/stat.h>
#include "../misc.h"
#include "../libdiscmage.h"
#include "../libdm_misc.h"
#include "format.h"
#include "cdi.h"
#include "cue.h"
#include "nero.h"
#include "other.h"
#include "toc.h"
#include "ccd.h"
#ifdef  DJGPP                                   // DXE's are specific to DJGPP
#include "../dxedll_priv.h"
#endif

/*
  callibrate()        a brute force function that tries to find a iso header
                      or anything else that could identify a file as an
                      image (can be very slow)
*/
#if 0
static FILE *
callibrate (const char *s, int len, FILE *fh)
// brute force callibration
{
  int32_t pos = ftell (fh);
  char buf[MAXBUFSIZE];
//   malloc ((len + 1) * sizeof (char));
  int size = 0;
  int tries = 0; //TODO: make this an arg

  fseek (fh, 0, SEEK_END);
  size = ftell (fh);
  fseek (fh, pos, SEEK_SET);

  for (; pos < size - len && tries < 32768; pos++, tries++)
    {
      fseek (fh, pos, SEEK_SET);
      fread (&buf, len, 1, fh);
#ifdef  DEBUG
  mem_hexdump (buf, len, ftell (fh) - len);
  mem_hexdump (s, len, ftell (fh) - len);
#endif
      if (!memcmp (s, buf, len))
        {
          fseek (fh, -len, SEEK_CUR);
          return fh;
        }
    }

  return NULL;
}
#endif


int
dm_track_init (dm_track_t *track, FILE *fh)
{
  int pos = 0, x = 0, identified = 0;
  const char sync_data[] = {0, (const char) 0xff, (const char) 0xff,
                               (const char) 0xff, (const char) 0xff,
                               (const char) 0xff, (const char) 0xff,
                               (const char) 0xff, (const char) 0xff,
                               (const char) 0xff, (const char) 0xff, 0};
  char value_s[32];
  uint8_t value8 = 0;

  fseek (fh, track->track_start, SEEK_SET);
#if 1
  fread (value_s, 1, 16, fh);
#else
// callibrate
  fseek (fh, -15, SEEK_CUR);
  for (x = 0; x < 64; x++)
    {
      if (fread (&value_s, 1, 16, fh) != 16)
        return -1;
      fseek (fh, -16, SEEK_CUR);
      if (!memcmp (sync_data, value_s, 12))
        break;
      fseek (fh, 1, SEEK_CUR);
    }
#endif

  if (!memcmp (sync_data, value_s, 12))
    {
      value8 = (uint8_t) value_s[15];

      for (x = 0; track_probe[x].sector_size; x++)
        if (track_probe[x].mode == value8)
          {
            // search for valid PVD in sector 16 of source image
            pos = (track_probe[x].sector_size * 16) +
                  track_probe[x].seek_header + track->track_start;
            fseek (fh, pos, SEEK_SET);
            fread (value_s, 1, 16, fh);
            if (!memcmp (pvd_magic, &value_s, 8) ||
                !memcmp (svd_magic, &value_s, 8) ||
                !memcmp (vdt_magic, &value_s, 8))
              {
                identified = 1;
                break;
              }
          }
    }

  // no sync_data found? probably MODE1/2048
  if (!identified)
    {
      x = 0;
      if (track_probe[x].sector_size != 2048)
        fprintf (stderr, "ERROR: dm_track_init()\n");

      fseek (fh, (track_probe[x].sector_size * 16) +
             track_probe[x].seek_header + track->track_start, SEEK_SET);
      fread (value_s, 1, 16, fh);

      if (!memcmp (pvd_magic, &value_s, 8) ||
          !memcmp (svd_magic, &value_s, 8) ||
          !memcmp (vdt_magic, &value_s, 8))
        identified = 1;
    }

  if (!identified)
    {
      fprintf (stderr, "ERROR: could not find iso header of current track\n");
      return -1;
    }

  track->sector_size = track_probe[x].sector_size;
  track->mode = track_probe[x].mode;
  track->seek_header = track_probe[x].seek_header;
  track->seek_ecc = track_probe[x].seek_ecc;
  track->iso_header_start = (track_probe[x].sector_size * 16) + track_probe[x].seek_header;
  track->id = dm_get_track_mode_id (track->mode, track->sector_size);

  return 0;
}


dm_image_t *
dm_reopen (const char *fname, uint32_t flags, dm_image_t *image)
// recurses through all <image_type>_init functions to find correct image type
{
  typedef struct
    {
      int type;
      int (*init) (dm_image_t *);
      int (*track_init) (dm_track_t *, FILE *);
    } st_probe_t;

  static st_probe_t probe[] =
    {
      {DM_CDI, cdi_init, cdi_track_init},
      {DM_NRG, nrg_init, nrg_track_init},
//      {DM_CCD, ccd_init, ccd_track_init},
      {DM_CUE, cue_init, dm_track_init},
      {DM_TOC, toc_init, dm_track_init},
      {DM_OTHER, other_init, dm_track_init},
      {0, NULL, NULL}
    };
  int x, identified = 0;
//  static dm_image_t image2;
  FILE *fh = NULL;

#ifdef  DEBUG
  printf ("sizeof (dm_track_t) == %d\n", sizeof (dm_track_t));
  printf ("sizeof (dm_image_t) == %d\n", sizeof (dm_image_t));
  fflush (stdout);
#endif

  if (image)
    dm_close (image);

  if (access (fname, F_OK) != 0)
    return NULL;

  if (!image)
#if 1
    image = (dm_image_t *) malloc (sizeof (dm_image_t));
#else
    image = (dm_image_t *) &image2;
#endif

  memset (image, 0, sizeof (dm_image_t));
  if (!image)
    return NULL;

  image->desc = ""; // deprecated

  for (x = 0; probe[x].type; x++)
    if (probe[x].init)
      {
        dm_clean (image);
        image->flags = flags;
        strcpy (image->fname, fname);

        if (!probe[x].init (image))
          {
            identified = 1;
            break;
          }
      }

  if (!identified) // unknown image
    return NULL;

  image->type = probe[x].type;

  if (!(fh = fopen (image->fname, "rb")))
    return image;

  // verify header or sheet informations
  for (x = 0; x < image->tracks; x++)
    {
      dm_track_t *track = (dm_track_t *) &image->track[x];

      if (track->mode != 0) // AUDIO/2352 has no iso header
        track->iso_header_start = track->track_start + (track->sector_size * (16 + track->pregap_len)) + track->seek_header;

#ifdef  DEBUG
      printf ("iso header offset: %d\n\n", (int) track->iso_header_start);
      fflush (stdout);
#endif

      track->id = dm_get_track_mode_id (track->mode, track->sector_size);
    }

  fclose (fh);

  return image;
}


dm_image_t *
dm_open (const char *fname, uint32_t flags)
{
  return dm_reopen (fname, flags, NULL);
}


int
dm_close (dm_image_t *image)
{
#if 1
  free (image);
#else
  memset (image, 0, sizeof (dm_image_t));
#endif
  image = NULL;
  return 0;
}


int
dm_read (char *buffer, int track_num, int sector, const dm_image_t *image)
{
  dm_track_t *track = (dm_track_t *) &image->track[track_num];
  FILE *fh;
  
  if (!(fh = fopen (image->fname, "rb")))
    return 0;

  if (fseek (fh, track->track_start + (track->sector_size * sector), SEEK_SET) != 0)
    {
      fclose (fh);
      return 0;
    }
  
  if (fread (buffer, track->sector_size, 1, fh) != track->sector_size)
    {
      fclose (fh);
      return 0;
    }

  fclose (fh);
  return track->sector_size;
}


int
dm_write (const char *buffer, int track_num, int sector, const dm_image_t *image)
{
  (void) buffer;
  (void) track_num;
  (void) sector;
  (void) image;
  return 0;
}


int
dm_disc_read (const dm_image_t *image)
{
  (void) image;
  fprintf (stderr, dm_msg[DEPRECATED], "dm_disc_read()");
  fflush (stderr);
  return 0;
}


int
dm_disc_write (const dm_image_t *image)
{
  (void) image;
  fprintf (stderr, dm_msg[DEPRECATED], "dm_disc_write()");
  fflush (stderr);
  return 0;
}


int
dm_seek (dm_image_t *image, int track_num, int sector)
{
  (void) image;                                 // warning remover
  (void) track_num;                             // warning remover
  (void) sector;                                // warning remover
  return 0;
}
