/*
libdm_misc.h - libdiscmage miscellaneous

Copyright (c) 2002 - 2003 NoisyB (noisyb@gmx.net)
Copyright (c) 2002 - 2004 dbjh


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
#ifndef LIBDM_MISC_H
#define LIBDM_MISC_H
/*
  libdm messages

  usage example: fprintf (stdout, dm_msg[DEPRECATED], filename);
*/
enum
{
  DEPRECATED = 0,
  UNKNOWN_IMAGE,
  ALREADY_2048,
  ALPHA
};
extern const char *dm_msg[];


typedef struct
{
  int mode;
  int seek_header; // sync, head, sub
  int sector_size; // data
  int seek_ecc;    // EDC, zero, ECC, spare

  int id;

  const char *suffix;
} st_track_probe_t;
extern const st_track_probe_t track_probe[];


typedef struct
{
  int id;
  const char *desc;
} st_track_desc_t; // used in toc.c and cue.c


enum {
  DM_AUDIO = 1,
#if 0
  DM_SIZERAW,
  DM_SIZEISO_MODE1,
  DM_SIZEISO_MODE2_RAW,
  DM_SIZEISO_MODE2_FORM1,
  DM_SIZEISO_MODE2_FORM2,
  DM_MODE1,
  DM_MODE2,
#endif
  DM_MODE1_2352,
  DM_MODE2_2352,
  DM_MODE1_2048,
  DM_MODE2_2336
};


enum {
  DM_UNKNOWN = -1,
  DM_CUE = 1,
  DM_TOC,
  DM_CDI,
  DM_NRG,
//  DM_CCD,
  DM_OTHER
};


/*
  dm_get_track_desc() returns a string like "MODE1/2352" depending on the 
                      mode and sector_size specified; if cue == FALSE
                      it will return the string in TOC format
*/
extern int dm_get_track_mode_id (int mode, int sector_size);
extern void dm_get_track_mode_by_id (int id, int8_t *mode, uint16_t *sector_size);
extern void dm_clean (dm_image_t *image);
extern void dm_gauge (int, int);

extern const char pvd_magic[];
extern const char svd_magic[];
extern const char vdt_magic[];


/*
  dm_lba_to_msf() convert LBA to minutes, seconds, frames
  dm_msf_to_lba() convert minutes, seconds, frames to LBA

  LBA represents the logical block address for the CD-ROM absolute
  address field or for the offset from the beginning of the current track
  expressed as a number of logical blocks in a CD-ROM track relative
  address field.
  MSF represents the physical address written on CD-ROM discs,
  expressed as a sector count relative to either the beginning of the
  medium or the beginning of the current track.

  dm_bcd_to_int() convert BCD to integer
  dm_int_to_bcd() convert integer to BCD
*/
extern int dm_lba_to_msf (int lba, int *m, int *s, int *f);
extern int dm_msf_to_lba (int m, int s, int f, int force_positive);
extern int dm_bcd_to_int (int b);
extern int dm_int_to_bcd (int i);
#endif  // LIBDM_MISC_H
