/*
nero.c - Nero image support for libdiscmage

Copyright (c) 2003 NoisyB (noisyb@gmx.net)
based on specs and code by Dext


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

// nero images are big-endian

// structs for chunks
typedef struct
{
  unsigned char type[4];
  int32_t tracks;        // ?
  int32_t track_type;
  int32_t pad;
} st_cues_t;


typedef struct
{
  unsigned char type[4];
  int32_t tracks0;       // ?
  int32_t tracks1;       // ?
  unsigned char pad[14];
  unsigned char unknown;      // ? 0x20
  unsigned char pad1;
  unsigned char unknown2;     // ? 0x01
  unsigned char tracks;
} st_daoi_t;


typedef struct
{
  int32_t current;       // global_current_track?
  int32_t current_msf;
  int32_t unknown;       // global_current_track |= 0x0100
  int32_t current_msf2;
} st_cues_track_t;


typedef struct
{
  unsigned char pad[10];
  int32_t sector_size;
  int32_t mode;
  int32_t offset;
  int32_t offset2;
  int32_t offset3;
} st_daoi_track_t;


typedef struct
{
  int32_t track_mode;
  int32_t current_msf;
} st_cues_tail_t;


typedef struct
{
  unsigned char type[4];
  int32_t unknown;       // 0x04 ?
  int32_t tracks;
} st_sinf_t;


typedef struct
{
  unsigned char type[4];
  int32_t tracks;
} st_etnf_t;


typedef struct
{
  int32_t offset;
  int32_t track_len;     // image->track_len * sector_size
  int32_t track_mode;
  int32_t start_lba;
  int32_t pad;
} st_etnf_track_t;


// header magic
#define NRG_CUES "CUES" // nero_pre5.5
#define NRG_CUEX "CUEX" // nero5.5
#define NRG_DAOI "DAOI" // nero_pre5.5
#define NRG_DAOX "DAOX" // nero5.5
#define NRG_SINF "SINF"
#define NRG_ETNF "ETNF"
#define NRG_ETN2 "ETN2"
#define NRG_END_ "END!"
#define NRG_NERO "NERO" // nero_pre5.5
#define NRG_NER5 "NER5" // nero5.5
#define NRG_CUES "CUES"
#define NRG_DAOI "DAOI"

static uint32_t header_start = 0;//, version = 0, position = 0;


static int
nrg_chunk_offset (const dm_image_t *image, const char *chunk_id)
{
  FILE *fh = NULL; 
  int pos = 0;
  char value_s[32];
  
  if (!(fh = fopen (image->fname, "rb")))
    return FALSE;
    
  fseek (fh, image->header_start, SEEK_SET);

// find
  while (fread (&value_s, 4, 1, fh))
    if (!memcmp (value_s, chunk_id, 4))
      {
        pos = ftell (fh);
        fclose (fh);
        return pos;
      }
  fclose (fh);
  
  return 0;
}


static int
nrg_chunk_size (const dm_image_t *image, const char *chunk_id)
// returns chunk length
{
  int pos = 0;
  FILE *fh = NULL; 
  int value_32 = 0;

  pos = nrg_chunk_offset (image, chunk_id);
  if (!pos)
   return 0;

  if (!(fh = fopen (image->fname, "rb")))
    return 0;
    
  fseek (fh, pos, SEEK_SET);

  fread (&value_32, 4, 1, fh);
  fclose (fh);

  return be2me_32 (value_32); // return chunk length
}


static int
nrg_ident_chunk (const char *chunk_id)
{
  static const char *chunk[] = 
    {
      NRG_CUEX,
      NRG_CUES,
      NRG_DAOX,
      NRG_DAOI,
      NRG_SINF,
      NRG_ETNF,
      NRG_ETN2,
      NRG_END_,
      NRG_NERO,
      NRG_NER5,
      NRG_CUES,
      NRG_DAOI,
      NULL
    };
  int x = 0;

  for (x = 0; chunk[x]; x++)
    if (!memcmp (chunk[x], chunk_id, 4))
      return TRUE;
  return FALSE;
}


int
nrg_track_init (dm_track_t *track, FILE *fh)
{
  uint8_t value8;
  uint32_t value32;

  fread (&value8, 1, 1, fh);
  if (value8 == 42) 
    track->mode = 2;
  else if (value8 == 01)
    track->mode = 0;
  else
    track->mode = 1;

  fread (&value8, 1, 1, fh); // track #
  fread (&value8, 1, 1, fh); // index?
  fread (&value8, 1, 1, fh); // pad byte

  fread (&value32, 4, 1, fh); // start_lba
  track->start_lba = be2me_32 (value32);
  
  return 0;
}


int
nrg_init (dm_image_t * image)
{
  typedef struct
  {
    char *version;
    char *version_s;
  } st_probe_t;

  static const st_probe_t probe[] = {
      {NRG_NERO, "Nero/NRG image (<=v5.0)"},
      {NRG_NER5, "Nero/NRG image (v5.5)"},
      {NULL, NULL}
    };
  int s = 0, t = 0, x = 0, size = q_fsize (image->fname);
  int result = 0;
  FILE *fh;
  char value_s[16];
//  uint16_t value_16;
  uint32_t value_32;
#ifdef  DEBUG
  unsigned char buf[MAXBUFSIZE];
#endif

  header_start = 0;
//   version =
//   position = 0;

  if (size < 12)
    return -1; // image file is too small

  if (!(fh = fopen (image->fname, "rb")))
    return -1;

  fseek (fh, -4, SEEK_END);
  fread (&value_32, 1, 4, fh);
  image->header_start = header_start = be2me_32 (value_32);
  if (image->header_start < 1)
    {
      fclose (fh);
      return -1; // bad image
    }

  image->desc = NULL;
  for (x = 0; probe[x].version; x++)
    if (nrg_chunk_offset (image, probe[x].version))
      {
        image->desc = probe[x].version_s;
        break;
      }

  if (!image->desc)
    {
      fclose (fh);
      return -1;
    }

  fseek (fh, image->header_start, SEEK_SET);
  fread (&value_s, 1, 4, fh);
  if (!nrg_ident_chunk (value_s))
    {
      fclose (fh);
      return -1;
    }

  image->header_len = size - image->header_start;

#ifdef  DEBUG
  fseek (fh, image->header_start, SEEK_SET);
  memset (&buf, 0, MAXBUFSIZE);
  result = fread (buf, 1, MAXBUFSIZE, fh);
  mem_hexdump (buf, result, image->header_start);
#endif

  fseek (fh, image->header_start, SEEK_SET);

  if (!(result = nrg_chunk_size (image, NRG_CUEX)))
    {
      fclose (fh);
      return -1;
    }

//  fread (&value_16, 2, 1, fh); // how many sessions?
  image->sessions = 1;
  image->tracks = (result / 16) - 1;

  for (s = 0; s < image->sessions; s++)
    for (t = 0; t < image->tracks; t++)
      { 
        if (!nrg_track_init (&image->track[t], fh))
          {
            image->session[s]++; // # of tracks for this session
          }
        else
          {
            fclose (fh);
            return -1;
          }
      }  

  fclose (fh);

  return 0;
}


#if 0
void
nrg_write_cues_hdr (char *fcues, int32_t *fcues_i, dm_image_t * image)
{
  int32_t value32;
  st_cues_t cues;

  memset (&cues, 0, sizeof (st_cues_t));

  strcpy (cues.type, CUES_S);

  cues.tracks = me2be_32 (((image->tracks + 1) * 16));

  value32 = (image->track_type != 0 ? 0x41000000 : 0x01000000);
  cues.track_type = me2be_32 (value32);

  memcpy (&(fcues[*fcues_i]), &cues, sizeof (st_cues_t));
  *fcues_i += sizeof (st_cues_t);
}


void
nrg_write_daoi (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image)
{
  st_daoi_t daoi;

  memset (&daoi, 0, sizeof (st_daoi_t));

  strcpy (daoi.type, DAOI_S);

  daoi.tracks0 = me2be_32 (22 + (30 * image->tracks));
  daoi.tracks1 = me2be_32 (22 + (30 * image->tracks));
  daoi.unknown = me2be_32 (0x20);
  daoi.unknown2 = me2be_32 (0x01);
  daoi.tracks = me2be_32 (image->tracks);

  memcpy (&(fdaoi[*fdaoi_i]), &daoi, sizeof (st_daoi_t));
  *fdaoi_i += sizeof (st_daoi_t);
}


void
nrg_write_cues_track (char *fcues, int32_t *fcues_i, dm_image_t * image)
{
  int32_t value32;
  int32_t current_msf;
  struct cdrom_msf msf;
  st_cues_track_t cues_track;

  value32 = (image->mode != 0 ? 0x41000000 : 0x01000000);
  value32 |= (to_bcd (image->global_current_track) << 16) & 0x00ff0000;
  cues_track.current = me2be_32 (value32);

  lba_to_msf (image->start_lba, &msf);
  current_msf = (to_bcd (msf.cdmsf_frame0) | to_bcd (msf.cdmsf_sec0) << 8 | to_bcd (msf.cdmsf_min0) << 16);
  cues_track.current_msf = me2be_32 (current_msf);

  value32 |= 0x0100;
  cues_track.unknown = me2be_32 (value32);

  lba_to_msf (image->start_lba + (image->track_length < 0 ? 0 : // pregap = 0
              image->pregap_length), &msf);
  current_msf = (to_bcd (msf.cdmsf_frame0) | to_bcd (msf.cdmsf_sec0) << 8 | to_bcd (msf.cdmsf_min0) << 16);

  cues_track.current_msf2 = me2be_32 (current_msf);

  memcpy (&(fcues[*fcues_i]), &cues_track, sizeof (st_cues_track_t));
  *fcues_i += sizeof (st_cues_track_t);
}


void
nrg_write_daoi_track (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image,
                      int32_t offset)
{
  int32_t value32;
  st_daoi_track_t daoi_track;

  memset (&daoi_track, 0, sizeof (st_daoi_track_t));

  daoi_track.sector_size = me2be_32 (image->sector_size);

  value32 = (!image->mode ? 0x07000001 : 0x03000001);
  daoi_track.mode = me2be_32 (value32);

  daoi_track.offset = me2be_32 (offset);

  value32 = offset + (image->track_length < 0 ? 0 : // pregap = 0
                        image->pregap_length * image->sector_size);
  daoi_track.offset2 = me2be_32 (value32);

  value32 = offset +
    ((image->pregap_length + image->track_length) * image->sector_size);
  daoi_track.offset3 = me2be_32 (value32);

  memcpy (&(fdaoi[*fdaoi_i]), &daoi_track, sizeof (st_daoi_track_t));
  *fdaoi_i += sizeof (st_daoi_track_t);
}


void
nrg_write_cues_tail (char *fcues, int32_t *fcues_i, dm_image_t * image)
{
  int32_t value32;
  int32_t current_msf;
  struct cdrom_msf msf;
  st_cues_tail_t cues_tail;

  memset (&cues_tail, 0, sizeof (st_cues_tail_t));

  value32 = (image->mode != 0 ? 0x41000000 : 0x01000000);
  value32 |= 0xAA0100;
  cues_tail.track_mode = me2be_32 (value32);

  lba_to_msf (image->start_lba + image->track_length + image->pregap_length, &msf);
  current_msf = (to_bcd (msf.cdmsf_frame0) | to_bcd (msf.cdmsf_sec0) << 8 | to_bcd (msf.cdmsf_min0) << 16);
  cues_tail.current_msf = me2be_32 (current_msf);

  memcpy (&(fcues[*fcues_i]), &cues_tail, sizeof (st_cues_tail_t));
  *fcues_i += sizeof (st_cues_tail_t);
}


void
nrg_write_sinf (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image)
{
  st_sinf_t sinf;

  memset (&sinf, 0, sizeof (st_sinf_t));

  strcpy (sinf.type, SINF_S);
  sinf.unknown = me2be_32 (0x04);
  sinf.tracks = me2be_32 (image->tracks);

  memcpy (&(fdaoi[*fdaoi_i]), &sinf, sizeof (st_sinf_t));
  *fdaoi_i += sizeof (st_sinf_t);
}


void
nrg_write_etnf (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image)
{
  st_etnf_t etnf;

  memset (&etnf, 0, sizeof (st_etnf_t));

  strcpy (etnf.type, ETNF_S);
  etnf.tracks = me2be_32 (image->tracks * 20);

  memcpy (&(fdaoi[*fdaoi_i]), &etnf, sizeof (st_etnf_t));
  *fdaoi_i += sizeof (st_etnf_t);
}


void
nrg_write_etnf_track (char *fdaoi, int32_t *fdaoi_i, dm_image_t * image,
                      int32_t offset)
{
  int32_t value32;
  int32_t sector_size = (image->do_convert ? 2336 : // conversion
                              image->sector_size);
  st_etnf_track_t etnf_track;

  memset (&etnf_track, 0, sizeof (st_etnf_track_t));

  etnf_track.offset = me2be_32 (offset);

  value32 = image->track_length - (image->do_cut ? 2 :   // equals savetrack
                           0);
  etnf_track.track_len = me2be_32 (value32 * sector_size);

  value32 = (image->mode != 0 ? 0x03 : 0x07);
  etnf_track.track_mode = me2be_32 (value32);

  etnf_track.start_lba = me2be_32 (image->start_lba);
  etnf_track.pad = 0;

  memcpy (&(fdaoi[*fdaoi_i]), &etnf_track, sizeof (st_etnf_track_t));
  *fdaoi_i += sizeof (st_etnf_track_t);
}
#endif
