/*
archive.c - g(un)zip and unzip support

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
#include "config.h"                             // USE_ZLIB
#endif
#ifdef  USE_ZLIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <zlib.h>
#ifdef  HAVE_BYTESWAP_H
#include <byteswap.h>
#else
#include "bswap.h"
#endif
#include "archive.h"
#include "misc.h"
#include "map.h"
#include "unzip.h"
#if     defined DJGPP && defined DLL
#include "dxedll_priv.h"
#endif


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768


extern int errno;


int
fsizeof (const char *filename)
// If USE_ZLIB is defined this function is very slow. Please avoid to use
//  it much.
{
  FILE *file;
  unsigned char magic[4] = { 0 };

#undef  fopen
#undef  fread
#undef  fclose
  if ((file = fopen (filename, "rb")) == NULL)
    {
      errno = ENOENT;
      return -1;
    }
  fread (magic, 1, sizeof (magic), file);
  fclose (file);
#define fopen   fopen2
#define fclose  fclose2
#define fread   fread2

  if (magic[0] == 0x1f && magic[1] == 0x8b && magic[2] == 0x08)
    {                                   // ID1, ID2 and CM. gzip uses Compression Method 8
      int size = 0;

      if ((file = (FILE *) gzopen (filename, "rb")) == NULL)
        { // Shouldn't fail because we could open it with fopen()
          errno = ENOENT;
          return -1;
        }
#if 1
      // This is not much faster than the other method
      while (!gzeof (file))
        gzseek (file, 1024 * 1024, SEEK_CUR);
      size = gztell (file);
#else
      // Is there a more efficient way to determine the uncompressed size?
      while ((bytesread = gzread (file, buf, MAXBUFSIZE)) > 0)
        size += bytesread;
#endif
      gzclose (file);
      return size;
    }
  else if (magic[0] == 'P' && magic[1] == 'K' && magic[2] == 0x03 && magic[3] == 0x04)
    {
      unz_file_info info;

      if ((file = (FILE *) unzOpen (filename)) == NULL)
        {
          errno = ENOENT;
          return -1;
        }
      unzip_goto_file (file, unzip_current_file_nr);
      unzGetCurrentFileInfo (file, &info, NULL, 0, NULL, 0, NULL, 0);
      unzClose (file);

      return info.uncompressed_size;
    }
  else
    {
      struct stat fstate;

      if (!stat (filename, &fstate))
        return fstate.st_size;

      errno = ENOENT;
      return -1;
    }
}


/*
  The zlib functions gzwrite() and gzputc() write compressed data if the file
  was opened with a "w" in the mode string, but we want to control whether we
  write compressed data. Note that for the mode string "w+", zlib ignores the
  '+'. zlib does the same for "r+".
  Currently we never write compressed data. That is, no code fopen()s files
  with a mode string containing 'f', 'h' or a number, but writing compressed
  output does work.
*/
st_map_t *fh_map = NULL;                        // associative array: file handle -> file mode

typedef enum { FM_NORMAL, FM_GZIP, FM_ZIP, FM_UNDEF } fmode2_t;

typedef struct st_finfo
{
  fmode2_t fmode;
  int compressed;
} st_finfo_t;

static st_finfo_t finfo_list[6] = { {FM_NORMAL, 0},
                                    {FM_NORMAL, 1},     // should never be used
                                    {FM_GZIP, 0},
                                    {FM_GZIP, 1},
                                    {FM_ZIP, 0},        // should never be used
                                    {FM_ZIP, 1} };

int unzip_current_file_nr = 0;


static void
init_fh_map (void)
{
  fh_map = map_create (20);                     // 20 simultaneous open files
  map_put (fh_map, stdin, &finfo_list[0]);      //  should be enough to start with
  map_put (fh_map, stdout, &finfo_list[0]);
  map_put (fh_map, stderr, &finfo_list[0]);
}


static st_finfo_t *
get_finfo (FILE *file)
{
  st_finfo_t *finfo;

  if (fh_map == NULL)
    init_fh_map ();
  if ((finfo = (st_finfo_t *) map_get (fh_map, file)) == NULL)
    {
      fprintf (stderr, "\nINTERNAL ERROR: File pointer was not present in map (%p)\n", file);
      map_dump (fh_map);
      exit (1);
    }
  return finfo;
}


static fmode2_t
get_fmode (FILE *file)
{
  return get_finfo (file)->fmode;
}


int
unzip_get_number_entries (const char *filename)
{
  FILE *file;
  unsigned char magic[4] = { 0 };

#undef  fopen
#undef  fread
#undef  fclose
  if ((file = fopen (filename, "rb")) == NULL)
    {
      errno = ENOENT;
      return -1;
    }
  fread (magic, 1, sizeof (magic), file);
  fclose (file);
#define fopen   fopen2
#define fclose  fclose2
#define fread   fread2

  if (magic[0] == 'P' && magic[1] == 'K' && magic[2] == 0x03 && magic[3] == 0x04)
    {
      unz_global_info info;

      file = (FILE *) unzOpen (filename);
      unzGetGlobalInfo (file, &info);
      unzClose (file);
      return info.number_entry;
    }
  else
    return -1;
}


int
unzip_goto_file (unzFile file, int file_index)
{
  int retval = unzGoToFirstFile (file), n = 0;

  if (file_index > 0)
    while (n < file_index)
      {
        retval = unzGoToNextFile (file);
        n++;
      }
  return retval;
}


static int
unzip_seek_helper (FILE *file, int offset)
{
  char buffer[MAXBUFSIZE];
  int n, tmp, pos = unztell (file);             // returns ftell() of the "current file"

  if (pos == offset)
    return 0;
  else if (pos > offset)
    {
      unzCloseCurrentFile (file);
      unzip_goto_file (file, unzip_current_file_nr);
      unzOpenCurrentFile (file);
      pos = 0;
    }
  n = offset - pos;
  while (n > 0 && !unzeof (file))
    {
      tmp = unzReadCurrentFile (file, buffer, n > MAXBUFSIZE ? MAXBUFSIZE : n);
      if (tmp < 0)
        return -1;
      n -= tmp;
    }
  return n > 0 ? -1 : 0;
}


FILE *
fopen2 (const char *filename, const char *mode)
{
#undef  fopen
  int n, len = strlen (mode), read = 0, compressed = 0;
  fmode2_t fmode = FM_UNDEF;
  st_finfo_t *finfo;
  FILE *file = NULL;

//  printf ("opening %s", filename);
  if (fh_map == NULL)
    init_fh_map ();

  for (n = 0; n < len; n++)
    {
      switch (mode[n])
        {
        case 'r':
          read = 1;
          break;
        case 'f':
        case 'h':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          fmode = FM_GZIP;
          break;
        case 'w':
        case 'a':
          fmode = FM_NORMAL;
          break;
        case '+':
          if (fmode == FM_UNDEF)
            fmode = FM_NORMAL;
          break;
        }
      }

  if (read)
    {
#undef  fread
#undef  fclose
      unsigned char magic[4] = { 0 };
      FILE *fh;

      // TODO?: check if mode is valid for fopen(), i.e., no 'f', 'h' or number
      if ((fh = fopen (filename, mode)) != NULL)
        {
          fread (magic, sizeof (magic), 1, fh);
          if (magic[0] == 0x1f && magic[1] == 0x8b && magic[2] == 0x08)
            {                           // ID1, ID2 and CM. gzip uses Compression Method 8
              fmode = FM_GZIP;
              compressed = 1;
            }
          else if (magic[0] == 'P' && magic[1] == 'K' &&
                   magic[2] == 0x03 && magic[3] == 0x04)
            {
              fmode = FM_ZIP;
              compressed = 1;
            }
          else
            /*
              Files that are opened with mode "r+" will probably be written to.
              zlib doesn't support mode "r+", so we have to use FM_NORMAL.
              Mode "r" doesn't require FM_NORMAL and FM_GZIP works, but we
              shouldn't introduce needless overhead.
            */
            fmode = FM_NORMAL;
          fclose (fh);
        }
#define fread   fread2
#define fclose  fclose2
    }

  if (fmode == FM_NORMAL)
    file = fopen (filename, mode);
  else if (fmode == FM_GZIP)
    file = (FILE *) gzopen (filename, mode);
  else if (fmode == FM_ZIP)
    {
      file = (FILE *) unzOpen (filename);
      if (file != NULL)
        {
          unzip_goto_file (file, unzip_current_file_nr);
          unzOpenCurrentFile (file);
        }
    }

  if (file == NULL)
    return NULL;

  finfo = &finfo_list[fmode * 2 + compressed];
  fh_map = map_put (fh_map, file, finfo);

/*
  printf (", ptr = %p, mode = %s, fmode = %s\n", file, mode,
    fmode == FM_NORMAL ? "FM_NORMAL" :
      (fmode == FM_GZIP ? "FM_GZIP" :
        (fmode == FM_ZIP ? "FM_ZIP" : "FM_UNDEF")));
  map_dump (fh_map);
*/
  return file;
#define fopen   fopen2
}


int
fclose2 (FILE *file)
{
#undef  fclose
  fmode2_t fmode = get_fmode (file);

  map_del (fh_map, file);
  if (fmode == FM_NORMAL)
    return fclose (file);
  else if (fmode == FM_GZIP)
    return gzclose (file);
  else if (fmode == FM_ZIP)
    {
      unzCloseCurrentFile (file);
      return unzClose (file);
    }
  else
    return EOF;
#define fclose  fclose2
}


int
fseek2 (FILE *file, long offset, int mode)
{
#undef  fseek
  st_finfo_t *finfo = get_finfo (file);

/*
//  if (fmode != FM_NORMAL)
  printf ("fmode = %s\n", finfo->fmode == FM_NORMAL ? "FM_NORMAL" :
                            (finfo->fmode == FM_GZIP ? "FM_GZIP" :
                              (finfo->fmode == FM_ZIP ? "FM_ZIP" : "FM_UNDEF")));
*/

  if (finfo->fmode == FM_NORMAL)
    return fseek (file, offset, mode);
  else if (finfo->fmode == FM_GZIP)
    {
      if (mode == SEEK_END)                     // zlib doesn't support SEEK_END
        {
          // Note that this is _slow_...
          while (!gzeof (file))
            {
              gzgetc (file); // necessary for _uncompressed_ files in order to set EOF
              gzseek (file, 1024 * 1024, SEEK_CUR);
            }
          offset += gztell (file);
          mode = SEEK_SET;
        }
      /*
        FUCKING zlib documentation! It took me around 4 hours of debugging time
        to find out that the doc is wrong! From the doc:
          gzrewind(file) is equivalent to (int)gzseek(file, 0L, SEEK_SET)
        That is not true for uncompressed files. gzrewind() doesn't change the
        file pointer for uncompressed files in the ports I tested (zlib 1.1.3,
        DJGPP, Cygwin & GNU/Linux). It clears the EOF indicator.
      */
      if (!finfo->compressed)
        gzrewind (file);
      return gzseek (file, offset, mode) == -1 ? -1 : 0;
    }
  else if (finfo->fmode == FM_ZIP)
    {
      int base;
      if (mode != SEEK_SET && mode != SEEK_CUR && mode != SEEK_END)
        {
          errno = EINVAL;
          return -1;
        }
      if (mode == SEEK_SET)
        base = 0;
      else if (mode == SEEK_CUR)
        base = unztell (file);
      else // mode == SEEK_END
        {
          unz_file_info info;

          unzip_goto_file (file, unzip_current_file_nr);
          unzGetCurrentFileInfo (file, &info, NULL, 0, NULL, 0, NULL, 0);
          base = info.uncompressed_size;
        }
      return unzip_seek_helper (file, base + offset);
    }
  return -1;
#define fseek   fseek2
}


size_t
fread2 (void *buffer, size_t size, size_t number, FILE *file)
{
#undef  fread
  fmode2_t fmode = get_fmode (file);

  if (size == 0 || number == 0)
    return 0;

  if (fmode == FM_NORMAL)
    return fread (buffer, size, number, file);
  else if (fmode == FM_GZIP)
    {
      int n = gzread (file, buffer, number * size);
      return n / size;
    }
  else if (fmode == FM_ZIP)
    {
      int n = unzReadCurrentFile (file, buffer, number * size);
      return n / size;
    }
  return 0;
#define fread   fread2
}


int
fgetc2 (FILE *file)
{
#undef  fgetc
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fgetc (file);
  else if (fmode == FM_GZIP)
    return gzgetc (file);
  else if (fmode == FM_ZIP)
    {
      char c;
      int retval = unzReadCurrentFile (file, &c, 1);
      return retval <= 0 ? EOF : c & 0xff;      // avoid sign bit extension
    }
  else
    return EOF;
#define fgetc   fgetc2
}


char *
fgets2 (char *buffer, int maxlength, FILE *file)
{
#undef  fgets
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fgets (buffer, maxlength, file);
  else if (fmode == FM_GZIP)
    {
      char *retval = gzgets (file, buffer, maxlength);
      return retval == Z_NULL ? NULL : retval;
    }
  else if (fmode == FM_ZIP)
    {
      int n = 0, c = 0;
      while (n < maxlength - 1 && (c = fgetc (file)) != EOF)
        {
          buffer[n] = c;                        // '\n' must also be stored in buffer
          n++;
          if (c == '\n')
            {
              buffer[n] = 0;
              break;
            }
        }
      if (n >= maxlength - 1 || c == EOF)
        buffer[n] = 0;
      return n > 0 ? buffer : NULL;
    }
  else
    return NULL;
#define fgets   fgets2
}


int
feof2 (FILE *file)
{
#undef  feof
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return feof (file);
  else if (fmode == FM_GZIP)
    return gzeof (file);
  else if (fmode == FM_ZIP)
    return unzeof (file);                       // returns feof() of the "current file"
  else
    return -1;
#define feof    feof2
}


size_t
fwrite2 (const void *buffer, size_t size, size_t number, FILE *file)
{
#undef  fwrite
  fmode2_t fmode = get_fmode (file);

  if (size == 0 || number == 0)
    return 0;

  if (fmode == FM_NORMAL)
    return fwrite (buffer, size, number, file);
  else if (fmode == FM_GZIP)
    {
      int n = gzwrite (file, (void *) buffer, number * size);
      return n / size;
    }
  else
    return 0;                                   // writing to zip files is not supported
#define fwrite  fwrite2
}


int
fputc2 (int character, FILE *file)
{
#undef  fputc
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return fputc (character, file);
  else if (fmode == FM_GZIP)
    return gzputc (file, character);
  else
    return EOF;                                 // writing to zip files is not supported
#define fputc   fputc2
}


long
ftell2 (FILE *file)
{
#undef  ftell
  fmode2_t fmode = get_fmode (file);

  if (fmode == FM_NORMAL)
    return ftell (file);
  else if (fmode == FM_GZIP)
    return gztell (file);
  else if (fmode == FM_ZIP)
    return unztell (file);                      // returns ftell() of the "current file"
  else
    return -1;
#define ftell   ftell2
}


void
rewind2 (FILE *file)
{
  fseek2 (file, 0, SEEK_SET);
}


FILE *
popen2 (const char *command, const char *mode)
{
#undef  popen
#if     defined _WIN32 || defined AMIGA
#define popen _popen
#endif
  int compressed = 0;
  fmode2_t fmode = FM_NORMAL;
  st_finfo_t *finfo;
  FILE *file;

  if (fh_map == NULL)
    init_fh_map ();

  file = popen (command, mode);
  if ((file = popen (command, mode)) == NULL)
    return NULL;

  finfo = &finfo_list[fmode * 2 + compressed];
  fh_map = map_put (fh_map, file, finfo);

  return file;
#undef  popen
#define popen   popen2
}


int
pclose2 (FILE *stream)
{
#undef  pclose
#if     defined _WIN32 || defined AMIGA
#define pclose _pclose
#endif
  fmode2_t fmode = get_fmode (stream);

  if (fmode == FM_NORMAL)
    return pclose (stream);
  else if (fmode == FM_GZIP || fmode == FM_ZIP)
    return -1;
  else
    return -1;
#undef  pclose
#define pclose  pclose2
}
#endif // USE_ZLIB
