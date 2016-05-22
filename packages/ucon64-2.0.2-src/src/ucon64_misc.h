/*
ucon64_misc.h - miscellaneous functions for uCON64

Copyright (c) 1999 - 2006 NoisyB
Copyright (c) 2001 - 2004 dbjh
Copyright (c) 2001        Caz


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
#ifndef UCON64_MISC_H
#define UCON64_MISC_H

#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_DISCMAGE
#endif
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <time.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/itypes.h"


/*
  UCON64_DM_VERSION_MAJOR
  UCON64_DM_VERSION_MINOR
  UCON64_DM_VERSION_STEP  min. version of libdiscmage supported by uCON64

  ucon64_load_discmage()  load libdiscmage
  discmage_usage          usage for libdiscmage
  discmage_gauge          gauge wrapper for libdiscmage
*/
#ifdef  USE_DISCMAGE
#include "libdiscmage/libdiscmage.h"            // dm_image_t

#define UCON64_DM_VERSION_MAJOR 0
#define UCON64_DM_VERSION_MINOR 0
#define UCON64_DM_VERSION_STEP 7

extern const st_getopt2_t discmage_usage[];
extern int ucon64_load_discmage (void);
extern int discmage_gauge (int pos, int size);
#endif


/*
  usage of miscellaneous options
*/
extern const st_getopt2_t ucon64_options_usage[];
extern const st_getopt2_t ucon64_padding_usage[];


/*
  uCON64 messages

  usage example: fprintf (stdout, ucon64_msg[WROTE], filename);
*/
enum
{
  PARPORT_ERROR = 0,
  CONSOLE_ERROR,
  WROTE,
  OPEN_READ_ERROR,
  OPEN_WRITE_ERROR,
  READ_ERROR,
  WRITE_ERROR,
  BUFFER_ERROR,                                 // not enough memory
  ROM_BUFFER_ERROR,
  FILE_BUFFER_ERROR,
  DAT_NOT_FOUND,
  DAT_NOT_ENABLED,
  READ_CONFIG_FILE,
  NO_LIB
};

extern const char *ucon64_msg[];

/*
  ucon64_file_handler() handles backups (before modifying the ROM) and ROMs
                        inside archives. Read the comment at the header to
                        see how it and the flags work
  remove_temp_file()    remove possible temp file created by ucon64_file_handler()
  ucon64_output_fname()
  ucon64_gauge()        wrapper for misc.c/gauge()
  ucon64_testpad()      test if ROM is padded
  ucon64_testsplit()    test if ROM is split
                          optionally a callback function can be used for specific
                          testing
  ucon64_configfile()   configfile handling
  ucon64_rename()       DAT or internal header based rename
  ucon64_e()            emulator "frontend"
  ucon64_pattern()      change file based on patterns specified in pattern_fname
*/
#define OF_FORCE_BASENAME 1
#define OF_FORCE_SUFFIX   2

extern int ucon64_file_handler (char *dest, char *src, int flags);
extern void remove_temp_file (void);
extern char *ucon64_output_fname (char *requested_fname, int flags);
extern int ucon64_gauge (time_t init_time, int pos, int size);
extern int ucon64_testpad (const char *filename);
extern int ucon64_testsplit (const char *filename, int (*testsplit_cb) (const char *));
extern int ucon64_set_property_array (void);
extern int ucon64_rename (int mode);
extern int ucon64_e (void);
extern int ucon64_pattern (const char *pattern_fname);


/*
  Some general file stuff that MUST NOT and WILL NOT be written again and again

  ucon64_fread()    same as fread but takes start and src is a filename
  ucon64_fwrite()   same as fwrite but takes start and dest is a filename; mode
                      is the same as fopen() modes
  ucon64_fgetc()    same as fgetc but takes filename instead of FILE and a pos
  ucon64_fputc()    same as fputc but takes filename instead of FILE and a pos
                      buf,s,bs,b,f,m == buffer,start,blksize,blks,filename,mode
  ucon64_bswap16_n() bswap16() n bytes of buffer
  ucon64_fbswap16() bswap16() len bytes of file from start
  ucon64_fwswap32() wswap32() len bytes of file from start
  ucon64_dump()     file oriented wrapper for memdump() (uses the same flags)
  ucon64_find()     file oriented wrapper for memsearch() (uses the same flags)
  ucon64_chksum()   file oriented wrapper for chksum()
                      if (!sha1) {sha1 won't be calculated!}
  ucon64_filefile() compare two files for similarities or differencies
*/
#define ucon64_fgetc(f, p)           (quick_io_c(0, p, f, "rb"))
#define ucon64_fputc(f, p, b, m)     (quick_io_c(b, p, f, m))
#define ucon64_fread(b, s, l, f)     (quick_io(b, s, l, f, "rb"))
#define ucon64_fwrite(b, s, l, f, m) (quick_io((void *) b, s, l, f, m))

extern int ucon64_bswap16_n (void *buffer, int n);
extern void ucon64_fbswap16 (const char *fname, size_t start, size_t len);
extern void ucon64_fwswap32 (const char *fname, size_t start, size_t len);
extern void ucon64_dump (FILE *output, const char *filename, size_t start,
                         size_t len, uint32_t flags);
// Be sure the following constant doesn't conflict with the MEMCMP2_* constants
#define UCON64_FIND_QUIET (1 << 31)
extern int ucon64_find (const char *filename, size_t start, size_t len,
                        const char *search, int searchlen, uint32_t flags);
extern int ucon64_chksum (char *sha1, char *md5, unsigned int *crc32, // uint16_t *crc16,
                          const char *filename, int file_size, size_t start);
extern void ucon64_filefile (const char *filename1, int start1,
                             const char *filename2, int start2, int similar);
#endif // #ifndef UCON64_MISC_H
