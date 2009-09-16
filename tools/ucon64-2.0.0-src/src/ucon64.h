/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

Copyright (c) 1999 - 2004 NoisyB <noisyb@gmx.net>
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
#ifndef UCON64_H
#define UCON64_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include "ucon64_defines.h"                     // MAXBUFSIZE, etc..


#define UCON64_ISSET(x) (x != UCON64_UNKNOWN)


typedef enum { UCON64_SPP, UCON64_EPP, UCON64_ECP } parport_mode_t;

/*
  st_rominfo_t    this struct contains very specific informations only
                    <console>_init() can supply after the correct console
                    type was identified.
  st_ucon64_t     this struct containes st_rominfo_t and unspecific
                    informations and some workflow stuff
  st_ucon64_obj_t ucon64 object for use in st_getopt2_t
*/
typedef struct
{
  const char *console_usage;                    // console system of the ROM
  const char *copier_usage;                     // backup unit of the ROM

  int interleaved;                              // ROM is interleaved (swapped)
  int data_size;                                // ROM data size without "red tape"

  const void *buheader;                         // (possible) header of backup unit
  int buheader_start;                           // start of backup unit header (mostly 0)
  int buheader_len;                             // length of backup unit header 0 == no bu hdr

  const void *header;                           // (possible) internal ROM header
  int header_start;                             // start of internal ROM header
  int header_len;                               // length of internal ROM header 0 == no hdr

  char name[MAXBUFSIZE];                        // internal ROM name
  const char *maker;                            // maker name of the ROM
  const char *country;                          // country name of the ROM
  char misc[MAXBUFSIZE];                        // some miscellaneous information
                                                //  about the ROM in one single string
  int has_internal_crc;                         // ROM has internal checksum (SNES, Mega Drive, Game Boy)
  unsigned int current_internal_crc;            // calculated checksum

  unsigned int internal_crc;                    // internal checksum
  int internal_crc_start;                       // start of internal checksum in ROM header
  int internal_crc_len;                         // length (in bytes) of internal checksum in ROM header

  char internal_crc2[MAXBUFSIZE];               // 2nd or inverse internal checksum
} st_rominfo_t;

typedef struct
{
  int argc;
  char **argv;

  int option;                                   // current option (UCON64_HEX, UCON64_FIND, ...)
  const char *optarg;                           // ptr to current options optarg

  // TODO?: rename to fname
  const char *rom;                              // ROM (cmdline) with path

  int fname_len;                                // force fname output format for --rrom, --rename, etc...
  // fname_len can be UCON64_RR83 (8.3) or UCON64_FORCE63 (63.x)
  char fname_arch[FILENAME_MAX];                // filename in archive (currently only for zip)
  int file_size;                                // (uncompressed) ROM file size (NOT console specific)
  unsigned int crc32;                           // crc32 value of ROM (used for DAT files) (NOT console specific)
  unsigned int fcrc32;                          // if non-zero: crc32 of ROM as it is on disk (NOT console specific)

  /*
    if console == UCON64_UNKNOWN or st_rominfo_t == NULL ucon64_rom_nfo() won't
    be shown
  */
  int console;                                  // the detected console system

  const char *file;                             // FILE (cmdline) with path

  char configfile[FILENAME_MAX];                // path and name of the config file
  char configdir[FILENAME_MAX];                 // directory for config
  char datdir[FILENAME_MAX];                    // directory for DAT files
  char output_path[FILENAME_MAX];               // -o argument (default: cwd)
#ifdef  USE_DISCMAGE
  char discmage_path[FILENAME_MAX];             // path to the discmage DLL
#endif
#if     defined USE_PPDEV || defined AMIGA
  char parport_dev[80];                         // parallel port device (e.g.
#endif                                          //  /dev/parport0 or parallel.device)
  int parport_needed;
  int parport;                                  // parallel port address
  parport_mode_t parport_mode;                  // parallel port mode: ECP, EPP, SPP
#ifdef  USE_USB
  int usbport;                                  // non-zero => use usbport, 1 = USB0, 2 = USB1
  char usbport_dev[80];                         // usb port device (e.g. /dev/usb/hiddev0)
#endif

#ifdef  USE_ANSI_COLOR
  int ansi_color;
#endif
  int backup;                                   // flag if backups files should be created
  int frontend;                                 // flag if uCON64 was started by a frontend
#ifdef  USE_DISCMAGE
  int discmage_enabled;                         // flag if discmage DLL is loaded
#endif
  int dat_enabled;                              // flag if DAT file(s) are usable/enabled
  int quiet;                                    // quiet == -1 means verbose + 1

  int force_disc;                               // --disc was used
  uint32_t flags;                               // detect and init ROM info

  int do_not_calc_crc;                          // disable checksum calc. to speed up --ls,--lsv, etc.

  /*
    These values override values in st_rominfo_t. Use UCON64_ISSET()
    to check them. When adding new ones don't forget to update
    ucon64_execute_options() too.
  */
#if 1
  int buheader_len;                             // length of backup unit header 0 == no bu hdr
  int interleaved;                              // ROM is interleaved (swapped)
#else
  st_rominfo_t *override;                       // overrides values in (st_rominfo_t *) rominfo
#endif

#if 1
  int id;                                       // generate unique name (currently
                                                //  only used by snes_gd3())
  // the following values are for SNES, NES, Genesis and Nintendo 64
  int battery;                                  // NES UNIF/iNES/Pasofami
  int bs_dump;                                  // SNES "ROM" is a Broadcast Satellaview dump
  const char *comment;                          // NES UNIF
  int controller;                               // NES UNIF & SNES NSRT
  int controller2;                              // SNES NSRT
  const char *dump_info;                        // NES UNIF
  int io_mode;                                  // SNES SWC, Nintendo 64 CD64 & Cyan's Megadrive copier
  const char *mapr;                             // NES UNIF board name or iNES mapper number
  int mirror;                                   // NES UNIF/iNES/Pasofami
  int part_size;                                // SNES/Genesis split part size
  int region;                                   // Genesis (for -multi)
  int snes_header_base;                         // SNES ROM is "Extended" (or Sufami Turbo)
  int snes_hirom;                               // SNES ROM is HiROM
  int split;                                    // ROM is split
  int tv_standard;                              // NES UNIF
  int use_dump_info;                            // NES UNIF
  int vram;                                     // NES UNIF
#else
  void *console_workflow;                       // ptr to a console specific workflow
#endif

#ifdef  USE_DISCMAGE
  void *image;                                  // info from libdiscmage (dm_image_t *)
#endif
  void *dat;                                    // info from DATabase (st_ucon64_dat_t *)
  st_rominfo_t *rominfo;                        // info from <console>_init() (st_rominfo_t *)
  const st_getopt2_t *options;                        // all options with help (st_getopt2_t)
} st_ucon64_t;

typedef struct
{
  int console;                                  // UCON64_SNES, etc...
  uint32_t flags;                               // WF_INIT, etc..
//  const char *optarg;                           // pointer to optarg
                                                // initialised with NULL and used later
} st_ucon64_obj_t;

/*
  ucon64_init()         init st_rominfo_t, st_ucon64_dat_t and dm_image_t
  ucon64_nfo()          display contents of st_rominfo_t, st_ucon64_dat_t and
                          dm_image_t
  ucon64_flush_rom()    flush only st_rominfo_t
  ucon64_usage()        print usage
  ucon64_fname_arch()

  ucon64                global (st_ucon64_t *)
*/
extern int ucon64_init (void);
extern int ucon64_nfo (void);
extern st_rominfo_t *ucon64_flush_rom (st_rominfo_t *);
extern void ucon64_usage (int argc, char *argv[]);
#ifdef  USE_ZLIB
extern void ucon64_fname_arch (const char *fname);
#endif

extern st_ucon64_t ucon64;
#endif // UCON64_H
