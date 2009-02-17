/*
ucon64_misc.c - miscellaneous functions for uCON64

Copyright (c) 1999 - 2004 NoisyB <noisyb@gmx.net>
Copyright (c) 2001 - 2004 dbjh
Copyright (c) 2001        Caz
Copyright (c) 2002 - 2003 Jan-Erik Karlsson (Amiga)


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
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include "misc/misc.h"
#include "misc/string.h"
#include "misc/property.h"
#include "misc/bswap.h"
#include "misc/chksum.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_opts.h"
#include "ucon64_misc.h"
#include "ucon64_dat.h"
#include "console/console.h"
#include "backup/backup.h"
#include "patch/patch.h"


/*
  uCON64 "workflow" objects

  We want to do things compile-time. Using ucon64_wf is necessary for VC 6. GCC
  (3) accepts casts in struct initialisations.
*/
st_ucon64_obj_t ucon64_wf[] =
  {
    {0, WF_SWITCH},                             // WF_OBJ_ALL_SWITCH
    {0, WF_DEFAULT},                            // WF_OBJ_ALL_DEFAULT
    {0, WF_DEFAULT | WF_NO_SPLIT},              // WF_OBJ_ALL_DEFAULT_NO_SPLIT
    {0, WF_STOP},                               // WF_OBJ_ALL_STOP
    {0, WF_STOP | WF_NO_ROM},                   // WF_OBJ_ALL_STOP_NO_ROM
    {0, WF_DEFAULT | WF_STOP | WF_NO_ROM},      // WF_OBJ_ALL_DEFAULT_STOP_NO_ROM
    {0, WF_INIT},                               // WF_OBJ_ALL_INIT
    {0, WF_INIT | WF_PROBE},                    // WF_OBJ_ALL_INIT_PROBE
    {0, WF_INIT | WF_PROBE | WF_STOP},          // WF_OBJ_ALL_INIT_PROBE_STOP,
    {0, WF_INIT | WF_PROBE | WF_NO_ROM},        // WF_OBJ_ALL_INIT_PROBE_NO_ROM
    {0, WF_INIT | WF_PROBE | WF_NO_SPLIT},      // WF_OBJ_ALL_INIT_PROBE_NO_SPLIT
    {0, WF_INIT | WF_PROBE | WF_NO_CRC32},      // WF_OBJ_ALL_INIT_PROBE_NO_CRC32
    {0, WF_INIT | WF_NO_SPLIT},                 // WF_OBJ_ALL_INIT_NO_SPLIT

    {UCON64_DC, WF_SWITCH},                     // WF_OBJ_DC_SWITCH
    {UCON64_DC, WF_DEFAULT},                    // WF_OBJ_DC_DEFAULT
    {UCON64_DC, WF_NO_ROM},                     // WF_OBJ_DC_NO_ROM
    {UCON64_GB, WF_SWITCH},                     // WF_OBJ_GB_SWITCH
    {UCON64_GB, WF_DEFAULT},                    // WF_OBJ_GB_DEFAULT
    {UCON64_GBA, WF_SWITCH},                    // WF_OBJ_GBA_SWITCH
    {UCON64_GBA, WF_DEFAULT},                   // WF_OBJ_GBA_DEFAULT
    {UCON64_GEN, WF_SWITCH},                    // WF_OBJ_GEN_SWITCH
    {UCON64_GEN, WF_DEFAULT},                   // WF_OBJ_GEN_DEFAULT
    {UCON64_GEN, WF_DEFAULT | WF_NO_SPLIT},     // WF_OBJ_GEN_DEFAULT_NO_SPLIT
    {UCON64_JAG, WF_SWITCH},                    // WF_OBJ_JAG_SWITCH
    {UCON64_LYNX, WF_SWITCH},                   // WF_OBJ_LYNX_SWITCH
    {UCON64_LYNX, WF_DEFAULT},                  // WF_OBJ_LYNX_DEFAULT
    {UCON64_N64, WF_SWITCH},                    // WF_OBJ_N64_SWITCH
    {UCON64_N64, WF_DEFAULT},                   // WF_OBJ_N64_DEFAULT
    {UCON64_N64, WF_INIT | WF_PROBE},           // WF_OBJ_N64_INIT_PROBE
    {UCON64_NG, WF_SWITCH},                     // WF_OBJ_NG_SWITCH
    {UCON64_NG, WF_DEFAULT},                    // WF_OBJ_NG_DEFAULT
    {UCON64_NES, WF_SWITCH},                    // WF_OBJ_NES_SWITCH
    {UCON64_NES, WF_DEFAULT},                   // WF_OBJ_NES_DEFAULT
    {UCON64_NGP, WF_SWITCH},                    // WF_OBJ_NGP_SWITCH
    {UCON64_PCE, WF_SWITCH},                    // WF_OBJ_PCE_SWITCH
    {UCON64_PCE, WF_DEFAULT},                   // WF_OBJ_PCE_DEFAULT
    {UCON64_PSX, WF_SWITCH},                    // WF_OBJ_PSX_SWITCH
    {UCON64_SMS, WF_SWITCH},                    // WF_OBJ_SMS_SWITCH
    {UCON64_SMS, WF_DEFAULT | WF_NO_SPLIT},     // WF_OBJ_SMS_DEFAULT_NO_SPLIT
    {UCON64_SNES, WF_SWITCH},                   // WF_OBJ_SNES_SWITCH
    {UCON64_SNES, WF_DEFAULT},                  // WF_OBJ_SNES_DEFAULT
    {UCON64_SNES, WF_DEFAULT | WF_NO_SPLIT},    // WF_OBJ_SNES_DEFAULT_NO_SPLIT
    {UCON64_SNES, WF_NO_ROM},                   // WF_OBJ_SNES_NO_ROM
    {UCON64_SNES, WF_INIT | WF_PROBE},          // WF_OBJ_SNES_INIT_PROBE
    {UCON64_SWAN, WF_SWITCH},                   // WF_OBJ_SWAN_SWITCH

    {UCON64_N64, WF_STOP | WF_NO_ROM},          // WF_OBJ_N64_STOP_NO_ROM
    {UCON64_N64, WF_DEFAULT | WF_STOP},         // WF_OBJ_N64_DEFAULT_STOP
    {UCON64_N64, WF_DEFAULT | WF_STOP | WF_NO_ROM}, // WF_OBJ_N64_DEFAULT_STOP_NO_ROM
    {UCON64_GEN, WF_STOP | WF_NO_ROM},          // WF_OBJ_GEN_STOP_NO_ROM
    {UCON64_GEN, WF_DEFAULT | WF_STOP | WF_NO_SPLIT | WF_NO_ROM}, // WF_OBJ_GEN_DEFAULT_STOP_NO_SPLIT_NO_ROM
    {UCON64_GBA, WF_STOP | WF_NO_ROM},          // WF_OBJ_GBA_STOP_NO_ROM
    {UCON64_GBA, WF_DEFAULT | WF_STOP},         // WF_OBJ_GBA_DEFAULT_STOP
    {UCON64_GBA, WF_DEFAULT | WF_STOP | WF_NO_ROM}, // WF_OBJ_GBA_DEFAULT_STOP_NO_ROM
    {UCON64_SNES, WF_STOP | WF_NO_ROM},         // WF_OBJ_SNES_STOP_NO_ROM
    {UCON64_SNES, WF_DEFAULT | WF_STOP | WF_NO_ROM}, // WF_OBJ_SNES_DEFAULT_STOP_NO_ROM
    {UCON64_SNES, WF_DEFAULT | WF_STOP | WF_NO_SPLIT | WF_NO_ROM}, // WF_OBJ_SNES_DEFAULT_STOP_NO_SPLIT_NO_ROM
    {UCON64_GB, WF_STOP | WF_NO_ROM},           // WF_OBJ_GB_STOP_NO_ROM
    {UCON64_GB, WF_DEFAULT | WF_STOP | WF_NO_ROM}, // WF_OBJ_GB_DEFAULT_STOP_NO_ROM
    {UCON64_LYNX, WF_STOP | WF_NO_ROM},         // WF_OBJ_LYNX_STOP_NO_ROM
    {UCON64_PCE, WF_DEFAULT | WF_STOP | WF_NO_SPLIT | WF_NO_ROM}, // WF_OBJ_PCE_DEFAULT_STOP_NO_SPLIT_NO_ROM
    {UCON64_NGP, WF_STOP | WF_NO_ROM},          // WF_OBJ_NGP_STOP_NO_ROM
    {UCON64_NGP, WF_DEFAULT | WF_STOP | WF_NO_ROM}, // WF_OBJ_NGP_DEFAULT_STOP_NO_ROM
    {UCON64_NES, WF_STOP | WF_NO_ROM},          // WF_OBJ_NES_STOP_NO_ROM
    {UCON64_NES, WF_DEFAULT | WF_STOP | WF_NO_SPLIT}, // WF_OBJ_NES_DEFAULT_STOP_NO_SPLIT
    {UCON64_SMS, WF_STOP | WF_NO_ROM},          // WF_OBJ_SMS_STOP_NO_ROM
    {UCON64_SMS, WF_DEFAULT | WF_STOP | WF_NO_SPLIT | WF_NO_ROM}, // WF_OBJ_SMS_DEFAULT_STOP_NO_SPLIT_NO_ROM

    {UCON64_GC, WF_SWITCH},                     // WF_OBJ_GC_SWITCH
    {UCON64_S16, WF_SWITCH},                    // WF_OBJ_S16_SWITCH
    {UCON64_ATA, WF_SWITCH},                    // WF_OBJ_ATA_SWITCH
    {UCON64_COLECO, WF_SWITCH},                 // WF_OBJ_COLECO_SWITCH
    {UCON64_VBOY, WF_SWITCH},                   // WF_OBJ_VBOY_SWITCH
    {UCON64_VEC, WF_SWITCH},                    // WF_OBJ_VEC_SWITCH
    {UCON64_INTELLI, WF_SWITCH},                // WF_OBJ_INTELLI_SWITCH
    {UCON64_GP32, WF_SWITCH},                   // WF_OBJ_GP32_SWITCH
    {UCON64_PS2, WF_SWITCH},                    // WF_OBJ_PS2_SWITCH
    {UCON64_XBOX, WF_SWITCH},                   // WF_OBJ_XBOX_SWITCH
    {UCON64_SAT, WF_SWITCH},                    // WF_OBJ_SAT_SWITCH
    {UCON64_3DO, WF_SWITCH},                    // WF_OBJ_3DO_SWITCH
    {UCON64_CD32, WF_SWITCH},                   // WF_OBJ_CD32_SWITCH
    {UCON64_CDI, WF_SWITCH},                    // WF_OBJ_CDI_SWITCH
  };

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

const st_getopt2_t libdm_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "All disc-based consoles",
      NULL
    },
    {
      "disc", 0, 0, UCON64_DISC,
      NULL, "force recognition; NEEDED",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "rip", 1, 0, UCON64_RIP,
      "N", "rip/dump track N from IMAGE",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
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
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
#endif
    {
      "bin2iso", 1, 0, UCON64_BIN2ISO,
      "N", "convert track N to ISO (if possible) by resizing\n"
      "sectors to 2048 Bytes",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      "isofix", 1, 0, UCON64_ISOFIX,
      "N", "fix corrupted track N (if possible)\n"
      "if PVD points to a bad DR offset it will add padding data\n"
      "so actual DR gets located in right absolute address",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      "mkcue", 0, 0, UCON64_MKCUE,
      NULL, "generate CUE sheet for IMAGE or existing TOC sheet",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      "mktoc", 0, 0, UCON64_MKTOC,
      NULL, "generate TOC sheet for IMAGE or existing CUE sheet",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      // hidden option
      "mksheet", 0, 0, UCON64_MKSHEET,
      NULL, /* "same as " OPTION_LONG_S "mktoc and " OPTION_LONG_S "mkcue" */ NULL,
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
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

    "Wrote output to: %s\n",                                            // WROTE
    "ERROR: Can't open \"%s\" for reading\n",                           // OPEN_READ_ERROR
    "ERROR: Can't open \"%s\" for writing\n",                           // OPEN_WRITE_ERROR
    "ERROR: Can't read from \"%s\"\n",                                  // READ_ERROR
    "ERROR: Can't write to \"%s\"\n",                                   // WRITE_ERROR
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

const st_getopt2_t unknown_usage[] =
  {
    {NULL, 0, 0, 0, NULL, "Unknown backup unit/emulator", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  gc_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Nintendo Game Cube/Panasonic Gamecube Q"
      /*"2001/2002 Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      "gc", 0, 0, UCON64_GC,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_GC_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  s16_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Sega System 16(A/B)/Sega System 18/dual 68000"
      /*"1987/19XX/19XX SEGA http://www.sega.com"*/,
      NULL
    },
    {
      "s16", 0, 0, UCON64_S16,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_S16_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  atari_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Atari VCS 2600(aka Stella)/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr"
      /*"1977/1982/1984/1986 Atari"*/,
      NULL
    },
    {
      "ata", 0, 0, UCON64_ATA,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_ATA_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  coleco_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "ColecoVision"/*"1982"*/,
      NULL
    },
    {
      "coleco", 0, 0, UCON64_COLECO,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_COLECO_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  vboy_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Nintendo Virtual Boy"/*"19XX Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      "vboy", 0, 0, UCON64_VBOY,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_VBOY_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  vectrex_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Vectrex"/*"1982"*/,
      NULL
    },
    {
      "vec", 0, 0, UCON64_VEC,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_VEC_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  intelli_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Intellivision"/*"1979 Mattel"*/,
      NULL
    },
    {
      "intelli", 0, 0, UCON64_INTELLI,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_INTELLI_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  gp32_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "GP32 Game System"/*"2002 Gamepark http://www.gamepark.co.kr"*/,
      NULL
    },
    {
      "gp32", 0, 0, UCON64_GP32,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_GP32_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  ps2_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Playstation 2"/*"2000 Sony http://www.playstation.com"*/,
      NULL
    },
    {
      "ps2", 0, 0, UCON64_PS2,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_PS2_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  xbox_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "XBox"/*"2001 Microsoft http://www.xbox.com"*/,
      NULL
    },
    {
      "xbox", 0, 0, UCON64_XBOX,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_XBOX_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  sat_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Saturn"/*"1994 SEGA http://www.sega.com"*/,
      NULL
    },
    {
      "sat", 0, 0, UCON64_SAT,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_SAT_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  real3do_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Real3DO"/*"1993 Panasonic/Goldstar/Philips"*/,
      NULL
    },
    {
      "3do", 0, 0, UCON64_3DO,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_3DO_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  cd32_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "CD32"/*"1993 Commodore"*/,
      NULL
    },
    {
      "cd32", 0, 0, UCON64_CD32,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_CD32_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  cdi_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "CD-i"/*"1991 Philips"*/,
      NULL
    },
    {
      "cdi", 0, 0, UCON64_CDI,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_CDI_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  vc4000_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Interton VC4000"/*"~1980"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  odyssey2_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "G7400+/Odyssey2"/*"1978"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  channelf_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "FC Channel F"/*"1976"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  odyssey_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Magnavox Odyssey"/*"1972 Ralph Baer (USA)"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  gamecom_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Game.com"/*"? Tiger"*/,
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  mame_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "M.A.M.E. (Multiple Arcade Machine Emulator)",
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

#if 0
Adv. Vision
Arcadia
Astrocade
Indrema
Microvision
N-Gage 2003 Nokia http://www.n-gage.com
Nuon
RCA Studio 2
RDI Halcyon
Telstar
XE System
#endif

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
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "nbak", 0, 0, UCON64_NBAK,
      NULL, "prevents backup files (*.BAK)",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
#ifdef  USE_ANSI_COLOR
    {
      "ncol", 0, 0, UCON64_NCOL,
      NULL, "disable ANSI colors in output",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
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
        "USB0, USB1, "
#endif
#ifdef  USE_PARALLEL
        "3bc, 378, 278, "
#endif
        "...}",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
#endif // defined USE_PARALLEL || defined USE_USB
    {
      "hdn", 1, 0, UCON64_HDN,
      "N", "force ROM has backup unit/emulator header with size of N Bytes",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "hd", 0, 0, UCON64_HD,
      NULL, "same as " OPTION_LONG_S "hdn=512\n"
      "most backup units use a header with a size of 512 Bytes",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "nhd", 0, 0, UCON64_NHD,
      NULL, "force ROM has no backup unit/emulator header",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "ns", 0, 0, UCON64_NS,
      NULL, "force ROM is not split",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "e", 0, 0, UCON64_E,
#ifdef  __MSDOS__
      NULL, "emulate/run ROM (check ucon64.cfg for all Emulator settings)",
#else
      NULL, "emulate/run ROM (check .ucon64rc for all Emulator settings)",
#endif
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      "crc", 0, 0, UCON64_CRC,
      NULL, "show CRC32 value of ROM",
#if 0
      "; this will also force calculation for\n"
      "files bigger than %d Bytes (%.4f Mb)"
#endif
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_CRC32]
    },
    {
      "sha1", 0, 0, UCON64_SHA1,
      NULL, "show SHA1 value of ROM",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_CRC32]
    },
    {
      "md5", 0, 0, UCON64_MD5,
      NULL, "show MD5 value of ROM",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_CRC32]
    },
    {
      "ls", 0, 0, UCON64_LS,
      NULL, "generate ROM list for all recognized ROMs",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE]
    },
    {
      "lsv", 0, 0, UCON64_LSV,
      NULL, "like " OPTION_LONG_S "ls but more verbose",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE]
    },
    {
      "hex", 2, 0, UCON64_HEX,
#ifdef  __MSDOS__
      "ST", "show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex ...|more\""
#else
      "ST", "show ROM as hexdump; use \"ucon64 " OPTION_LONG_S "hex ...|less\"" // less is more ;-)
#endif
      "\nST is the optional start value in bytes",
      NULL
    },
    {
      "dual", 2, 0, UCON64_DUAL,                // TODO: Think of a decent name - dbjh
#ifdef  __MSDOS__
      "ST", "show ROM as dualdump; use \"ucon64 " OPTION_LONG_S "dual ...|more\"",
#else
      "ST", "show ROM as dualdump; use \"ucon64 " OPTION_LONG_S "dual ...|less\"",
#endif
      NULL
    },
    {
      "code", 2, 0, UCON64_CODE,
#ifdef  __MSDOS__
      "ST", "show ROM as code; use \"ucon64 " OPTION_LONG_S "code ...|more\"",
#else
      "ST", "show ROM as code; use \"ucon64 " OPTION_LONG_S "code ...|less\"",
#endif
      NULL
    },
    {
      "print", 2, 0, UCON64_PRINT,
#ifdef  __MSDOS__
      "ST", "show ROM in printable characters; use \"ucon64 " OPTION_LONG_S "print ...|more\"",
#else
      "ST", "show ROM in printable characters; use \"ucon64 " OPTION_LONG_S "print ...|less\"",
#endif
      NULL
    },
    {
      "find", 1, 0, UCON64_FIND,
      "STRING", "find STRING in ROM (wildcard: '?')",
      &ucon64_wf[WF_OBJ_ALL_INIT]
    },
    {
      "findi", 1, 0, UCON64_FINDI,
      "STR", "like " OPTION_LONG_S "find but ignores the case of alpha bytes",
      &ucon64_wf[WF_OBJ_ALL_INIT]
    },
    {
      "findr", 1, 0, UCON64_FINDR,
      "STR", "like " OPTION_LONG_S "find but looks also for shifted/relative similarities\n"
      "(wildcard: disabled)",
      &ucon64_wf[WF_OBJ_ALL_INIT]
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
      "help", 0, 0, UCON64_HELP,
      NULL, "display this help and exit",
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {
      "version", 0, 0, UCON64_VER,
      NULL, "output version information and exit",
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {
      "q", 0, 0, UCON64_Q,
      NULL, "be quiet (don't show ROM info)",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
#if 0
    {
      "qq", 0, 0, UCON64_QQ,
      NULL, "be even more quiet",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
#endif
    {
      "v", 0, 0, UCON64_V,
      NULL, "be more verbose (show backup unit headers also)",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

const st_getopt2_t ucon64_options_without_usage[] =
  {
    {
      "crchd", 0, 0, UCON64_CRCHD,              // backward compat.
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_CRC32]
    },
    {
      "file", 1, 0, UCON64_FILE,                // obsolete?
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "frontend", 0, 0, UCON64_FRONTEND,        // no usage?
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "?", 0, 0, UCON64_HELP,                   // same as --help
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {
      "h", 0, 0, UCON64_HELP,                   // same as --help
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {
      "id", 0, 0, UCON64_ID,                    // currently only used in snes.c
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "rom", 0, 0, UCON64_ROM,                  // obsolete?
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "83", 0, 0, UCON64_RR83,                  // is now "rr83"
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_SPLIT]
    },
#if 0
    {
      "xcdrw", 0, 0, UCON64_XCDRW, // obsolete
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_DEFAULT_STOP_NO_ROM]
    },
    {
      "cdmage", 1, 0, UCON64_CDMAGE, // obsolete
      NULL, NULL,
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
#endif
    // these consoles are (still) not supported
    {
      "3do", 0, 0, UCON64_3DO,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_3DO_SWITCH]
    },
    {
      "gp32", 0, 0, UCON64_GP32,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_GP32_SWITCH]
    },
    {
      "intelli", 0, 0, UCON64_INTELLI,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_INTELLI_SWITCH]
    },
    {
      "ps2", 0, 0, UCON64_PS2,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_PS2_SWITCH]
    },
    {
      "s16", 0, 0, UCON64_S16,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_S16_SWITCH]
    },
    {
      "sat", 0, 0, UCON64_SAT,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_SAT_SWITCH]
    },
    {
      "vboy", 0, 0, UCON64_VBOY,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_VBOY_SWITCH]
    },
    {
      "vec", 0, 0, UCON64_VEC,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_VEC_SWITCH]
    },
    {
      "xbox", 0, 0, UCON64_XBOX,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_XBOX_SWITCH]
    },
    {
      "coleco", 0, 0, UCON64_COLECO,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_COLECO_SWITCH]
    },
    {
      "gc", 0, 0, UCON64_GC,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_GC_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
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
      &ucon64_wf[WF_OBJ_ALL_INIT_NO_SPLIT]
    },
    {
      "pad", 0, 0, UCON64_PAD,
      NULL, "pad ROM to next Mb",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      "p", 0, 0, UCON64_P,
      NULL, "same as " OPTION_LONG_S "pad",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      "padn", 1, 0, UCON64_PADN,
      "N", "pad ROM to N Bytes (put Bytes with value 0x00 after end)",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
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

const st_getopt2_t ucon64_patching_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Patching",
      NULL
    },
    {
      "poke", 1, 0, UCON64_POKE,
      "OFF:V", "change byte at file offset OFF to value V (both in hexadecimal)",
      NULL
    },
    {
      "pattern", 1, 0, UCON64_PATTERN,
      "FILE", "change ROM based on patterns specified in FILE",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE]
    },
    {
      "patch", 1, 0, UCON64_PATCH,
      "PATCH", "specify the PATCH for the following options\n"
      "use this option or uCON64 expects the last commandline\n"
      "argument to be the name of the PATCH file",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

char *ucon64_temp_file = NULL;
int (*ucon64_testsplit_callback) (const char *filename) = NULL;

// _publisher_ strings for SNES, GB, GBC and GBA games
const char *nintendo_maker[NINTENDO_MAKER_LEN] =
  {
    NULL, "Nintendo", "Rocket Games/Ajinomoto", "Imagineer-Zoom", "Gray Matter",
    "Zamuse", "Falcom", NULL, "Capcom", "Hot B Co.",
    "Jaleco", "Coconuts Japan", "Coconuts Japan/G.X.Media",
      "Micronet", "Technos",
    "Mebio Software", "Shouei System", "Starfish", NULL, "Mitsui Fudosan/Dentsu",
    NULL, "Warashi Inc.", NULL, "Nowpro", NULL,
    "Game Village", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 0Z
    NULL, "Starfish", "Infocom", "Electronic Arts Japan", NULL,
    "Cobra Team", "Human/Field", "KOEI", "Hudson Soft", "S.C.P./Game Village",
    "Yanoman", NULL, "Tecmo Products", "Japan Glary Business", "Forum/OpenSystem",
    "Virgin Games (Japan)", "SMDE", NULL, NULL, "Daikokudenki",
    NULL, NULL, NULL, NULL, NULL,
    "Creatures Inc.", "TDK Deep Impresion", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 1Z
    "Destination Software/KSS", "Sunsoft/Tokai Engineering",
      "POW (Planning Office Wada)/VR 1 Japan", "Micro World", NULL,
    "San-X", "Enix", "Loriciel/Electro Brain", "Kemco Japan", "Seta Co., Ltd.",
    "Culture Brain", NULL, "Palsoft", "Visit Co., Ltd.", "Intec",
    "System Sacom", "Poppo", "Ubisoft Japan", NULL, "Media Works",
    "NEC InterChannel", "Tam", "Gajin/Jordan", "Smilesoft", NULL,
    NULL, "Mediakite", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 2Z
    "Viacom", "Carrozzeria", "Dynamic", NULL, "Magifact",
    "Hect", "Codemasters", "Taito/GAGA Communications", "Laguna",
      "Telstar Fun & Games/Event/Taito",
    NULL, "Arcade Zone Ltd.", "Entertainment International/Empire Software", "Loriciel",
      "Gremlin Graphics",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 3Z
    "Seika Corp.", "UBI SOFT Entertainment Software", "Sunsoft US", NULL, "Life Fitness",
    NULL, "System 3", "Spectrum Holobyte", NULL, "IREM",
    NULL, "Raya Systems", "Renovation Products", "Malibu Games", NULL,
    "Eidos/U.S. Gold", "Playmates Interactive", NULL, NULL, "Fox Interactive",
    "Time Warner Interactive", NULL, NULL, NULL, NULL,
    NULL, "Disney Interactive", NULL, "Black Pearl", NULL,
    "Advanced Productions", NULL, NULL, "GT Interactive", "RARE",
    "Crave Entertainment",                      // 4Z
    "Absolute Entertainment", "Acclaim", "Activision", "American Sammy", "Take 2/GameTek",
    "Hi Tech", "LJN Ltd.", NULL, "Mattel", NULL,
    "Mindscape/Red Orb Entertainment", "Romstar", "Taxan", "Midway/Tradewest", NULL,
    "American Softworks Corp.", "Majesco Sales Inc.", "3DO", NULL, NULL,
    "Hasbro", "NewKidCo", "Telegames", "Metro3D", NULL,
    "Vatical Entertainment", "LEGO Media", NULL, "Xicat Interactive", "Cryo Interactive",
    NULL, NULL, "Red Storm Entertainment", "Microids", NULL,
    "Conspiracy/Swing",                         // 5Z
    "Titus", "Virgin Interactive", "Maxis", NULL, "LucasArts Entertainment",
    NULL, NULL, "Ocean", NULL, "Electronic Arts",
    NULL, "Laser Beam", NULL, NULL, "Elite Systems",
    "Electro Brain", "The Learning Company", "BBC", NULL, "Software 2000",
    NULL, "BAM! Entertainment", "Studio 3", NULL, NULL,
    NULL, "Classified Games", NULL, "TDK Mediactive", NULL,
    "DreamCatcher", "JoWood Produtions", "SEGA", "Wannado Edition",
      "LSP (Light & Shadow Prod.)",
    "ITE Media",                                // 6Z
    "Infogrames", "Interplay", "JVC (US)", "Parker Brothers", NULL,
    "SCI (Sales Curve Interactive)/Storm", NULL, NULL, "THQ Software", "Accolade Inc.",
    "Triffix Entertainment", NULL, "Microprose Software",
      "Universal Interactive/Sierra/Simon & Schuster", NULL,
    "Kemco", "Rage Software", "Encore", NULL, "Zoo",
    "BVM", "Simon & Schuster Interactive", "Asmik Ace Entertainment Inc./AIA",
      "Empire Interactive", NULL,
    NULL, "Jester Interactive", NULL, NULL, "Scholastic",
    "Ignition Entertainment", NULL, "Stadlbauer", NULL, NULL,
    NULL,                                       // 7Z
    "Misawa", "Teichiku", "Namco Ltd.", "LOZC", "KOEI",
    NULL, "Tokuma Shoten Intermedia", "Tsukuda Original", "DATAM-Polystar", NULL,
    NULL, "Bulletproof Software", "Vic Tokai Inc.", NULL, "Character Soft",
    "I'Max", "Saurus", NULL, NULL, "General Entertainment",
    NULL, NULL, "I'Max", "Success", NULL,
    "SEGA Japan", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 8Z
    "Takara", "Chun Soft", "Video System Co., Ltd./McO'River", "BEC", NULL,
    "Varie", "Yonezawa/S'pal", "Kaneko", NULL, "Victor Interactive Software/Pack in Video",
    "Nichibutsu/Nihon Bussan", "Tecmo", "Imagineer", NULL, NULL,
    "Nova", "Den'Z", "Bottom Up", NULL, "TGL (Technical Group Laboratory)",
    NULL, "Hasbro Japan", NULL, "Marvelous Entertainment", NULL,
    "Keynet Inc.", "Hands-On Entertainment", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // 9Z
    "Telenet", "Hori", NULL, NULL, "Konami",
    "K.Amusement Leasing Co.", "Kawada", "Takara", NULL, "Technos Japan Corp.",
    "JVC (Europe/Japan)/Victor Musical Industries", NULL, "Toei Animation", "Toho", NULL,
    "Namco", "Media Rings Corp.", "J-Wing", NULL, "Pioneer LDC",
    "KID", "Mediafactory", NULL, NULL, NULL,
    "Infogrames Hudson", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // AZ
    "Acclaim Japan", "ASCII Co./Nexoft" /*/Activision*/, "Bandai", NULL, "Enix",
    NULL, "HAL Laboratory/Halken", "SNK", NULL, "Pony Canyon Hanbai",
    "Culture Brain", "Sunsoft", "Toshiba EMI", "Sony Imagesoft", NULL,
    "Sammy", "Magical", "Visco", NULL, "Compile",
    NULL, "MTO Inc.", NULL, "Sunrise Interactive", NULL,
    "Global A Entertainment", "Fuuki", NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // BZ
    "Taito", NULL, "Kemco", "Square", "Tokuma Shoten",
    "Data East", "Tonkin House", NULL, "KOEI", NULL,
    "Konami/Ultra/Palcom", "NTVIC/VAP", "Use Co., Ltd.", "Meldac",
      "Pony Canyon (Japan)/FCI (US)",
    "Angel/Sotsu Agency/Sunrise", "Yumedia/Aroma Co., Ltd.", NULL, NULL, "Boss",
    "Axela/Crea-Tech", "Sekaibunka-Sha/Sumire kobo/Marigul Management Inc.",
      "Konami Computer Entertainment Osaka", NULL, NULL,
    "Enterbrain", NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // CZ
    "Taito/Disco", "Sofel", "Quest Corp.", "Sigma", "Ask Kodansha",
    NULL, "Naxat", "Copya System", "Capcom Co., Ltd.", "Banpresto",
    "TOMY", "Acclaim/LJN Japan", NULL, "NCS", "Human Entertainment",
    "Altron", "Jaleco", "Gaps Inc.", NULL, NULL,
    NULL, NULL, NULL, "Elf", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // DZ
    "Jaleco", NULL, "Yutaka", "Varie", "T&ESoft",
    "Epoch Co., Ltd.", NULL, "Athena", "Asmik", "Natsume",
    "King Records", "Atlus", "Epic/Sony Records (Japan)", NULL,
      "IGS (Information Global Service)",
    NULL, "Chatnoir", "Right Stuff", NULL, NULL,
    NULL, "Spike", "Konami Computer Entertainment Tokyo", "Alphadream Corp.", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // EZ
    "A Wave", "Motown Software", "Left Field Entertainment", "Extreme Ent. Grp.",
      "TecMagik",
    NULL, NULL, NULL, NULL, "Cybersoft",
    NULL, "Psygnosis", NULL, NULL, "Davidson/Western Tech.",
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // FZ
    NULL, "PCCW Japan", NULL, NULL, "KiKi Co. Ltd.",
    "Open Sesame Inc.", "Sims", "Broccoli", "Avex", NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // GZ
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL,                                       // HZ
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, "Yojigen", NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL,
    NULL
  };                                            // IZ


#ifdef  USE_DISCMAGE
int
ucon64_load_discmage (void)
{
  uint32_t version;
#ifdef  DLOPEN
  get_property_fname (ucon64.configfile, "discmage_path", ucon64.discmage_path, "");

  // if ucon64.discmage_path points to an existing file then load it
  if (!access (ucon64.discmage_path, F_OK))
    {
      libdm = open_module (ucon64.discmage_path);

      dm_get_version_ptr = (uint32_t (*) (void)) get_symbol (libdm, "dm_get_version");
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
          dm_get_version_s_ptr = (const char *(*) (void)) get_symbol (libdm, "dm_get_version_s");
          dm_set_gauge_ptr = (void (*) (void (*) (int, int))) get_symbol (libdm, "dm_set_gauge");

          dm_open_ptr = (dm_image_t *(*) (const char *, uint32_t)) get_symbol (libdm, "dm_open");
          dm_reopen_ptr = (dm_image_t *(*) (const char *, uint32_t, dm_image_t *))
                      get_symbol (libdm, "dm_reopen");
          dm_fdopen_ptr = (FILE *(*) (dm_image_t *, int, const char *))
                      get_symbol (libdm, "dm_fdopen");
          dm_close_ptr = (int (*) (dm_image_t *)) get_symbol (libdm, "dm_close");
          dm_nfo_ptr = (void (*) (const dm_image_t *, int, int)) get_symbol (libdm, "dm_nfo");

          dm_read_ptr = (int (*) (char *, int, int, const dm_image_t *)) get_symbol (libdm, "dm_read");
          dm_write_ptr = (int (*) (const char *, int, int, const dm_image_t *)) get_symbol (libdm, "dm_write");

          dm_disc_read_ptr = (int (*) (const dm_image_t *)) get_symbol (libdm, "dm_disc_read");
          dm_disc_write_ptr = (int (*) (const dm_image_t *)) get_symbol (libdm, "dm_disc_write");

          dm_toc_read_ptr = (dm_image_t *(*) (dm_image_t *, const char *)) get_symbol (libdm, "dm_toc_read");
          dm_toc_write_ptr = (int (*) (const dm_image_t *)) get_symbol (libdm, "dm_toc_write");

          dm_cue_read_ptr = (dm_image_t *(*) (dm_image_t *, const char *)) get_symbol (libdm, "dm_cue_read");
          dm_cue_write_ptr = (int (*) (const dm_image_t *)) get_symbol (libdm, "dm_cue_write");

          dm_rip_ptr = (int (*) (const dm_image_t *, int, uint32_t)) get_symbol (libdm, "dm_rip");

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
    sprintf (djimport_path, "%s"FILE_SEPARATOR_S"%s", dir, "discmage.dxe");
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
libdm_gauge (int pos, int size)
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
unknown_init (st_rominfo_t *rominfo)
// init routine for all consoles missing in console/.
{
  ucon64.rominfo = rominfo;
  ucon64.dat = NULL;
#ifdef  USE_DISCMAGE
  ucon64.image = NULL;
#endif

  return 0;
}


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
  // ucon64_temp_file will be reset in remove_temp_file()
  ucon64_temp_file = NULL;
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
            printf ("Wrote backup to: %s\n", mkbak (dest, BAK_DUPE));
          return 1;
        }

      if (one_file (src, dest))
        {                                       // case 1
          if (ucon64.backup)
            {                                   // case 1a
              strcpy (src, mkbak (dest, BAK_DUPE));
              printf ("Wrote backup to: %s\n", src);
            }
          else
            {                                   // case 1b
              strcpy (src, mkbak (dest, BAK_MOVE));
              ucon64_temp_file = src;
            }
        }
      else
        {                                       // case 2
          if (ucon64.backup)                    // case 2a
            printf ("Wrote backup to: %s\n", mkbak (dest, BAK_DUPE));
        }
      return 1;
    }
  return 0;
}


void
remove_temp_file (void)
{
  if (ucon64_temp_file)
    {
      printf ("Removing: %s\n", ucon64_temp_file);
      remove (ucon64_temp_file);
      ucon64_temp_file = NULL;
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
    because we currently don't write to zip or gzip files. Otherwise the output
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

  while ((blocksize = fread (buffer, 1, MAXBUFSIZE, file)))
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
ucon64_gauge (time_t init_time, int pos, int size)
{
  return gauge (stdout, init_time, pos, size, ucon64.frontend ? GAUGE_PERCENT : GAUGE_DEFAULT);
}


int
ucon64_testsplit (const char *filename)
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
          if (ucon64_testsplit_callback)
            ucon64_testsplit_callback (buf);
          (*p)++;
          parts++;
        }

      if (parts > 1)
        return parts;
    }

  return 0;
}


// configfile handling
static int
ucon64_configfile_update (void)
{
  char buf[MAXBUFSIZE];

  sprintf (buf, "%d", UCON64_CONFIG_VERSION);
  set_property (ucon64.configfile, "version", buf, "uCON64 configuration");

  return 0;
}


typedef struct
{
  int id;
  const char *command;
} st_command_t;


static int
ucon64_configfile_create (void)
{
  const st_getopt2_t *options = ucon64.options;
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
      {
        "discmage_path",
#if     defined __MSDOS__
        "~\\discmage.dxe",                      // realpath2() expands the tilde
#elif   defined __CYGWIN__
        "~/discmage.dll",
#elif   defined _WIN32
        "~\\discmage.dll",
#elif   defined __APPLE__                       // Mac OS X actually
        "~/.ucon64/discmage.dylib",
#elif   defined __unix__ || defined __BEOS__
        "~/.ucon64/discmage.so",
#else
        "",
#endif
        "complete path to the discmage library for CD image support"
      },
      {
        "ucon64_configdir",
#if     defined __MSDOS__ || defined __CYGWIN__ || defined _WIN32
        "~",                                    // realpath2() expands the tilde
#elif   defined __unix__ || defined __BEOS__ || defined __APPLE__ // Mac OS X actually
        "~/.ucon64",
#else
        "",
#endif
        "directory with additional config files"
      },
      {
        "ucon64_datdir",
#if     defined __MSDOS__ || defined __CYGWIN__ || defined _WIN32
        "~",                                    // realpath2() expands the tilde
#elif   defined __unix__ || defined __BEOS__ || defined __APPLE__ // Mac OS X actually
        "~/.ucon64/dat",
#else
        "",
#endif
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
      {NULL, NULL, NULL}
    };

  st_command_t emulate[] =
    {
      {UCON64_3DO,      ""},
      {UCON64_ATA,      ""},
      {UCON64_CD32,     ""},
      {UCON64_CDI,      ""},
      {UCON64_COLECO,   ""},
      {UCON64_DC,       ""},
      {UCON64_GB,       "vgb -sound -sync 50 -sgb -scale 2"},
      {UCON64_GBA,      "vgba -scale 2 -uperiod 6"},
      {UCON64_GC,       ""},
      {UCON64_GEN,      "dgen -f -S 2"},
      {UCON64_INTELLI,  ""},
      {UCON64_JAG,      ""},
      {UCON64_LYNX,     ""},
      {UCON64_MAME,     ""},
      {UCON64_N64,      ""},
      {UCON64_NES,      "tuxnes -E2 -rx11 -v -s/dev/dsp -R44100"},
      {UCON64_NG,       ""},
      {UCON64_NGP,      ""},
      {UCON64_PCE,      ""},
      {UCON64_PS2,      ""},
      {UCON64_PSX,      "pcsx"},
      {UCON64_S16,      ""},
      {UCON64_SAT,      ""},
      {UCON64_SMS,      ""},
      {UCON64_GAMEGEAR, ""},
      {UCON64_SNES,     "snes9x -tr -sc -hires -dfr -r 7 -is -joymap1 2 3 5 0 4 7 6 1"},
      {UCON64_SWAN,     ""},
      {UCON64_VBOY,     ""},
      {UCON64_VEC,      ""},
      {UCON64_XBOX,     ""},
      {0, NULL}
    };
  int x = 0, y = 0;

  ucon64_configfile_update ();

  set_property_array (ucon64.configfile, props);

  for (x = 0; emulate[x].command; x++)
    for (y = 0; options[y].name || options[y].help; y++)
      if (emulate[x].id == options[y].val)
        {
          char buf[MAXBUFSIZE];

          sprintf (buf, "emulate_%s", options[y].name);

          set_property (ucon64.configfile, buf, emulate[x].command, !x ?
            "emulate_<console shortcut>=<emulator with options>\n\n"
            "You can also use CRC32 values for ROM specific emulation options:\n\n"
            "emulate_0x<crc32>=<emulator with options>\n"
            "emulate_<crc32>=<emulator with options>" : NULL);

          break;
        }

  return 0;
}


int
ucon64_configfile (void)
{
  char buf[MAXBUFSIZE], *dirname;
  int result = -1;

  dirname = getenv2 ("UCON64_HOME");
  if (!dirname[0])
    dirname = getenv2 ("HOME");
  sprintf (ucon64.configfile, "%s" FILE_SEPARATOR_S
#ifdef  __MSDOS__
    "ucon64.cfg"
#else
    ".ucon64rc"
#endif
    , dirname);

//  if (!access (ucon64.configfile, F_OK))
//    fprintf (stderr, ucon64_msg[READ_CONFIG_FILE], ucon64.configfile);

  if (access (ucon64.configfile, F_OK) != 0)
    {
      FILE *fh;

      printf ("WARNING: %s not found: creating...", ucon64.configfile);

      if (!(fh = fopen (ucon64.configfile, "w"))) // opening the file in text mode
        {                                         //  avoids trouble under DOS
          printf ("FAILED\n\n");
          return -1;
        }
      fclose (fh);                              // we'll use set_property() from now

      result = ucon64_configfile_create ();

      if (!result)
        {
          sync ();
          printf ("OK\n\n");
        }
      else
        printf ("FAILED\n\n");
    }
  else if (get_property_int (ucon64.configfile, "version") < UCON64_CONFIG_VERSION)
    {
      strcpy (buf, ucon64.configfile);
      set_suffix (buf, ".old");

      printf ("NOTE: Updating config, old version will be renamed to %s...", buf);

      fcopy (ucon64.configfile, 0, fsizeof (ucon64.configfile), buf, "wb"); // "wb" is correct for copying

      result = ucon64_configfile_update ();

      if (!result)
        {
          sync ();
          printf ("OK\n\n");
        }
      else
        printf ("FAILED\n\n");
    }

  return result;
}


static inline char *
to_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = func (*p);

  return s;
}


int
ucon64_rename (int mode)
{
  char buf[FILENAME_MAX + 1], buf2[FILENAME_MAX + 1], suffix[80];
  const char *p, *p2;
  int good_name;

  buf[0] = 0;
  strncpy (suffix, get_suffix (ucon64.rom), sizeof (suffix))[sizeof (suffix) - 1] = 0; // in case suffix is >= 80 chars

  switch (mode)
    {
    case UCON64_RROM:
      if (ucon64.rominfo)
        if (ucon64.rominfo->name)
          {
            strcpy (buf, ucon64.rominfo->name);
            strtriml (strtrimr (buf));
          }
      break;

    case UCON64_RENAME:                         // GoodXXXX style rename
      if (ucon64.dat)
        if (((st_ucon64_dat_t *) ucon64.dat)->fname)
          {
            p = (char *) get_suffix (((st_ucon64_dat_t *) ucon64.dat)->fname);
            strcpy (buf, ((st_ucon64_dat_t *) ucon64.dat)->fname);

            // get_suffix() never returns NULL
            if (p[0])
              if (strlen (p) < 5)
                if (!(stricmp (p, ".nes") &&    // NES
                      stricmp (p, ".fds") &&    // NES FDS
                      stricmp (p, ".gb") &&     // Game Boy
                      stricmp (p, ".gbc") &&    // Game Boy Color
                      stricmp (p, ".gba") &&    // Game Boy Advance
                      stricmp (p, ".smc") &&    // SNES
                      stricmp (p, ".sc") &&     // Sega Master System
                      stricmp (p, ".sg") &&     // Sega Master System
                      stricmp (p, ".sms") &&    // Sega Master System
                      stricmp (p, ".gg") &&     // Game Gear
                      stricmp (p, ".smd") &&    // Genesis
                      stricmp (p, ".v64")))     // Nintendo 64
                  buf[strlen (buf) - strlen (p)] = 0;
          }
      break;

    default:
      return 0;                                 // invalid mode
    }

  if (!buf[0])
    return 0;

  if (ucon64.fname_len == UCON64_FORCE63)
    buf[63] = 0;
  else if (ucon64.fname_len == UCON64_RR83)
    buf[8] = 0;

  // replace chars the fs might not like
  strcpy (buf2, to_func (buf, strlen (buf), tofname));
  strcpy (buf, basename2 (ucon64.rom));

  p = (char *) get_suffix (buf);
  // Remove the suffix from buf (ucon64.rom). Note that this isn't fool-proof.
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

  if (ucon64.fname_len == UCON64_RR83)
    buf2[12] = 0;

  ucon64_output_fname (buf2, OF_FORCE_BASENAME | OF_FORCE_SUFFIX);

  p = basename2 (ucon64.rom);
  p2 = basename2 (buf2);

  if (one_file (ucon64.rom, buf2) && !strcmp (p, p2))
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
  rename2 (ucon64.rom, buf2);                   // rename2() must be used!
#endif
#ifdef  USE_ZLIB
  unzip_current_file_nr = 0x7fffffff - 1;       // dirty hack
#endif
  return 0;
}


int
ucon64_e (void)
{
  int result = 0;
  char buf[MAXBUFSIZE], value[MAXBUFSIZE], name[MAXBUFSIZE];
  const char *value_p = NULL;
  const st_getopt2_t *p = NULL, *options = ucon64.options;

  if (access (ucon64.configfile, F_OK) != 0)
    {
      fprintf (stderr, "ERROR: %s does not exist\n", ucon64.configfile);
      return -1;
    }

  sprintf (name, "emulate_%08x", ucon64.crc32); // look for emulate_<crc32>
  value_p = get_property (ucon64.configfile, name, value, NULL);

  if (value_p == NULL)
    {
      sprintf (name, "emulate_0x%08x", ucon64.crc32); // look for emulate_0x<crc32>
      value_p = get_property (ucon64.configfile, name, value, NULL);
    }

  if (value_p == NULL)
    if ((p = getopt2_get_index_by_val (options, ucon64.console)))
      {
        sprintf (name, "emulate_%s", p->name);  // look for emulate_<console>
        value_p = get_property (ucon64.configfile, name, value, NULL);
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

  sprintf (buf, "%s \"%s\"", value_p, ucon64.rom);

  puts (buf);
  fflush (stdout);
  sync ();

  result = system (buf)
#if     !(defined __MSDOS__ || defined _WIN32)
           >> 8                                 // the exit code is coded in bits 8-15
#endif                                          //  (does not apply to DJGPP, MinGW & VC++)
           ;

#if 1
  // Snes9x (Linux) for example returns a non-zero value on a normal exit
  //  (3)...
  // under WinDOS, system() immediately returns with exit code 0 when
  //  starting a Windows executable (as if fork() was called) it also
  //  returns 0 when the exe could not be started
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
/*
  In order for this function to be really useful for general purposes
  change_mem2() should be changed so that it will return detailed status
  information. Since we don't use it for general purposes, this has not a high
  priority. It will be updated as soon as there is a need.
  The thing that currently goes wrong is that offsets that fall outside the
  buffer (either positive or negative) won't result in a change. It will result
  in memory corruption...
*/
int
ucon64_pattern (st_rominfo_t *rominfo, const char *pattern_fname)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX],
       buffer[PATTERN_BUFSIZE];
  FILE *srcfile, *destfile;
  int bytesread = 0, n, n_found = 0, n_patterns, overlap = 0;
  st_cm_pattern_t *patterns = NULL;

  realpath2 (pattern_fname, src_name);
  // First try the current directory, then the configuration directory
  if (access (src_name, F_OK | R_OK) == -1)
    sprintf (src_name, "%s" FILE_SEPARATOR_S "%s", ucon64.configdir, pattern_fname);
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

      if ((patterns[n].offset < 0 && patterns[n].offset <= -patterns[n].search_size) ||
           patterns[n].offset > 0)
        printf ("WARNING: The offset of pattern %d falls outside the search pattern.\n"
                "         This can cause problems with the current implementation of --pattern.\n"
                "         Please consider enlarging the search pattern.\n",
                n + 1);
    }
  overlap--;

  puts ("Searching for patterns...");

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
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
  if (rominfo->buheader_len)                    // copy header (if present)
    {
      n = rominfo->buheader_len;
      while ((bytesread = fread (buffer, 1, MIN (n, PATTERN_BUFSIZE), srcfile)))
        {
          fwrite (buffer, 1, bytesread, destfile);
          n -= bytesread;
        }
    }

  n = fread (buffer, 1, overlap, srcfile);      // keep bytesread set to 0
  if (n < overlap)                              // DAMN special cases!
    {
      n_found += change_mem2 (buffer, n, patterns[n].search,
                              patterns[n].search_size, patterns[n].wildcard,
                              patterns[n].escape, patterns[n].replace,
                              patterns[n].replace_size, patterns[n].offset,
                              patterns[n].sets);
      fwrite (buffer, 1, n, destfile);
      n = -1;
    }
  else
    do
      {
        if (bytesread)                          // the code also works without this if
          {
            for (n = 0; n < n_patterns; n++)
              {
                int x = 1 - patterns[n].search_size;
                n_found += change_mem2 (buffer + overlap + x,
                                        bytesread + patterns[n].search_size - 1,
                                        patterns[n].search, patterns[n].search_size,
                                        patterns[n].wildcard, patterns[n].escape,
                                        patterns[n].replace, patterns[n].replace_size,
                                        patterns[n].offset, patterns[n].sets);
              }
            fwrite (buffer, 1, bytesread, destfile);
            memmove (buffer, buffer + bytesread, overlap);
          }
      }
    while ((bytesread = fread (buffer + overlap, 1, PATTERN_BUFSIZE - overlap, srcfile)));
  if (n != -1)
    fwrite (buffer, 1, overlap, destfile);

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
  st_ucon64_dump_t o = {output, start, flags};

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
              dumper (stdout, compare, (o->searchlen + 0x0f) & ~0x0f, o->found, DUMPER_HEX);
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
              dumper (stdout, ptr1, (o->searchlen + 0x0f) & ~0x0f, o->found, DUMPER_HEX);
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
  int result = 0;
  st_ucon64_find_t o = { search, flags, searchlen, start, -2 };
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

  result = quick_io_func (ucon64_find_func, MAXBUFSIZE, &o, start, len,
                          filename, "rb");

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
               const char *filename, size_t start)
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
                          fsizeof (filename) - start, filename, "rb");
  if (sha1_s)
    {
      unsigned char buf[MAXBUFSIZE];

      sha1_end (buf, &m_sha1);
      for (*sha1_s = i = 0; i < 20; i++, sha1_s = strchr (sha1_s, 0))
        sprintf (sha1_s, "%02x", buf[i] & 0xff);
    }

  if (md5_s)
    {
      md5_final (&m_md5);
      for (*md5_s = i = 0; i < 16; i++, md5_s = strchr (md5_s, 0))
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

  printf ("Comparing %s", basename2 (ucon64.rom));
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

  printf ("Comparing %s", basename2 (ucon64.rom));
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
  if (!(buf1 = (unsigned char *) malloc (bufsize)))
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], bufsize);
      return;
    }

  if (!(buf2 = (unsigned char *) malloc (bufsize)))
    {
      free (buf1);
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], bufsize);
      return;
    }
#endif

  if (!(file1 = fopen (filename1, "rb")))
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename1);
#ifdef  FILEFILE_LARGE_BUF
      free (buf1);
      free (buf2);
#endif
      return ;
    }
  if (!(file2 = fopen (filename2, "rb")))
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
