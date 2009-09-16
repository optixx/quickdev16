/*
ucon64_misc.h - miscellaneous functions for uCON64

Copyright (c) 1999 - 2004 NoisyB <noisyb@gmx.net>
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
#include "config.h"      // USE_DISCMAGE
#endif
#include "ucon64.h"      // st_rominfo_t


/*
  UCON64_DM_VERSION_MAJOR
  UCON64_DM_VERSION_MINOR
  UCON64_DM_VERSION_STEP  min. version of libdiscmage supported by uCON64

  ucon64_load_discmage()  load libdiscmage
  libdm_usage             usage for libdiscmage
  libdm_gauge             gauge for libdiscmage
*/
#ifdef  USE_DISCMAGE
#include "libdiscmage/libdiscmage.h" // dm_image_t

#define UCON64_DM_VERSION_MAJOR 0
#define UCON64_DM_VERSION_MINOR 0
#define UCON64_DM_VERSION_STEP 7

extern const st_getopt2_t libdm_usage[];
extern int ucon64_load_discmage (void);
extern int libdm_gauge (int pos, int size);
#endif


/*
  defines for unknown backup units/emulators
*/
typedef struct // st_unknown_header
{
  /*
    Don't create fields that are larger than one byte! For example size_low and
    size_high could be combined in one unsigned short int. However, this gives
    problems with little endian vs. big endian machines (e.g. writing the header
    to disk).
  */
  unsigned char size_low;
  unsigned char size_high;
  unsigned char emulation;
  unsigned char hirom;
  unsigned char emulation1;
  unsigned char emulation2;
  unsigned char pad[2];
  unsigned char id1;
  unsigned char id2;
  unsigned char type;
  unsigned char pad2[501];
} st_unknown_header_t;

#define UNKNOWN_HEADER_START 0
#define UNKNOWN_HEADER_LEN (sizeof (st_unknown_header_t))

extern int unknown_init (st_rominfo_t *rominfo);

/*
  usage for consoles not directly supported by uCON64
*/
extern const st_getopt2_t unknown_usage[],
                          atari_usage[],
                          cd32_usage[],
                          cdi_usage[],
                          channelf_usage[],
                          coleco_usage[],
                          gamecom_usage[],
                          gc_usage[],
                          gp32_usage[],
                          intelli_usage[],
                          odyssey2_usage[],
                          odyssey_usage[],
                          ps2_usage[],
                          real3do_usage[],
                          s16_usage[],
                          sat_usage[],
                          vboy_usage[],
                          vc4000_usage[],
                          vectrex_usage[],
                          xbox_usage[],
                          mame_usage[],

                          ucon64_options_usage[],
                          ucon64_options_without_usage[],
                          ucon64_padding_usage[],
                          ucon64_patching_usage[];

#define NINTENDO_MAKER_LEN 684

extern const char *nintendo_maker[];

/*
  uCON64 "workflow" objects

  We want to do things compile-time. Using ucon64_wf is necessary for VC 6. GCC
  (3) accepts casts in struct initialisations.
*/
enum
{
  WF_OBJ_ALL_SWITCH = 0,
  WF_OBJ_ALL_DEFAULT,
  WF_OBJ_ALL_DEFAULT_NO_SPLIT,
  WF_OBJ_ALL_STOP,
  WF_OBJ_ALL_STOP_NO_ROM,
  WF_OBJ_ALL_DEFAULT_STOP_NO_ROM,
  WF_OBJ_ALL_INIT,
  WF_OBJ_ALL_INIT_PROBE,
  WF_OBJ_ALL_INIT_PROBE_STOP,
  WF_OBJ_ALL_INIT_PROBE_NO_ROM,
  WF_OBJ_ALL_INIT_PROBE_NO_SPLIT,
  WF_OBJ_ALL_INIT_PROBE_NO_CRC32,
  WF_OBJ_ALL_INIT_NO_SPLIT,

  WF_OBJ_DC_SWITCH,
  WF_OBJ_DC_DEFAULT,
  WF_OBJ_DC_NO_ROM,
  WF_OBJ_GB_SWITCH,
  WF_OBJ_GB_DEFAULT,
  WF_OBJ_GBA_SWITCH,
  WF_OBJ_GBA_DEFAULT,
  WF_OBJ_GEN_SWITCH,
  WF_OBJ_GEN_DEFAULT,
  WF_OBJ_GEN_DEFAULT_NO_SPLIT,
  WF_OBJ_JAG_SWITCH,
  WF_OBJ_LYNX_SWITCH,
  WF_OBJ_LYNX_DEFAULT,
  WF_OBJ_N64_SWITCH,
  WF_OBJ_N64_DEFAULT,
  WF_OBJ_N64_INIT_PROBE,
  WF_OBJ_NG_SWITCH,
  WF_OBJ_NG_DEFAULT,
  WF_OBJ_NES_SWITCH,
  WF_OBJ_NES_DEFAULT,
  WF_OBJ_NGP_SWITCH,
  WF_OBJ_PCE_SWITCH,
  WF_OBJ_PCE_DEFAULT,
  WF_OBJ_PSX_SWITCH,
  WF_OBJ_SMS_SWITCH,
  WF_OBJ_SMS_DEFAULT_NO_SPLIT,
  WF_OBJ_SNES_SWITCH,
  WF_OBJ_SNES_DEFAULT,
  WF_OBJ_SNES_DEFAULT_NO_SPLIT,
  WF_OBJ_SNES_NO_ROM,
  WF_OBJ_SNES_INIT_PROBE,
  WF_OBJ_SWAN_SWITCH,

  WF_OBJ_N64_STOP_NO_ROM,
  WF_OBJ_N64_DEFAULT_STOP,
  WF_OBJ_N64_DEFAULT_STOP_NO_ROM,
  WF_OBJ_GEN_STOP_NO_ROM,
  WF_OBJ_GEN_DEFAULT_STOP_NO_SPLIT_NO_ROM,
  WF_OBJ_GBA_STOP_NO_ROM,
  WF_OBJ_GBA_DEFAULT_STOP,
  WF_OBJ_GBA_DEFAULT_STOP_NO_ROM,
  WF_OBJ_SNES_STOP_NO_ROM,
  WF_OBJ_SNES_DEFAULT_STOP_NO_ROM,
  WF_OBJ_SNES_DEFAULT_STOP_NO_SPLIT_NO_ROM,
  WF_OBJ_GB_STOP_NO_ROM,
  WF_OBJ_GB_DEFAULT_STOP_NO_ROM,
  WF_OBJ_LYNX_STOP_NO_ROM,
  WF_OBJ_PCE_DEFAULT_STOP_NO_SPLIT_NO_ROM,
  WF_OBJ_NGP_STOP_NO_ROM,
  WF_OBJ_NGP_DEFAULT_STOP_NO_ROM,
  WF_OBJ_NES_STOP_NO_ROM,
  WF_OBJ_NES_DEFAULT_STOP_NO_SPLIT,
  WF_OBJ_SMS_STOP_NO_ROM,
  WF_OBJ_SMS_DEFAULT_STOP_NO_SPLIT_NO_ROM,

  WF_OBJ_GC_SWITCH,
  WF_OBJ_S16_SWITCH,
  WF_OBJ_ATA_SWITCH,
  WF_OBJ_COLECO_SWITCH,
  WF_OBJ_VBOY_SWITCH,
  WF_OBJ_VEC_SWITCH,
  WF_OBJ_INTELLI_SWITCH,
  WF_OBJ_GP32_SWITCH,
  WF_OBJ_PS2_SWITCH,
  WF_OBJ_XBOX_SWITCH,
  WF_OBJ_SAT_SWITCH,
  WF_OBJ_3DO_SWITCH,
  WF_OBJ_CD32_SWITCH,
  WF_OBJ_CDI_SWITCH,
};

extern st_ucon64_obj_t ucon64_wf[];

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
extern int ucon64_testsplit (const char *filename);
extern int ucon64_configfile ();
extern int ucon64_rename (int mode);
extern int ucon64_e (void);
extern int ucon64_pattern (st_rominfo_t *rominfo, const char *pattern_fname);
extern char *ucon64_temp_file;
extern int (*ucon64_testsplit_callback) (const char *filename);

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
                          const char *filename, size_t start);
extern void ucon64_filefile (const char *filename1, int start1,
                             const char *filename2, int start2, int similar);
#endif // #ifndef UCON64_MISC_H
