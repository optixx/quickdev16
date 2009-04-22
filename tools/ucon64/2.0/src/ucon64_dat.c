/*
ucon64_dat.c - support for DAT files as known from RomCenter, GoodXXXX, etc.

Copyright (c) 1999 - 2004 NoisyB <noisyb@gmx.net>
Copyright (c) 2002 - 2004 dbjh


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
#include <sys/stat.h>
#include <time.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef  _WIN32
#include <windows.h>
#endif
#include "misc/misc.h"
#include "misc/getopt2.h"
#include "misc/property.h"
#include "misc/string.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ucon64_dat.h"
#include "console/console.h"

#define MAX_FIELDS_IN_DAT 32
#define DAT_FIELD_SEPARATOR (0xac)
#define DAT_FIELD_SEPARATOR_S "\xac"
#define MAX_GAMES_FOR_CONSOLE 50000             // TODO?: dynamic size

typedef struct
{
  const char *id;                               // string to detect console from datfile name
  int (*compare) (const void *a, const void *b); // the function which compares the id with the filename
                                                // compare() == 0 means success
  int8_t console;                               // UCON64_SNES, UCON64_NES, etc.
  const st_getopt2_t *console_usage;
} st_console_t;

typedef struct
{
  uint32_t crc32;
  long filepos;
} st_idx_entry_t;

typedef struct
{
  uint32_t crc32;
  char *fname;
} st_mkdat_entry_t;

#ifndef _WIN32
static DIR *ddat = NULL;
#else
static HANDLE ddat = NULL;
#endif
static FILE *fdat = NULL;
static int ucon64_n_files = 0, filepos_line = 0, warning = 1; // show the warning only
static FILE *ucon64_datfile;                                  //  once when indexing
static char ucon64_dat_fname[FILENAME_MAX];
static st_mkdat_entry_t *ucon64_mkdat_entries = NULL;

const st_getopt2_t ucon64_dat_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "DATabase (support for DAT files)",
      NULL
    },
    {
      "db", 0, 0, UCON64_DB,
      NULL, "DATabase statistics",
      &ucon64_wf[WF_OBJ_ALL_STOP_NO_ROM]
    },
    {
      "dbv", 0, 0, UCON64_DBV,
      NULL, "like " OPTION_LONG_S "db but more verbose",
      &ucon64_wf[WF_OBJ_ALL_STOP_NO_ROM]
    },
    {
      "dbs", 1, 0, UCON64_DBS,
      "CRC32", "search ROM with CRC32 in DATabase",
      &ucon64_wf[WF_OBJ_ALL_STOP_NO_ROM]
    },
    {
      "scan", 0, 0, UCON64_SCAN,
      NULL, "generate ROM list for all ROMs using DATabase\n"
      "like: GoodXXXX scan ...",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_SPLIT]
    },
    {
      "lsd", 0, 0, UCON64_LSD,
      NULL, "same as " OPTION_LONG_S "scan",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE]
    },
    {
      "mkdat", 1, 0, UCON64_MKDAT,
      "DATFILE", "create DAT file; use -o to specify an output directory",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE]
    },
    {
      "rrom", 0, 0, UCON64_RROM,
      NULL, "rename ROMs to their internal names",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_SPLIT]
    },
    {
      "rename", 0, 0, UCON64_RENAME,
      NULL, "rename ROMs to their DATabase names\n"
      "use -o to specify an output directory",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_SPLIT]
    },
    {
      "rr83", 0, 0, UCON64_RR83,
      NULL, "force to rename to 8.3 filenames",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_SPLIT]
    },
    {
      "force63", 0, 0, UCON64_FORCE63,
      NULL, "force to rename all filenames into Joliet CD format\n"
      "like: GoodXXXX rename inplace force63 ...\n"
      "TIP: using " OPTION_LONG_S "nes would process only NES ROMs",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "rl", 0, 0, UCON64_RL,
      NULL, "rename ROMs to lowercase",
      NULL
    },
    {
      "ru", 0, 0, UCON64_RU,
      NULL, "rename ROMs to uppercase",
      NULL
    },
#if 0
    {
      "good", 0, 0, UCON64_GOOD,
      NULL, "used with " OPTION_LONG_S "rrom and " OPTION_LONG_S "rr83 ROMs will be renamed using\n"
      "the DATabase",
      NULL
    },
#endif
    {
      NULL, 0, 0, 0,
      NULL, NULL,
      NULL
    }
  };


static void
closedir_ddat (void)
{
  if (ddat)
#ifndef _WIN32
    closedir (ddat);
#else
    FindClose (ddat);
#endif
  ddat = NULL;
}


static void
fclose_fdat (void)
{
  if (fdat)
    fclose (fdat);
  fdat = NULL;
}


static int
custom_stristr (const void *a, const void *b)
{
  return !stristr ((const char *) a, (const char *) b);
}


static int
custom_strnicmp (const void *a, const void *b)
{
  return strnicmp ((const char *) a, (const char *) b,
                   MIN (strlen ((const char *) a), strlen ((const char *) b)));
}


#if 0
static int
custom_stricmp (const void *a, const void *b)
{
  return stricmp ((const char *) a, (const char *) b);
}
#endif


static char *
get_next_file (char *fname)
{
#ifndef _WIN32
  struct dirent *ep;

  if (!ddat)
    if (!(ddat = opendir (ucon64.datdir)))
      {
        fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.datdir);
        return NULL;
      }
  while ((ep = readdir (ddat)) != NULL)
    if (!stricmp (get_suffix (ep->d_name), ".dat"))
      {
        sprintf (fname, "%s" FILE_SEPARATOR_S "%s", ucon64.datdir, ep->d_name);
        return fname;
      }
#else
  char search_pattern[FILENAME_MAX];
  WIN32_FIND_DATA find_data;

  if (!ddat)
    {
      // Note that FindFirstFile() & FindNextFile() are case insensitive
      sprintf (search_pattern, "%s" FILE_SEPARATOR_S "*.dat", ucon64.datdir);
      if ((ddat = FindFirstFile (search_pattern, &find_data)) == INVALID_HANDLE_VALUE)
        {
          // Not being able to find a DAT file is not a real error
          if (GetLastError () != ERROR_FILE_NOT_FOUND)
            {
              fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.datdir);
              return NULL;
            }
        }
      else
        {
          sprintf (fname, "%s" FILE_SEPARATOR_S "%s", ucon64.datdir, find_data.cFileName);
          return fname;
        }
    }
  while (FindNextFile (ddat, &find_data))
    {
      sprintf (fname, "%s" FILE_SEPARATOR_S "%s", ucon64.datdir, find_data.cFileName);
      return fname;
    }
#endif
  closedir_ddat ();
  return NULL;
}


static st_ucon64_dat_t *
get_dat_header (char *fname, st_ucon64_dat_t *dat)
{
  char buf[50 * 80];                            // should be enough

  // Hell yes! I (NoisyB) use get_property() here...
  strncpy (dat->author, get_property (fname, "author", buf, "Unknown"), sizeof (dat->author))[sizeof (dat->author) - 1] = 0;
  strncpy (dat->version, get_property (fname, "version", buf, "?"), sizeof (dat->version))[sizeof (dat->version) - 1] = 0;
  strncpy (dat->refname, get_property (fname, "refname", buf, ""), sizeof (dat->refname))[sizeof (dat->refname) - 1] = 0;
  strcpy (dat->comment, get_property (fname, "comment", buf, ""));
  strncpy (dat->date, get_property (fname, "date", buf, "?"), sizeof (dat->date))[sizeof (dat->date) - 1] = 0;

  return dat;
}


static int
fname_to_console (const char *fname, st_ucon64_dat_t *dat)
{
  int pos;
  // We use the filename to find out for what console a DAT file is meant.
  //  The field "refname" seems too unreliable.
  static const st_console_t console_type[] =
    {
      {"GoodSNES", custom_strnicmp, UCON64_SNES, snes_usage},
      {"SNES", custom_strnicmp, UCON64_SNES, snes_usage},
      {"GoodNES", custom_strnicmp, UCON64_NES, nes_usage},
      {"NES", custom_strnicmp, UCON64_NES, nes_usage},
      {"FDS", custom_stristr, UCON64_NES, nes_usage},
      {"GoodGBA", custom_strnicmp, UCON64_GBA, gba_usage},
      {"GBA", custom_strnicmp, UCON64_GBA, gba_usage},
      {"GoodGBX", custom_strnicmp, UCON64_GB, gameboy_usage},
      {"GBX", custom_strnicmp, UCON64_GB, gameboy_usage},
      {"GoodGEN", custom_strnicmp, UCON64_GEN, genesis_usage},
      {"GEN", custom_strnicmp, UCON64_GEN, genesis_usage},
      {"GoodGG", custom_strnicmp, UCON64_SMS, sms_usage},
      {"GG", custom_strnicmp, UCON64_SMS, sms_usage},
      {"GoodSMS", custom_strnicmp, UCON64_SMS, sms_usage},
      {"SMS", custom_strnicmp, UCON64_SMS, sms_usage},
      {"GoodJAG", custom_strnicmp, UCON64_JAG, jaguar_usage},
      {"JAG", custom_strnicmp, UCON64_JAG, jaguar_usage},
      {"GoodLynx", custom_strnicmp, UCON64_LYNX, lynx_usage},
      {"Lynx", custom_strnicmp, UCON64_LYNX, lynx_usage},
      {"GoodN64", custom_strnicmp, UCON64_N64, n64_usage},
      {"N64", custom_strnicmp, UCON64_N64, n64_usage},
      {"GoodPCE", custom_strnicmp, UCON64_PCE, pcengine_usage},
      {"PCE", custom_strnicmp, UCON64_PCE, pcengine_usage},
      {"Good2600", custom_strnicmp, UCON64_ATA, atari_usage},
      {"2600", custom_strnicmp, UCON64_ATA, atari_usage},
      {"Good5200", custom_strnicmp, UCON64_ATA, atari_usage},
      {"5200", custom_strnicmp, UCON64_ATA, atari_usage},
      {"Good7800", custom_strnicmp, UCON64_ATA, atari_usage},
      {"7800", custom_strnicmp, UCON64_ATA, atari_usage},
      {"GoodVECT", custom_strnicmp, UCON64_VEC, vectrex_usage},
      {"Vectrex", custom_stristr, UCON64_VEC, vectrex_usage},
      {"GoodWSX", custom_strnicmp, UCON64_SWAN, swan_usage},
      {"swan", custom_stristr, UCON64_SWAN, swan_usage},
      {"GoodCOL", custom_strnicmp, UCON64_COLECO, coleco_usage},
      {"Coleco", custom_stristr, UCON64_COLECO, coleco_usage},
      {"GoodINTV", custom_strnicmp, UCON64_INTELLI, intelli_usage},
      {"Intelli", custom_stristr, UCON64_INTELLI, intelli_usage},
      {"GoodNGPX", custom_strnicmp, UCON64_NGP, ngp_usage},
      {"NGP", custom_strnicmp, UCON64_NGP, ngp_usage},
      {"GoodVBOY", custom_strnicmp, UCON64_VBOY, vboy_usage},
      {"VBOY", custom_strnicmp, UCON64_VBOY, vboy_usage},
      
      {"Neo-Geo", custom_strnicmp, UCON64_NG, neogeo_usage},
      {"MAME", custom_stristr, UCON64_MAME, mame_usage},
      {"Dreamcast", custom_stristr, UCON64_DC, dc_usage},
      {"Saturn", custom_stristr, UCON64_SAT, sat_usage},
      {"3do", custom_stristr, UCON64_3DO, real3do_usage},
      {"CDi", custom_stristr, UCON64_CDI, cdi_usage},
      {"XBox", custom_stristr, UCON64_XBOX, xbox_usage},
      {"CD32", custom_stristr, UCON64_CD32, cd32_usage},
/* TODO:
      {"psx", custom_stristr, UCON64_PSX, psx_usage},
      {"ps1", custom_stristr, UCON64_PSX, psx_usage},
      {"psone", custom_stristr, UCON64_PSX, psx_usage},
      {"ps2", custom_stristr, UCON64_PS2, ps2_usage},
      {"dc", custom_stristr, UCON64_DC, dc_usage},
      {"system", custom_stristr, UCON64_S16, s16_usage},
      {"pocket", custom_stristr, UCON64_NGP, ngp_usage},
      {"virtual", custom_stristr, UCON64_VBOY, vboy_usage},
      {"", custom_stristr, 0, channelf_usage},
      {"", custom_stristr, 0, gamecom_usage},
      {"", custom_stristr, 0, gc_usage},
      {"", custom_stristr, 0, gp32_usage},
      {"", custom_stristr, 0, odyssey2_usage},
      {"", custom_stristr, 0, odyssey_usage},
      {"", custom_stristr, 0, s16_usage},
      {"", custom_stristr, 0, sat_usage},
      {"", custom_stristr, 0, vc4000_usage},
*/
      {0, 0, 0, 0}
    };

  for (pos = 0; console_type[pos].id; pos++)
    {
      if (!console_type[pos].compare (fname, console_type[pos].id))
        {
          dat->console = console_type[pos].console;
          dat->console_usage = (console_type[pos].console_usage[0].help);
          break;
        }
    }

  if (console_type[pos].id == 0)
    {
      if (warning)
        {
          printf ("WARNING: \"%s\" is meant for a console unknown to uCON64\n\n", fname);
          warning = 0;
        }
      dat->console = UCON64_UNKNOWN;
      dat->console_usage = NULL;
    }

  return dat->console;
}


static st_ucon64_dat_t *
line_to_dat (const char *fname, const char *dat_entry, st_ucon64_dat_t *dat)
// parse a dat entry into st_ucon64_dat_t
{
  static const char *dat_country[28][2] =
    {
      {"(FC)", "French Canada"},
      {"(FN)", "Finland"},
      {"(G)", "Germany"},
      {"(GR)", "Greece"},
      {"(H)", "Holland"},               // other (incorrect) name for The Netherlands
      {"(HK)", "Hong Kong"},
      {"(I)", "Italy"},
      {"(J)", "Japan"},
      {"(JE)", "Japan & Europe"},
      {"(JU)", "Japan & U.S.A."},
      {"(JUE)", "Japan, U.S.A. & Europe"},
      {"(K)", "Korea"},
      {"(NL)", "The Netherlands"},
      {"(PD)", "Public Domain"},
      {"(S)", "Spain"},
      {"(SW)", "Sweden"},
      {"(U)", "U.S.A."},
      {"(UE)", "U.S.A. & Europe"},
      {"(UK)", "England"},
      {"(Unk)", "Unknown country"},
      /*
        At least (A), (B), (C), (E) and (F) have to come after the other
        countries, because some games have (A), (B) etc. in their name (the
        non-country part). For example, the SNES games
        "SD Gundam Generations (A) 1 Nen Sensouki (J) (ST)" or
        "SD Gundam Generations (B) Guripus Senki (J) (ST)".
      */
      {"(1)", "Japan & Korea"},
      {"(4)", "U.S.A. & Brazil NTSC"},
      {"(A)", "Australia"},
      {"(B)", "non U.S.A. (Genesis)"},
      {"(C)", "China"},
      {"(E)", "Europe"},
      {"(F)", "France"},
      {NULL, NULL}
    };
  static const char *dat_flags[][2] =
    {
      // Often flags contain numbers, so don't search for the closing bracket
      {"[a", "Alternate"},
      {"[p", "Pirate"},
      {"[b", "Bad dump"},
      {"[t", "Trained"},
      {"[f", "Fixed"},
      {"[T", "Translation"},
      {"[h", "Hack"},
      {"[x", "Bad checksum"},
      {"[o", "Overdump"},
      {"[!]", "Verified good dump"}, // [!] is ok
      {NULL, NULL}
    };
  char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL }, buf[MAXBUFSIZE], *p = NULL;
  uint32_t pos = 0;
  int x = 0;

  if ((unsigned char) dat_entry[0] != DAT_FIELD_SEPARATOR)
    return NULL;

  strcpy (buf, dat_entry);

  strarg (dat_field, buf, DAT_FIELD_SEPARATOR_S, MAX_FIELDS_IN_DAT);

  memset (dat, 0, sizeof (st_ucon64_dat_t));

  strcpy (dat->datfile, basename2 (fname));

  if (dat_field[3])
    strcpy (dat->name, dat_field[3]);

  if (dat_field[4])
    strcpy (dat->fname, dat_field[4]);

  if (dat_field[5])
    sscanf (dat_field[5], "%x", (unsigned int *) &dat->crc32);

  if (dat_field[6][0] == 'N' && dat_field[7][0] == 'O')
    // e.g. GoodSNES bad crc & Nintendo FDS DAT
    sscanf (dat_field[8], "%d", (int *) &dat->fsize);
  else
    sscanf (dat_field[6], "%d", (int *) &dat->fsize);

  *buf = 0;
  for (x = 0, p = buf; dat_flags[x][0]; x++, p += strlen (p))
    if (strstr (dat->name, dat_flags[x][0]))
      sprintf (p, "%s, ", dat_flags[x][1]);
  if (buf[0])
    {
      if ((p = strrchr (buf, ',')))
        *p = 0;
      sprintf (dat->misc, "Flags: %s", buf);
    }

  p = dat->name;
  dat->country = NULL;
  for (pos = 0; dat_country[pos][0]; pos++)
    if (stristr (p, dat_country[pos][0]))
      {
        dat->country = dat_country[pos][1];
        break;
      }

  fname_to_console (dat->datfile, dat);
  dat->copier_usage = unknown_usage[0].help;

  return dat;
}


uint32_t
line_to_crc (const char *dat_entry)
// get crc32 of current line
{
  char *dat_field[MAX_FIELDS_IN_DAT + 2] = { NULL }, buf[MAXBUFSIZE];
  unsigned int crc32 = 0;                       // has to be unsigned int to
                                                //  avoid a stupid gcc warning
  if ((unsigned char) dat_entry[0] != DAT_FIELD_SEPARATOR)
    return 0;

  strcpy (buf, dat_entry);

  strarg (dat_field, buf, DAT_FIELD_SEPARATOR_S, MAX_FIELDS_IN_DAT);

  if (dat_field[5])
    sscanf (dat_field[5], "%x", &crc32);

  return (uint32_t) crc32;
}


static st_ucon64_dat_t *
get_dat_entry (char *fname, st_ucon64_dat_t *dat, uint32_t crc32, long start)
{
  char buf[MAXBUFSIZE];

  if (!fdat)
    if (!(fdat = fopen (fname, "rb")))
      {
        fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], fname);
#if     defined _WIN32 || defined __CYGWIN__ || defined __MSDOS__
        if (!stricmp (basename2 (fname), "ntuser.dat"))
          fputs ("       Please see the FAQ, question 47 & 36\n", stderr);
          //     "ERROR: "
#endif
        return NULL;
      }

  if (start >= 0)
    fseek (fdat, start, SEEK_SET);

  filepos_line = ftell (fdat);
  while (fgets (buf, MAXBUFSIZE, fdat) != NULL)
    {
      if ((unsigned char) buf[0] == DAT_FIELD_SEPARATOR)
        if (!crc32 || line_to_crc (buf) == crc32)
          if (line_to_dat (fname, buf, dat))
            return dat;
      filepos_line = ftell (fdat);
    }

  fclose_fdat ();
  return NULL;
}


int
ucon64_dat_view (int console, int verbose)
{
  char fname_dat[FILENAME_MAX], fname_index[FILENAME_MAX];
  const char *fname;
  unsigned char *p;
  static st_ucon64_dat_t dat;
  int n, fsize, n_entries, n_entries_sum = 0, n_datfiles = 0;
  st_idx_entry_t *idx_entry;

  while (get_next_file (fname_dat))
    {
      fname = basename2 (fname_dat);
      if (console != UCON64_UNKNOWN)
        if (fname_to_console (fname, &dat) != console)
          continue;

      get_dat_header (fname_dat, &dat);
      strcpy (fname_index, fname_dat);
      set_suffix (fname_index, ".idx");

      fsize = fsizeof (fname_index);
      n_entries = fsize / sizeof (st_idx_entry_t);
      n_entries_sum += n_entries;
      n_datfiles++;

      printf ("DAT info:\n"
        "  %s\n"
//        "  Console: %s\n"
        "  Version: %s (%s, %s)\n"
        "  Author: %s\n"
        "  Comment: %s\n"
        "  Entries: %d\n\n",
        fname,
//        dat.console_usage[0],
        dat.version,
        dat.date,
        dat.refname,
        dat.author,
        dat.comment,
        n_entries);

      if (!(p = (unsigned char *) malloc (fsize)))
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], fsize);
          continue;
        }

      if (ucon64_fread (p, 0, fsize, fname_index) != fsize)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], fname_index);
          free (p);
          continue;
        }

      if (verbose)
        {
          // display all DAT entries
          for (n = 0; n < n_entries; n++)
            {
              idx_entry = &((st_idx_entry_t *) p)[n];
              printf ("Checksum (CRC32): 0x%08x\n", (unsigned int) idx_entry->crc32);
              if (get_dat_entry (fname_dat, &dat, idx_entry->crc32, idx_entry->filepos))
                ucon64_dat_nfo (&dat, 0);
              fputc ('\n', stdout);
            }
          fclose_fdat ();
        }
      free (p);
    }

  printf ("DAT files: %d; entries: %d; total entries: %d\n",
    n_datfiles, n_entries_sum, ucon64_dat_total_entries ());

  return 0;
}


unsigned int
ucon64_dat_total_entries (void)
{
  uint32_t entries = 0;
  int fsize;
  char fname[FILENAME_MAX];

  if (!ucon64.dat_enabled)
    return 0;

  while (get_next_file (fname))
    {
      set_suffix (fname, ".idx");
      fsize = fsizeof (fname);
      entries += (fsize < 0 ? 0 : fsize / sizeof (st_idx_entry_t));
    }

  return entries;
}


static int
idx_compare (const void *key, const void *found)
{
  /*
    The return statement looks overly complicated, but is really necessary.
    This construct:
      return ((st_idx_entry_t *) key)->crc32 - ((st_idx_entry_t *) found)->crc32;
    does *not* work correctly for all cases.
  */
  return (int) (((int64_t) ((st_idx_entry_t *) key)->crc32 -
                 (int64_t) ((st_idx_entry_t *) found)->crc32) / 2);
}


st_ucon64_dat_t *
ucon64_dat_search (uint32_t crc32, st_ucon64_dat_t *datinfo)
{
  char fname_dat[FILENAME_MAX], fname_index[FILENAME_MAX];
  const char *fname;
  unsigned char *p = NULL;
  int32_t fsize = 0;
  st_idx_entry_t *idx_entry, key;
  static st_ucon64_dat_t dat;
  st_ucon64_dat_t *dat_p = NULL;

  memset (&dat, 0, sizeof (st_ucon64_dat_t));

  if (!crc32)
    return NULL;

  while (get_next_file (fname_dat))
    {
      fname = basename2 (fname_dat);

      if (ucon64.console != UCON64_UNKNOWN)
        if (fname_to_console (fname, &dat) != ucon64.console)
          continue;

      strcpy (fname_index, fname_dat);
      set_suffix (fname_index, ".idx");
      if (access (fname_index, F_OK) != 0)      // for a "bad" DAT file
        continue;
      fsize = fsizeof (fname_index);

      if (!(p = (unsigned char *) malloc (fsize)))
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], fsize);
          closedir_ddat ();
          return NULL;
        }

      // load the index for the current dat file
      if (ucon64_fread (p, 0, fsize, fname_index) != fsize)
        {
          fprintf (stderr, ucon64_msg[READ_ERROR], fname_index);
          closedir_ddat ();
          free (p);
          return NULL;
        }

      // search index for crc
      key.crc32 = crc32;
      idx_entry = (st_idx_entry_t *) bsearch (&key, p, fsize / sizeof (st_idx_entry_t),
                                              sizeof (st_idx_entry_t), idx_compare);
      if (idx_entry)                            // crc32 found
        {
          if (!datinfo)
            dat_p = (st_ucon64_dat_t *) &dat;   // TODO?: malloc()
          else
            dat_p = (st_ucon64_dat_t *) &datinfo;

          // open dat file and read entry
          if (get_dat_entry (fname_dat, dat_p, crc32, idx_entry->filepos))
            if (crc32 == dat_p->crc32)
              {
                strcpy (dat_p->datfile, basename2 (fname_dat));
                get_dat_header (fname_dat, dat_p);
                closedir_ddat ();
                fclose_fdat ();
                free (p);
                return dat_p;
              }
          fclose_fdat ();
        }
      free (p);
    }

  return NULL;
}


int
ucon64_dat_indexer (void)
// create or update index of DAT file
{
  char fname_dat[FILENAME_MAX], fname_index[FILENAME_MAX], errorfname[FILENAME_MAX];
  struct stat fstate_dat, fstate_index;
  st_ucon64_dat_t dat;
  FILE *errorfile;
  time_t start_time = 0;
  int update = 0, n_duplicates, n, size = 0, pos;
  st_idx_entry_t *idx_entries, *idx_entry;

  warning = 1; // enable warning again for DATs with unrecognized console systems

  if (!(idx_entries = (st_idx_entry_t *)
          malloc (MAX_GAMES_FOR_CONSOLE * sizeof (st_idx_entry_t))))
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR],
        MAX_GAMES_FOR_CONSOLE * sizeof (st_idx_entry_t));
      exit (1);
    }

  while (get_next_file (fname_dat))
    {
      strcpy (fname_index, fname_dat);
      set_suffix (fname_index, ".idx");

      if (!stat (fname_dat, &fstate_dat) && !stat (fname_index, &fstate_index))
        {
          if (fstate_dat.st_mtime < fstate_index.st_mtime)
            continue;                   // index file seems to be present and up-to-date
          update = 1;
        }

      start_time = time (0);
      size = fsizeof (fname_dat);

      printf ("%s: %s\n", (update ? "Update" : "Create"), basename2 (fname_index));
      pos = 0;
      n_duplicates = 0;
      errorfile = NULL;
      while (get_dat_entry (fname_dat, &dat, 0, -1))
        {
          if (pos == MAX_GAMES_FOR_CONSOLE)
            {
              fprintf (stderr,
                       "\n"
                       "INTERNAL ERROR: MAX_GAMES_FOR_CONSOLE is too small (%d)\n",
                       MAX_GAMES_FOR_CONSOLE);
              break;
            }

          /*
            Doing a linear search removes the need of using the slow qsort()
            function inside the loop. Doing a binary search doesn't improve the
            speed much, but is much more efficient of course. Using qsort()
            inside the loop slows it down with a factor greater than 10.
          */
          idx_entry = NULL;
          for (n = 0; n < pos; n++)
            if (idx_entries[n].crc32 == dat.crc32)
              idx_entry = &idx_entries[n];
          if (idx_entry)
            {
              // This really makes one loose trust in the DAT files...
              char current_name[2 * 80];
              long current_filepos = ftell (fdat);

              if (!errorfile)
                {
                  strcpy (errorfname, fname_index);
                  set_suffix (errorfname, ".err");
                  if (!(errorfile = fopen (errorfname, "w"))) // text file for WinDOS
                    {
                      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], errorfname);
                      continue;
                    }
                }

              strcpy (current_name, dat.name);
              get_dat_entry (fname_dat, &dat, 0, idx_entry->filepos);
              fprintf (errorfile,
                       "\n"
                       "WARNING: DAT file contains a duplicate CRC32 (0x%x)!\n"
                       "  First game with this CRC32: \"%s\"\n"
                       "  Ignoring game:              \"%s\"\n",
                       (unsigned int) dat.crc32, dat.name, current_name);

              n_duplicates++;
              fseek (fdat, current_filepos, SEEK_SET);
              continue;
            }

          idx_entries[pos].crc32 = dat.crc32;
          idx_entries[pos].filepos = filepos_line;

          if (!(pos % 20))
            ucon64_gauge (start_time, ftell (fdat), size);
          pos++;
        }
      fclose_fdat ();

      if (pos > 0)
        {
          qsort (idx_entries, pos, sizeof (st_idx_entry_t), idx_compare);
          if (ucon64_fwrite (idx_entries, 0, pos * sizeof (st_idx_entry_t), fname_index, "wb")
              != (int) (pos * sizeof (st_idx_entry_t)))
            {
              fputc ('\n', stderr);
              fprintf (stderr, ucon64_msg[WRITE_ERROR], fname_index);
            }
          ucon64_gauge (start_time, size, size);
        }

      if (n_duplicates > 0)
        printf ("\n"
                "\n"
                "WARNING: DAT file contains %d duplicate CRC32%s\n"
                "         Warnings have been written to \"%s\"",
                n_duplicates, n_duplicates != 1 ? "s" : "", errorfname);
      if (errorfile)
        {
          fclose (errorfile);
          errorfile = NULL;
        }
      fputs ("\n\n", stdout);
    }
  free (idx_entries);

  return 0;
}


#if 0
st_ucon64_dat_t *
ucon64_dat_flush (st_ucon64_dat_t *dat)
{
  memset (dat, 0, sizeof (st_ucon64_dat_t));
  ucon64.dat = NULL;
  return NULL;
}
#endif


void
ucon64_dat_nfo (const st_ucon64_dat_t *dat, int display_version)
{
  char buf[MAXBUFSIZE], *p = NULL;
  int n;

  if (!dat)
    {
      printf (ucon64_msg[DAT_NOT_FOUND], ucon64.crc32);
      return;
    }

  fputs ("DAT info:\n", stdout);
  // console type?
  if (dat->console_usage != NULL)
    {
      strcpy (buf, dat->console_usage);
      // fix ugly multi-line console "usages" (PC-Engine)
      if ((p = strchr (buf, '\n')) != NULL)
        *p = 0;
      printf ("  %s\n", buf);
    }

  printf ("  %s\n", dat->name);

  if (dat->country)
    printf ("  %s\n", dat->country);

  /*
    The DAT files are not consistent. Some include the file suffix, but
    others don't. We want to display the canonical file name only if it
    really differs from the canonical game name (usually file name without
    suffix).
  */
  n = strlen (dat->fname);
  p = (char *) get_suffix (dat->fname);
  if (!(stricmp (p, ".nes") &&                  // NES
        stricmp (p, ".fds") &&                  // NES FDS
        stricmp (p, ".gb") &&                   // Game Boy
        stricmp (p, ".gbc") &&                  // Game Boy Color
        stricmp (p, ".gba") &&                  // Game Boy Advance
        stricmp (p, ".smc") &&                  // SNES
        stricmp (p, ".sc") &&                   // Sega Master System
        stricmp (p, ".sg") &&                   // Sega Master System
        stricmp (p, ".sms") &&                  // Sega Master System
        stricmp (p, ".gg") &&                   // Game Gear
        stricmp (p, ".smd") &&                  // Genesis
        stricmp (p, ".v64")))                   // Nintendo 64
    ((char *) dat->fname)[strlen (dat->fname) - strlen (p)] = 0;

  if (stricmp (dat->name, dat->fname) != 0)
    printf ("  Filename: %s\n", dat->fname);

  printf ("  %d Bytes (%.4f Mb)\n", (int) dat->fsize, TOMBIT_F (dat->fsize));

  if (dat->misc[0])
    printf ("  %s\n", dat->misc);

  if (display_version)
    {
      if (stristr (dat->datfile, dat->version))
        printf ("  %s (%s, %s)\n",
          dat->datfile,
          dat->date,
          dat->refname);
      else
        printf ("  %s (%s, %s, %s)\n",
          dat->datfile,
          dat->version,
          dat->date,
          dat->refname);
    }
}


static void
ucon64_close_datfile (void)
{
  int n;

  if (ucon64_datfile)
    {
      fclose (ucon64_datfile);
      printf (ucon64_msg[WROTE], ucon64_dat_fname);
      ucon64_datfile = NULL;

      for (n = 0; n < ucon64_n_files; n++)
        {
          free (ucon64_mkdat_entries[n].fname);
          ucon64_mkdat_entries[n].fname = NULL;
        }
      ucon64_n_files = 0;
    }
}


int
ucon64_create_dat (const char *dat_file_name, const char *filename,
                   int buheader_len)
{
  static int first_file = 1, console;
  int n, x;
  static char *console_name;
  char fname[FILENAME_MAX], *ptr;
  time_t time_t_val;
  struct tm *t;

  if (first_file)
    {
      char *plugin = "";

      first_file = 0;
      console = ucon64.console;
      switch (ucon64.console)
        {
          case UCON64_3DO:
            console_name = "3DO";
            break;
          case UCON64_ATA:
            console_name = "NES";
            break;
          case UCON64_CD32:
            console_name = "CD32";
            break;
          case UCON64_CDI:
            console_name = "CD-i";
            break;
          case UCON64_COLECO:
            console_name = "Coleco";
            break;
          case UCON64_DC:
            console_name = "Dreamcast";
            break;
          case UCON64_GB:
            console_name = "Game Boy";
            break;
          case UCON64_GBA:
            console_name = "Game Boy Advance";
            break;
          case UCON64_GC:
            console_name = "Game Cube";
            break;
          case UCON64_GEN:
            console_name = "Genesis/Mega Drive";
            plugin = "genesis.dll";
            break;
          case UCON64_INTELLI:
            console_name = "Intellivision";
            break;
          case UCON64_JAG:
            console_name = "Jaguar";
            break;
          case UCON64_LYNX:
            console_name = "Lynx";
            break;
          case UCON64_MAME:
            console_name = "M.A.M.E.";
            plugin = "arcade.dll";
            break;
          case UCON64_N64:
            console_name = "Nintendo 64";
            plugin = "n64.dll";
            break;
          case UCON64_NES:
            console_name = "NES";
            plugin = "nes.dll";
            break;
          case UCON64_NG:
            console_name = "Neo Geo";
            plugin = "arcade.dll";
            break;
          case UCON64_NGP:
            console_name = "Neo Geo Pocket";
            break;
          case UCON64_PCE:
            console_name = "PC-Engine";
            break;
          case UCON64_PS2:
            console_name = "Playstation 2";
            break;
          case UCON64_PSX:
            console_name = "Playstation";
            break;
          case UCON64_S16:
            console_name = "S16";
            break;
          case UCON64_SAT:
            console_name = "Saturn";
            break;
          case UCON64_SMS:
            console_name = "SMS/Game Gear";
            break;
          case UCON64_SNES:
            console_name = "SNES";
            plugin = "snes.dll";        // be sure to use the new SNES plug-in (RC 2.62)
            break;
          case UCON64_SWAN:
            console_name = "Wonderswan";
            break;
          case UCON64_VBOY:
            console_name = "Virtual Boy";
            break;
          case UCON64_VEC:
            console_name = "Vectrex";
            break;
          case UCON64_XBOX:
            console_name = "XBox";
            break;
          default:
            fputs (ucon64_msg[CONSOLE_ERROR], stderr);
            exit (1);
            break;
        }

      if (!(ucon64_mkdat_entries = (st_mkdat_entry_t *)
              malloc (MAX_GAMES_FOR_CONSOLE * sizeof (st_mkdat_entry_t))))
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR],
            MAX_GAMES_FOR_CONSOLE * sizeof (st_mkdat_entry_t));
          exit (1);
        }

      strcpy (ucon64_dat_fname, dat_file_name);
      ucon64_file_handler (ucon64_dat_fname, NULL, OF_FORCE_BASENAME);
      if (!(ucon64_datfile = fopen (ucon64_dat_fname, "wb")))
        {
          fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], ucon64_dat_fname);
          exit (1);
        }
      register_func (ucon64_close_datfile);

      time_t_val = time (NULL);
      t = localtime (&time_t_val);
      // RomCenter uses files in DOS text format, so we generate a file in that format
      fprintf (ucon64_datfile, "[CREDITS]\r\n"
                               "author=uCON64\r\n"
                               "email=noisyb@gmx.net\r\n"
                               "homepage=uCON64 homepage\r\n"
                               "url=ucon64.sf.net\r\n"
                               "version=%s-%s\r\n"
                               "date=%d/%d/%d\r\n"
                               "comment=%s DAT file generated by uCON64\r\n"
                               "[DAT]\r\n"
                               "version=2.50\r\n" // required by RomCenter!
                               "plugin=%s\r\n"
                               "[EMULATOR]\r\n"
                               "refname=%s\r\n"
                               "version=\r\n"
                               "[GAMES]\r\n",
                               UCON64_VERSION_S, console_name,
                               t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                               console_name,
                               plugin,
                               console_name);
    } // first_file

  if (ucon64_n_files == MAX_GAMES_FOR_CONSOLE)
    {
      fprintf (stderr,
               "INTERNAL ERROR: MAX_GAMES_FOR_CONSOLE is too small (%d)\n",
               MAX_GAMES_FOR_CONSOLE);
      exit (1);
    }
  strcpy (fname, basename2 (filename));

  // Check the console type
  n = 0;
  if (ucon64.console != console)
    {
      if (ucon64.quiet == -1)
        printf ("WARNING: Skipping (!%s) ", console_name);
      else
        return -1;
    }
  else
    {
      // Check if the CRC32 is unique. We don't want to be as stupid as
      //  the tool used to create the GoodDAT files.
      // Yes, a plain and simple linear search. Analysing the files is orders
      //  of magnitude slower than this search
      for (; n < ucon64_n_files; n++)
        if (ucon64_mkdat_entries[n].crc32 == ucon64.crc32)
          break;
      if (n != ucon64_n_files)
        {
          if (ucon64.quiet < 1)                 // better print this by default
            fputs ("WARNING: Skipping (duplicate) ", stdout);
          else
            return -1;
        }
    }

  fputs (filename, stdout);
  if (ucon64.quiet == -1)                       // -v was specified
    if (ucon64.fname_arch[0])
      printf (" (%s)", ucon64.fname_arch);
  fputc ('\n', stdout);

  if (ucon64.console != console)                // ucon64.quiet == -1
    return -1;
  if (n != ucon64_n_files)
    {
      if (ucon64.quiet < 1)                     // better print this by default
        printf ("         First file with this CRC32 (0x%x) is:\n"
                "         \"%s\"\n", ucon64.crc32, ucon64_mkdat_entries[n].fname);
      return -1;
    }

  // Store the CRC32 to check if a file is unique
  ucon64_mkdat_entries[ucon64_n_files].crc32 = ucon64.crc32;
  /*
    Also store the name of the file to display a helpful error message if a
    file is not unique (a duplicate). We store the current filename inside the
    archive as well, to be even more helpful :-)
  */
  x = strlen (fname) + (ucon64.fname_arch[0] ? strlen (ucon64.fname_arch) + 4 : 1);
  if (!(ucon64_mkdat_entries[ucon64_n_files].fname = (char *) malloc (x)))
    {                                                 // + 3 for " ()"
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], x);  //  + 1 for ASCII-z
      exit (1);
    }
  sprintf (ucon64_mkdat_entries[ucon64_n_files].fname, "%s%s%s%s",
    fname,
    ucon64.fname_arch[0] ? " (" : "",
    ucon64.fname_arch[0] ? ucon64.fname_arch : "",
    ucon64.fname_arch[0] ? ")" : "");

  ptr = (char *) get_suffix (fname);
  if (*ptr)
    *ptr = 0;
  fprintf (ucon64_datfile, DAT_FIELD_SEPARATOR_S "%s" // set file name
                           DAT_FIELD_SEPARATOR_S "%s" // set full name
                           DAT_FIELD_SEPARATOR_S "%s" // clone file name
                           DAT_FIELD_SEPARATOR_S "%s" // clone full name
                           DAT_FIELD_SEPARATOR_S "%s" // rom file name
                           DAT_FIELD_SEPARATOR_S "%08x" // RC quirck: leading zeroes are required
                           DAT_FIELD_SEPARATOR_S "%d"
                           DAT_FIELD_SEPARATOR_S // merged clone name
                           DAT_FIELD_SEPARATOR_S // merged rom name
                           DAT_FIELD_SEPARATOR_S "\r\n",
                           fname,
                           fname,
                           fname,
                           fname,
                           fname,
                           ucon64.crc32,
                           ucon64.file_size - buheader_len);
  ucon64_n_files++;
  return 0;
}
