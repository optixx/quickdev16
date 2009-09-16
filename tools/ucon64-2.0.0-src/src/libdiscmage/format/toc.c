/*
toc.c - TOC support for libdiscmage

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


const st_track_desc_t toc_desc[] = 
  {
    {DM_MODE1_2048, "MODE1"}, // MODE2_FORM1
    {DM_MODE1_2352, "MODE1_RAW"},
    {DM_MODE2_2336, "MODE2"}, // MODE2_FORM_MIX
    {DM_MODE2_2352, "MODE2_RAW"},
    {DM_AUDIO, "AUDIO"},
    {0, NULL}
  };


static const char *
toc_get_desc (int id)
{
  int x = 0;
  
  for (x = 0; toc_desc[x].desc; x++)
    if (id == toc_desc[x].id)
      return toc_desc[x].desc;
  return "";
}


dm_image_t *
dm_toc_read (dm_image_t *image, const char *toc_file)
{
  (void) image;
  (void) toc_file;

  return NULL;
}


int
dm_toc_write (const dm_image_t *image)
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
//      int m = 0, s = 0, f = 0;
      FILE *fh = NULL;

      strcpy (buf, image->fname);
#if 0
      sprintf (buf2, "_%d.TOC", t);
      set_suffix (buf, buf2);
#else
      set_suffix (buf, ".TOC");
#endif

      if (!(fh = fopen (buf, "wb")))
        {
          result = -1;
          continue;
        }
      else result = 0;

      switch (track->mode)
        {
          case 0: // audio
            fprintf (fh, "AUDIO\n\n");
              break;

          case 1: // mode1
            fprintf (fh, "CD_ROM\n\n");
            break;
            
          default: // mode2
            fprintf (fh, "CD_ROM_XA\n\n");
            break;
        }

      fprintf (fh, "TRACK \"%s\"\n"
//                 "NO COPY\n"
                   "DATAFILE \"%s\" %u// length in bytes: %u\n",
                   toc_get_desc (track->id),
                   image->fname,
                   (unsigned int) (track->total_len * track->sector_size),
                   (unsigned int) (track->total_len * track->sector_size));

//TODO: pregap? postgap?

      fclose (fh);
    }
    
  return result;
}


int
toc_init (dm_image_t *image)
{
  int t = 0;
  FILE *fh = NULL;
  char buf[FILENAME_MAX];
  
  strcpy (buf, image->fname);
  set_suffix (buf, ".TOC");
  if ((dm_toc_read (image, buf))) // read and parse toc into dm_image_t
    {
      image->desc = "ISO/BIN track (with TOC file)";
      return 0;
    }

  // missing or invalid cue? try the image itself
  if (!(fh = fopen (image->fname, "rb")))
    return -1;
        
#if 1
  image->sessions =
  image->tracks =
  image->session[0] = 1;
#endif

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

  dm_toc_write (image); // write the missing cue
  
  image->desc = "ISO/BIN track (missing TOC file created)";
    
  fclose (fh);
  return 0;
}
