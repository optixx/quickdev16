/*
ucon64_misc.c - miscellaneous functions for uCON64

Copyright (c) 1999 - 2006       NoisyB
Copyright (c) 2001 - 2005, 2015 dbjh
Copyright (c) 2001              Caz
Copyright (c) 2002 - 2003       Jan-Erik Karlsson (Amiga)


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
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <sys/stat.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "misc/archive.h"
#include "misc/bswap.h"
#include "misc/chksum.h"
#include "misc/file.h"
#include "misc/misc.h"
#include "misc/property.h"
#include "misc/string.h"
#include "misc/term.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"


#ifdef  USE_DISCMAGE
#ifdef  DLOPEN
#include "misc/dlopen.h"


static void *libdm;
static uint32_t (*dm_get_version_ptr) (void) = NULL;
static const char *(*dm_get_version_s_ptr) (void) = NULL;
static void (*dm_set_gauge_ptr) (void (*) (int, int)) = NULL;
static void (*dm_nfo_ptr) (const dm_image_t *, int, int) = NULL;

static FILE *(*dm_fdopen_ptr) (dm_image_t *, int, const char *) = NULL;
static dm_image_t *(*dm_open_ptr) (const char *, uint32_t) = NULL;
static dm_image_t *(*dm_reopen_ptr) (const char *, uint32_t, dm_image_t *) = NULL;
static int (*dm_close_ptr) (dm_image_t *) = NULL;

static int (*dm_disc_read_ptr) (const dm_image_t *) = NULL;
static int (*dm_disc_write_ptr) (const dm_image_t *) = NULL;

static int (*dm_read_ptr) (char *, int, int, const dm_image_t *) = NULL;
static int (*dm_write_ptr) (const char *, int, int, const dm_image_t *) = NULL;

static dm_image_t *(*dm_toc_read_ptr) (dm_image_t *, const char *) = NULL;
static int (*dm_toc_write_ptr) (const dm_image_t *) = NULL;

static dm_image_t *(*dm_cue_read_ptr) (dm_image_t *, const char *) = NULL;
static int (*dm_cue_write_ptr) (const dm_image_t *) = NULL;

static int (*dm_rip_ptr) (const dm_image_t *, int, uint32_t) = NULL;
#endif // DLOPEN


static st_ucon64_obj_t discmage_obj[] =
  {
    {0, WF_SWITCH},
    {0, WF_DEFAULT}
  };

const st_getopt2_t discmage_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "All disc-based consoles",
      NULL
    },
    {
      "disc", 0, 0, UCON64_DISC,
      NULL, "force recognition",
      &discmage_obj[0]
    },
    {
      "rip", 1, 0, UCON64_RIP,
      "N", "rip/dump track N from IMAGE",
      &discmage_obj[1]
    },
#if 0
    {
      "filerip", 1, 0, UCON64_FILERIP,
      "N", "rip/dump files from a track N in IMAGE",
      NULL
    },
    {
      "cdmage", 1, 0, UCON64_CDMAGE,
      "N", "like " OPTION_LONG_S "rip but writes always (padded) sectors with 2352 Bytes;\n"
      "this is what CDmage would do",
      &discmage_obj[1]
    },
#endif
    {
      "bin2iso", 1, 0, UCON64_BIN2ISO,
      "N", "convert track N to ISO (if possible) by resizing\n"
      "sectors to 2048 Bytes",
      &discmage_obj[1]
    },
    {
      "isofix", 1, 0, UCON64_ISOFIX,
      "N", "fix corrupted track N (if possible)\n"
      "if PVD points to a bad DR offset it will add padding data\n"
      "so actual DR gets located in right absolute address",
      &discmage_obj[1]
    },
    {
      "mkcue", 0, 0, UCON64_MKCUE,
      NULL, "generate CUE sheet for IMAGE or existing TOC sheet",
      &discmage_obj[1]
    },
    {
      "mktoc", 0, 0, UCON64_MKTOC,
      NULL, "generate TOC sheet for IMAGE or existing CUE sheet",
      &discmage_obj[1]
    },
    {
      // hidden option
      "mksheet", 0, 0, UCON64_MKSHEET,
      NULL, /* "same as " OPTION_LONG_S "mktoc and " OPTION_LONG_S "mkcue" */ NULL,
      &discmage_obj[1]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };
#endif // USE_DISCMAGE


/*
  This is a string pool. gcc 2.9x generates something like this itself, but it
  seems gcc 3.x does not. By using a string pool the executable will be
  smaller than without it.
  It's also handy in order to be consistent with messages.
*/
const char *ucon64_msg[] =
  {
    "ERROR: Communication with backup unit failed\n"                    // PARPORT_ERROR
    "TIP:   Check cables and connection\n"
    "       Turn the backup unit off and on\n"
//    "       Split ROMs must be joined first\n" // handled with WF_NO_SPLIT
    "       Use " OPTION_LONG_S "port={3bc, 378, 278, ...} to specify a parallel port address\n"
    "       Set the port to SPP (standard, normal) mode in your BIOS as some backup\n"
    "         units do not support EPP and ECP style parallel ports\n"
    "       Read the backup unit's manual\n",

    "ERROR: Could not auto detect the right ROM/IMAGE/console type\n"   // CONSOLE_ERROR
    "TIP:   If this is a ROM or CD IMAGE you might try to force the recognition\n"
    "       The force recognition option for SNES would be " OPTION_LONG_S "snes\n",

    "Wrote output to %s\n",                                             // WROTE
    "ERROR: Cannot open \"%s\" for reading\n",                          // OPEN_READ_ERROR
    "ERROR: Cannot open \"%s\" for writing\n",                          // OPEN_WRITE_ERROR
    "ERROR: Cannot read from \"%s\"\n",                                 // READ_ERROR
    "ERROR: Cannot write to \"%s\"\n",                                  // WRITE_ERROR
    "ERROR: Not enough memory for buffer (%d bytes)\n",                 // BUFFER_ERROR
    "ERROR: Not enough memory for ROM buffer (%d bytes)\n",             // ROM_BUFFER_ERROR
    "ERROR: Not enough memory for file buffer (%d bytes)\n",            // FILE_BUFFER_ERROR
    "DAT info: No ROM with 0x%08x as checksum found\n",                 // DAT_NOT_FOUND
    "WARNING: Support for DAT files is disabled, because \"ucon64_datdir\" (either\n" // DAT_NOT_ENABLED
    "         in the configuration file or the environment) points to an incorrect\n"
    "         directory. Read the FAQ for more information.\n",
    "Reading config file %s\n",                                         // READ_CONFIG_FILE
    "NOTE: %s not found or too old, support for discmage disabled\n",   // NO_LIB
    NULL
  };


static st_ucon64_obj_t ucon64_option_obj[] =
  {
    {0, WF_SWITCH},
    {0, WF_DEFAULT},
    {0, WF_STOP},
    {0, WF_NO_ROM},
    {0, WF_DEFAULT | WF_STOP | WF_NO_ROM},
    {0, WF_NO_ARCHIVE},
    {0, WF_INIT},
    {0, WF_INIT | WF_PROBE},
    {0, WF_INIT | WF_PROBE | WF_NO_SPLIT},
    {0, WF_INIT | WF_PROBE | WF_NO_CRC32}
  };


const st_getopt2_t ucon64_options_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Options",
      NULL
    },
    {
      "o", 1, 0, UCON64_O,
      "DIRECTORY", "specify output directory",
      &ucon64_option_obj[0]
    },
    {
      "r", 0, 0, UCON64_R,
      NULL, "process subdirectories recursively",
      &ucon64_option_obj[0]
    },
    {
      "nbak", 0, 0, UCON64_NBAK,
      NULL, "prevents backup files (*.BAK)",
      &ucon64_option_obj[0]
    },
#ifdef  USE_ANSI_COLOR
    {
      "ncol", 0, 0, UCON64_NCOL,
      NULL, "disable ANSI colors in output",
      &ucon64_option_obj[0]
    },
#endif
#if     defined USE_PARALLEL || defined USE_USB
    {
      "port", 1, 0, UCON64_PORT,
      "PORT", "specify "
#ifdef  USE_USB
        "USB"
#endif
#if     defined USE_PARALLEL && defined USE_USB
        " or "
#endif
#ifdef  USE_PARALLEL
        "parallel"
#endif
        " PORT={"
#ifdef  USE_USB
        "USB0, USB1, ... "
#endif
#ifdef  USE_PARALLEL
        "3bc, 378, 278"
#endif
        "}\n"
        "In order to connect a copier to a PC's parallel port\n"
        "you need a standard bidirectional parallel cable",
      &ucon64_option_obj[0]
    },
#endif // defined USE_PARALLEL || defined USE_USB
#ifdef  USE_PARALLEL
    {
      "xreset", 0, 0, UCON64_XRESET,
      NULL, "reset parallel port",
      &ucon64_option_obj[3]                     // it's NOT a stop option
    },
#endif
    {
      "hdn", 1, 0, UCON64_HDN,
      "N", "force ROM has backup unit/emulator header with size of N Bytes",
      &ucon64_option_obj[0]
    },
    {
      "hd", 0, 0, UCON64_HD,
      NULL, "same as " OPTION_LONG_S "hdn=512\n"
      "most backup units use a header with a size of 512 Bytes",
      &ucon64_option_obj[0]
    },
    {
      "nhd", 0, 0, UCON64_NHD,
      NULL, "force ROM has no backup unit/emulator header",
      &ucon64_option_obj[0]
    },
    {
      "ns", 0, 0, UCON64_NS,
      NULL, "force ROM is not split",
      &ucon64_option_obj[0]
    },
    {
      "e", 0, 0, UCON64_E,
      NULL, "emulate/run ROM (check " PROPERTY_HOME_RC("ucon64") " for all Emulator settings)",
      &ucon64_option_obj[1]
    },
    {
      "crc", 0, 0, UCON64_CRC,
      NULL, "show CRC32 value of ROM",
      &ucon64_option_obj[9]
    },
    {
      "sha1", 0, 0, UCON64_SHA1,
      NULL, "show SHA1 value of ROM",
      &ucon64_option_obj[9]
    },
    {
      "md5", 0, 0, UCON64_MD5,
      NULL, "show MD5 value of ROM",
      &ucon64_option_obj[9]
    },
    {
      "ls", 0, 0, UCON64_LS,
      NULL, "generate ROM list for all recognized ROMs",
      &ucon64_option_obj[7]
    },
    {
      "lsv", 0, 0, UCON64_LSV,
      NULL, "like " OPTION_LONG_S "ls but more verbose",
      &ucon64_option_obj[7]
    },
    {
      "hex", 2, 0, UCON64_HEX,
      "ST", "show ROM as hexdump\n"
      "ST is the optional start value in bytes",
      NULL
    },
    {
      "dual", 2, 0, UCON64_DUAL,                // TODO: Think of a decent name - dbjh
      "ST", "show ROM as dualdump",
      NULL
    },
    {
      "code", 2, 0, UCON64_CODE,
      "ST", "show ROM as code",
      NULL
    },
    {
      "print", 2, 0, UCON64_PRINT,
      "ST", "show ROM in printable characters",
      NULL
    },
    {
      "find", 1, 0, UCON64_FIND,
      "STRING", "find STRING in ROM (wildcard: '?')",
      &ucon64_option_obj[6]
    },
    {
      "findi", 1, 0, UCON64_FINDI,
      "STR", "like " OPTION_LONG_S "find but ignores the case of alpha bytes",
      &ucon64_option_obj[6]
    },
    {
      "findr", 1, 0, UCON64_FINDR,
      "STR", "like " OPTION_LONG_S "find but looks also for shifted/relative similarities\n"
      "(no wildcard supported)",
      &ucon64_option_obj[6]
    },
    {
      "hfind", 1, 0, UCON64_HFIND,
      "HEX", "find HEX codes in ROM; use quotation " OPTION_LONG_S "hfind=\"75 ? 4f 4e\"\n"
             "(wildcard: '?')",
      &ucon64_option_obj[6]
    },
    {
      "hfindr", 1, 0, UCON64_HFINDR,
      "HEX", "like " OPTION_LONG_S "hfind but looks also for shifted/relative similarities\n"
      "(no wildcard supported)",
      &ucon64_option_obj[6]
    },
    {
      "dfind", 1, 0, UCON64_DFIND,
      "DEC", "find DEC values in ROM; use quotation " OPTION_LONG_S "dfind=\"117 ? 79 78\"\n"
             "(wildcard: '?')",
      &ucon64_option_obj[6]
    },
    {
      "dfindr", 1, 0, UCON64_DFINDR,
      "DEC", "like " OPTION_LONG_S "dfind but looks also for shifted/relative similarities\n"
      "(no wildcard supported)",
      &ucon64_option_obj[6]
    },
    {
      "c", 1, 0, UCON64_C,
      "FILE", "compare FILE with ROM for differences",
      NULL
    },
    {
      "cs", 1, 0, UCON64_CS,
      "FILE", "compare FILE with ROM for similarities",
      NULL
    },
    {
      "help", 2, 0, UCON64_HELP,
      "WHAT", "display help and exit\n"
              "WHAT=\"long\"   show long help (default)\n"
              "WHAT=\"pad\"    show help for padding ROMs\n"
              "WHAT=\"dat\"    show help for DAT support\n"
              "WHAT=\"patch\"  show help for patching ROMs\n"
              "WHAT=\"backup\" show help for backup units\n"
#ifdef  USE_DISCMAGE
              "WHAT=\"disc\"   show help for DISC image support\n"
#endif
              OPTION_LONG_S "help " OPTION_LONG_S "snes would show only SNES related help",
      &ucon64_option_obj[2]
    },
    {
      "version", 0, 0, UCON64_VER,
      NULL, "output version information and exit",
      &ucon64_option_obj[2]
    },
    {
      "q", 0, 0, UCON64_Q,
      NULL, "be quiet (don't show ROM info)",
      &ucon64_option_obj[0]
    },
#if 0
    {
      "qq", 0, 0, UCON64_QQ,
      NULL, "be even more quiet",
      &ucon64_option_obj[0]
    },
#endif
    {
      "v", 0, 0, UCON64_V,
      NULL, "be more verbose (show backup unit headers also)",
      &ucon64_option_obj[0]
    },
    // hidden options
    {
      "crchd", 0, 0, UCON64_CRCHD,              // backward compat.
      NULL, NULL,
      &ucon64_option_obj[9]
    },
    {
      "file", 1, 0, UCON64_FILE,                // obsolete?
      NULL, NULL,
      &ucon64_option_obj[0]
    },
    {
      "frontend", 0, 0, UCON64_FRONTEND,        // no usage?
      NULL, NULL,
      &ucon64_option_obj[0]
    },
    {
      "?", 0, 0, UCON64_HELP,                   // same as --help
      NULL, NULL,
      &ucon64_option_obj[2]
    },
    {
      "h", 0, 0, UCON64_HELP,                   // same as --help
      NULL, NULL,
      &ucon64_option_obj[2]
    },
    {
      "id", 0, 0, UCON64_ID,                    // currently only used in snes.c
      NULL, NULL,
      &ucon64_option_obj[0]
    },
    {
      "rom", 0, 0, UCON64_ROM,                  // obsolete?
      NULL, NULL,
      &ucon64_option_obj[0]
    },
    {
      "rename", 0, 0, UCON64_RDAT,              // is now "rdat"
      NULL, NULL,
      &ucon64_option_obj[8]
    },
    {
      "force63", 0, 0, UCON64_RJOLIET,          // is now "rjoilet"
      NULL, NULL,
      &ucon64_option_obj[5]
    },
    {
      "rr83", 0, 0, UCON64_R83,                 // is now "r83"
      NULL, NULL,
      &ucon64_option_obj[5]
    },
    {
      "83", 0, 0, UCON64_R83,                   // is now "r83"
      NULL, NULL,
      &ucon64_option_obj[5]
    },
#if 0
    {
      "xcdrw", 0, 0, UCON64_XCDRW,              // obsolete
      NULL, NULL,
      &ucon64_option_obj[4]
    },
    {
      "cdmage", 1, 0, UCON64_CDMAGE,            // obsolete
      NULL, NULL,
      &ucon64_option_obj[1]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


static st_ucon64_obj_t ucon64_padding_obj[] =
  {
    {0, WF_DEFAULT},
    {0, WF_INIT | WF_NO_SPLIT}
  };


const st_getopt2_t ucon64_padding_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Padding",
      NULL
    },
    {
      "ispad", 0, 0, UCON64_ISPAD,
      NULL, "check if ROM is padded",
      &ucon64_padding_obj[1]
    },
    {
      "pad", 0, 0, UCON64_PAD,
      NULL, "pad ROM to next Mb",
      &ucon64_padding_obj[0]
    },
    {
      "p", 0, 0, UCON64_P,
      NULL, "same as " OPTION_LONG_S "pad",
      &ucon64_padding_obj[0]
    },
    {
      "padn", 1, 0, UCON64_PADN,
      "N", "pad ROM to N Bytes (put Bytes with value 0x00 after end)",
      &ucon64_padding_obj[0]
    },
    {
      "strip", 1, 0, UCON64_STRIP,
      "N", "strip N Bytes from end of ROM",
      NULL
    },
    {
      "stpn", 1, 0, UCON64_STPN,
      "N", "strip N Bytes from start of ROM",
      NULL
    },
    {
      "stp", 0, 0, UCON64_STP,
      NULL, "same as " OPTION_LONG_S "stpn=512\n"
      "most backup units use a header with a size of 512 Bytes",
      NULL
    },
    {
      "insn", 1, 0, UCON64_INSN,
      "N", "insert N Bytes (0x00) before ROM",
      NULL
    },
    {
      "ins", 0, 0, UCON64_INS,
      NULL, "same as " OPTION_LONG_S "insn=512\n"
      "most backup units use a header with a size of 512 Bytes",
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_DISCMAGE
int
ucon64_load_discmage (void)
{
  uint32_t version;
#ifdef  DLOPEN
  const char *p = NULL;

  p = get_property (ucon64.configfile, "discmage_path", PROPERTY_MODE_FILENAME);
  if (p)
    strcpy (ucon64.discmage_path, p);
  else
    *(ucon64.discmage_path) = 0;

  // if ucon64.discmage_path points to an existing file then load it
  if (!access (ucon64.discmage_path, F_OK))
    {
      u_func_ptr_t sym;

      libdm = open_module (ucon64.discmage_path);

      sym.void_ptr = get_symbol (libdm, "dm_get_version");
      dm_get_version_ptr = (uint32_t (*) (void)) sym.func_ptr;
      version = dm_get_version_ptr ();
      if (version < LIB_VERSION (UCON64_DM_VERSION_MAJOR,
                                 UCON64_DM_VERSION_MINOR,
                                 UCON64_DM_VERSION_STEP))
        {
          printf ("WARNING: Your libdiscmage is too old (%u.%u.%u)\n"
                  "         You need at least version %u.%u.%u\n\n",
                  (unsigned int) version >> 16,
                  (unsigned int) ((version >> 8) & 0xff),
                  (unsigned int) (version & 0xff),
                  UCON64_DM_VERSION_MAJOR,
                  UCON64_DM_VERSION_MINOR,
                  UCON64_DM_VERSION_STEP);
          return 0;
        }
      else
        {
          sym.void_ptr = get_symbol (libdm, "dm_get_version_s");
          dm_get_version_s_ptr = (const char *(*) (void)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_set_gauge");
          dm_set_gauge_ptr = (void (*) (void (*) (int, int))) sym.func_ptr;

          sym.void_ptr = get_symbol (libdm, "dm_open");
          dm_open_ptr = (dm_image_t *(*) (const char *, uint32_t)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_reopen");
          dm_reopen_ptr = (dm_image_t *(*) (const char *, uint32_t, dm_image_t *)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_fdopen");
          dm_fdopen_ptr = (FILE *(*) (dm_image_t *, int, const char *)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_close");
          dm_close_ptr = (int (*) (dm_image_t *)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_nfo");
          dm_nfo_ptr = (void (*) (const dm_image_t *, int, int)) sym.func_ptr;

          sym.void_ptr = get_symbol (libdm, "dm_read");
          dm_read_ptr = (int (*) (char *, int, int, const dm_image_t *)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_write");
          dm_write_ptr = (int (*) (const char *, int, int, const dm_image_t *)) sym.func_ptr;

          sym.void_ptr = get_symbol (libdm, "dm_disc_read");
          dm_disc_read_ptr = (int (*) (const dm_image_t *)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_disc_write");
          dm_disc_write_ptr = (int (*) (const dm_image_t *)) sym.func_ptr;

          sym.void_ptr = get_symbol (libdm, "dm_toc_read");
          dm_toc_read_ptr = (dm_image_t *(*) (dm_image_t *, const char *)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_toc_write");
          dm_toc_write_ptr = (int (*) (const dm_image_t *)) sym.func_ptr;

          sym.void_ptr = get_symbol (libdm, "dm_cue_read");
          dm_cue_read_ptr = (dm_image_t *(*) (dm_image_t *, const char *)) sym.func_ptr;
          sym.void_ptr = get_symbol (libdm, "dm_cue_write");
          dm_cue_write_ptr = (int (*) (const dm_image_t *)) sym.func_ptr;

          sym.void_ptr = get_symbol (libdm, "dm_rip");
          dm_rip_ptr = (int (*) (const dm_image_t *, int, uint32_t)) sym.func_ptr;

          return 1;
        }
    }
  else
    return 0;
#else // !defined DLOPEN
#ifdef  DJGPP
  {
    /*
      The following piece of code makes the DLL "search" behaviour a bit like
      the search behaviour for Windows programs. A bit, because the import
      library just opens the file with the name that is stored in
      djimport_path. It won't search for the DXE in the Windows system
      directory, nor will it search the directories of the PATH environment
      variable.
    */
    extern char djimport_path[FILENAME_MAX];
    char dir[FILENAME_MAX];
    int n, l;

    dirname2 (ucon64.argv[0], dir);
    sprintf (djimport_path, "%s" DIR_SEPARATOR_S "%s", dir, "discmage.dxe");
    // this is specific to DJGPP - not necessary, but prevents confusion
    l = strlen (djimport_path);
    for (n = 0; n < l; n++)
      if (djimport_path[n] == '/')
        djimport_path[n] = '\\';
  }
#endif // DJGPP
  version = dm_get_version ();
  if (version < LIB_VERSION (UCON64_DM_VERSION_MAJOR,
                             UCON64_DM_VERSION_MINOR,
                             UCON64_DM_VERSION_STEP))
    {
      printf ("WARNING: Your libdiscmage is too old (%u.%u.%u)\n"
              "         You need at least version %u.%u.%u\n\n",
              (unsigned int) version >> 16,
              (unsigned int) ((version >> 8) & 0xff),
              (unsigned int) (version & 0xff),
              UCON64_DM_VERSION_MAJOR,
              UCON64_DM_VERSION_MINOR,
              UCON64_DM_VERSION_STEP);
      return 0;
    }
  return 1;                                     // discmage could be "loaded"
#endif // !defined DLOPEN
}


int
discmage_gauge (int pos, int size)
{
  static time_t init_time = 0;

  if (!init_time || !pos /* || !size */)
    init_time = time (0);

  return ucon64_gauge (init_time, pos, size);
}


#ifdef  DLOPEN
uint32_t
dm_get_version (void)
{
  return dm_get_version_ptr ();
}


const char *
dm_get_version_s (void)
{
  return dm_get_version_s_ptr ();
}


void
dm_set_gauge (void (*a) (int, int))
{
  dm_set_gauge_ptr (a);
}


FILE *
dm_fdopen (dm_image_t *a, int b, const char *c)
{
  return dm_fdopen_ptr (a, b, c);
}


dm_image_t *
dm_open (const char *a, uint32_t b)
{
  return dm_open_ptr (a, b);
}


dm_image_t *
dm_reopen (const char *a, uint32_t b, dm_image_t *c)
{
  return dm_reopen_ptr (a, b, c);
}


int
dm_close (dm_image_t *a)
{
  return dm_close_ptr (a);
}


void
dm_nfo (const dm_image_t *a, int b, int c)
{
  dm_nfo_ptr (a, b, c);
}


int
dm_disc_read (const dm_image_t *a)
{
  return dm_disc_read_ptr (a);
}


int
dm_disc_write (const dm_image_t *a)
{
  return dm_disc_write_ptr (a);
}


int
dm_read (char *a, int b, int c, const dm_image_t *d)
{
  return dm_read_ptr (a, b, c, d);
}


int
dm_write (const char *a, int b, int c, const dm_image_t *d)
{
  return dm_write_ptr (a, b, c, d);
}


dm_image_t *
dm_toc_read (dm_image_t *a, const char *b)
{
  return dm_toc_read_ptr (a, b);
}


int
dm_toc_write (const dm_image_t *a)
{
  return dm_toc_write_ptr (a);
}


dm_image_t *
dm_cue_read (dm_image_t *a, const char *b)
{
  return dm_cue_read_ptr (a, b);
}


int
dm_cue_write (const dm_image_t *a)
{
  return dm_cue_write_ptr (a);
}


int
dm_rip (const dm_image_t *a, int b, uint32_t c)
{
  return dm_rip_ptr (a, b, c);
}
#endif // DLOPEN
#endif // USE_DISCMAGE


int
ucon64_file_handler (char *dest, char *src, int flags)
/*
  We have to handle the following cases (for example -swc and rom.swc exists):
  1) ucon64 -swc rom.swc
    a) with backup creation enabled
       Create backup of rom.swc
       postcondition: src == name of backup
    b) with backup creation disabled
       Create temporary backup of rom.swc by renaming rom.swc
       postcondition: src == name of backup
  2) ucon64 -swc rom.fig
    a) with backup creation enabled
       Create backup of rom.swc
       postcondition: src == rom.fig
    b) with backup creation disabled
       Do nothing
       postcondition: src == rom.fig

  This function returns 1 if dest existed (in the directory specified with -o).
  Otherwise it returns 0;
*/
{
  struct stat dest_info;

  ucon64_output_fname (dest, flags);            // call this function unconditionally

#if 0
  // ucon64.temp_file will be reset in remove_temp_file()
  ucon64.temp_file = NULL;
#endif
  if (!access (dest, F_OK))
    {
      stat (dest, &dest_info);
      // *Trying* to make dest writable here avoids having to change all code
      //  that might (try to) operate on a read-only file
      chmod (dest, dest_info.st_mode | S_IWUSR);

      if (src == NULL)
        {
          if (ucon64.backup)
            printf ("Wrote backup to %s\n", mkbak (dest, BAK_DUPE));
          return 1;
        }

      if (one_file (src, dest))
        {                                       // case 1
          if (ucon64.backup)
            {                                   // case 1a
              strcpy (src, mkbak (dest, BAK_DUPE));
              printf ("Wrote backup to %s\n", src);
            }
          else
            {                                   // case 1b
              strcpy (src, mkbak (dest, BAK_MOVE));
              ucon64.temp_file = src;
            }
        }
      else
        {                                       // case 2
          if (ucon64.backup)                    // case 2a
            printf ("Wrote backup to %s\n", mkbak (dest, BAK_DUPE));
        }
      return 1;
    }
  return 0;
}


void
remove_temp_file (void)
{
  if (ucon64.temp_file)
    {
      printf ("Removing %s\n", ucon64.temp_file);
      remove (ucon64.temp_file);
      ucon64.temp_file = NULL;
    }
}


char *
ucon64_output_fname (char *requested_fname, int flags)
{
  char suffix[80], fname[FILENAME_MAX];

  // We have to make a copy, because get_suffix() returns a pointer to a
  //  location in the original string
  strncpy (suffix, get_suffix (requested_fname), sizeof (suffix))[sizeof (suffix) - 1] = 0; // in case suffix is >= 80 chars

  // OF_FORCE_BASENAME is necessary for options like -gd3. Of course that
  //  code should handle archives and come up with unique filenames for
  //  archives with more than one file.
  if (!ucon64.fname_arch[0] || (flags & OF_FORCE_BASENAME))
    {
      strcpy (fname, basename2 (requested_fname));
      sprintf (requested_fname, "%s%s", ucon64.output_path, fname);
    }
  else                                          // an archive (for now: zip file)
    sprintf (requested_fname, "%s%s", ucon64.output_path, ucon64.fname_arch);

  /*
    Keep the requested suffix, but only if it isn't ".zip" or ".gz". This
    because we don't write to zip or gzip files. Otherwise the output
    file would have the suffix ".zip" or ".gz" while it isn't a zip or gzip
    file. uCON64 handles such files correctly, because it looks at the file
    data itself, but many programs don't.
    If the flag OF_FORCE_SUFFIX was used we keep the suffix, even if it's
    ".zip" or ".gz". Now ucon64_output_fname() can be used when renaming/moving
    files.
  */
  if (!(flags & OF_FORCE_SUFFIX) &&
      !(stricmp (suffix, ".zip") && stricmp (suffix, ".gz")))
    strcpy (suffix, ".tmp");
  set_suffix (requested_fname, suffix);
  return requested_fname;
}


#if 1
int
ucon64_testpad (const char *filename)
/*
  Test if EOF is padded (repeated byte values)
  This (new) version is not efficient for uncompressed files, but *much* more
  efficient for compressed files. For example (a bad case), on a Celeron 850
  just viewing info about a zipped dump of Mario Party (U) takes more than 3
  minutes when the old version of ucon64_testpad() is used. A gzipped dump
  can take more than 6 minutes. With this version it takes about 9 seconds for
  the zipped dump and 12 seconds for the gzipped dump.
*/
{
  int c = 0, blocksize, i, n = 0, start_n;
  unsigned char buffer[MAXBUFSIZE];
  FILE *file = fopen (filename, "rb");

  if (!file)
    return -1;

  while ((blocksize = fread (buffer, 1, MAXBUFSIZE, file)) != 0)
    {
      if (buffer[blocksize - 1] != c)
        {
          c = buffer[blocksize - 1];
          n = 0;
        }
      start_n = n;
      for (i = blocksize - 1; i >= 0; i--)
        {
          if (buffer[i] != c)
            {
              n -= start_n;
              break;
            }
          else
            {
              /*
                A file is either padded with 2 or more bytes or it isn't
                padded at all. It can't be detected that a file is padded with
                1 byte.
              */
              if (i == blocksize - 2)
                n += 2;
              else if (i < blocksize - 2)
                n++;
              // NOT else, because i == blocksize - 1 must initially be skipped
            }
        }
    }

  fclose (file);
  return n;
}
#else
int
ucon64_testpad (const char *filename)
// test if EOF is padded (repeating bytes)
{
  int pos = ucon64.file_size - 1, buf_pos = pos % MAXBUFSIZE,
      c = ucon64_fgetc (filename, pos);
  unsigned char buf[MAXBUFSIZE];
  FILE *fh = fopen (filename, "rb");

  if (!fh)
    return -1;

  for (pos -= buf_pos; !fseek (fh, pos, SEEK_SET) && pos > -1;
       pos -= MAXBUFSIZE, buf_pos = MAXBUFSIZE)
    {
      fread (buf, 1, buf_pos, fh);

      for (; buf_pos > 0; buf_pos--)
        if (buf[buf_pos - 1] != c)
          {
            fclose (fh);

            return ucon64.file_size - (pos + buf_pos) > 1 ?
              ucon64.file_size - (pos + buf_pos) : 0;
          }
    }

  fclose (fh);

  return ucon64.file_size;                      // the whole file is "padded"
}
#endif


int
ucon64_gauge (time_t start_time, int pos, int size)
{
  int bps, percentage, col1, col2;

  if (pos > size || !size)
    return -1;

  percentage = misc_percent (pos, size);

  if (ucon64.frontend)
    {
      fprintf (stdout, "%u\n", percentage);
      fflush (stdout);

      return 0;
    }

  fprintf (stdout, "\r%10d Bytes [", pos);

#ifdef  USE_ANSI_COLOR
  if (ucon64.ansi_color)
    {
      col1 = 1;
      col2 = 2;
    }
  else
#endif
    {
      col1 = -1;
      col2 = -1;
    }
  gauge (percentage, 22, '=', '-', col1, col2);

  bps = bytes_per_second (start_time, pos);
  fprintf (stdout, "] %d%%, BPS=%d, ", percentage, bps);

  if (pos == size)
    {
      int curr = (int) difftime (time (0), start_time);
      // "round up" to at least 1 sec (to be consistent with ETA)
      if (curr < 1)
        curr = 1;
      fprintf (stdout, "TOTAL=%02d:%02d", curr / 60, curr % 60);
    }
  else if (pos)
    {
      int left = (size - pos) / MAX (bps, 1);
      fprintf (stdout, "ETA=%02d:%02d  ", left / 60, left % 60);
    }
  else                                          // don't display a nonsense ETA
    fputs ("ETA=?  ", stdout);

  fflush (stdout);

  return 0;
}


int
ucon64_testsplit (const char *filename, int (*testsplit_cb) (const char *))
// test if ROM is split into parts based on the name of files
{
  int x, parts = 0, l;
  char buf[FILENAME_MAX], *p = NULL;

  for (x = -1; x < 2; x += 2)
    {
      parts = 0;
      strcpy (buf, filename);
      p = strrchr (buf, '.');
      l = strlen (buf);

      if (p == NULL)                            // filename doesn't contain a period
        p = buf + l - 1;
      else
        p += x;                                 // if x == -1 change char before '.'
                                                //  else if x == 1 change char after '.'
      if (buf > p ||                            // filename starts with '.' (x == -1)
          p - buf > l - 1)                      // filename ends with '.' (x == 1)
        continue;

      while (!access (buf, F_OK))
        (*p)--;                                 // "rewind" (find the first part)
      (*p)++;

      while (!access (buf, F_OK))               // count split parts
        {
          if (testsplit_cb)
            testsplit_cb (buf);
          (*p)++;
          parts++;
        }

      if (parts > 1)
        return parts;
    }

  return 0;
}


int
ucon64_set_property_array (void)
{
  const st_property_t props[] =
    {
      {
        "backups", "1",
        "create backups of files? (1=yes; 0=no)\n"
        "before processing a ROM uCON64 will make a backup of it"
      },
      {
        "ansi_color", "1",
        "use ANSI colors in output? (1=yes; 0=no)"
      },
#ifdef  USE_PPDEV
      {
        "parport_dev", "/dev/parport0",
        "parallel port"
      },
#elif   defined AMIGA
      {
        "parport_dev", "parallel.device",
        "parallel port"
      },
      {
        "parport", "0",
        NULL
      },
#else
      {
        "parport", "378",
        "parallel port"
      },
#endif
#ifdef  USE_USB
#endif
      {
        "discmage_path",
#if     defined __MSDOS__
        PROPERTY_MODE_DIR ("ucon64") "discmage.dxe",
#elif   defined __CYGWIN__ || defined _WIN32
        PROPERTY_MODE_DIR ("ucon64") "discmage.dll",
#elif   defined __APPLE__                       // Mac OS X actually
        PROPERTY_MODE_DIR ("ucon64") "discmage.dylib",
#elif   defined __unix__ || defined __BEOS__
        PROPERTY_MODE_DIR ("ucon64") "discmage.so",
#else
        "",
#endif
        "complete path to the discmage library for DISC image support"
      },
      {
        "ucon64_configdir",
        PROPERTY_MODE_DIR ("ucon64"),
        "directory with additional config files"
      },
      {
        "ucon64_datdir",
        PROPERTY_MODE_DIR ("ucon64/dat"),
        "directory with DAT files"
      },
      {
        "f2afirmware", "f2afirm.hex",
        "F2A support files\n"
        "path to F2A USB firmware"
      },
      {
        "iclientu", "iclientu.bin",
        "path to GBA client binary (for USB code)"
      },
      {
        "iclientp", "iclientp.bin",
        "path to GBA client binary (for parallel port code)"
      },
      {
        "ilogo", "ilogo.bin",
        "path to iLinker logo file"
      },
      {
        "gbaloader", "loader.bin",
        "path to GBA multi-game loader"
      },
      {
        "gbaloader_sc", "sc_menu.bin",
        "path to GBA multi-game loader (Super Card)"
      },
      {
        "emulate_" UCON64_3DO_S,      "",
        "emulate_<console shortcut>=<emulator with options>\n\n"
        "You can also use CRC32 values for ROM specific emulation options:\n\n"
        "emulate_0x<crc32>=<emulator with options>\n"
        "emulate_<crc32>=<emulator with options>"
      },
      {"emulate_" UCON64_ATA_S,      "", NULL},
      {"emulate_" UCON64_CD32_S,     "", NULL},
      {"emulate_" UCON64_CDI_S,      "", NULL},
      {"emulate_" UCON64_COLECO_S,   "", NULL},
      {"emulate_" UCON64_DC_S,       "", NULL},
      {"emulate_" UCON64_GB_S,       "vgb -sound -sync 50 -sgb -scale 2", NULL},
      {"emulate_" UCON64_GBA_S,      "vgba -scale 2 -uperiod 6", NULL},
      {"emulate_" UCON64_GC_S,       "", NULL},
      {"emulate_" UCON64_GEN_S,      "dgen -f -S 2", NULL},
      {"emulate_" UCON64_INTELLI_S,  "", NULL},
      {"emulate_" UCON64_JAG_S,      "", NULL},
      {"emulate_" UCON64_LYNX_S,     "", NULL},
      {"emulate_" UCON64_ARCADE_S,   "", NULL},
      {"emulate_" UCON64_N64_S,      "", NULL},
      {"emulate_" UCON64_NES_S,      "tuxnes -E2 -rx11 -v -s/dev/dsp -R44100", NULL},
      {"emulate_" UCON64_NG_S,       "", NULL},
      {"emulate_" UCON64_NGP_S,      "", NULL},
      {"emulate_" UCON64_PCE_S,      "", NULL},
      {"emulate_" UCON64_PS2_S,      "", NULL},
      {"emulate_" UCON64_PSX_S,      "pcsx", NULL},
      {"emulate_" UCON64_S16_S,      "", NULL},
      {"emulate_" UCON64_SAT_S,      "", NULL},
      {"emulate_" UCON64_SMS_S,      "", NULL},
      {"emulate_" UCON64_GAMEGEAR_S, "", NULL},
      {"emulate_" UCON64_SNES_S,     "snes9x -tr -sc -hires -dfr -r 7 -is -joymap1 2 3 5 0 4 7 6 1", NULL},
      {"emulate_" UCON64_SWAN_S,     "", NULL},
      {"emulate_" UCON64_VBOY_S,     "", NULL},
      {"emulate_" UCON64_VEC_S,      "", NULL},
      {"emulate_" UCON64_XBOX_S,     "", NULL},
      {NULL, NULL, NULL}
    };

  return set_property_array (ucon64.configfile, props);
}


static inline char *
to_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = (char) func (*p);

  return s;
}


int
ucon64_rename (int mode)
{
#define SUFFIX_MAX 80
  char buf[FILENAME_MAX], buf2[FILENAME_MAX], suffix[SUFFIX_MAX];
  const char *p, *p2;
  unsigned int crc = 0;
  int good_name;

  *buf = 0;
  strncpy (suffix, get_suffix (ucon64.fname), SUFFIX_MAX)[SUFFIX_MAX - 1] = 0; // in case suffix is >= SUFFIX_MAX chars

  switch (mode)
    {
    case UCON64_RROM:
      if (ucon64.nfo && ucon64.nfo->name)
        {
          strcpy (buf, ucon64.nfo->name);
          strtriml (strtrimr (buf));
        }
      break;

    case UCON64_RDAT:                           // GoodXXXX style rename
      if (ucon64.dat && ((st_ucon64_dat_t *) ucon64.dat)->fname)
        {
          p = (char *) get_suffix (((st_ucon64_dat_t *) ucon64.dat)->fname);
          strcpy (buf, ((st_ucon64_dat_t *) ucon64.dat)->fname);

          // get_suffix() never returns NULL
          if (p[0] && strlen (p) < 5)
            if (!(stricmp (p, ".nes") &&        // NES
                  stricmp (p, ".fds") &&        // NES FDS
                  stricmp (p, ".gb") &&         // Game Boy
                  stricmp (p, ".gbc") &&        // Game Boy Color
                  stricmp (p, ".gba") &&        // Game Boy Advance
                  stricmp (p, ".smc") &&        // SNES
                  stricmp (p, ".sc") &&         // Sega Master System
                  stricmp (p, ".sg") &&         // Sega Master System
                  stricmp (p, ".sms") &&        // Sega Master System
                  stricmp (p, ".gg") &&         // Game Gear
                  stricmp (p, ".smd") &&        // Genesis
                  stricmp (p, ".v64")))         // Nintendo 64
              buf[strlen (buf) - strlen (p)] = 0;
        }
      break;

    case UCON64_RJOLIET:
      /*
        We *have* to look at the structure of the file name, i.e., handle
        "base name" and suffix differently. This is necessary, because it's
        usual that file names are identified by their suffix (especially on
        Windows).
        In order to be able to say that the base name and/or the suffix is too
        long, we have to specify a maximum length for both. We chose maximum
        lengths for the base name and the suffix of 48 and 16 characters
        respectively. These are arbitrary limits of course. The limits could
        just as well have been 60 and 4 characters. Note that the boundary will
        be adjusted if only one part is too long.
        If either the base name or the suffix is too long, we replace the last
        three characters of the base name with the three most significant
        digits of the CRC32 value of the full name.
      */
      {
        int len, len2;
        p = basename2 (ucon64.fname);
        len = strlen (p);               // it's safe to assume that len is <= FILENAME_MAX
        if (len <= 64)                  // Joliet maximum file name length is 64 chars
          {
            printf ("Skipping \"%s\"\n", p);
            return 0;
          }
        strcpy (buf, p);
        crc = crc32 (0, (unsigned char *) buf, len);
        len2 = strlen (suffix);
        len -= len2;
        if (len2 <= 16)                 // len > 48
          len = 64 - len2 - 3;
        else                            // len2 > 16
          {
            if (len <= 48 - 3)
              len2 = 64 - len - 3;
            else                        // len > 48 - 3
              {
                len = 48 - 3;
                len2 = 16;
              }
            suffix[len2] = 0;
          }
        // NOTE: The implementation of snprintf() in glibc 2.3.5-10 (FC4)
        //  terminates the string. So, a size argument of 4 results in 3
        //  characters plus a string terminator.
        snprintf (buf + len, 4, "%0x", crc);
        buf[len + 3] = 0;
      }
      break;

    case UCON64_R83:
      /*
        The code for handling "FAT" file names is similar to the code that
        handles Joliet file names, except that the maximum lengths for base
        name and suffix are fixed (8 and 4 respectively).
        Note that FAT is quoted, as this code mainly limits the file name
        length. It doesn't guarantee that the file name is correct for FAT file
        systems. For example, a file with a name with a leading period (not a
        valid file name on a FAT file system) doesn't get special treatment.
      */
      {
        int len, len2;
        p = basename2 (ucon64.fname);
        len = strlen (p);               // it's safe to assume that len is <= FILENAME_MAX
        strcpy (buf, p);
        crc = crc32 (0, (unsigned char *) buf, len);
        len2 = strlen (suffix);
        len -= len2;
        if (len <= 8 && len2 <= 4)      // FAT maximum file name length is 8 + 4 chars
          {                             //  (we include the period with the suffix)
            printf ("Skipping \"%s\"\n", p);
            return 0;
          }
        if (len > 8 - 3)
          len = 8 - 3;
        if (len2 > 4)
          suffix[4] = 0;
        snprintf (buf + len, 4, "%0x", crc);
        buf[len + 3] = 0;
      }
      break;

    default:
      return 0;                                 // invalid mode
    }

  if (!buf[0])
    return 0;

  // replace chars the fs might not like
  strcpy (buf2, to_func (buf, strlen (buf), tofname));
  strcpy (buf, basename2 (ucon64.fname));

  p = (char *) get_suffix (buf);
  // Remove the suffix from buf (ucon64.fname). Note that this isn't fool-proof.
  //  However, this is the best solution, because several DAT files contain
  //  "canonical" file names with a suffix. That is a STUPID bug.
  if (p)
    buf[strlen (buf) - strlen (p)] = 0;

#ifdef  DEBUG
//  printf ("buf: \"%s\"; buf2: \"%s\"\n", buf, buf2);
#endif
  if (!strcmp (buf, buf2))
    // also process files with a correct name, so that -rename can be used to
    //  "weed" out good dumps when -o is used (like GoodXXXX without inplace
    //  command)
    good_name = 1;
  else
    {
      // Another test if the file already has a correct name. This is necessary
      //  for files without a "normal" suffix (e.g. ".smc"). Take for example a
      //  name like "Final Fantasy III (V1.1) (U) [!]".
      strcat (buf, suffix);
      if (!strcmp (buf, buf2))
        {
          good_name = 1;
          suffix[0] = 0;                        // discard "suffix" (part after period)
        }
      else
        good_name = 0;
    }

  // DON'T use set_suffix()! Consider file names (in the DAT file) like
  //  "Final Fantasy III (V1.1) (U) [!]". The suffix is ".1) (U) [!]"...
  strcat (buf2, suffix);

  if (mode == UCON64_R83)
    buf2[12] = 0;

  ucon64_output_fname (buf2, OF_FORCE_BASENAME | OF_FORCE_SUFFIX);

  p = basename2 (ucon64.fname);
  p2 = basename2 (buf2);

  if (one_file (ucon64.fname, buf2) && !strcmp (p, p2))
    {                                           // skip only if the letter case
      printf ("Skipping \"%s\"\n", p);          //  also matches (Windows...)
      return 0;
    }

  if (!good_name)
    /*
      Note that the previous statement causes whatever file is present in the
      dir specified with -o (or the current dir) to be overwritten (if the file
      already has a correct name). This seems bad, but is actually better than
      making a backup. It isn't so bad, because the file that gets overwritten
      is either the same as the file it is overwritten with or doesn't deserve
      its name.
      Without this statement repeating a rename action for already renamed
      files would result in a real mess. And I (dbjh) mean a *real* mess...
    */
    if (!access (buf2, F_OK) && !strcmp (p, p2)) // a file with that name exists already?
      ucon64_file_handler (buf2, NULL, OF_FORCE_BASENAME | OF_FORCE_SUFFIX);

  if (!good_name)
    printf ("Renaming \"%s\" to \"%s\"\n", p, p2);
  else
    printf ("Moving \"%s\"\n", p);
#ifndef DEBUG
  if (rename2 (ucon64.fname, buf2) == -1)       // rename_2_() must be used!
    {
      fprintf (stderr, "ERROR: Could not rename \"%s\"\n", p);
      return -1;
    }
#endif
#ifdef  USE_ZLIB
  unzip_current_file_nr = 0x7fffffff - 1;       // dirty hack
#endif
  return 0;
}


int
ucon64_e (void)
{
  int result = 0, x = 0;
  char buf[MAXBUFSIZE], name[MAXBUFSIZE];
  const char *value_p = NULL;
  typedef struct
  {
    int id;
    const char *s;
  } st_strings_t;
  st_strings_t s[] =
    {
      {UCON64_3DO,      "emulate_" UCON64_3DO_S},
      {UCON64_ATA,      "emulate_" UCON64_ATA_S},
      {UCON64_CD32,     "emulate_" UCON64_CD32_S},
      {UCON64_CDI,      "emulate_" UCON64_CDI_S},
      {UCON64_COLECO,   "emulate_" UCON64_COLECO_S},
      {UCON64_DC,       "emulate_" UCON64_DC_S},
      {UCON64_GB,       "emulate_" UCON64_GB_S},
      {UCON64_GBA,      "emulate_" UCON64_GBA_S},
      {UCON64_GC,       "emulate_" UCON64_GC_S},
      {UCON64_GEN,      "emulate_" UCON64_GEN_S},
      {UCON64_GP32,     "emulate_" UCON64_GP32_S},
      {UCON64_INTELLI,  "emulate_" UCON64_INTELLI_S},
      {UCON64_JAG,      "emulate_" UCON64_JAG_S},
      {UCON64_LYNX,     "emulate_" UCON64_LYNX_S},
      {UCON64_ARCADE,   "emulate_" UCON64_ARCADE_S},
      {UCON64_N64,      "emulate_" UCON64_N64_S},
      {UCON64_NDS,      "emulate_" UCON64_NDS_S},
      {UCON64_NES,      "emulate_" UCON64_NES_S},
      {UCON64_NG,       "emulate_" UCON64_NG_S},
      {UCON64_NGP,      "emulate_" UCON64_NGP_S},
      {UCON64_PCE,      "emulate_" UCON64_PCE_S},
      {UCON64_PS2,      "emulate_" UCON64_PS2_S},
      {UCON64_PSX,      "emulate_" UCON64_PSX_S},
      {UCON64_S16,      "emulate_" UCON64_S16_S},
      {UCON64_SAT,      "emulate_" UCON64_SAT_S},
      {UCON64_SMS,      "emulate_" UCON64_SMS_S},
      {UCON64_GAMEGEAR, "emulate_" UCON64_GAMEGEAR_S},
      {UCON64_SNES,     "emulate_" UCON64_SNES_S},
      {UCON64_SWAN,     "emulate_" UCON64_SWAN_S},
      {UCON64_VBOY,     "emulate_" UCON64_VBOY_S},
      {UCON64_VEC,      "emulate_" UCON64_VEC_S},
      {UCON64_XBOX,     "emulate_" UCON64_XBOX_S},
      {0,               NULL}
    };

  if (access (ucon64.configfile, F_OK) != 0)
    {
      fprintf (stderr, "ERROR: %s does not exist\n", ucon64.configfile);
      return -1;
    }

  sprintf (name, "emulate_%08x", ucon64.crc32); // look for emulate_<crc32>
  value_p = get_property (ucon64.configfile, name, PROPERTY_MODE_TEXT);

  if (value_p == NULL)
    {
      sprintf (name, "emulate_0x%08x", ucon64.crc32); // look for emulate_0x<crc32>
      value_p = get_property (ucon64.configfile, name, PROPERTY_MODE_TEXT);
    }

  if (value_p == NULL)
    for (x = 0; s[x].s; x++)
      if (s[x].id == ucon64.console)
        {
          value_p = get_property (ucon64.configfile, s[x].s, PROPERTY_MODE_TEXT);
          break;
        }

  if (value_p == NULL)
    {
      fprintf (stderr, "ERROR: Could not find the correct settings (%s) in\n"
               "       %s\n"
               "TIP:   If the wrong console was detected you might try to force recognition\n"
               "       The force recognition option for SNES would be " OPTION_LONG_S "snes\n",
               name, ucon64.configfile);
      return -1;
    }

  sprintf (buf, "%s \"%s\"", value_p, ucon64.fname);

  puts (buf);
  fflush (stdout);

  result = system (buf)
#if     !(defined __MSDOS__ || defined _WIN32)
           >> 8                                 // the exit code is coded in bits 8-15
#endif                                          //  (does not apply to DJGPP, MinGW & VC++)
           ;

#if 1
  // Snes9x (Linux) for example returns a non-zero value on a normal exit
  //  (3)...
  // on WinDOS, system() immediately returns with exit code 0 when starting a
  //  Windows executable (as if fork() was called) it also returns 0 when the
  //  exe could not be started
  if (result != 127 && result != -1 && result != 0)        // 127 && -1 are system() errors, rest are exit codes
    {
      fprintf (stderr, "ERROR: The emulator returned an error (?) code: %d\n"
                       "TIP:   If the wrong emulator was used you might try to force recognition\n"
                       "       The force recognition option for SNES would be " OPTION_LONG_S "snes\n",
               result);
    }
#endif
  return result;
}


#define PATTERN_BUFSIZE (64 * 1024)
int
ucon64_pattern (const char *pattern_fname)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX],
       buffer[PATTERN_BUFSIZE];
  FILE *srcfile, *destfile;
  int bytesread = 0, totalbytesread = 0, n, n_found = 0, n_patterns,
      overlap = 0, effective_overlap = 0;
  st_cm_pattern_t *patterns = NULL;

  realpath2 (pattern_fname, src_name);
  // First try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    sprintf (src_name, "%s" DIR_SEPARATOR_S "%s", ucon64.configdir, pattern_fname);
  n_patterns = build_cm_patterns (&patterns, src_name, ucon64.quiet == -1 ? 1 : 0);
  if (n_patterns == 0)
    {
      fprintf (stderr, "ERROR: No patterns found in %s\n", src_name);
      cleanup_cm_patterns (&patterns, n_patterns);
      return -1;
    }
  else if (n_patterns < 0)
    {
      char dir1[FILENAME_MAX], dir2[FILENAME_MAX];
      dirname2 (pattern_fname, dir1);
      dirname2 (src_name, dir2);

      fprintf (stderr, "ERROR: Could not read from %s, not in %s nor in %s\n",
               basename2 (pattern_fname), dir1, dir2);
      // when build_cm_patterns() returns -1, cleanup_cm_patterns() should not be called
      return -1;
    }

  printf ("Found %d pattern%s in %s\n", n_patterns, n_patterns != 1 ? "s" : "", src_name);

  for (n = 0; n < n_patterns; n++)
    {
      if (patterns[n].search_size > overlap)
        {
          overlap = patterns[n].search_size;
          if (overlap > PATTERN_BUFSIZE)
            {
              fprintf (stderr,
                       "ERROR: Pattern %d is too large, specify a shorter pattern\n",
                       n + 1);
              cleanup_cm_patterns (&patterns, n_patterns);
              return -1;
            }
        }

      if (patterns[n].offset <= -patterns[n].search_size || patterns[n].offset > 0)
        printf ("WARNING: The offset of pattern %d falls outside the search pattern.\n"
                "         This can cause matches to be ignored with the current implementation\n"
                "         of --pattern. Please consider enlarging the search pattern\n",
                n + 1);
    }
  overlap--;

  puts ("Searching for patterns...");

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }

  while ((bytesread = fread (buffer + effective_overlap, 1,
                             PATTERN_BUFSIZE - effective_overlap, srcfile)) != 0)
    {
      for (n = 0; n < n_patterns; n++)
        {
          int search_overlap = patterns[n].search_size - 1;
          char *buffer_start = buffer + (totalbytesread > 0 ? overlap - search_overlap : 0);
          int buffer_size = bytesread + (totalbytesread > 0 ? search_overlap : 0);

          n_found += change_mem2 (buffer_start, buffer_size, patterns[n].search,
                                  patterns[n].search_size, patterns[n].wildcard,
                                  patterns[n].escape, patterns[n].replace,
                                  patterns[n].replace_size, patterns[n].offset,
                                  patterns[n].sets);
        }
      fwrite (buffer + effective_overlap, 1, bytesread, destfile);

      totalbytesread += bytesread;
      if (totalbytesread < ucon64.file_size)
        memmove (buffer, buffer + effective_overlap - overlap + bytesread, overlap);
      effective_overlap = overlap;
    }

  fclose (srcfile);
  fclose (destfile);
  cleanup_cm_patterns (&patterns, n_patterns);

  printf ("Found %d pattern%s\n", n_found, n_found != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return n_found;
}
#undef PATTERN_BUFSIZE


int
ucon64_bswap16_n (void *buffer, int n)
// bswap16() n bytes of buffer
{
  int i = n;
  uint16_t *w = (uint16_t *) buffer;

  for (; i > 1; i -= 2, w++)
    *w = bswap_16 (*w);

  return n;                                     // return # of bytes swapped
}


static inline int
ucon64_fbswap16_func (void *buffer, int n, void *object)
// bswap16() n bytes of buffer
{
  (void) object;
  return ucon64_bswap16_n (buffer, n);
}


static inline int
ucon64_fwswap32_func (void *buffer, int n, void *object)
// wswap32() n/2 words of buffer
{
  int i = n;
  uint32_t *l = (uint32_t *) buffer;
  (void) object;

  i >>= 1;                                      // # words = # bytes / 2
  for (; i > 1; i -= 2, l++)
    *l = wswap_32 (*l);

  return n;                                     // return # of bytes swapped
}


void
ucon64_fbswap16 (const char *fname, size_t start, size_t len)
{
  quick_io_func (ucon64_fbswap16_func, MAXBUFSIZE, NULL, start, len, fname, "r+b");
}


void
ucon64_fwswap32 (const char *fname, size_t start, size_t len)
{
  quick_io_func (ucon64_fwswap32_func, MAXBUFSIZE, NULL, start, len, fname, "r+b");
}


typedef struct
{
  FILE *output;
  int virtual_pos;
  uint32_t flags;
} st_ucon64_dump_t;


static inline int
ucon64_dump_func (void *buffer, int n, void *object)
{
  st_ucon64_dump_t *o = (st_ucon64_dump_t *) object;

  dumper (o->output, buffer, n, o->virtual_pos, o->flags);
  o->virtual_pos += n;

  return n;
}


void
ucon64_dump (FILE *output, const char *filename, size_t start, size_t len,
             uint32_t flags)
{
  st_ucon64_dump_t o;
  o.output = output;
  o.virtual_pos = start;
  o.flags = flags;

  quick_io_func (ucon64_dump_func, MAXBUFSIZE, &o, start, len, filename, "rb");
}


typedef struct
{
  const void *search;
  uint32_t flags;
  int searchlen;
  int pos;
  int found;
} st_ucon64_find_t;


static inline int
ucon64_find_func (void *buffer, int n, void *object)
{
  st_ucon64_find_t *o = (st_ucon64_find_t *) object;
  char *ptr0 = (char *) buffer, *ptr1 = (char *) buffer;
  int m;
  static char match[MAXBUFSIZE - 1], compare[MAXBUFSIZE + 16 + 1];
  static int matchlen;

  // reset matchlen if this is the first call for a new file
  if (o->found == -2)
    {
      o->found = -1;                            // -1 is default (return) value
      matchlen = 0;
    }

  // check if we can match the search string across the buffer boundary
  for (m = 0; matchlen; matchlen--)
    {
      memcpy (compare, match + m++, matchlen);
      memcpy (compare + matchlen, ptr1, ((o->searchlen + 0x0f) & ~0x0f) - matchlen);
      if (memcmp2 (compare, o->search, o->searchlen, o->flags) == 0)
        {
          o->found = o->pos - matchlen;
          if (!(o->flags & UCON64_FIND_QUIET))
            {
              dumper (stdout, compare,
                MIN ((o->searchlen + 0x0f) & ~0x0f, n - (ptr1 - ptr0 + 1)),
                o->found, DUMPER_HEX);
              fputc ('\n', stdout);
            }
        }
    }

  while (ptr1 - ptr0 < n)
    {
      ptr1 = (char *) memmem2 (ptr1, n - (ptr1 - ptr0), o->search, o->searchlen,
                               o->flags);
      if (ptr1)
        {
          o->found = o->pos + ptr1 - ptr0;
          if (!(o->flags & UCON64_FIND_QUIET))
            {
              dumper (stdout, ptr1,
                MIN ((o->searchlen + 0x0f) & ~0x0f, n - (ptr1 - ptr0 + 1)),
                o->found, DUMPER_HEX);
              fputc ('\n', stdout);
            }
          ptr1++;
        }
      else
        {
          // try to find a partial match at the end of buffer
          ptr1 = ptr0 + n - o->searchlen;
          for (m = 1; m < o->searchlen; m++)
            if (memcmp2 (ptr1 + m, o->search, o->searchlen - m, o->flags) == 0)
              {
                memcpy (match, ptr1 + m, o->searchlen - m);
                matchlen = o->searchlen - m;
                break;
              }
          if (!matchlen)                          // && o->flags & MEMMEM2_REL
            {
              match[0] = ptr0[n - 1];             // we must not split the string
              matchlen = 1;                       //  for a relative search
            }
          break;
        }
    }

  o->pos += n;
  return n;
}


int
ucon64_find (const char *filename, size_t start, size_t len,
             const char *search, int searchlen, uint32_t flags)
{
  st_ucon64_find_t o;
  o.search = search;
  o.flags = flags;
  o.searchlen = searchlen;
  o.pos = start;
  o.found = -2;
  // o.found == -2 signifies a new find operation (usually for a new file)

  if (searchlen < 1)
    {
      fprintf (stderr, "ERROR: No search string specified\n");
      exit (1);
    }
  else if (flags & MEMCMP2_REL)
    if (searchlen < 2)
      {
        fprintf (stderr, "ERROR: Search string must be longer than 1 character for a relative search\n");
        exit (1);
      }
  if (searchlen > MAXBUFSIZE)
    {
      fprintf (stderr, "ERROR: Search string must be <= %d characters\n", MAXBUFSIZE);
      exit (1);                                 // see ucon64_find_func() for why
    }

  if (!(flags & UCON64_FIND_QUIET))
    {
      fputs (basename2 (filename), stdout);
      if (ucon64.fname_arch[0])
        printf (" (%s)\n", basename2 (ucon64.fname_arch));
      else
        fputc ('\n', stdout);

    // TODO: display "b?a" as "b" "a"
    if (!(flags & (MEMCMP2_CASE | MEMCMP2_REL)))
      printf ("Searching: \"%s\"\n\n", search);
    else if (flags & MEMCMP2_CASE)
      printf ("Case insensitive searching: \"%s\"\n\n", search);
    else if (flags & MEMCMP2_REL)
      {
        char *p = (char *) search;

        printf ("Relative searching: \"%s\"\n\n", search);
        for (; *(p + 1); p++)
          printf ("'%c' - '%c' = %d\n", *p, *(p + 1), *p - *(p + 1));
        printf ("\n");
      }
    }

  quick_io_func (ucon64_find_func, MAXBUFSIZE, &o, start, len, filename, "rb");

  return o.found;                               // return last occurrence or -1
}


typedef struct
{
  s_sha1_ctx_t *m_sha1;
  s_md5_ctx_t *m_md5;
//  uint16_t *crc16;
  unsigned int *crc32;
} st_ucon64_chksum_t;


static inline int
ucon64_chksum_func (void *buffer, int n, void *object)
{
  st_ucon64_chksum_t *o = (st_ucon64_chksum_t *) object;

  if (o->m_sha1)
    sha1 (o->m_sha1, (const unsigned char *) buffer, n);

  if (o->m_md5)
    md5_update (o->m_md5, (unsigned char *) buffer, n);

//  if (o->crc16)
//    *(o->crc16) = crc16 (*(o->crc16), (const unsigned char *) buffer, n);

  if (o->crc32)
    *(o->crc32) = crc32 (*(o->crc32), (const unsigned char *) buffer, n);

  return n;
}


int
ucon64_chksum (char *sha1_s, char *md5_s, unsigned int *crc32_i, // uint16_t *crc16_i,
               const char *filename, int file_size, size_t start)
{
  int i = 0, result;
  s_sha1_ctx_t m_sha1;
  s_md5_ctx_t m_md5;
  st_ucon64_chksum_t o;

  memset (&o, 0, sizeof (st_ucon64_chksum_t));

  if (sha1_s)
    sha1_begin (o.m_sha1 = &m_sha1);

  if (md5_s)
    md5_init (o.m_md5 = &m_md5, 0);

//  if (crc16_i)
//    o.crc16 = crc16_i;

  if (crc32_i)
    o.crc32 = crc32_i;

  result = quick_io_func (ucon64_chksum_func, MAXBUFSIZE, &o, start,
                          file_size - start, filename, "rb");

  if (sha1_s)
    {
      unsigned char buf[MAXBUFSIZE];

      sha1_end (buf, &m_sha1);
      for (*sha1_s = 0, i = 0; i < 20; i++, sha1_s = strchr (sha1_s, 0))
        sprintf (sha1_s, "%02x", buf[i] & 0xff);
    }

  if (md5_s)
    {
      md5_final (&m_md5);
      for (*md5_s = 0, i = 0; i < 16; i++, md5_s = strchr (md5_s, 0))
        sprintf (md5_s, "%02x", m_md5.digest[i]);
    }

//  if (crc16_i)
//    *(crc16_i) = *(o.crc16);

//  if (crc32_i)
//    *(crc32_i) = *(o.crc32);

  return result;
}


#if 0
#define FILEFILE_LARGE_BUF (1024 * 1024)


typedef struct
{
  FILE *output;
  int pos0;
  int pos;
  int similar;
  unsigned char *buffer;
  const char *fname0;
  const char *fname;
  int found;
} st_ucon64_filefile_t;


static inline int
ucon64_filefile_func (void *buffer, int n, void *object)
{
  st_ucon64_filefile_t *o = (st_ucon64_filefile_t *) object;
  int i = 0, j = 0, len = MIN (FILEFILE_LARGE_BUF, fsizeof (o->fname) - o->pos);
  char *b = (char *) buffer;

  ucon64_fread (o->buffer, o->pos, len, o->fname);

  for (; i < n; i++)
    if (o->similar == TRUE ?                    // find start
        *(b + i) == *(o->buffer + i) :
        *(b + i) != *(o->buffer + i))
      {
        for (j = 0; i + j < n; j++)
          if (o->similar == TRUE ?              // find end (len)
              *(b + i + j) != *(o->buffer + i + j) :
              *(b + i + j) == *(o->buffer + i + j))
            break;

        fprintf (o->output, "%s:\n", o->fname0);
        dumper (o->output, &b[i], j, o->pos0 + i, DUMPER_HEX);

        fprintf (o->output, "%s:\n", o->fname);
        dumper (o->output, &o->buffer[i], j, o->pos + i, DUMPER_HEX);

        fputc ('\n', o->output);

        i += j;
        o->found++;
      }

  return n;
}


void
ucon64_filefile (const char *filename1, int start1, const char *filename2,
                 int start2, int similar)
{
  st_ucon64_filefile_t o;

  printf ("Comparing %s", basename2 (ucon64.fname));
  if (ucon64.fname_arch[0])
    printf (" (%s)", basename2 (ucon64.fname_arch));
  printf (" with %s\n", filename1);

  if (one_file (filename1, filename2))
    {
      printf ("%s and %s refer to one file\n", filename1, filename2);
      return;
    }

  if (fsizeof (filename1) < start1 || fsizeof (filename2) < start2)
    return;

  if (!(o.buffer = (unsigned char *) malloc (FILEFILE_LARGE_BUF)))
    {
      fputs ("ERROR: File not found/out of memory\n", stderr);
      return;                            // it's logical to stop for this file
    }

  o.fname0 = filename1;
  o.pos0 = start1;

  o.fname = filename2;
  o.pos = start2;
  o.output = stdout;
  o.similar = similar;

  o.found = 0;

  quick_io_func (ucon64_filefile_func, FILEFILE_LARGE_BUF, &o, start1,
                 fsizeof (filename1), filename1, "rb");

  if (o.found)
    printf ("Found %d %s\n",
      o.found,
      similar ? (o.found == 1 ? "similarity" : "similarities") :
                (o.found == 1 ? "difference" : "differences"));
}
#else
#define FILEFILE_LARGE_BUF
// When verifying if the code produces the same output when FILEFILE_LARGE_BUF
//  is defined as when it's not, be sure to use the same buffer size
void
ucon64_filefile (const char *filename1, int start1, const char *filename2,
                 int start2, int similar)
{
  int base, fsize1, fsize2, len, chunksize1, chunksize2, readok = 1,
      bytesread1, bytesread2, bytesleft1, bytesleft2, n_bytes = 0;
#ifdef  FILEFILE_LARGE_BUF
  int bufsize = 1024 * 1024;
  unsigned char *buf1, *buf2;
#else
  int bufsize = MAXBUFSIZE;
  unsigned char buf1[MAXBUFSIZE], buf2[MAXBUFSIZE];
#endif
  FILE *file1, *file2;

  printf ("Comparing %s", basename2 (ucon64.fname));
  if (ucon64.fname_arch[0])
    printf (" (%s)", basename2 (ucon64.fname_arch));
  printf (" with %s\n", filename1);

  if (one_file (filename1, filename2))
    {
      printf ("%s and %s refer to one file\n\n", filename1, filename2);
      return;
    }

  fsize1 = fsizeof (filename1);                 // fsizeof() returns size in bytes
  fsize2 = fsizeof (filename2);
  if (fsize1 < start1 || fsize2 < start2)
    return;

#ifdef  FILEFILE_LARGE_BUF
  if ((buf1 = (unsigned char *) malloc (bufsize)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], bufsize);
      return;
    }
  if ((buf2 = (unsigned char *) malloc (bufsize)) == NULL)
    {
      free (buf1);
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], bufsize);
      return;
    }
#endif

  if ((file1 = fopen (filename1, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename1);
#ifdef  FILEFILE_LARGE_BUF
      free (buf1);
      free (buf2);
#endif
      return ;
    }
  if ((file2 = fopen (filename2, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename2);
      fclose (file1);
#ifdef  FILEFILE_LARGE_BUF
      free (buf1);
      free (buf2);
#endif
      return;
    }

  fseek (file1, start1, SEEK_SET);
  fseek (file2, start2, SEEK_SET);
  bytesleft1 = fsize1;
  bytesread1 = 0;
  bytesleft2 = fsize2;
  bytesread2 = 0;

  while (bytesleft1 > 0 && bytesread1 < fsize2 && readok)
    {
      chunksize1 = fread (buf1, 1, bufsize, file1);
      if (chunksize1 == 0)
        readok = 0;
      else
        {
          bytesread1 += chunksize1;
          bytesleft1 -= chunksize1;
        }

      while (bytesleft2 > 0 && bytesread2 < bytesread1 && readok)
        {
          chunksize2 = fread (buf2, 1, chunksize1, file2);
          if (chunksize2 == 0)
            readok = 0;
          else
            {
              base = 0;
              while (base < chunksize2)
                {
                  if (similar == TRUE ?
                      buf1[base] == buf2[base] :
                      buf1[base] != buf2[base])
                    {
                      for (len = 0; base + len < chunksize2; len++)
                        if (similar == TRUE ?
                            buf1[base + len] != buf2[base + len] :
                            buf1[base + len] == buf2[base + len])
                          break;

                      printf ("%s:\n", filename1);
                      dumper (stdout, &buf1[base], len, start1 + base + bytesread2, DUMPER_HEX);
                      printf ("%s:\n", filename2);
                      dumper (stdout, &buf2[base], len, start2 + base + bytesread2, DUMPER_HEX);
                      fputc ('\n', stdout);
                      base += len;
                      n_bytes += len;
                    }
                  else
                    base++;
                }

              bytesread2 += chunksize2;
              bytesleft2 -= chunksize2;
            }
        }
    }

  fclose (file1);
  fclose (file2);
#ifdef  FILEFILE_LARGE_BUF
  free (buf1);
  free (buf2);
#endif

  printf ("Found %d %s\n\n",
          n_bytes,
          similar ? (n_bytes == 1 ? "similarity" : "similarities") :
                    (n_bytes == 1 ? "difference" : "differences"));

  return;
}
#endif
