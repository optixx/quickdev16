/*
cue.c - CUE support for libdiscmage

Copyright (c) 2002 - 2003 NoisyB (noisyb@gmx.net)


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
#include "../misc.h"
#include "../libdiscmage.h"
#include "../libdm_misc.h"
#include "format.h"
#ifdef  DJGPP
#include "../dxedll_priv.h"
#endif


const st_track_desc_t cue_desc[] = 
  {
    {DM_MODE1_2048, "MODE1/2048"}, // CD-ROM Mode1 Data (cooked)
    {DM_MODE1_2352, "MODE1/2352"}, // CD-ROM Mode1 Data (raw)
    {DM_MODE2_2336, "MODE2/2336"}, // CD-ROM XA Mode2 Data
    {DM_MODE2_2352, "MODE2/2352"}, // CD-ROM XA Mode2 Data
    {DM_AUDIO, "AUDIO"},           // Audio/Music (2352)
#if 0
    {DM_UNKNOWN, "CDG"},           // Karaoke CD+G (2448)
    {DM_UNKNOWN, "CDI/2336"},      // CD-I Mode2 Data
    {DM_UNKNOWN, "CDI/2352"},      // CD-I Mode2 Data
#endif
    {0, NULL}
  };


static const char *
cue_get_desc (int id)
{
  int x = 0;
  
  for (x = 0; cue_desc[x].desc; x++)
    if (id == cue_desc[x].id)
      return cue_desc[x].desc;
  return "";
}


dm_image_t *
dm_cue_read (dm_image_t *image, const char *cue_file)
{
  char buf[MAXBUFSIZE];
#if 0
  , *p = NULL;
  char inum[3];
  char min;
  char sec;
  char fps;
  int already_set = 0;
#endif
  int t = 0, x = 0;
  FILE *fh = NULL;

  if (!(fh = fopen (cue_file, "rb")))
    return NULL; // cue_file not found

  for (; fgets (buf, MAXBUFSIZE, fh); t++)
    {
      if (strstr (buf, " TRACK "))
        {
          dm_track_t *track = (dm_track_t *) &image->track[t];

          track->sector_size = track->mode = 0;
            
          for (x = 0; cue_desc[x].desc; x++)
            if (stristr (buf, cue_desc[x].desc))
              {
                dm_get_track_mode_by_id (cue_desc[x].id, &track->mode, &track->sector_size);
                break;
              }       

          if (!track->sector_size)
            {
              fclose (fh);
              return !t ? NULL : image;
            }
        }
#if 0
      else if (strstr (buf, " INDEX "))
        {
          /* check stuff here so if the answer is false the else stuff below won't be executed */
          strncpy (inum, &buf[10], 2);
          inum[2] = '\0';
          if ((already_set == 0) &&
              ((strcmp (inum, "00") == 0) || (strcmp (inum, "01") == 0)))
            {
              already_set = 1;

              min = ((buf[13] - '0') << 4) | (buf[14] - '0');
              sec = ((buf[16] - '0') << 4) | (buf[17] - '0');
              fps = ((buf[19] - '0') << 4) | (buf[20] - '0');

              track->minute = (((min >> 4) * 10) + (min & 0xf));
              track->second = (((sec >> 4) * 10) + (sec & 0xf));
              track->frame = (((fps >> 4) * 10) + (fps & 0xf));
            }
        }
      else if (strstr (buf, "PREGAP "))
        {
        }
      else if (strstr (buf, "POSTGAP "))
        {
        }
#endif
    }

  fclose (fh);

  return image;
}


int
dm_cue_write (const dm_image_t *image)
{
  int result = (-1), t = 0;

  for (t = 0; t < image->tracks; t++)
    {
#if     FILENAME_MAX > MAXBUFSIZE
      char buf[FILENAME_MAX];
#else
      char buf[MAXBUFSIZE];
#endif
//      char buf2[MAXBUFSIZE];
      dm_track_t *track = (dm_track_t *) &image->track[t];
      int m = 0, s = 0, f = 0;
      FILE *fh = NULL;

      strcpy (buf, image->fname);
#if 0
      sprintf (buf2, "_%d.CUE", t);
      set_suffix (buf, buf2);
#else
      set_suffix (buf, ".CUE");
#endif

      if (!(fh = fopen (buf, "wb")))
        {
          result = -1;
          continue;
        }
      else
        result = 0;

      switch (track->mode)
        {
          case 0: // audio
            // TODO: WAVE, AIFF, ...
            fprintf (fh, "FILE \"%s\" WAVE\r\n", image->fname);
            break;

          case 1: // iso
            fprintf (fh, "FILE \"%s\" BINARY\r\n", image->fname);
            break;

          default: // bin
            fprintf (fh, "FILE \"%s\" BINARY\r\n", image->fname);
            break;
        }

      fprintf (fh, "  TRACK %02d %s\r\n",
               t + 1,
               cue_get_desc (track->id));

      /*
        You can use the PREGAP command to specify the length of a track
        pre-gap. CDRWIN internally generates the pre-gap data. No data is
        consumed from the current data file.
      */
      if (track->pregap_len > 0)
        {
          dm_lba_to_msf (track->pregap_len, &m, &s, &f);
          fprintf (fh, "    PREGAP %02d:%02d:%02d\r\n", m, s, f);
        }

      /*
        You can use the INDEX command to specify indexes (or
        subindexes) within a track.
      */
      fprintf (fh, "    INDEX 01 00:00:00\r\n"); // default


      /*
        You can use the POSTGAP command to specify the length of a
        track post-gap. CDRWIN internally generates the post-gap data.
        No data is consumed from the current data file.
      */
      if (track->postgap_len > 0)
        {
          dm_lba_to_msf (track->postgap_len, &m, &s, &f);
          fprintf (fh, "    POSTGAP %02d:%02d:%02d\r\n", m, s, f);
        }

      fclose (fh);
    }

  return result;
}


int
cue_init (dm_image_t *image)
{
  int t = 0;
  FILE *fh = NULL;
#if 0
  char buf[FILENAME_MAX];
  
  strcpy (buf, image->fname);
  set_suffix (buf, ".CUE");
  if (dm_cue_read (image, buf)) // read and parse cue into dm_image_t
    {
      image->desc = "ISO/BIN track (with CUE file)";
      return 0;
    }
#endif

#if 1
  image->sessions =
  image->tracks =
  image->session[0] = 1;
#endif

  // missing or invalid cue? try the image itself
  if (!(fh = fopen (image->fname, "rb")))
    return -1;

  for (t = 0; t < image->tracks; t++)
    {
      dm_track_t *track = (dm_track_t *) &image->track[t];
      
      if (!dm_track_init (track, fh))
        {
          track->track_len =
          track->total_len = q_fsize (image->fname) / track->sector_size;
        }
      else
        {
          fclose (fh);
          return !t ? (-1) : 0;
        }
    }

  dm_cue_write (image); // write the missing cue

  image->desc = "ISO/BIN track (missing CUE file created)";

  fclose (fh);
  return 0;
}
