/*
file.h - miscellaneous file functions

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
#ifndef MISC_FILE_H
#define MISC_FILE_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>                          // off_t
#ifdef  HAVE_INTTYPES_H
#include <inttypes.h>
#else                                           // __MSDOS__, _WIN32 (VC++)
#include "itypes.h"
#endif


#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined AMIGA || defined __APPLE__      // Mac OS X actually
// GNU/Linux, Solaris, FreeBSD, OpenBSD, Cygwin, BeOS, Amiga, Mac (OS X)
#define FILE_SEPARATOR '/'
#define FILE_SEPARATOR_S "/"
#else // DJGPP, Win32
#define FILE_SEPARATOR '\\'
#define FILE_SEPARATOR_S "\\"
#endif


/*
  Miscellaneous file operations

  isfname()   test if char could be used for filenames
  tofname()   replaces char that can not be used for filenames
  strpath()   a general routine to parse paths
  set_suffix() set/replace suffix of filename with suffix
                suffix means in this case the suffix INCLUDING the dot '.'
  get_suffix() get suffix of filename
  basename2() basename() replacement
  dirname2()  dirname() replacement
  realpath2() realpath() replacement
  one_file()  returns 1 if two filenames refer to one file, otherwise it
                returns 0
  one_filesystem() returns 1 if two filenames refer to files on one file
                system, otherwise it returns 0
  rename2()   renames oldname to newname even if oldname and newname are not
                on one file system
  truncate2() don't use truncate() to enlarge files, because the result is
                undefined (by POSIX) use truncate2() instead which does both
  tmpnam2()   replacement for tmpnam() temp must have the size of FILENAME_MAX
  mkbak()     modes
                BAK_DUPE (default)
                  rename file to keep attributes and copy it back to old name
                  and return new name
                  filename -> rename() -> buf -> f_cpy() -> filename -> return buf
                BAK_MOVE
                 just rename file and return new name (static)
                 filename -> rename() -> buf -> return buf
  fcopy()     copy src from start for len to dest with mode
  fcopy_raw() copy src to dest without looking at the file data (no
                decompression like with fcopy())
  fsizeof()   returns size of a file in bytes
  quick_io()  returns number of bytes read or written
  quick_io_c() returns byte read or fputc()'s status
  quick_io_func()
              runs func() everytime it loads a new buffer
                write modes will overwrite the file specified
                NOTE: modes of all quick_*() are similar to fopen() modes
                mode: "r.."
                func(void *, int) must always return the number of bytes or a
                negative value if a problem occured
                mode: "a.." or "w.."
                func(void *, int) must always return the exact number of bytes
                (int) or the buffer won't be written
*/
extern int isfname (int c);
extern int tofname (int c);
#if 0
typedef struct
{
  char realpath[FILENAME_MAX];            // absolute path
  char drive[FILENAME_MAX];               // "/", "C:\\", etc.. (depending on platform)
  char dirname[FILENAME_MAX];             // (relative) dirname
  char basename[FILENAME_MAX];            // filename
  char suffix[FILENAME_MAX];              // suffix, extension (including dot)
} st_strpath_t;

extern st_strpath_t *strpath (st_strpath_t *path, const char *path_s);
#endif
extern char *realpath2 (const char *path, char *full_path);
extern char *dirname2 (const char *path, char *dir);
extern const char *basename2 (const char *path);
extern const char *get_suffix (const char *filename);
extern char *set_suffix (char *filename, const char *suffix);
extern int one_file (const char *filename1, const char *filename2);
extern int one_filesystem (const char *filename1, const char *filename2);
extern int rename2 (const char *oldname, const char *newname);
extern int truncate2 (const char *filename, off_t size);
extern char *tmpnam2 (char *temp);
typedef enum { BAK_DUPE, BAK_MOVE } backup_t;
extern char *mkbak (const char *filename, backup_t type);
extern int fcopy (const char *src, size_t start, size_t len, const char *dest,
                  const char *dest_mode);
extern int fcopy_raw (const char *src, const char *dest);
#ifndef  USE_ZLIB
// archive.h's definition gets higher "precedence"
extern int fsizeof (const char *filename);
#endif
extern int quick_io (void *buffer, size_t start, size_t len, const char *fname,
                     const char *mode);
extern int quick_io_c (int value, size_t pos, const char *fname, const char *mode);
extern int quick_io_func (int (*callback_func) (void *, int, void *),
                          int func_maxlen, void *object, size_t start,
                          size_t len, const char *fname, const char *mode);

#endif // MISC_FILE_H
