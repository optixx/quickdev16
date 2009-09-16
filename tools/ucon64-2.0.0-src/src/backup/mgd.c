/*
mgd.c - Multi Game Doctor/Hunter support for uCON64

Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2004 dbjh


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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "misc/misc.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "misc/string.h"
#include "mgd.h"


const st_getopt2_t mgd_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Multi Game Doctor (2)/Multi Game Hunter/MGH"
      /*"19XX Bung Enterprises Ltd http://www.bung.com.hk\n" "?Makko Toys Co., Ltd.?"*/,
      NULL
    },
#if 0
    {
      "xmgd", 0, 0, UCON64_XMGD,
      NULL, "(TODO) send/receive ROM to/from Multi Game* /MGD2/MGH; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when " OPTION_LONG_S "rom does not exist",
      NULL
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


// the following four functions are used by non-transfer code in genesis.c
void
mgd_interleave (unsigned char **buffer, int size)
{
  int n;
  unsigned char *src = *buffer;

  if (!(*buffer = (unsigned char *) malloc (size)))
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      exit (1);
    }
  for (n = 0; n < size / 2; n++)
    {
      (*buffer)[n] = src[n * 2 + 1];
      (*buffer)[size / 2 + n] = src[n * 2];
    }
  free (src);
}


void
mgd_deinterleave (unsigned char **buffer, int data_size, int buffer_size)
{
  int n = 0, offset;
  unsigned char *src = *buffer;

  if (!(*buffer = (unsigned char *) malloc (buffer_size)))
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], buffer_size);
      exit (1);
    }
  for (offset = 0; offset < data_size / 2; offset++)
    {
      (*buffer)[n++] = src[data_size / 2 + offset];
      (*buffer)[n++] = src[offset];
    }
  free (src);
}


int
fread_mgd (void *buffer, size_t size, size_t number, FILE *fh)
/*
  This function is used to handle a Genesis MGD file as if it wasn't
  interleaved, without the overhead of reading the entire file into memory.
  This is important for genesis_init(). When the file turns out to be a Genesis
  dump in MGD format it is much more efficient for compressed files to read the
  entire file into memory and then deinterleave it (as load_rom() does).
  In order to speed this function up a bit ucon64.file_size is used. That means
  it can't be used for an arbitrary file.
*/
{
  int n = 0, bpos = 0, fpos, fpos_org, block_size, bytesread = 0,
      len = number * size, fsize = ucon64.file_size /* fsizeof (filename) */;
  unsigned char tmp1[MAXBUFSIZE], tmp2[MAXBUFSIZE];

  fpos = fpos_org = ftell (fh);
  if (fpos >= fsize)
    return 0;

  if (len == 0)
    return 0;
  else if (len == 1)
    {
      if (fpos_org & 1)
        {
          fseek (fh, fpos / 2, SEEK_SET);
          *((unsigned char *) buffer) = fgetc (fh);
        }
      else
        {
          fseek (fh, fpos / 2 + fsize / 2, SEEK_SET);
          *((unsigned char *) buffer) = fgetc (fh);
        }
      fseek (fh, fpos_org + 1, SEEK_SET);
      return 1;
    }

  while (len > 0 && !feof (fh))
    {
      block_size = len > MAXBUFSIZE ? MAXBUFSIZE : len;

      fseek (fh, fpos / 2, SEEK_SET);
      bytesread += fread (tmp1, 1, block_size / 2, fh); // read odd bytes
      fseek (fh, (fpos + 1) / 2 + fsize / 2, SEEK_SET);
      bytesread += fread (tmp2, 1, block_size / 2, fh); // read even bytes

      if (fpos_org & 1)
        for (n = 0; n < block_size / 2; n++)
          {
            ((unsigned char *) buffer)[bpos + n * 2] = tmp1[n];
            ((unsigned char *) buffer)[bpos + n * 2 + 1] = tmp2[n];
          }
      else
        for (n = 0; n < block_size / 2; n++)
          {
            ((unsigned char *) buffer)[bpos + n * 2] = tmp2[n];
            ((unsigned char *) buffer)[bpos + n * 2 + 1] = tmp1[n];
          }
      fpos += block_size;
      bpos += block_size;
      len -= block_size;
    }
  fseek (fh, fpos_org + bytesread, SEEK_SET);
  return bytesread / size;
}


int
q_fread_mgd (void *buffer, size_t start, size_t len, const char *filename)
{
  int result;
  FILE *fh;

  if ((fh = fopen (filename, "rb")) == NULL)
    return -1;
  fseek (fh, start, SEEK_SET);
  result = (int) fread_mgd (buffer, 1, len, fh);
  fclose (fh);

  return result;
}


static void
remove_mgd_id (char *name, const char *id)
{
  char *p = name;
  while ((p = strstr (p, id)))
    {
      *p = 'X';
      *(p + 1) = 'X';
      p += 2;
    }
}


void
mgd_make_name (const char *filename, int console, int size, char *name)
// these characters are also valid in MGD file names: !@#$%^&_
{
  char *prefix = 0, *p, *size_str = 0, *suffix = 0;
  const char *fname;
  int n;

  switch (console)
    {
    default:                                    // falling through
    case UCON64_SNES:
      prefix = "SF";
      suffix = ".048";
      if (size <= 1 * MBIT)
        size_str = "1";
      else if (size <= 2 * MBIT)
        size_str = "2";
      else if (size <= 4 * MBIT)
        size_str = "4";
      else if (size <= 8 * MBIT)
        {
          size_str = "8";
          suffix = ".058";
        }
      else
        {
          suffix = ".078";
          if (size <= 10 * MBIT)
            size_str = "10";
          else if (size <= 12 * MBIT)
            size_str = "12";
          else if (size <= 16 * MBIT)
            size_str = "16";
          else if (size <= 20 * MBIT)
            size_str = "20";
          else if (size <= 24 * MBIT)
            size_str = "24";
          else // MGD supports SNES games with sizes up to 32 Mbit
            size_str = "32";
        }
      break;
    case UCON64_GEN:
      prefix = "MD";
      suffix = ".000";
      if (size <= 1 * MBIT)
        size_str = "1";
      else if (size <= 2 * MBIT)
        size_str = "2";
      else if (size <= 4 * MBIT)
        size_str = "4";
      else
        {
          if (size <= 8 * MBIT)
            {
              size_str = "8";
              suffix = ".008";
            }
          else if (size <= 16 * MBIT)
            {
              size_str = "16";
              suffix = ".018";
            }
          else
            {
              suffix = ".038";
              if (size <= 20 * MBIT)
                size_str = "20";
              else if (size <= 24 * MBIT)
                size_str = "24";
              else // MGD supports Genesis games with sizes up to 32 Mbit
                size_str = "32";
            }
        }
      break;
    case UCON64_PCE:
      prefix = "PC";
      suffix = ".040";
      if (size <= 1 * MBIT)
        size_str = "1";
      else if (size <= 2 * MBIT)
        size_str = "2";
      else if (size <= 3 * MBIT)
        {
          size_str = "3";
          suffix = ".030";
        }
      else if (size <= 4 * MBIT)
        {
          size_str = "4";
          suffix = ".048";
        }
      else
        {
          suffix = ".058";
          if (size <= 6 * MBIT)
            size_str = "6";
          else // MGD supports PC-Engine games with sizes up to 8 Mbit
            size_str = "8";
        }
      break;
    case UCON64_SMS:
      prefix = "GG";
      suffix = ".060";
      if (size < 1 * MBIT)
        size_str = "0";
      else if (size == 1 * MBIT)
        size_str = "1";
      else if (size <= 2 * MBIT)
        size_str = "2";
      else
        {
          suffix = ".078";
          if (size <= 3 * MBIT)
            size_str = "3";
          else if (size <= 4 * MBIT)
            size_str = "4";
          else if (size <= 6 * MBIT)
            size_str = "6";
          else // MGD supports Sega Master System games with sizes up to 8 Mbit
            size_str = "8";
        }
      break;
    case UCON64_GAMEGEAR:
      prefix = "GG";
      suffix = ".040";
      if (size < 1 * MBIT)
        size_str = "0";
      else if (size == 1 * MBIT)
        size_str = "1";
      else if (size <= 2 * MBIT)
        size_str = "2";
      else
        {
          suffix = ".048";
          if (size <= 3 * MBIT)
            size_str = "3";
          else if (size <= 4 * MBIT)
            size_str = "4";
          else
            {
              suffix = ".078";
              if (size <= 6 * MBIT)
                size_str = "6";
              else // MGD supports Game Gear games with sizes up to 8 Mbit
                size_str = "8";
            }
        }
      break;
    case UCON64_GB:
      prefix = "GB";
      /*
        What is the maximum game size the MGD2 supports for GB (color) games?
        At least one 64 Mbit game exists, Densha De Go! 2 (J) [C][!].
      */
      suffix = ".040";
      if (size < 1 * MBIT)
        size_str = "0";
      else if (size == 1 * MBIT)
        size_str = "1";
      else if (size <= 2 * MBIT)
        size_str = "2";
      else if (size <= 3 * MBIT)
        {
          size_str = "3";
          suffix = ".030";
        }
      else if (size <= 4 * MBIT)
        {
          size_str = "4";
          suffix = ".048";
        }
      else
        {
          suffix = ".058";
          if (size <= 6 * MBIT)
            size_str = "6";
          else
            size_str = "8";
        }
      break;
    }

  fname = basename2 (filename);
  // Do NOT mess with prefix (strupr()/strlwr()). See below (remove_mgd_id()).
  sprintf (name, "%s%s%s", prefix, size_str, fname);
  if (size >= 10 * MBIT)
    {
      if (!strnicmp (name, fname, 4))
        strcpy (name, fname);
    }
  else
    {
      if (!strnicmp (name, fname, 3))
        strcpy (name, fname);
    }
  if ((p = strchr (name, '.')))
    *p = 0;
  n = strlen (name);
  if (size >= 10 * MBIT)
    {
      if (n < 7)
        strcat (name, "XXX");                   // in case fname is 1 character long
      n = 7;
    }
  else
    {
      if (n < 6)
        strcat (name, "XX");
      n = 6;
    }
  name[n] = '0';                                // last character must be a number
  name[n + 1] = 0;
  for (n = 3; n < 8; n++)                       // we can skip the prefix
    if (name[n] == ' ')
      name[n] = 'X';

  /*
    the transfer program "pclink" contains a bug in that it looks at the
    entire file name for an ID string (it should look only at the first 2
    characters).
  */
  strupr (name);
  remove_mgd_id (name + 3, "SF");
  remove_mgd_id (name + 3, "MD");
  remove_mgd_id (name + 3, "PC");
  remove_mgd_id (name + 3, "GG");
  remove_mgd_id (name + 3, "GB");

  set_suffix (name, suffix);
}


void
mgd_write_index_file (void *ptr, int n_names)
{
  char buf[100 * 10], *p, name[16], dest_name[FILENAME_MAX];
  // one line in the index file takes 10 bytes at max (name (8) + "\r\n" (2)),
  //  so buf is large enough for 44 files of 1/4 Mbit (max for 1 diskette)

  if (n_names == 1)
    {
      strcpy (name, (char *) ptr);
      if ((p = strrchr (name, '.')))
        *p = 0;
      sprintf (buf, "%s\r\n", name);            // DOS text file format
    }
  else if (n_names > 1)
    {
      int n = 0, offset = 0;

      for (; n < n_names; n++)
        {
          strcpy (name, ((char **) ptr)[n]);
          if ((p = strrchr (name, '.')))
            *p = 0;
          sprintf (buf + offset, "%s\r\n", name);
          offset += strlen (name) + 2;          // + 2 for "\r\n"
        }
    }
  else // n_names <= 0
    return;

  strcpy (dest_name, "MULTI-GD");
  ucon64_file_handler (dest_name, NULL, OF_FORCE_BASENAME);
  ucon64_fwrite (buf, 0, strlen (buf), dest_name, "wb");
  printf (ucon64_msg[WROTE], dest_name);
}
