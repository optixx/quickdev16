/*
lynx.c - Atari Lynx support for uCON64

Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2002 - 2005 dbjh


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
#include <stdlib.h>
#include <string.h>
#include "misc/archive.h"
#include "misc/bswap.h"
#include "misc/file.h"
#include "ucon64_misc.h"
#include "console/lynx.h"
#include "backup/backup.h"


static st_ucon64_obj_t lynx_obj[] =
  {
    {0, WF_DEFAULT},
    {UCON64_LYNX, WF_SWITCH},
    {UCON64_LYNX, WF_DEFAULT}
  };

const st_getopt2_t lynx_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Handy (prototype)/Lynx/Lynx II"/*"1987 Epyx/1989 Atari/1991 Atari"*/,
      NULL
    },
    {
      UCON64_LYNX_S, 0, 0, UCON64_LYNX,
      NULL, "force recognition",
      &lynx_obj[1]
    },
    {
      "lyx", 0, 0, UCON64_LYX,
      NULL, "convert to LYX/RAW (strip 64 Bytes LNX header)",
      &lynx_obj[2]
    },
    {
      "lnx", 0, 0, UCON64_LNX,
      NULL, "convert to LNX (uses default values for the header);\n"
      "adjust the LNX header with the following options",
      &lynx_obj[2]
    },
    {
      "n", 1, 0, UCON64_N,
      "NEW_NAME", "change internal ROM name to NEW_NAME (LNX only)",
      &lynx_obj[0]
    },
    {
      "nrot", 0, 0, UCON64_NROT,
      NULL, "set no rotation (LNX only)",
      &lynx_obj[2]
    },
    {
      "rotl", 0, 0, UCON64_ROTL,
      NULL, "set rotation left (LNX only)",
      &lynx_obj[2]
    },
    {
      "rotr", 0, 0, UCON64_ROTR,
      NULL, "set rotation right (LNX only)",
      &lynx_obj[2]
    },
    {
      "b0", 1, 0, UCON64_B0,
      "N", "change Bank0 kBytes size to N={0,64,128,256,512} (LNX only)",
      &lynx_obj[2]
    },
    {
      "b1", 1, 0, UCON64_B1,
      "N", "change Bank1 kBytes size to N={0,64,128,256,512} (LNX only)",
      &lynx_obj[2]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
};

const char *lynx_lyx_desc = "convert to LYX/RAW (strip 64 Bytes LNX header)";

//static const char *lnx_usage[] = "LNX header";
#define LNX_HEADER_START 0
#define LNX_HEADER_LEN (sizeof (st_lnx_header_t))

static st_lnx_header_t lnx_header;


int
lynx_lyx (st_ucon64_nfo_t *rominfo)
{
  char dest_name[FILENAME_MAX];

  if (!rominfo->backup_header_len)
    {
      fprintf (stderr, "ERROR: This is no LNX file\n");
      return -1;
    }

  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".lyx");

  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, rominfo->backup_header_len, ucon64.file_size, dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
lynx_lnx (st_ucon64_nfo_t *rominfo)
{
  st_lnx_header_t header;
  char dest_name[FILENAME_MAX];
  int size = ucon64.file_size;

  if (rominfo->backup_header_len != 0)
    {
      fprintf (stderr, "ERROR: This seems to already be an LNX file\n");
      return -1;
    }

  memset (&header, 0, sizeof (st_lnx_header_t));

  header.page_size_bank0 = (short) (size > 4 * MBIT ? 4 * MBIT / 256 : size / 256);
  header.page_size_bank1 = (short) (size > 4 * MBIT ? (size - (4 * MBIT)) / 256 : 0);
#ifdef  WORDS_BIGENDIAN
  header.page_size_bank0 = bswap_16 (header.page_size_bank0);
  header.page_size_bank1 = bswap_16 (header.page_size_bank1);
#endif

#ifdef  WORDS_BIGENDIAN
  header.version = bswap_16 (1);
#else
  header.version = 1;
#endif

  memcpy (header.magic, "LYNX", 4);
  header.rotation = 0;
  strncpy (header.cartname, basename2 (ucon64.fname), sizeof (header.cartname))[sizeof (header.cartname) - 1] = 0;
  strcpy (header.manufname, "Atari");

  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".lnx");

  ucon64_file_handler (dest_name, NULL, 0);
  ucon64_fwrite (&header, 0, sizeof (st_lnx_header_t), dest_name, "wb");
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


static int
lynx_rot (st_ucon64_nfo_t *rominfo, unsigned char rotation)
{
  st_lnx_header_t header;
  char dest_name[FILENAME_MAX];

  if (!rominfo->backup_header_len)
    {
      fprintf (stderr, "ERROR: This is no LNX file\n");
      return -1;
    }

  ucon64_fread (&header, 0, sizeof (st_lnx_header_t), ucon64.fname);

  header.rotation = rotation;

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");
  ucon64_fwrite (&header, 0, sizeof (st_lnx_header_t), dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
lynx_nrot (st_ucon64_nfo_t *rominfo)
{
  return lynx_rot (rominfo, 0);                 // no rotation
}


int
lynx_rotl (st_ucon64_nfo_t *rominfo)
{
  return lynx_rot (rominfo, 1);                 // rotate left
}


int
lynx_rotr (st_ucon64_nfo_t *rominfo)
{
  return lynx_rot (rominfo, 2);                 // rotate right
}


int
lynx_n (st_ucon64_nfo_t *rominfo, const char *name)
{
  st_lnx_header_t header;
  char dest_name[FILENAME_MAX];

  if (!rominfo->backup_header_len)
    {
      fprintf (stderr, "ERROR: This is no LNX file\n");
      return -1;
    }

  ucon64_fread (&header, 0, sizeof (st_lnx_header_t), ucon64.fname);

  memset (header.cartname, 0, sizeof (header.cartname));
  strncpy (header.cartname, name, sizeof (header.cartname));

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");
  ucon64_fwrite (&header, 0, sizeof (st_lnx_header_t), dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


static int
lynx_b (st_ucon64_nfo_t *rominfo, int bank, const char *value)
{
  st_lnx_header_t header;
  short int *bankvar;
  char dest_name[FILENAME_MAX];

  if (!rominfo->backup_header_len)
    {
      fprintf (stderr, "ERROR: This is no LNX file\n");
      return -1;
    }

  ucon64_fread (&header, 0, sizeof (st_lnx_header_t), ucon64.fname);

  bankvar = (bank == 0 ? &header.page_size_bank0 : &header.page_size_bank1);
  if ((atol (value) % 64) != 0 || (atol (value) > 512))
    *bankvar = 0;
  else
#ifdef  WORDS_BIGENDIAN
    *bankvar = bswap_16 (atol (value) * 4);
#else
    *bankvar = (short int) atol (value) * 4;
#endif

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");
  ucon64_fwrite (&header, 0, sizeof (st_lnx_header_t), dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
lynx_b0 (st_ucon64_nfo_t *rominfo, const char *value)
{
  return lynx_b (rominfo, 0, value);
}


int
lynx_b1 (st_ucon64_nfo_t *rominfo, const char *value)
{
  return lynx_b (rominfo, 1, value);
}


int
lynx_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1;

  rominfo->console_usage = lynx_usage[0].help;
  rominfo->backup_usage = unknown_backup_usage[0].help;

  ucon64_fread (&lnx_header, 0, LNX_HEADER_LEN, ucon64.fname);
  if (!strncmp (lnx_header.magic, "LYNX", 4))
    result = 0;
  else
    result = -1;
  if (ucon64.console == UCON64_LYNX)
    result = 0;

  if (!strncmp (lnx_header.magic, "LYNX", 4))
    {
      rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ?
        ucon64.backup_header_len : (int) LNX_HEADER_LEN;

      if (UCON64_ISSET (ucon64.backup_header_len) && !ucon64.backup_header_len)
        return ucon64.console == UCON64_LYNX ? 0 : result;

      ucon64_fread (&lnx_header, 0, LNX_HEADER_LEN, ucon64.fname);
      rominfo->backup_header = &lnx_header;

      // internal ROM name
      strcpy (rominfo->name, lnx_header.cartname);

      // ROM maker
      rominfo->maker = lnx_header.manufname;

      // misc stuff
      sprintf (rominfo->misc,
        "Internal Size: Bank0 %hd Bytes (%.4f Mb)\n"
        "               Bank1 %hd Bytes (%.4f Mb)\n"
        "Version: %hd\n"
        "Rotation: %s",
#ifdef  WORDS_BIGENDIAN
        bswap_16 (lnx_header.page_size_bank0) * 256,
        TOMBIT_F (bswap_16 (lnx_header.page_size_bank0) * 256),
        bswap_16 (lnx_header.page_size_bank1) * 256,
        TOMBIT_F (bswap_16 (lnx_header.page_size_bank1) * 256),
        bswap_16 (lnx_header.version),
#else
        lnx_header.page_size_bank0 * 256,
        TOMBIT_F (lnx_header.page_size_bank0 * 256),
        lnx_header.page_size_bank1 * 256,
        TOMBIT_F (lnx_header.page_size_bank1 * 256),
        lnx_header.version,
#endif
        (!lnx_header.rotation) ? "No" : ((lnx_header.rotation == 1) ? "Left" : "Right"));
    }

  return result;
}
