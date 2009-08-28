/*
ips.c - IPS support for uCON64

Copyright (c) ???? - ???? madman
Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
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
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include "misc/misc.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ips.h"


#define NO_RLE  256
// for some strange reason 6 seems to be a general optimum value for RLE_START_THRESHOLD
#define RLE_START_THRESHOLD 6                   // must be smaller than RLE_RESTART_THRESHOLD!
#define RLE_RESTART_THRESHOLD 13
#define BRIDGE_LEN 5
#define BUFSIZE 65535
//#define DEBUG_IPS

const st_getopt2_t ips_usage[] =
  {
    {
      "i", 0, 0, UCON64_I,
      NULL, "apply IPS PATCH to ROM (IPS<=v1.2)",
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {
      "mki", 1, 0, UCON64_MKI,
      "ORG_ROM", "create IPS patch; ROM should be the modified ROM",
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

static FILE *orgfile, *modfile, *ipsfile, *destfile;
static int ndiffs = 0, totaldiffs = 0, address = -1, rle_value = NO_RLE, filepos = 0;
static const char *destfname = NULL;


static unsigned char
read_byte (FILE *file)
{
  int byte;                                     // this must be an int!

  byte = fgetc (file);
  if (byte == EOF)
    {
      fprintf (stderr, "ERROR: Unexpected end of file\n");
      exit (1);
    }
  return byte;
}


static void
remove_destfile (void)
{
  if (destfname)
    {
      printf ("Removing: %s\n", destfname);
      fclose (destfile);                        // necessary under DOS/Win9x for DJGPP port
      remove (destfname);
      destfname = NULL;
    }
}


// based on IPS v1.0 for UNIX by madman
int
ips_apply (const char *mod, const char *ipsname)
{
  unsigned char byte, byte2, byte3;
  char modname[FILENAME_MAX], magic[6];
  unsigned int offset, length, i;

  strcpy (modname, mod);
  ucon64_file_handler (modname, NULL, 0);
  fcopy (mod, 0, fsizeof (mod), modname, "wb"); // no copy if one file

  if ((modfile = fopen (modname, "r+b")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], modname);
      exit (1);
    }
  if ((ipsfile = fopen (ipsname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ipsname);
      exit (1);
    }

  destfname = modname;
  destfile = modfile;
  register_func (remove_destfile);

  fgets (magic, 6, ipsfile);
  if (strcmp (magic, "PATCH") != 0)
    {                                           // do at least one check for validity
      fprintf (stderr, "ERROR: %s is not a valid IPS file\n", ipsname);
      exit (1);
    }

  printf ("Applying IPS patch...\n");
  while (!feof (ipsfile))
    {
      byte = read_byte (ipsfile);
      byte2 = read_byte (ipsfile);
      byte3 = read_byte (ipsfile);
      offset = (byte << 16) + (byte2 << 8) + byte3;
      if (offset == 0x454f46)                   // numerical representation of ASCII "EOF"
        break;
      fseek (modfile, offset, SEEK_SET);

      byte = read_byte (ipsfile);
      byte2 = read_byte (ipsfile);
      length = (byte << 8) + byte2;
      if (length == 0)
        {                                       // code for RLE compressed block
          byte = read_byte (ipsfile);
          byte2 = read_byte (ipsfile);
          length = (byte << 8) + byte2;
          byte = read_byte (ipsfile);
#ifdef  DEBUG_IPS
          printf ("[%02x] <= %02x (* %d)\n", offset, byte, length);
#endif
          for (i = 0; i < length; i++)
            fputc (byte, modfile);
        }
      else
        {                                       // non compressed
          for (i = 0; i < length; i++)
            {
              byte = read_byte (ipsfile);
#ifdef  DEBUG_IPS
              printf ("[%02x] <= %02x\n", offset + i, byte);
#endif
              fputc (byte, modfile);
            }
        }
    }
  fclose (modfile);                             // commit changes before calling truncate2()

  byte = fgetc (ipsfile);                       // don't use read_byte() here;
  if (!feof (ipsfile))                          //  this part is optional
    {                                           // IPS2 stuff
      byte2 = read_byte (ipsfile);
      byte3 = read_byte (ipsfile);
      length = (byte << 16) + (byte2 << 8) + byte3;
      truncate2 (modname, length);
      printf ("File truncated to %.4f MBit\n", length / (float) MBIT);
    }

  printf ("Patching complete\n\n");
  printf (ucon64_msg[WROTE], modname);
  printf ("\n"
          "NOTE: Sometimes you have to add/strip a 512 bytes header when you patch a ROM\n"
          "      This means you must modify for example a SNES ROM with -swc or -stp or\n"
          "      the patch will not work\n");

  unregister_func (remove_destfile);            // unregister _after_ possible padding
  fclose (ipsfile);

  return 0;
}


static void
write_address (int new_address)
{
  address = new_address;
  if (address < 16777216)
    /*
      16777216 = 2^24. The original code checked for 16711680 (2^24 - 64K), but
      that is an artificial limit.
    */
    {
      fputc (address >> 16, ipsfile);
      fputc (address >> 8, ipsfile);
      fputc (address, ipsfile);
    }
  else
    {
      fprintf (stderr, "ERROR: IPS doesn't support addresses greater than 16777215\n"
                       "       Consider using another patch format\n");
      exit (1);                                 // will call remove_destfile() (indirectly)
    }
}


static void
write_block (int ndiffs, unsigned char *buffer, int rle_value)
{
  if (rle_value == NO_RLE)
    {
      fputc (ndiffs >> 8, ipsfile);
      fputc (ndiffs, ipsfile);
      fwrite (buffer, 1, ndiffs, ipsfile);
    }
  else
    {
      fputc (0, ipsfile);
      fputc (0, ipsfile);
      fputc (ndiffs >> 8, ipsfile);
      fputc (ndiffs, ipsfile);
      fputc (rle_value, ipsfile);
#ifdef  DEBUG_IPS
      printf ("RLE ");
#endif
    }
#ifdef  DEBUG_IPS
  printf ("length: %d\n", ndiffs);
#endif
}


static void
flush_diffs (unsigned char *buffer)
{
  if (ndiffs)
    {
      totaldiffs += ndiffs;
      write_block (ndiffs, buffer, rle_value);
      ndiffs = 0;
      rle_value = NO_RLE;
      address = -1;
    }
}


static int
rle_end (int value, unsigned char *buffer)
{
  if (rle_value != NO_RLE && rle_value != value)
    {
      flush_diffs (buffer);
      filepos--;
      fseek (orgfile, filepos, SEEK_SET);
      fseek (modfile, filepos, SEEK_SET);
#ifdef  DEBUG_IPS
      printf ("->normal\n");
#endif
      return 1;
    }
  return 0;
}


static int
check_for_rle (unsigned char byte, unsigned char *buf)
{
  int use_rle, i, retval = 0;

  if (rle_value == NO_RLE)
    {
      /*
        Start with a new block (and stop with the current one) only if there
        are at least RLE_RESTART_THRESHOLD + 1 equal bytes in a row. Restarting
        for RLE has some overhead: possibly 5 bytes for the interrupted current
        block plus 7 bytes for the RLE block. So, values smaller than 11 for
        RLE_RESTART_THRESHOLD only make the IPS file larger than if no RLE
        compression would be used.
      */
      if (ndiffs > RLE_RESTART_THRESHOLD)
        {
          use_rle = 1;
          for (i = ndiffs - RLE_RESTART_THRESHOLD; i < ndiffs; i++)
            if (buf[i] != byte)
              {
                use_rle = 0;
                break;
              }
          if (use_rle)
            // we are not using RLE, but we should => start a new block
            {
              ndiffs -= RLE_RESTART_THRESHOLD;
              filepos -= RLE_RESTART_THRESHOLD;
              fseek (orgfile, filepos, SEEK_SET);
              fseek (modfile, filepos, SEEK_SET);

              flush_diffs (buf);
              write_address (filepos);
              retval = 1;
#ifdef  DEBUG_IPS
              printf ("restart (%x)\n", address);
#endif
            }
        }
      /*
        Use RLE only if the last RLE_START_THRESHOLD + 1 (or more) bytes were
        the same. Values smaller than 7 for RLE_START_THRESHOLD will make the
        IPS file larger than if no RLE would be used (for this block).
        normal block:
        i      address high byte
        i + 1  address medium byte
        i + 2  address low byte
        i + 3  length high byte
        i + 4  length low byte
        i + 5  new byte
        ...
        RLE block:
        i      address high byte
        i + 1  address medium byte
        i + 2  address low byte
        i + 3  0
        i + 4  0
        i + 5  length high byte
        i + 6  length low byte
        i + 7  new byte
        The value 7 for RLE_START_THRESHOLD (instead of 2) is to compensate for
        normal blocks that immediately follow the RLE block.
      */
      else if (ndiffs > RLE_START_THRESHOLD)
        {
          use_rle = 1;
          for (i = 0; i < ndiffs; i++)
            if (buf[i] != byte)
              {
                use_rle = 0;
                break;
              }
          if (use_rle)
            {
              rle_value = byte;
#ifdef  DEBUG_IPS
              printf ("->RLE\n");
#endif
            }
        }
    }
  return retval;
}


int
ips_create (const char *orgname, const char *modname)
{
  int i, orgfilesize, modfilesize;
  char ipsname[FILENAME_MAX];
  unsigned char byte, byte2, buf[BUFSIZE];

  if ((orgfile = fopen (orgname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], orgname);
      exit (1);
    }
  if ((modfile = fopen (modname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], modname);
      exit (1);
    }
  strcpy (ipsname, modname);
  set_suffix (ipsname, ".ips");
  ucon64_file_handler (ipsname, NULL, 0);
  if ((ipsfile = fopen (ipsname, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], ipsname);
      exit (1);
    }

  destfname = ipsname;
  destfile = ipsfile;
  register_func (remove_destfile);

  orgfilesize = fsizeof (orgname);
  modfilesize = fsizeof (modname);

  fprintf (ipsfile, "PATCH");

next_byte:
  while (1)
    {
      byte = fgetc (orgfile);
      byte2 = fgetc (modfile);
      filepos++;
      if (feof (modfile))
        break;

      if (modfilesize > 0x454f46 && filepos == 0x454f46)
        /*
          We must avoid writing 0x454f46 (4542278) as offset, because it has a
          special meaning. Offset 0x454f46 is interpreted as EOF marker. It is
          a numerical representation of the ASCII string "EOF".
          We solve the problem by writing 2 patch bytes for offsets 0x454f45
          and 0x454f46 if at least one of those bytes differs for orgfile and
          modfile.
        */
        {
          int byte3 = fgetc (orgfile);
          unsigned char byte4 = read_byte (modfile);
          filepos++;

          if (byte3 == EOF || byte != byte2 || byte3 != byte4)
            {
              if (address < 0 || address + ndiffs != 0x454f46 - 1 ||
                  ndiffs > BUFSIZE - 2)
                {
                  flush_diffs (buf);            // commit any pending data
                  write_address (0x454f46 - 1);
                }
              // write 2 patch bytes (for offsets 0x454f45 and 0x454f46)
              buf[ndiffs++] = byte2;
              buf[ndiffs++] = byte4;

#ifdef  DEBUG_IPS
              printf ("[%02x] => %02x\n"
                      "[%02x] => %02x\n",
                      filepos - 2, buf[ndiffs - 2], filepos - 1, buf[ndiffs - 1]);
#endif
            }
          continue;
        }

      if (byte != byte2 || feof (orgfile))
        {
          if (rle_end (byte2, buf))
            continue;

          if (address < 0)
            {
              flush_diffs (buf);                // commit previous block
              write_address (filepos - 1);
            }

          buf[ndiffs++] = byte2;
#ifdef  DEBUG_IPS
          printf ("[%02x] => %02x%s\n", filepos - 1, byte2,
            rle_value != NO_RLE ? " *" : "");
#endif
          check_for_rle (byte2, buf);
        }
      else if (address >= 0)                    // byte == byte2 && !feof (orgfile)
        {
          int n, n2, n_compare;
          unsigned char bridge[BRIDGE_LEN + 1];

          bridge[0] = byte;
          buf[ndiffs] = byte2;
          n = fread (bridge + 1, 1, BRIDGE_LEN, orgfile);
          n2 = fread (&buf[ndiffs + 1], 1, MIN (BRIDGE_LEN, BUFSIZE - ndiffs), modfile);
          n_compare = 1 + MIN (n, n2);

          for (i = 0; i < n_compare; i++)
            if (buf[ndiffs + i] != bridge[i])
              {
                for (n = 0; n < i; n++)
                  {
                    if (rle_end (buf[ndiffs], buf))
                      goto next_byte;           // yep, ugly
#ifdef  DEBUG_IPS
                    printf ("[%02x] => %02x b%s\n", filepos - 1,
                      buf[ndiffs], rle_value != NO_RLE ? " *" : "");
#endif
                    ndiffs++;
                    if (check_for_rle (buf[ndiffs - 1], buf))
                      goto next_byte;           // even uglier
                    filepos++;
                  }
                filepos--;
//                byte = bridge[n];
//                byte2 = buf[ndiffs];
                break;
              }

          fseek (orgfile, filepos, SEEK_SET);
          fseek (modfile, filepos, SEEK_SET);

          if (i == n_compare)                   // next few bytes are equal (between the files)
            address = -1;
        }
      if (ndiffs == BUFSIZE)
        flush_diffs (buf);
    }

  flush_diffs (buf);
  fprintf (ipsfile, "EOF");
  if (modfilesize < orgfilesize)
    write_address (modfilesize);

  unregister_func (remove_destfile);
  fclose (orgfile);
  fclose (modfile);
  fclose (ipsfile);

  if (totaldiffs == 0)
    {
      printf ("%s and %s are identical\n"
              "Removing: %s\n", orgname, modname, ipsname);
      remove (ipsname);
      return -1;
    }

  printf (ucon64_msg[WROTE], ipsname);
  return 0;
}
