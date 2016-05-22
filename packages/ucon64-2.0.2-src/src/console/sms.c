/*
sms.c - Sega Master System/Game Gear support for uCON64

Copyright (c) 1999 - 2001 NoisyB
Copyright (c) 2003 - 2005 dbjh


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
#include <ctype.h>
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
#include "misc/chksum.h"
#include "misc/file.h"
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/misc.h"
#include "misc/string.h"
#include "ucon64_misc.h"
#include "console/sms.h"
#include "backup/mgd.h"
#include "backup/smd.h"
#include "backup/smsgg-pro.h"


#define SMS_HEADER_START 0x7ff0
#define SMS_HEADER_LEN (sizeof (st_sms_header_t))

static int sms_chksum (unsigned char *rom_buffer, int rom_size);


static st_ucon64_obj_t sms_obj[] =
  {
    {0, WF_SWITCH},
    {0, WF_DEFAULT},
    {0, WF_DEFAULT | WF_NO_SPLIT},
    {0, WF_INIT | WF_PROBE | WF_STOP},
    {UCON64_SMS, WF_SWITCH},
    {UCON64_SMS, WF_DEFAULT | WF_NO_SPLIT}
  };

const st_getopt2_t sms_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Sega Master System(II/III)/Game Gear (Handheld)"/*"1986/1990 SEGA http://www.sega.com"*/,
      NULL
    },
    {
      UCON64_SMS_S, 0, 0, UCON64_SMS,
      NULL, "force recognition",
      &sms_obj[4]
    },
    {
      "int", 0, 0, UCON64_INT,
      NULL, "force ROM is in interleaved format",
      &sms_obj[0]
    },
    {
      "nint", 0, 0, UCON64_NINT,
      NULL, "force ROM is not in interleaved format",
      &sms_obj[0]
    },
    {
      "mgd", 0, 0, UCON64_MGD,
      NULL, "convert to Multi Game*/MGD2/MGH/RAW (gives SMS name)",
      &sms_obj[2]
    },
    {
      "mgdgg", 0, 0, UCON64_MGDGG,
      NULL, "same as " OPTION_LONG_S "mgd, but gives GG name",
      &sms_obj[5]
    },
    {
      "smd", 0, 0, UCON64_SMD,
      NULL, "convert to Super Magic Drive/SMD",
      &sms_obj[2]
    },
    {
      "smds", 0, 0, UCON64_SMDS,
      NULL, "convert *.srm (SRAM) file to Super Magic Drive/SMD",
      NULL
    },
    {
      "chk", 0, 0, UCON64_CHK,
      NULL, "fix ROM checksum (SMS only)",
      &sms_obj[1]
    },
    {
      "multi", 1, 0, UCON64_MULTI,
      "SIZE", "make multi-game file for use with SMS-PRO/GG-PRO flash card,\n"
      "truncated to SIZE Mbit; file with loader must be specified\n"
      "first, then all the ROMs, multi-game file to create last",
      &sms_obj[3]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

typedef struct st_sms_header
{
  char signature[8];                            // "TMR "{"SEGA", "ALVS", "SMSC"}/"TMG SEGA"
  unsigned char pad[2];                         // 8
  unsigned char checksum_low;                   // 10
  unsigned char checksum_high;                  // 11
  unsigned char partno_low;                     // 12
  unsigned char partno_high;                    // 13
  unsigned char version;                        // 14
  unsigned char checksum_range;                 // 15, and country info
} st_sms_header_t;

static st_sms_header_t sms_header;
static int is_gamegear;


// see src/backup/mgd.h for the file naming scheme
int
sms_mgd (st_ucon64_nfo_t *rominfo, int console)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *buffer;
  int size = ucon64.file_size - rominfo->backup_header_len;

  strcpy (src_name, ucon64.fname);
  mgd_make_name (ucon64.fname, console, size, dest_name);
  ucon64_file_handler (dest_name, src_name, OF_FORCE_BASENAME);

  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  ucon64_fread (buffer, rominfo->backup_header_len, size, src_name);
  if (rominfo->interleaved)
    smd_deinterleave (buffer, size);

  ucon64_fwrite (buffer, 0, size, dest_name, "wb");
  free (buffer);

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  mgd_write_index_file ((char *) basename2 (dest_name), 1);

  /*
    Not sure if this is correct (taken from a mgd converter script):
    1MBIT: .040, 2MBIT: .040, 4MBIT: .058, 8MBIT: .058
  */
  if (size <= 4 * MBIT)
    puts ("NOTE: It may be necessary to change the suffix in order to make the game work\n"
          "      on an MGD2. You could try suffixes like .010, .024, .040, .048 or .078.");
  return 0;
}


int
sms_smd (st_ucon64_nfo_t *rominfo)
{
  st_smd_header_t header;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *buffer;
  int size = ucon64.file_size - rominfo->backup_header_len;

  memset (&header, 0, SMD_HEADER_LEN);
  header.size = (unsigned char) (size / 16384);
  header.id0 = 1;
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 6;

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".smd");
  ucon64_file_handler (dest_name, src_name, 0);

  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      exit (1);
    }
  ucon64_fread (buffer, rominfo->backup_header_len, size, src_name);
  // uCON64 1.9.8-2, 1.9.8-3, 1.9.8-4 and 2.0.0 interleave SMS ROMs, but SMS
  //  ROMs should never be interleaved
  if (rominfo->interleaved)
    smd_deinterleave (buffer, size);

  ucon64_fwrite (&header, 0, SMD_HEADER_LEN, dest_name, "wb");
  ucon64_fwrite (buffer, rominfo->backup_header_len, size, dest_name, "ab");
  free (buffer);

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
sms_smds (void)
{
  st_smd_header_t header;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];

  memset (&header, 0, SMD_HEADER_LEN);
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 7;                              // SRAM file

  strcpy (src_name, ucon64.fname);
  strcpy (dest_name, ucon64.fname);
  set_suffix (dest_name, ".sav");
  ucon64_file_handler (dest_name, src_name, 0);

  ucon64_fwrite (&header, 0, SMD_HEADER_LEN, dest_name, "wb");
  fcopy (src_name, 0, ucon64.file_size, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
sms_chk (st_ucon64_nfo_t *rominfo)
{
  char buf[2], dest_name[FILENAME_MAX];
  int offset = rominfo->header_start + 10;

  if (is_gamegear)
    {
      fputs ("ERROR: This option works only for SMS (not Game Gear) files\n", stderr);
      return -1;
    }

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");

  buf[0] = (char) rominfo->current_internal_crc; // low byte
  buf[1] = (char) (rominfo->current_internal_crc >> 8); // high byte
  // change checksum
  if (rominfo->interleaved)
    {
      ucon64_fputc (dest_name, rominfo->backup_header_len +
        (offset & ~0x3fff) + 0x2000 + (offset & 0x3fff) / 2, buf[0], "r+b");
      ucon64_fputc (dest_name, rominfo->backup_header_len +
        (offset & ~0x3fff) + (offset & 0x3fff) / 2, buf[1], "r+b");
    }
  else
    ucon64_fwrite (buf, rominfo->backup_header_len + offset, 2, dest_name, "r+b");

  dumper (stdout, buf, 2, rominfo->backup_header_len + offset, DUMPER_HEX);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


static void
write_game_table_entry (FILE *destfile, int file_no, int totalsize, int size)
{
  long int fpos = ftell (destfile);             // save file pointer
  static int sram_page = 0, sram_msg_printed = 0;
  int n;
  unsigned char name[0x0c], flags = 0;  // x, D (reserved), x, x, x, x, S1, S0
  const char *p;

  fseek (destfile, 0x2000 + (file_no - 1) * 0x10, SEEK_SET);
  fputc (0xff, destfile);                       // 0x0 = 0xff

  memset (name, ' ', 0x0c);
  p = basename2 (ucon64.fname);
  n = strlen (p);
  if (n > 0x0c)
    n = 0x0c;
  memcpy (name, p, n);
  for (n = 0; n < 0x0c; n++)
    {
      if (!isprint ((int) name[n]))
        name[n] = '.';
      else
        name[n] = (unsigned char) toupper (name[n]); // loader only supports upper case characters
    }
  fwrite (name, 1, 0x0c, destfile);             // 0x01 - 0x0c = name
  fputc (size / 16384, destfile);               // 0x0d = ROM size (not used by loader)
  fputc (totalsize / 16384, destfile);          // 0x0e = bank code

  if (sram_page > 3)
    {
      if (!sram_msg_printed)
        {
          puts ("NOTE: This ROM (and all following) will share SRAM with ROM 4");
          sram_msg_printed = 1;
        }
      sram_page = 3;
    }
  flags = (unsigned char) sram_page++;
  fputc (flags, destfile);                      // 0x0f = flags

  fseek (destfile, fpos, SEEK_SET);             // restore file pointer
}


int
sms_multi (int truncate_size, char *fname)
{
#define BUFSIZE 0x20000
// BUFSIZE must be a multiple of 16 kB (for deinterleaving) and larger than or
//  equal to 1 Mbit (for checksum calculation)
  int n, n_files, file_no, bytestowrite, byteswritten, done, truncated = 0,
      totalsize = 0, size, org_do_not_calc_crc = ucon64.do_not_calc_crc;
  struct stat fstate;
  FILE *srcfile, *destfile;
  char destname[FILENAME_MAX];
  unsigned char *buffer;

  if (truncate_size == 0)
    {
      fputs ("ERROR: Cannot make multi-game file of 0 bytes\n", stderr);
      return -1;
    }
  if ((buffer = (unsigned char *) malloc (BUFSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFSIZE);
      return -1;
    }

  if (fname != NULL)
    {
      strcpy (destname, fname);
      n_files = ucon64.argc;
    }
  else
    {
      strcpy (destname, ucon64.argv[ucon64.argc - 1]);
      n_files = ucon64.argc - 1;
    }

  ucon64_file_handler (destname, NULL, OF_FORCE_BASENAME);
  if ((destfile = fopen (destname, "w+b")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], destname);
      return -1;
    }

  printf ("Creating multi-game file for SMS-PRO/GG-PRO: %s\n", destname);

  file_no = 0;
  for (n = 1; n < n_files; n++)
    {
      if (access (ucon64.argv[n], F_OK))
        continue;                               // "file" does not exist (option)
      stat (ucon64.argv[n], &fstate);
      if (!S_ISREG (fstate.st_mode))
        continue;
      if (file_no == 32)                        // loader + 31 games
        {
          puts ("WARNING: A multi-game file can contain a maximum of 31 games. The other files\n"
                "         are ignored.");
          break;
        }

      ucon64.console = UCON64_UNKNOWN;
      ucon64.fname = ucon64.argv[n];
      ucon64.file_size = fsizeof (ucon64.fname);
      // DON'T use fstate.st_size, because file could be compressed
      ucon64.do_not_calc_crc = 1;
      if (sms_init (ucon64.nfo) != 0)
        printf ("WARNING: %s does not appear to be an SMS/GG ROM\n", ucon64.fname);

      if ((srcfile = fopen (ucon64.fname, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.fname);
          continue;
        }
      if (ucon64.nfo->backup_header_len)
        fseek (srcfile, ucon64.nfo->backup_header_len, SEEK_SET);
      size = ucon64.file_size - ucon64.nfo->backup_header_len;

      if (file_no == 0)
        {
          printf ("Loader: %s\n", ucon64.fname);
          if (size != SMSGG_PRO_LOADER_SIZE)
            printf ("WARNING: Are you sure %s is a loader binary?\n", ucon64.fname);
        }
      else
        {
          printf ("ROM%d: %s\n", file_no, ucon64.fname);
          write_game_table_entry (destfile, file_no, totalsize, size);
        }
      file_no++;

      done = 0;
      byteswritten = 0;                         // # of bytes written per file
      while (!done)
        {
          bytestowrite = fread (buffer, 1, BUFSIZE, srcfile);
          if (ucon64.nfo->interleaved)
            smd_deinterleave (buffer, BUFSIZE);
            // yes, BUFSIZE. bytestowrite might not be n * 16 kB
          if (totalsize + bytestowrite > truncate_size)
            {
              bytestowrite = truncate_size - totalsize;
              done = 1;
              truncated = 1;
              printf ("Output file needs %d Mbit on flash card, truncating %s, skipping %d bytes\n",
                      truncate_size / MBIT, ucon64.fname, size - (byteswritten + bytestowrite));
            }
          totalsize += bytestowrite;
          if (bytestowrite == 0)
            done = 1;
          fwrite (buffer, 1, bytestowrite, destfile);
          byteswritten += bytestowrite;
        }

      // Be sure the ROM size is a multiple of the buffer size used in
      //  smsgg_write_rom(), which is 0x4000 (16 kB, so using "buffer" is OK).
      bytestowrite = ((size + 0x3fff) & ~0x3fff) - size;
      if (bytestowrite)
        {
          memset (buffer, 0xff, bytestowrite);
          totalsize += bytestowrite;
          fwrite (buffer, 1, bytestowrite, destfile);
        }

      // smsgg_write_rom() handles alignment. Games have to be aligned to a 16
      //  kB boundary.
      totalsize = (totalsize + 16384 - 1) & ~(16384 - 1);
      fclose (srcfile);
      if (truncated)
        break;
    }
  // fill the next game table entry
  fseek (destfile, 0x2000 + (file_no - 1) * 0x10, SEEK_SET);
  fputc (0, destfile);                          // indicate no next game

  // Make it possible for smsgg_write_rom() to detect that the file is a
  //  multi-game file.
  fseek (destfile, 0x21f4, SEEK_SET);
  strncpy ((char *) buffer, "uCON64 " UCON64_VERSION_S, 12);
  buffer[12] = 0;
  fwrite (buffer, 1, strlen ((char *) buffer), destfile);

  /*
    The SMS requires the checksum to match the data. The ToToTEK loaders have
    the lower nibble of the "checksum range byte" set to 0x0f. Maybe ToToTEK
    will change this or maybe somebody else will write a loader. To avoid extra
    code to handle those cases we just overwite the value.
    We don't handle a GG multi-game file differently, because we cannot detect
    that the user intended to make such a file (other than by adding a new GG
    multi-game option). ToToTEK's GG loader has an SMS header.
  */
  fseek (destfile, 0, SEEK_SET);
  n = fread (buffer, 1, 0x20000, destfile);     // 0x0f => checksum range = 0x20000
  buffer[SMS_HEADER_START + 15] |= 0x0f;        // overwrite checksum range byte
  sms_header.checksum_range = 0x0f;             // sms_chksum() uses this variable
  n = sms_chksum (buffer, n);

  buffer[SMS_HEADER_START + 10] = (unsigned char) n; // low byte
  buffer[SMS_HEADER_START + 11] = (unsigned char) (n >> 8); // high byte
  fseek (destfile, SMS_HEADER_START + 10, SEEK_SET);
  fwrite (buffer + SMS_HEADER_START + 10, 1, 6, destfile);

  fclose (destfile);
  ucon64.console = UCON64_SMS;
  ucon64.do_not_calc_crc = org_do_not_calc_crc;

  return 0;
}


static int
sms_testinterleaved (st_ucon64_nfo_t *rominfo)
{
  unsigned char buf[0x4000] = { 0 };

  ucon64_fread (buf, rominfo->backup_header_len + 0x4000, // header in 2nd 16 kB block
    0x2000 + (SMS_HEADER_START - 0x4000 + 8) / 2, ucon64.fname);
  if (!(memcmp (buf + SMS_HEADER_START - 0x4000, "TMR SEGA", 8) &&
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMR ALVS", 8) && // SMS
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMR SMSC", 8) && // SMS (unofficial)
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMG SEGA", 8)))  // GG
    return 0;

  smd_deinterleave (buf, 0x4000);
  if (!(memcmp (buf + SMS_HEADER_START - 0x4000, "TMR SEGA", 8) &&
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMR ALVS", 8) &&
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMR SMSC", 8) &&
        memcmp (buf + SMS_HEADER_START - 0x4000, "TMG SEGA", 8)))
    return 1;

  return 0;                                     // unknown, act as if it's not interleaved
}


#define SEARCHBUFSIZE (SMS_HEADER_START + 8 + 16 * 1024)
#define N_SEARCH_STR 4
static int
sms_header_len (void)
/*
  At first sight it seems reasonable to also determine whether the file is
  interleaved in this function. However, we run into a chicken-and-egg problem:
  in order to deinterleave the data we have to know the header length. And in
  order to determine the header length we have to know whether the file is
  interleaved :-) Of course we could assume the header has an even size, but it
  turns out that that is not always the case. For example, there is a copy of
  GG Shinobi (E) [b1] floating around with a "header" of 5 bytes.
  In short: this function works only for files that are not interleaved.
*/
{
  // first two hacks for Majesco Game Gear BIOS (U) [!]
  if (ucon64.file_size == 1024)
    return 0;
  else if (ucon64.file_size == 1024 + SMD_HEADER_LEN)
    return SMD_HEADER_LEN;
  else
    {
      char buffer[SEARCHBUFSIZE] = { 0 }, *ptr, *ptr2 = NULL,
           search_str[N_SEARCH_STR][9] = { "TMR SEGA", "TMR ALVS", "TMR SMSC",
             "TMG SEGA" };
      int n;

      ucon64_fread (buffer, 0, SEARCHBUFSIZE, ucon64.fname);

      for (n = 0; n < N_SEARCH_STR; n++)
        {
          ptr = buffer;
          /*
            A few games contain several copies of the identification string
            (Alien 3 (UE) [!] (2 copies), Back to the Future 3 (UE) [!] (2
            copies), Sonic Spinball (UE) [!] (7 copies)). The "correct" one is
            the last where the corresponding checksum bytes are non-zero...
            However, finding *a* occurence is more important than the checksum
            bytes being non-zero.
          */
          while ((ptr = (char *) memmem2 (ptr, SEARCHBUFSIZE - (ptr - buffer),
                   search_str[n], 8, 0)) != NULL)
            {
              if (!ptr2 ||
                  (ptr - buffer >= 12 && ptr[10] != 0 && ptr[11] != 0))
                ptr2 = ptr;
              ptr++;
            }
          if (ptr2)
            {
              n = ptr2 - buffer - SMS_HEADER_START;
              return n < 0 ? 0 : n;
            }
        }

      n = ucon64.file_size % (16 * 1024);       // SMD_HEADER_LEN
      if (ucon64.file_size > n)
        return n;
      else
        return 0;
    }
}
#undef SEARCHBUFSIZE
#undef N_SEARCH_STR


int
sms_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1, x;
  unsigned char buf[16384] = { 0 }, *rom_buffer;

  is_gamegear = 0;
  memset (&sms_header, 0, SMS_HEADER_LEN);

  if (UCON64_ISSET (ucon64.backup_header_len))       // -hd, -nhd or -hdn option was specified
    rominfo->backup_header_len = ucon64.backup_header_len;
  else
    rominfo->backup_header_len = sms_header_len ();

  rominfo->interleaved = UCON64_ISSET (ucon64.interleaved) ?
    ucon64.interleaved : sms_testinterleaved (rominfo);

  if (rominfo->interleaved)
    {
      ucon64_fread (buf, rominfo->backup_header_len + 0x4000, // header in 2nd 16 kB block
        0x2000 + (SMS_HEADER_START - 0x4000 + SMS_HEADER_LEN) / 2, ucon64.fname);
      smd_deinterleave (buf, 0x4000);
      memcpy (&sms_header, buf + SMS_HEADER_START - 0x4000, SMS_HEADER_LEN);
    }
  else
    ucon64_fread (&sms_header, rominfo->backup_header_len + SMS_HEADER_START,
      SMS_HEADER_LEN, ucon64.fname);

  rominfo->header_start = SMS_HEADER_START;
  rominfo->header_len = SMS_HEADER_LEN;
  rominfo->header = &sms_header;

  ucon64_fread (buf, 0, 11, ucon64.fname);
  // Note that the identification bytes are the same as for Genesis SMD files
  //  The init function for Genesis files is called before this function so it
  //  is alright to set result to 0
  if ((buf[8] == 0xaa && buf[9] == 0xbb && buf[10] == 6) ||
      !(memcmp (sms_header.signature, "TMR SEGA", 8) &&  // SMS or GG
        memcmp (sms_header.signature, "TMR ALVS", 8) &&  // SMS
        memcmp (sms_header.signature, "TMR SMSC", 8) &&  // SMS (unofficial)
        memcmp (sms_header.signature, "TMG SEGA", 8)) || // GG
      ucon64.console == UCON64_SMS)
    result = 0;
  else
    result = -1;

  x = sms_header.checksum_range & 0xf0;
  if (x == 0x50 || x == 0x60 || x == 0x70)
    is_gamegear = 1;

  switch (x)
    {
    case 0x30:                                  // SMS, falling through
    case 0x50:                                  // GG
      rominfo->country = "Japan";
      break;
    case 0x40:                                  // SMS, falling through
    case 0x70:                                  // GG
      rominfo->country = "U.S.A. & Europe";
      break;
    case 0x60:                                  // GG
      rominfo->country = "Japan, U.S.A. & Europe";
      break;
    default:
      rominfo->country = "Unknown";
      break;
    }

  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      int size = ucon64.file_size - rominfo->backup_header_len;
      if ((rom_buffer = (unsigned char *) malloc (size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
          return -1;
        }
      ucon64_fread (rom_buffer, rominfo->backup_header_len, size, ucon64.fname);

      if (rominfo->interleaved)
        {
          ucon64.fcrc32 = crc32 (0, rom_buffer, size);
          smd_deinterleave (rom_buffer, size);
        }
      ucon64.crc32 = crc32 (0, rom_buffer, size);

      if (!is_gamegear)
        {
          rominfo->has_internal_crc = 1;
          rominfo->internal_crc_len = 2;
          rominfo->current_internal_crc = sms_chksum (rom_buffer, size);
          rominfo->internal_crc = sms_header.checksum_low;
          rominfo->internal_crc += sms_header.checksum_high << 8;
        }

      free (rom_buffer);
    }

  sprintf ((char *) buf, "Part number: 0x%04x\n",
    sms_header.partno_low + (sms_header.partno_high << 8) +
    ((sms_header.version & 0xf0) << 12));
  strcat (rominfo->misc, (char *) buf);

  sprintf ((char *) buf, "Version: %d", sms_header.version & 0xf);
  strcat (rominfo->misc, (char *) buf);

  rominfo->console_usage = sms_usage[0].help;
  rominfo->backup_usage = !rominfo->backup_header_len ? mgd_usage[0].help : smd_usage[0].help;

  return result;
}


static int
sms_chksum (unsigned char *rom_buffer, int rom_size)
{
  unsigned short int sum;
  int i, i_end;

  switch (sms_header.checksum_range & 0xf)
    {
    case 0xc:
      i_end = 0x7ff0;
      break;
    case 0xe:                                   // falling through
    case 0xf:
      i_end = 0x20000;
      break;
    case 0:
      i_end = 0x40000;
      break;
    case 1:
      i_end = 0x80000;
      break;
    default:
      i_end = rom_size;
      break;
    }
  if (i_end > rom_size)
    i_end = rom_size;

  sum = 0;
  for (i = 0; i < i_end; i++)
    sum += rom_buffer[i];

  if (i_end >= (int) (SMS_HEADER_START + SMS_HEADER_LEN))
    for (i = SMS_HEADER_START; i < (int) (SMS_HEADER_START + SMS_HEADER_LEN); i++)
      sum -= rom_buffer[i];

  return sum;
}
