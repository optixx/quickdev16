/*
file.c - miscellaneous file functions

Copyright (c) 1999 - 2004 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2004 dbjh
Copyright (c) 2002 - 2004 Jan-Erik Karlsson (Amiga)


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
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>                             // va_arg()
#include <sys/stat.h>                           // for S_IFLNK

#ifdef  __MSDOS__
#include <dos.h>                                // delay(), milliseconds
#elif   defined __unix__
#include <unistd.h>                             // usleep(), microseconds
#elif   defined __BEOS__
#include <OS.h>                                 // snooze(), microseconds
// Include OS.h before misc.h, because OS.h includes StorageDefs.h which
//  includes param.h which unconditionally defines MIN and MAX.
#elif   defined AMIGA
#include <unistd.h>
#include <fcntl.h>
#include <dos/dos.h>
#include <dos/var.h>
#include <libraries/lowlevel.h>                 // GetKey()
#include <proto/dos.h>
#include <proto/lowlevel.h>
#elif   defined _WIN32
#include <windows.h>                            // Sleep(), milliseconds
#endif

#ifdef  HAVE_INTTYPES_H
#include <inttypes.h>
#else                                           // __MSDOS__, _WIN32 (VC++)
#include "itypes.h"
#endif
#ifdef  USE_ZLIB
#include "archive.h"
#endif
#include "file.h"
#include "misc.h"                               // getenv2()
#include "getopt.h"                             // struct option

#ifdef  DJGPP
#ifdef  DLL
#include "dxedll_priv.h"
#endif
#endif

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifdef  _MSC_VER
// Visual C++ doesn't allow inline in C source code
#define inline __inline
#endif


extern int errno;


int
isfname (int c)
{
  // characters that are allowed in filenames
  return ((isalnum (c) || (c && strchr (".,'+- ()[]!&_", c))));
}


int
tofname (int c)
{
  return isfname (c) ? c : '_';
}


#ifndef HAVE_REALPATH
#undef  realpath
char *
realpath (const char *path, char *full_path)
{
#if     defined __unix__ || defined __BEOS__ || defined __MSDOS__
/*
  Keep the "defined _WIN32"'s in this code in case GetFullPathName() turns out
  to have some unexpected problems. This code works for Visual C++, but it
  doesn't return the same paths as GetFullPathName() does. Most notably,
  GetFullPathName() expands <drive letter>:. to the current directory of
  <drive letter>: while this code doesn't.
*/
#define MAX_READLINKS 32
  char copy_path[FILENAME_MAX], got_path[FILENAME_MAX], *new_path = got_path,
       *max_path;
#if     defined __MSDOS__ || defined _WIN32 || defined __CYGWIN__
  char c;
#endif
#ifdef  S_IFLNK
  char link_path[FILENAME_MAX];
  int readlinks = 0;
#endif
  int n;

  // Make a copy of the source path since we may need to modify it
  n = strlen (path);
  if (n >= FILENAME_MAX - 2)
    return NULL;
  else if (n == 0)
    return NULL;

  strcpy (copy_path, path);
  path = copy_path;
  max_path = copy_path + FILENAME_MAX - 2;
#if     defined __MSDOS__ || defined _WIN32 || defined __CYGWIN__
  c = toupper (*path);
  if (c >= 'A' && c <= 'Z' && path[1] == ':')
    {
      *new_path++ = *path++;
      *new_path++ = *path++;
      if (*path == FILE_SEPARATOR)
        *new_path++ = *path++;
    }
  else
#endif
  if (*path != FILE_SEPARATOR)
    {
      getcwd (new_path, FILENAME_MAX - 1);
#ifdef  DJGPP
      // DJGPP's getcwd() returns a path with forward slashes
      {
        int l = strlen (new_path);
        for (n = 0; n < l; n++)
          if (new_path[n] == '/')
            new_path[n] = FILE_SEPARATOR;
      }
#endif
      new_path += strlen (new_path);
      if (*(new_path - 1) != FILE_SEPARATOR)
        *new_path++ = FILE_SEPARATOR;
    }
  else
    {
      *new_path++ = FILE_SEPARATOR;
      path++;
    }

  // Expand each (back)slash-separated pathname component
  while (*path != 0)
    {
      // Ignore stray FILE_SEPARATOR
      if (*path == FILE_SEPARATOR)
        {
          path++;
          continue;
        }
      if (*path == '.')
        {
          // Ignore "."
          if (path[1] == 0 || path[1] == FILE_SEPARATOR)
            {
              path++;
              continue;
            }
          if (path[1] == '.')
            {
              if (path[2] == 0 || path[2] == FILE_SEPARATOR)
                {
                  path += 2;
                  // Ignore ".." at root
                  if (new_path == got_path + 1)
                    continue;
                  // Handle ".." by backing up
                  while (*((--new_path) - 1) != FILE_SEPARATOR)
                    ;
                  continue;
                }
            }
        }
      // Safely copy the next pathname component
      while (*path != 0 && *path != FILE_SEPARATOR)
        {
          if (path > max_path)
            return NULL;

          *new_path++ = *path++;
        }
#ifdef  S_IFLNK
      // Protect against infinite loops
      if (readlinks++ > MAX_READLINKS)
        return NULL;

      // See if latest pathname component is a symlink
      *new_path = 0;
      n = readlink (got_path, link_path, FILENAME_MAX - 1);
      if (n < 0)
        {
          // EINVAL means the file exists but isn't a symlink
          if (errno != EINVAL
#ifdef  __BEOS__
              // Make this function work for a mounted ext2 fs ("/:")
              && errno != B_NAME_TOO_LONG
#endif
             )
            {
              // Make sure it's null terminated
              *new_path = 0;
              strcpy (full_path, got_path);
              return NULL;
            }
        }
      else
        {
          // NOTE: readlink() doesn't add the null byte
          link_path[n] = 0;
          if (*link_path == FILE_SEPARATOR)
            // Start over for an absolute symlink
            new_path = got_path;
          else
            // Otherwise back up over this component
            while (*(--new_path) != FILE_SEPARATOR)
              ;
          if (strlen (path) + n >= FILENAME_MAX - 2)
            return NULL;
          // Insert symlink contents into path
          strcat (link_path, path);
          strcpy (copy_path, link_path);
          path = copy_path;
        }
#endif // S_IFLNK
      *new_path++ = FILE_SEPARATOR;
    }
  // Delete trailing slash but don't whomp a lone slash
  if (new_path != got_path + 1 && *(new_path - 1) == FILE_SEPARATOR)
    {
#if     defined __MSDOS__ || defined _WIN32 || defined __CYGWIN__
      if (new_path >= got_path + 3)
        {
          if (*(new_path - 2) == ':')
            {
              c = toupper (*(new_path - 3));
              if (!(c >= 'A' && c <= 'Z'))
                new_path--;
            }
          else
            new_path--;
        }
      else
        new_path--;
#else
      new_path--;
#endif
    }
  // Make sure it's null terminated
  *new_path = 0;
  strcpy (full_path, got_path);

  return full_path;
#elif   defined _WIN32
  char *p, c;
  int n;

  if (GetFullPathName (path, FILENAME_MAX, full_path, &p) == 0)
    return NULL;

  c = toupper (full_path[0]);
  n = strlen (full_path) - 1;
  // Remove trailing separator if full_path is not the root dir of a drive,
  //  because Visual C++'s run-time system is *really* stupid
  if (full_path[n] == FILE_SEPARATOR &&
      !(c >= 'A' && c <= 'Z' && full_path[1] == ':' && full_path[3] == 0)) // && full_path[2] == FILE_SEPARATOR
    full_path[n] = 0;

  return full_path;
#elif   defined AMIGA
  strcpy (full_path, path);
  return full_path;
#endif
}
#endif


#if 0
st_strpath_t *
strpath (st_strpath_t *path, const char *path_s)
{
// TODO: compare this with splitpath()
  if (path_s == NULL)
    return NULL;

  realpath2 (path_s, path->realpath);

#if     defined DJGPP || defined __CYGWIN__ || defined _WIN32
  if (isalpha (path->realpath[0]) &&
      path->realpath[1] == ':' &&
      path->realpath[2] == FILE_SEPARATOR)
    sprintf (path->drive, "%c:\\", toupper (path->realpath[0]));
#else
  strcpy (path->drive, FILE_SEPARATOR_S);
#endif

  dirname2 (path_s, path->dirname);
  strcpy (path->basename, basename2 (path_s));
  strcpy (path->suffix, get_suffix (path_s));

  return path;
}
#endif


char *
realpath2 (const char *path, char *full_path)
// enhanced realpath() which returns the absolute path of a file
{
  char path1[FILENAME_MAX];
  const char *path2;

  if (path[0] == '~')
    {
      if (path[1] == FILE_SEPARATOR
#ifdef  __CYGWIN__
          || path[1] == '\\'
#endif
         )
        sprintf (path1, "%s"FILE_SEPARATOR_S"%s", getenv2 ("HOME"), &path[2]);
      else if (path[1] == 0)
        strcpy (path1, getenv2 ("HOME"));
      path2 = path1;
    }
  else
    path2 = path;

  return realpath (path2, full_path);
}


char *
dirname2 (const char *path, char *dir)
{
  char *p1;
#if     defined DJGPP || defined __CYGWIN__
  char *p2;
#endif

  if (path == NULL)
    return NULL;

  strcpy (dir, path);
#if     defined DJGPP || defined __CYGWIN__
  // Yes, DJGPP, not __MSDOS__, because DJGPP's dirname() behaves the same
  // Cygwin has no dirname()
  p1 = strrchr (dir, '/');
  p2 = strrchr (dir, '\\');
  if (p2 > p1)                                  // use the last separator in path
    p1 = p2;
#else
  p1 = strrchr (dir, FILE_SEPARATOR);
#endif

#if     defined DJGPP || defined __CYGWIN__ || defined _WIN32
  if (p1 == NULL)                               // no slash, perhaps a drive?
    {
      if ((p1 = strrchr (dir, ':')))
        {
          p1[1] = '.';
          p1 += 2;
        }
    }
#endif

  while (p1 > dir &&                            // find first of last separators (we have to strip trailing ones)
#if     defined DJGPP || defined __CYGWIN__
         ((*(p1 - 1) == '/' && (*p1 == '/' || *p1 == '\\'))
          ||
          (*(p1 - 1) == '\\' && (*p1 == '\\' || *p1 == '/'))))
#else
         (*(p1 - 1) == FILE_SEPARATOR && *p1 == FILE_SEPARATOR))
#endif
    p1--;

  if (p1 == dir)
    p1++;                                       // don't overwrite single separator (root dir)
#if     defined DJGPP || defined __CYGWIN__ || defined _WIN32
  else if (p1 > dir)
    if (*(p1 - 1) == ':')
      p1++;                                     // we must not overwrite the last separator if
#endif                                          //  it was directly preceded by a drive letter

  if (p1)
    *p1 = 0;                                    // terminate string (overwrite the separator)
  else
    {
      dir[0] = '.';
      dir[1] = 0;
    }

  return dir;
}


const char *
basename2 (const char *path)
{
  char *p1;
#if     defined DJGPP || defined __CYGWIN__
  char *p2;
#endif

  if (path == NULL)
    return NULL;

#if     defined DJGPP || defined __CYGWIN__
  // Yes, DJGPP, not __MSDOS__, because DJGPP's basename() behaves the same
  // Cygwin has no basename()
  p1 = strrchr (path, '/');
  p2 = strrchr (path, '\\');
  if (p2 > p1)                                  // use the last separator in path
    p1 = p2;
#else
  p1 = strrchr (path, FILE_SEPARATOR);
#endif

#if     defined DJGPP || defined __CYGWIN__ || defined _WIN32
  if (p1 == NULL)                               // no slash, perhaps a drive?
    p1 = strrchr (path, ':');
#endif

  return p1 ? p1 + 1 : path;
}


const char *
get_suffix (const char *filename)
// Note that get_suffix() never returns NULL. Other code relies on that!
{
  const char *p, *s;

  if (!(p = basename2 (filename)))
    p = filename;
  if (!(s = strrchr (p, '.')))
    s = strchr (p, 0);                          // strchr(p, 0) and NOT "" is the
  if (s == p)                                   //  suffix of a file without suffix
    s = strchr (p, 0);                          // files can start with '.'

  return s;
}


char *
set_suffix (char *filename, const char *suffix)
{
  // always use set_suffix() and NEVER the code below
  strcpy ((char *) get_suffix (filename), suffix);

  return filename;
}


int
one_file (const char *filename1, const char *filename2)
// returns 1 if filename1 and filename2 refer to one file, 0 if not (or error)
{
#ifndef _WIN32
  struct stat finfo1, finfo2;

  /*
    Not the name, but the combination inode & device identify a file.
    Note that stat() doesn't need any access rights except search rights for
    the directories in the path to the file.
  */
  if (stat (filename1, &finfo1) != 0)
    return 0;
  if (stat (filename2, &finfo2) != 0)
    return 0;
  if (finfo1.st_dev == finfo2.st_dev && finfo1.st_ino == finfo2.st_ino)
    return 1;
  else
    return 0;
#else
  HANDLE file1, file2;
  BY_HANDLE_FILE_INFORMATION finfo1, finfo2;

  file1 = CreateFile (filename1, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, NULL);
  if (file1 == INVALID_HANDLE_VALUE)
    return 0;
  file2 = CreateFile (filename2, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, NULL);
  if (file2 == INVALID_HANDLE_VALUE)
    {
      CloseHandle (file1);
      return 0;
    }
  GetFileInformationByHandle (file1, &finfo1);
  GetFileInformationByHandle (file2, &finfo2);
  CloseHandle (file1);
  CloseHandle (file2);
  if (finfo1.dwVolumeSerialNumber == finfo2.dwVolumeSerialNumber &&
      (finfo1.nFileIndexHigh << 16 | finfo1.nFileIndexLow) ==
      (finfo2.nFileIndexHigh << 16 | finfo2.nFileIndexLow))
    return 1;
  else
    return 0;
#endif
}


int
one_filesystem (const char *filename1, const char *filename2)
// returns 1 if filename1 and filename2 reside on one file system, 0 if not
//  (or an error occurred)
{
#ifndef _WIN32
  struct stat finfo1, finfo2;

  if (stat (filename1, &finfo1) != 0)
    return 0;
  if (stat (filename2, &finfo2) != 0)
    return 0;
  if (finfo1.st_dev == finfo2.st_dev)
    return 1;
  else
    return 0;
#else
  DWORD fattrib1, fattrib2;
  char path1[FILENAME_MAX], path2[FILENAME_MAX], *p, d1, d2;
  HANDLE file1, file2;
  BY_HANDLE_FILE_INFORMATION finfo1, finfo2;

  if ((fattrib1 = GetFileAttributes (filename1)) == (DWORD) -1)
    return 0;
  if ((fattrib2 = GetFileAttributes (filename2)) == (DWORD) -1)
    return 0;
  if (fattrib1 & FILE_ATTRIBUTE_DIRECTORY || fattrib2 & FILE_ATTRIBUTE_DIRECTORY)
    /*
      We special-case directories, because we can't use
      FILE_FLAG_BACKUP_SEMANTICS as argument to CreateFile() under
      Windows 9x/ME. There seems to be no Win32 function other than
      CreateFile() to obtain a handle to a directory.
    */
    {
      if (GetFullPathName (filename1, FILENAME_MAX, path1, &p) == 0)
        return 0;
      if (GetFullPathName (filename2, FILENAME_MAX, path2, &p) == 0)
        return 0;
      d1 = toupper (path1[0]);
      d2 = toupper (path2[0]);
      if (d1 == d2 && d1 >= 'A' && d1 <= 'Z' && d2 >= 'A' && d2 <= 'Z')
        if (strlen (path1) >= 2 && strlen (path2) >= 2)
          // We don't handle unique volume names
          if (path1[1] == ':' && path2[1] == ':')
            return 1;
      return 0;
    }

  file1 = CreateFile (filename1, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, NULL);
  if (file1 == INVALID_HANDLE_VALUE)
    return 0;
  file2 = CreateFile (filename2, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, NULL);
  if (file2 == INVALID_HANDLE_VALUE)
    {
      CloseHandle (file1);
      return 0;
    }
  GetFileInformationByHandle (file1, &finfo1);
  GetFileInformationByHandle (file2, &finfo2);
  CloseHandle (file1);
  CloseHandle (file2);
  if (finfo1.dwVolumeSerialNumber == finfo2.dwVolumeSerialNumber)
    return 1;
  else
    return 0;
#endif
}


int
rename2 (const char *oldname, const char *newname)
{
  int retval;
  char dir1[FILENAME_MAX], dir2[FILENAME_MAX];
  struct stat fstate;

  dirname2 (oldname, dir1);
  dirname2 (newname, dir2);

  // We should use dirname{2}() in case oldname or newname doesn't exist yet
  if (one_filesystem (dir1, dir2))
    {
      if (access (newname, F_OK) == 0 && !one_file (oldname, newname))
        {
          stat (newname, &fstate);
          chmod (newname, fstate.st_mode | S_IWUSR);
          remove (newname);                     // *try* to remove or rename() will fail
        }
      retval = rename (oldname, newname);
    }
  else
    {
      retval = fcopy_raw (oldname, newname);
      // don't remove unless the file can be copied
      if (retval == 0)
        {
          stat (oldname, &fstate);
          chmod (oldname, fstate.st_mode | S_IWUSR);
          remove (oldname);
        }
    }

  return retval;
}


int
truncate2 (const char *filename, off_t new_size)
{
  int size = fsizeof (filename);
  struct stat fstate;

  stat (filename, &fstate);
  if (chmod (filename, fstate.st_mode | S_IWUSR))
    return -1;

  if (size < new_size)
    {
      FILE *file;
      unsigned char padbuffer[MAXBUFSIZE];
      int n_bytes;

      if ((file = fopen (filename, "ab")) == NULL)
        return -1;

      memset (padbuffer, 0, MAXBUFSIZE);

      while (size < new_size)
        {
          n_bytes = new_size - size > MAXBUFSIZE ? MAXBUFSIZE : new_size - size;
          fwrite (padbuffer, 1, n_bytes, file);
          size += n_bytes;
        }

      fclose (file);
    }
  else
    truncate (filename, new_size);

  return 0;                                     // success
}


char *
tmpnam2 (char *temp)
// tmpnam() clone
{
  char *p = getenv2 ("TEMP");

  srand (time (0));

  *temp = 0;
  while (!(*temp) || !access (temp, F_OK))      // must work for files AND dirs
    sprintf (temp, "%s%s%08x.tmp", p, FILE_SEPARATOR_S, rand());

  return temp;
}


static inline int
fcopy_func (void *buffer, int n, void *object)
{
  fwrite (buffer, 1, n, (FILE *) object);
  return n;
}


int
fcopy (const char *src, size_t start, size_t len, const char *dest, const char *mode)
{
  FILE *output;
  int result = 0;

  if (one_file (dest, src))                     // other code depends on this
    return -1;                                  //  behaviour!

  if (!(output = fopen (dest, mode)))
    {
      errno = ENOENT;
      return -1;
    }

  fseek (output, 0, SEEK_END);

  result = quick_io_func (fcopy_func, MAXBUFSIZE, output, start, len, src, "rb");

  fclose (output);
  sync ();

  return result == -1 ? result : 0;
}


int
fcopy_raw (const char *src, const char *dest)
// Raw file copy function. Raw, because it will copy the file data as it is,
//  unlike fcopy(). Don't merge fcopy_raw() with fcopy(). They have both their
//  uses.
{
#ifdef  USE_ZLIB
#undef  fopen
#undef  fread
#undef  fwrite
#undef  fclose
#endif
  FILE *fh, *fh2;
  int seg_len;
  char buf[MAXBUFSIZE];

  if (one_file (dest, src))
    return -1;

  if (!(fh = fopen (src, "rb")))
    return -1;
  if (!(fh2 = fopen (dest, "wb")))
    {
      fclose (fh);
      return -1;
    }
  while ((seg_len = fread (buf, 1, MAXBUFSIZE, fh)))
    fwrite (buf, 1, seg_len, fh2);

  fclose (fh);
  fclose (fh2);
  return 0;
#ifdef  USE_ZLIB
#define fopen   fopen2
#define fread   fread2
#define fwrite  fwrite2
#define fclose  fclose2
#endif
}


#ifndef USE_ZLIB
int
fsizeof (const char *filename)
{
  struct stat fstate;

  if (!stat (filename, &fstate))
    return fstate.st_size;

  errno = ENOENT;
  return -1;
}
#endif


static FILE *
quick_io_open (const char *filename, const char *mode)
{
  FILE *fh = NULL;

  if (*mode == 'w' || *mode == 'a' || mode[1] == '+') // will we write to it?
    if (!access (filename, F_OK))               // exists?
      {
        struct stat fstate;
        // First (try to) change the file mode or we won't be able to write to
        //  it if it's a read-only file.
        stat (filename, &fstate);
        if (chmod (filename, fstate.st_mode | S_IWUSR))
          {
            errno = EACCES;
            return NULL;
          }
      }

  if ((fh = fopen (filename, (const char *) mode)) == NULL)
    {
#ifdef  DEBUG
      fprintf (stderr, "ERROR: Could not open \"%s\" in mode \"%s\"\n"
                       "CAUSE: %s\n", filename, mode, strerror (errno));
#endif
      return NULL;
    }

#ifdef  DEBUG
  fprintf (stderr, "\"%s\": \"%s\"\n", filename, (char *) mode);
#endif

  return fh;
}


int
quick_io_c (int value, size_t pos, const char *filename, const char *mode)
{
  int result;
  FILE *fh;

  if (!(fh = quick_io_open (filename, (const char *) mode)))
    return -1;

  fseek (fh, pos, SEEK_SET);

  if (*mode == 'r' && mode[1] != '+')           // "r+b" always writes
    result = fgetc (fh);
  else
    result = fputc (value, fh);

  fclose (fh);
  return result;
}


int
quick_io (void *buffer, size_t start, size_t len, const char *filename,
          const char *mode)
{
  int result;
  FILE *fh;

  if (!(fh = quick_io_open (filename, (const char *) mode)))
    return -1;

  fseek (fh, start, SEEK_SET);

  // Note the order of arguments of fread() and fwrite(). Now quick_io()
  //  returns the number of characters read or written. Some code relies on
  //  this behaviour!
  if (*mode == 'r' && mode[1] != '+')           // "r+b" always writes
    result = (int) fread (buffer, 1, len, fh);
  else
    result = (int) fwrite (buffer, 1, len, fh);

  fclose (fh);
  return result;
}


static inline int
quick_io_func_inline (int (*func) (void *, int, void *), int func_maxlen,
                      void *object, void *buffer, int buffer_len)
{
  int i = 0, func_size = MIN (func_maxlen, buffer_len), func_result = 0;

  for (; i < buffer_len; i += func_size)
    {
      func_size = MIN (func_size, buffer_len - i);
      func_result = func ((char *) buffer + i, func_size, object);
      if (func_result < func_size)
        break;
    }

  return i + func_result;
}


int
quick_io_func (int (*func) (void *, int, void *), int func_maxlen, void *object,
               size_t start, size_t len, const char *filename, const char *mode)
// func() takes buffer, length and object (optional), func_maxlen is maximum
//  length passed to func()
{
  void *buffer = NULL;
  int buffer_maxlen = 0, buffer_len = 0, func_len = 0;
  size_t len_done = 0;
  FILE *fh = NULL;

  if (len <= 5 * 1024 * 1024)                   // files up to 5 MB are loaded
    if ((buffer = malloc (len)))                //  in their entirety
      buffer_maxlen = len;
  if (!buffer)
    {
      if ((buffer = malloc (func_maxlen)))
        buffer_maxlen = func_maxlen;
      else
        return -1;
    }

  if (!(fh = quick_io_open (filename, (const char *) mode)))
    {
      free (buffer);
      return -1;
    }

  fseek (fh, start, SEEK_SET);

  for (len_done = 0; len_done < len; len_done += buffer_len)
    {
      if (!(buffer_len = fread (buffer, 1, buffer_maxlen, fh)))
        break;

      func_len = quick_io_func_inline (func, func_maxlen, object, buffer, buffer_len);

      if (func_len < buffer_len) // less than buffer_len? this must be the end
        break;                   //  or a problem (if write mode)

      if (*mode == 'w' || *mode == 'a' || mode[1] == '+')
        {
          fseek (fh, -buffer_len, SEEK_CUR);
          fwrite (buffer, 1, buffer_len, fh);
          /*
            This appears to be a bug in DJGPP and Solaris (for ecample, when
            called from ucon64_fbswap16()). Without an extra call to fseek() a
            part of the file won't be written (DJGPP: after 8 MB, Solaris: after
            12 MB).
          */
          fseek (fh, 0, SEEK_CUR);
        }
    }

  fclose (fh);
  free (buffer);
  sync ();

  // returns total bytes processed or if (func() < 0) it returns that error value
  return func_len < 0 ? func_len : ((int) len_done + func_len);
}


char *
mkbak (const char *filename, backup_t type)
{
  static char buf[FILENAME_MAX];

  if (access (filename, R_OK) != 0)
    return (char *) filename;

  strcpy (buf, filename);
  set_suffix (buf, ".bak");
  if (strcmp (filename, buf) != 0)
    {
      remove (buf);                             // *try* to remove or rename() will fail
      if (rename (filename, buf))               // keep file attributes like date, etc.
        {
          fprintf (stderr, "ERROR: Can't rename \"%s\" to \"%s\"\n", filename, buf);
          exit (1);
        }
    }
  else // handle the case where filename has the suffix ".bak".
    {
      char buf2[FILENAME_MAX];

      if (!dirname2 (filename, buf))
        {
          fprintf (stderr, "INTERNAL ERROR: dirname2() returned NULL\n");
          exit (1);
        }
      if (buf[0] != 0)
        if (buf[strlen (buf) - 1] != FILE_SEPARATOR)
          strcat (buf, FILE_SEPARATOR_S);

      strcat (buf, basename2 (tmpnam2 (buf2)));
      if (rename (filename, buf))
        {
          fprintf (stderr, "ERROR: Can't rename \"%s\" to \"%s\"\n", filename, buf);
          exit (1);
        }
    }

  switch (type)
    {
    case BAK_MOVE:
      return buf;

    case BAK_DUPE:
    default:
      if (fcopy (buf, 0, fsizeof (buf), filename, "wb"))
        {
          fprintf (stderr, "ERROR: Can't open \"%s\" for writing\n", filename);
          exit (1);
        }
      sync ();
      return buf;
    }
}
