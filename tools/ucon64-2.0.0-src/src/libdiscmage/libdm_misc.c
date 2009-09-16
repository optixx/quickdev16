/*
libdm_misc.c - libdiscmage miscellaneous

Copyright (c) 2002 - 2004 NoisyB (noisyb@gmx.net)


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
#include "config.h"
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
#include "misc.h"
#include "libdiscmage.h"
#include "libdm_misc.h"
#include "format/format.h"
#ifdef  DJGPP                                   // DXE's are specific to DJGPP
#include "dxedll_priv.h"
#endif
#include "misc_wav.h"

#ifndef CD_MINS
#define CD_MINS              74 /* max. minutes per CD, not really a limit */
#define CD_SECS              60 /* seconds per minute */
#define CD_FRAMES            75 /* frames per second */
#endif

#define MBIT (131072)
#define TOMBIT(x) ((int)(x) / MBIT)
#define TOMBIT_F(x) ((float)(x) / MBIT)


const char pvd_magic[] = {0x01, 'C', 'D', '0', '0', '1', 0x01, 0};
const char svd_magic[] = {0x02, 'C', 'D', '0', '0', '1', 0x01, 0};
const char vdt_magic[] = {(const char) 0xff, 'C', 'D', '0', '0', '1', 0x01, 0};


#define ISODCL(from, to) (to - from + 1)
typedef struct
  {
    char type[ISODCL (1, 1)];   /* 711 */
    char id[ISODCL (2, 6)];
    char version[ISODCL (7, 7)];        /* 711 */
    char unused1[ISODCL (8, 8)];
    char system_id[ISODCL (9, 40)];     /* achars */
    char volume_id[ISODCL (41, 72)];    /* dchars */
    char unused2[ISODCL (73, 80)];
    char volume_space_size[ISODCL (81, 88)];    /* 733 */
    char unused3[ISODCL (89, 120)];
    char volume_set_size[ISODCL (121, 124)];    /* 723 */
    char volume_sequence_number[ISODCL (125, 128)];     /* 723 */
    char logical_block_size[ISODCL (129, 132)]; /* 723 */
    char path_table_size[ISODCL (133, 140)];    /* 733 */
    char type_l_path_table[ISODCL (141, 144)];  /* 731 */
    char opt_type_l_path_table[ISODCL (145, 148)];      /* 731 */
    char type_m_path_table[ISODCL (149, 152)];  /* 732 */
    char opt_type_m_path_table[ISODCL (153, 156)];      /* 732 */
    char root_directory_record[ISODCL (157, 190)];      /* 9.1 */
    char volume_set_id[ISODCL (191, 318)];      /* dchars */
    char publisher_id[ISODCL (319, 446)];       /* achars */
    char preparer_id[ISODCL (447, 574)];        /* achars */
    char application_id[ISODCL (575, 702)];     /* achars */
    char copyright_file_id[ISODCL (703, 739)];  /* 7.5 dchars */
    char abstract_file_id[ISODCL (740, 776)];   /* 7.5 dchars */
    char bibliographic_file_id[ISODCL (777, 813)];      /* 7.5 dchars */
    char creation_date[ISODCL (814, 830)];      /* 8.4.26.1 */
    char modification_date[ISODCL (831, 847)];  /* 8.4.26.1 */
    char expiration_date[ISODCL (848, 864)];    /* 8.4.26.1 */
    char effective_date[ISODCL (865, 881)];     /* 8.4.26.1 */
    char file_structure_version[ISODCL (882, 882)];     /* 711 */
    char unused4[ISODCL (883, 883)];
    char application_data[ISODCL (884, 1395)];
    char unused5[ISODCL (1396, 2048)];
  } st_iso_header_t;


/*
 * A CD-ROM physical sector size is 2048, 2052, 2056, 2324, 2332, 2336,
 * 2340, or 2352 bytes long.  

*         Sector types of the standard CD-ROM data formats:
 *
 * format   sector type               user data size (bytes)
 * -----------------------------------------------------------------------------
 *   1     (Red Book)    CD-DA          2352    (CD_FRAMESIZE_RAW)
 *   2     (Yellow Book) Mode1 Form1    2048    (CD_FRAMESIZE)
 *   3     (Yellow Book) Mode1 Form2    2336    (CD_FRAMESIZE_RAW0)
 *   4     (Green Book)  Mode2 Form1    2048    (CD_FRAMESIZE)
 *   5     (Green Book)  Mode2 Form2    2328    (2324+4 spare bytes)
 *
 *
 *       The layout of the standard CD-ROM data formats:
 * -----------------------------------------------------------------------------
 * - audio (red):                  | audio_sample_bytes |
 *                                 |        2352        |
 *
 * - data (yellow, mode1):         | sync - head - data - EDC - zero - ECC |
 *                                 |  12  -   4  - 2048 -  4  -   8  - 276 |
 *
 * - data (yellow, mode2):         | sync - head - data |
 *                                 |  12  -   4  - 2336 |
 *
 * - XA data (green, mode2 form1): | sync - head - sub - data - EDC - ECC |
 *                                 |  12  -   4  -  8  - 2048 -  4  - 276 |
 *
 * - XA data (green, mode2 form2): | sync - head - sub - data - Spare |
 *                                 |  12  -   4  -  8  - 2324 -  4    |
 *
 */

/*
  from CDmage
  
  m1_2048=iso
  m1_2352=tao
  m2_2336=mm2
  m2_2352=tao
  cf_2048=fcd
  a1_2352=pcm
  a2_2352=wav
  cg_2448=cdg
  ci_2336=mci
  ci_2352=cdi
  cv_2048=vcd

CDRWin cuesheet file (*.cue)|*.cue
M1/2048 track (*.iso)|*.iso
M1/2352 track (*.bin;*.tao;*.iso;*.img;*.bwi)|*.bin;*.tao;*.iso;*.img;*.bwi
M2/2336 track (*.mm2)|*.mm2
M2/2352 track (*.bin;*.tao;*.iso;*.img;*.bwi)|*.bin;*.tao;*.iso;*.img;*.bwi
VirtualCD uncompressed container file (*.fcd)|*.fcd
Audio track image file (*.pcm;*.bin;*.img;*.bwi)|*.pcm;*.bin;*img;*.bwi
Wave file 44.1KHz 16-bit stereo (*.wav)|*.wav
Virtual Drive uncomp. container file (*.vcd)|*.vcd
Nero Burning Rom image file (*.nrg)|*.nrg
CloneCD image control file (*.ccd)|*.ccd
Raw Image (*.iso;*.bin;*.tao;*.img;*.bwi;*.mm2)|*.iso;*.bin;*.tao;*.img;*.bwi;*.mm2
BlindWrite TOC file (*.bwt)|*.bwt
DiscJuggler CD image file (*.cdi)|*.cdi
Gear image files (*.rdb;*.md1;*.xa)|*.rdb;*.md1;*.xa
NTI CD image files (*.cdp;*.ncd)|*.cdp;*.ncd
Prassi Global-CD image file (*.gcd)|*.gcd
WinOnCD full CD image file (*.c2d)|*.c2d
Easy CD Creator image file (*.cif)|*.cif
*/

const st_track_probe_t track_probe[] =
  {
    {1, 0,  2048, 0,   DM_MODE1_2048, ".iso"}, // MODE2_FORM1
    {1, 16, 2352, 288, DM_MODE1_2352, ".bin.tao.iso.img.bwi"},
    {2, 8,  2336, 280, DM_MODE2_2336, ".mm2"}, // MODE2_FORM_MIX
    {2, 24, 2352, 280, DM_MODE2_2352, ".bin.tao.iso.img.bwi"},
#if 0
    {2, 24, 2324, 4, DM_UNKNOWN, NULL},   // MODE2/2328, MODE2_FORM2
    {2, 0,  2340, 0, DM_UNKNOWN, NULL},
    {2, 0,  2368, 0, DM_UNKNOWN, NULL},
    {2, 0,  2448, 0, DM_UNKNOWN, NULL},
    {2, 0,  2646, 0, DM_UNKNOWN, NULL},
    {2, 0,  2647, 0, DM_UNKNOWN, NULL},
    {2, 0,  2336, 280, DM_UNKNOWN, NULL},  // MODE2/2336, Macintosh
    {2, 16, 2352, 280, DM_UNKNOWN, NULL}, // MODE2/2352, Macintosh
    {2, 0,  2056, 0, DM_UNKNOWN, NULL},  // MODE2/2056, Macintosh
    {0, 0,  0,    0, DM_UNKNOWN, NULL},
    {0, 0,  0,    0, DM_UNKNOWN, NULL},
#endif
    {0, 0,  2352, 0, DM_AUDIO, ".pcm.bin.img.bwi"},
    // TODO: remove this!
//    {0, 0, 1, 0, DM_AUDIO, ".pcm.bin.img.bwi"},
    {0, 0, 0, 0, 0, NULL}
  };

const char *dm_msg[] = {
  "ERROR: %s has been deprecated\n",
  "ERROR: Unknown/unsupported track mode\n",
  "ERROR: The images track mode is already MODE1/2048\n",
  "WARNING: This function is still in ALPHA stage. It might not work properly\n",
  NULL
};

static void (*dm_gauge_ptr) (int pos, int size) = NULL;


int
dm_get_track_mode_id (int mode, int sector_size)
{
  int x = 0;

  for (x = 0; track_probe[x].sector_size; x++)
    if (track_probe[x].mode == mode &&
        track_probe[x].sector_size == sector_size)
      return track_probe[x].id;

  return 0; // no id
}


void
dm_get_track_mode_by_id (int id, int8_t *mode, uint16_t *sector_size)
{
  int x = 0;

  for (x = 0; track_probe[x].sector_size; x++)
    if (track_probe[x].id == id)
      {
        *mode = track_probe[x].mode;
        *sector_size = track_probe[x].sector_size;
        return;
      }
}



int
dm_bcd_to_int (int b)
{
//  return (b & 0x0F) + 10 * (((b) >> 4) & 0x0F);
  return (b & 15) + ((b & 240) >> 4) * 10;
}


int
dm_int_to_bcd (int i)
{
#if 0
#if 1
  return i % 10 | ((i / 10) % 10) << 4;
#else
  return ((i / 10) * 16) + (i % 10);
#endif
#else
  return ((i / 10) << 4) | (i % 10);
#endif
}


int
dm_is_bcd (int b)
{
  return ((b & 15) < 10) && ((b >> 4) < 10);
}


int
dm_lba_to_msf (int lba, int *m0, int *s0, int *f0)
#if 1
{
  static int m, s, f;

  m = s = f = -1;

#ifdef  __follow_redbook__
  if (lba >= -150 && lba < 405000) /* lba <= 404849 */
#else
  if (lba >= -150)
#endif
    {
      m = (lba + 150) / CD_SECS / CD_FRAMES;
      s = (lba + 150 - m * CD_SECS * CD_FRAMES) / CD_FRAMES;
      f = (lba + 150 - m * CD_SECS * CD_FRAMES - s * CD_FRAMES);

    }
  else if (lba >= -45150 && lba <= -151)
    {
      m = (lba + 450150) / CD_SECS / CD_FRAMES;
      s = (lba + 450150 - m * CD_SECS * CD_FRAMES) / CD_FRAMES;
      f = (lba + 450150 - m * CD_SECS * CD_FRAMES - s * CD_FRAMES);
    }

  *m0 = m;
  *s0 = s;
  *f0 = f;

  if (lba > 404849 || *m0 == -1 || *s0 == -1 || *f0 == -1) /* 404850 -> 404999: lead out */
    return FALSE;
  return TRUE;
}
#else
{
  static int m, s, f;

  m = s = f = -1;

  if (lba >= -150)
    {
      m = (lba + 150) / (CD_SECS * CD_FRAMES);
      lba -= m * CD_SECS * CD_FRAMES;
      s = (lba + 150) / CD_FRAMES;
      lba -= s * CD_FRAMES;
      f = (lba + 150);
    }
  else
    {
      m = (lba + 450150) / (CD_SECS * CD_FRAMES);
      lba -= m * CD_SECS * CD_FRAMES;
      s = (lba + 450150) / CD_FRAMES;
      lba -= s * CD_FRAMES;
      f = (lba + 450150);
    }

  *m0 = m;
  *s0 = s;
  *f0 = f;

  return TRUE;
}
#endif


int
dm_msf_to_lba (int m, int s, int f, int force_positive)
#if 1
{
  long ret = (m * CD_SECS + s)
#if 0
    * CD_FRAMES + f;
#else
    ;
  ret *= CD_FRAMES;
  ret += f;
#endif

  if (m < 90 || force_positive)
    ret -= 150;
  else
    ret -= 450150;

  return ret;
}
#else
{
  return m * CD_SECS * CD_FRAMES + s * CD_FRAMES + f;
}
#endif


void
dm_clean (dm_image_t *image)
{
  int x = 0;

  memset (image, 0, sizeof (dm_image_t));
  for (x = 0; x < DM_MAX_TRACKS; x++)
    image->track[x].iso_header_start = (-1);
}


uint32_t
dm_get_version (void)
{
  static const uint32_t dm_version = LIB_VERSION (DM_VERSION_MAJOR,
                                                  DM_VERSION_MINOR,
                                                  DM_VERSION_STEP);
  return dm_version;
}


const char *
dm_get_version_s (void)
{
  return "supported: BIN, ISO, CDI";
}


void
dm_set_gauge (void (*gauge) (int, int))
{
  dm_gauge_ptr = gauge;
}


void
dm_gauge (int pos, int size)
{
  if (dm_gauge_ptr)
    dm_gauge_ptr (pos, size);
}


FILE *
dm_fdopen (dm_image_t *image, int track_num, const char *mode)
{
  dm_track_t *track = (dm_track_t *) &image->track[track_num];
  FILE *fh;

  if (!(fh = fopen (image->fname, mode)))
    return NULL;

  if (!fseek (fh, track->track_start, SEEK_SET))
    return fh;

  fclose (fh);
  return NULL;
}




int
dm_rip (const dm_image_t *image, int track_num, uint32_t flags)
{
  dm_track_t *track = (dm_track_t * ) &image->track[track_num];
#if     FILENAME_MAX > MAXBUFSIZE
  char buf[FILENAME_MAX], buf2[FILENAME_MAX];
#else
  char buf[MAXBUFSIZE], buf2[MAXBUFSIZE];
#endif
  unsigned int x = 0;
  int result = 0, m = 0, s = 0, f = 0;
  char *p = NULL;
  FILE *fh = NULL, *fh2 = NULL;

  if (flags & DM_FIX || flags & DM_2048)
    fprintf (stderr, dm_msg[ALPHA]);

// set dest. name
  strcpy (buf, basename (image->fname));
  p = (char *) get_suffix (buf);
  if (p)
    buf[strlen (buf) - strlen (p)] = 0;
  sprintf (buf2, "%s_%d", buf, track_num + 1);

  switch (track->mode)
    {
      case 0:
        if (flags & DM_WAV)
          set_suffix (buf2, ".WAV");
        else
          set_suffix (buf2, ".RAW");
        break;

      case 1:
      case 2:
      default:
        if (flags & DM_2048 || track->sector_size == 2048)
          set_suffix (buf2, ".ISO");
        else
          set_suffix (buf2, ".BIN");
        break;
    }

#if 0
  if (track->total_len < track->track_len + track->pregap_len)
    {
      fprintf (stderr, "SKIPPING: track seems truncated\n");
      return -1;
    }
#endif

#if 0
// this is not a problem
  if (flags & DM_2048 && track->mode == 1 && track->sector_size == 2048)
    {
      fprintf (stderr, dm_msg[ALREADY_2048]);
      return -1;
    }
#endif

  if (track->pregap_len != 150) // 0x96
    fprintf (stderr, "WARNING: track seems to have a non-standard pregap (%d Bytes)\n",
             track->pregap_len);
 
#if 0
  if (image->pregap && track->mode == 0 && remaining_tracks > 1) // quick hack to save next track pregap (audio tracks only)
    track->track_len += pregap_length;       // if this isn't last track in current session
// does this mean i shouldn't skip pregrap if it's a audio track?
#endif

// open source and check
  if (!(fh = fopen (image->fname, "rb")))
    return -1;

// open dest.
  if (!(fh2 = fopen (buf2, "wb")))
    {
      fclose (fh);
      return -1;
    }

  if (!track->mode && flags & DM_WAV)
    misc_wav_write_header_v3 (fh2, track->track_len * 2352); //TODO: get that 2352 value from somewhere

  fseek (fh, track->track_start, SEEK_SET); // start of track
  // skip pregap (always?)
  fseek (fh, track->pregap_len * track->sector_size, SEEK_CUR);

  for (x = 0; x < track->track_len; x++)
    {
      memset (buf, 0, sizeof (buf));
      fread (&buf, 1, track->sector_size, fh);

      if (flags & DM_2048)
        result = fwrite (&buf[track->seek_header], 1, 2048, fh2);
      else
        {
          const char sync_data[] = {0, (const char) 0xff, (const char) 0xff,
                                       (const char) 0xff, (const char) 0xff,
                                       (const char) 0xff, (const char) 0xff,
                                       (const char) 0xff, (const char) 0xff,
                                       (const char) 0xff, (const char) 0xff, 0};
//          uint32_t value_32 = 0;

          memset (&buf2, 0, sizeof (buf2));
          result = 0;
          result += fwrite (&sync_data, 1, 12, fh2);

          dm_lba_to_msf (150 + x, &m, &s, &f);
#ifdef  DEBUG
          fprintf (stderr, "%d: %d %d %d\n", track->sector_size, m, s, f);
          fflush (stderr);
#endif

          result += fwrite (&buf2, 1, 3, fh2); //TODO: MSF
          if (fputc (track->mode, fh2))
            result++;
          result += fwrite (&buf2, 1, track->seek_header, fh2); // padding
          result += fwrite (&buf, 1, track->sector_size, fh2);
          result += fwrite (&buf2, 1, track->seek_ecc, fh2); // padding
        }

      if (!result)
        {
          fprintf (stderr, "ERROR: writing sector %d\n", x);
          fclose (fh);
          fclose (fh2);
          return -1;
        }

      if (!(x % 100))
        dm_gauge (x * track->sector_size, track->track_len * track->sector_size);
    }
                  
  dm_gauge (x * track->sector_size, track->track_len * track->sector_size);
                        
//  fseek (fh, track->total_len * track->sector_size, SEEK_CUR);

  fclose (fh);
  fclose (fh2);

  return 0;
}


#if 0
// TODO: merge into dm_rip
int
dm_isofix (const dm_image_t * image, int start_lba, int track_num)
{
#define BOOTFILE_S "bootfile.bin"
#define HEADERFILE_S "header.iso"
  int32_t size_left, last_pos, i, size;
  dm_track_t *track = (dm_track_t *) &image->track[track_num];
  char buf[MAXBUFSIZE], buf2[FILENAME_MAX];
  FILE *dest = NULL, *src = NULL, *boot = NULL, *header = NULL;
  int mac = FALSE;
  const char sub_header[] = {0, 0, 0x08, 0, 0, 0, 0x08, 0};
  
  if (start_lba <= 0)
    {
      fprintf (stderr, "ERROR: Bad LBA value");
      return -1;
    }

  if (!track)
    return -1;
  
  mac = (track->sector_size == 2056 ? TRUE : FALSE);

  if (!(src = fopen (image->fname, "rb")))
    return -1;

  strcpy (buf2, basename (image->fname));
  set_suffix (buf2, ".FIX");
  if (!(dest = fopen (buf2, "wb")))
    {
      fclose (src);
      return -1;
    }

  // Saving boot area to file 'bootfile.bin'...
  if (!(boot = fopen (BOOTFILE_S, "wb")))
    {
      fclose (src);

      fclose (dest);
      remove (buf);

      return -1;
    }

  // Saving ISO header to file 'header.iso'...
  if (!(header = fopen (HEADERFILE_S, "wb")))
    {
      fclose (src);

      fclose (dest);
      remove (buf);

      fclose (boot);
      remove (BOOTFILE_S);

      return -1;
    }

  // save boot area
  for (i = 0; i < 16; i++)
    {
      fseek (src, track->seek_header, SEEK_CUR);
      fread (buf, 2048, 1, src);
      fseek (src, track->seek_ecc, SEEK_CUR);

      if (mac)
        fwrite (sub_header, 8, 1, dest);
      fwrite (buf, 2048, 1, dest);

      if (mac)
        fwrite (sub_header, 8, 1, boot);
      fwrite (buf, 2048, 1, boot);

      if (mac)
        fwrite (sub_header, 8, 1, header);
      fwrite (buf, 2048, 1, header);
    }
  fclose (boot); // boot area written

  // seek & copy pvd etc.
//  last_pos = ftell (src);   // start of pvd
  for (last_pos = ftell (src); memcmp (vdt_magic, buf, 8) != 0; last_pos = ftell (src))
    {
      fseek (src, track->seek_header, SEEK_CUR);
      fread (buf, 2048, 1, src);
      fseek (src, track->seek_ecc, SEEK_CUR);

      if (memcmp (pvd_magic, buf, 8) != 0 &&
          memcmp (svd_magic, buf, 8) != 0 &&
          memcmp (vdt_magic, buf, 8) != 0)
        {
          fprintf (stderr, "ERROR: Found unknown Volume Descriptor");
// which sector?
          return -1;
        }

      printf ("Found %s at sector %d\n",
        !memcmp (pvd_magic, buf, 8) ? "PVD" :
        !memcmp (svd_magic, buf, 8) ? "SVD" :
        !memcmp (vdt_magic, buf, 8) ? "VDT" : "unknown Volume Descriptor",
        last_pos / track->sector_size);

      if (mac)
        fwrite (sub_header, 8, 1, dest);
      fwrite (buf, 2048, 1, dest);

      if (mac)
        fwrite (sub_header, 8, 1, header);
      fwrite (buf, 2048, 1, header);
    }

  // add padding data to header file
  memset (&buf, 0, sizeof (buf));
  size_left = 300 - (last_pos / track->sector_size);
  for (i = 0; i < size_left; i++)
    {
      if (mac == TRUE)
        fwrite (sub_header, 8, 1, header);
      fwrite (buf, 2048, 1, header);
    }
  fclose (header);

  // add padding data to iso image
  if (last_pos > (int) (start_lba * track->sector_size))
    {
      fprintf (stderr, "ERROR: LBA value is too small\n"
               "       It should be at least %d for current ISO image (probably greater)",
               last_pos / track->sector_size);
      return -1;
    }

  if (start_lba < 11700)
    fprintf (stderr,
             "WARNING: LBA value should be greater or equal to 11700 for multisession\n"
             "         images\n");

  // adding padding data up to start LBA value...
  size_left = start_lba - (last_pos / track->sector_size);
  memset (&buf, 0, sizeof (buf));
  for (i = 0; i < size_left; i++)
    {
      if (mac)
        fwrite (sub_header, 8, 1, dest);
      if (!fwrite (buf, 2048, 1, dest))
        return -1;
    }

// append original iso image
  fseek (src, 0L, SEEK_SET);
  size = q_fsize (buf2);
  size_left = size / track->sector_size;
  for (i = 0; i < size_left; i++)
    {
      fseek (src, track->seek_header, SEEK_CUR);
      if (!fread (buf, 2048, 1, src))
        break;
      fseek (src, track->seek_ecc, SEEK_CUR);

      if (mac)
        fwrite (sub_header, 8, 1, dest);
      if (!fwrite (buf, 2048, 1, dest))
        return -1;
    }

  fclose (src);
  fclose (dest);

  return 0;
}
#endif


void
dm_nfo (const dm_image_t *image, int verbose, int ansi_color)
{
#define COLS 80
  int t = 0, s = 0, x = 0, min = 0, sec = 0, frames = 0,
      filesize = q_fsize (image->fname);
  char buf[MAXBUFSIZE];
  st_iso_header_t iso_header;
  FILE *fh = NULL;
  int result = -1;

#ifndef USE_ANSI_COLOR
  (void) ansi_color;                            // warning remover
#endif

#if 0
  if (image->console_usage != NULL)
    {
      strcpy (buf, rominfo->console_usage[0].desc);
      puts (to_func (buf, strlen (buf), toprint2));

#if 0
      if (image->console_usage[1].desc)
        {
          strcpy (buf, rominfo->console_usage[1].desc);
          printf ("  %s\n", to_func (buf, strlen (buf), toprint2));
        }
#endif
    }
#endif

  printf ("%d Bytes (%.4f MB)\n\n", filesize, (float) filesize / (1024 * 1024));
  printf ("Type: %s\n", image->desc);

  if (image->misc[0])
    puts (image->misc);

  printf ("Sessions: %d\n", image->sessions);
  printf ("Tracks: %d\n", image->tracks);

  if ((COLS / image->tracks) > 1 && image->sessions && image->tracks)
    {
      printf ("Layout: ");

      for (s = t = 0; s < image->sessions; s++)
        {
#ifdef  USE_ANSI_COLOR
          if (ansi_color)
            printf ("\x1b[0m[\x1b[30;41m%2d \x1b[0m", s + 1);
//            printf ("\x1b[0m[\x1b[30;41m");
          else
#endif
            printf ("[%2d ", s + 1);
//            printf ("[");

          for (x = 0; x < image->session[s]; x++, t++)
#ifdef  USE_ANSI_COLOR
            if (ansi_color)
              printf ("\x1b[0m[\x1b[30;42m%2d \x1b[0m]", t + 1);
            else
#endif
              printf ("[%2d ]", t + 1);

#ifdef  USE_ANSI_COLOR
          if (ansi_color)
            printf ("\x1b[0m] ");
          else
#endif
          printf ("] "); 
        }

      fputc ('\n', stdout);
    }

  for (t = 0; t < image->tracks; t++)
    {
      const dm_track_t *track = (const dm_track_t *) &image->track[t];

      if (!track)
        continue;

      if (!track->mode && track->sector_size == 2352)
        strcpy (buf, "AUDIO");
      else
        sprintf (buf, "MODE%d/%d", track->mode, track->sector_size);

      printf ("Track: %d %s", t + 1, buf);

      dm_lba_to_msf (track->track_len, &min, &sec, &frames);
      printf ("\n  %d Sectors, %d:%02d/%02d MSF, %d Bytes (%.4f MB)",
        (int) track->total_len,
        min, sec, frames,
        (int) (track->total_len * track->sector_size),
        (float) (track->total_len * track->sector_size) / (1024 * 1024));

      fputc ('\n', stdout);

      if (verbose)
        {
          printf ("  Pregap: %d, Start Sector: %d, End Sector: %d, Postgap: %d\n",
            track->pregap_len,
            (int) (track->track_start / track->sector_size),
            (int) ((track->track_start / track->sector_size) + track->total_len),
            track->postgap_len);

          dm_lba_to_msf (track->track_len, &min, &sec, &frames);
          printf ("  Total Time: %d:%02d/%02d MSF, File Start Pos: %d, End Pos: %d\n",
            min, sec, frames,
            (int) track->track_start,
            (int) track->track_end);
        }

      memset (&iso_header, 0, sizeof (st_iso_header_t));
      result = -1;

      if (track->iso_header_start != -1)
        if ((fh = fopen (image->fname, "rb")))
          if (fread (&iso_header, track->iso_header_start, sizeof (st_iso_header_t), fh))
            {
              if (verbose)
                mem_hexdump (&iso_header, sizeof (st_iso_header_t), track->iso_header_start);

              // name, maker, country and size
              // some have a name with control chars in it -> replace control chars
              strncpy2 (buf, NULL_TO_EMPTY (iso_header.volume_id),
                sizeof (iso_header.volume_id));
              to_func (buf, strlen (buf), toprint2);
              if (*strtrim (buf))
                printf ("  %s\n", buf);

              strncpy2 (buf, NULL_TO_EMPTY (iso_header.publisher_id),
                sizeof (iso_header.publisher_id));
              to_func (buf, strlen (buf), toprint2);
              if (*strtrim (buf))
                printf ("  %s\n", buf);

              strncpy2 (buf, NULL_TO_EMPTY (iso_header.preparer_id),
                sizeof (iso_header.preparer_id));
              to_func (buf, strlen (buf), toprint2);
              if (*strtrim (buf))
                printf ("  %s\n", buf);
#if 0
              strncpy2 (buf, NULL_TO_EMPTY (iso_header.logical_block_size),
                sizeof (iso_header.logical_block_size));
              to_func (buf, strlen (buf), toprint2);
              if (*strtrim (buf))
                printf ("  %s\n", buf);
#endif
              strncpy2 (buf, NULL_TO_EMPTY (iso_header.application_id),
                sizeof (iso_header.application_id));
              to_func (buf, strlen (buf), toprint2);
              if (*strtrim (buf))
                printf ("  %s\n", buf);
            }
    }
}
