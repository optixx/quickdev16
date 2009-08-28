/*
ppf.c - Playstation Patch File support for uCON64

Copyright (c) ???? - ???? Icarus/Paradox
Copyright (c) 2001        NoisyB <noisyb@gmx.net>
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
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/bswap.h"
#include "misc/misc.h"
#include "misc/file.h"
#include "misc/string.h"                        // MEMMEM2_CASE
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "ppf.h"


#define MAX_ID_SIZE 3072
#define DIFF_FSIZE
/*
  I (dbjh) couldn't tell from the specification below if it is required that
  the original file and the modified file have the same size. By defining
  DIFF_FSIZE, PPF as I understand it becomes quite a generic patch file
  format. It can be used to patch any file up to 4 GB.
*/


const st_getopt2_t ppf_usage[] =
  {
    {
      "ppf", 0, 0, UCON64_PPF,
      NULL, "apply PPF PATCH to IMAGE (PPF<=v2.0); ROM should be an IMAGE",
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {
      "mkppf", 1, 0, UCON64_MKPPF,
      "ORG_IMG", "create PPF patch; ROM should be the modified IMAGE",
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {
      "nppf", 1, 0, UCON64_NPPF,
      "DESC", "change PPF single line DESCRIPTION",
      NULL
    },
    {
      "idppf", 1, 0, UCON64_IDPPF,
      "FILE_ID.DIZ", "change FILE_ID.DIZ of PPF PATCH (PPF v2.0)",
      NULL
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

/*

.-----------------------------------------------------------------.
| PLAYSTATION PATCH FILE VERSION 2.0 FILE-STRUCTURE FOR DEVELOPERS|
'-----------------------------------------------------------------'

1. The PPF 2.0 Header:

@START_PPF20HEADER
.----------+--------+---------------------------------------------.
| POSITION |  SIZE  |              E X P L A N A T I O N          |
+----------|--------|---------------------------------------------+
| 00-04    |   05   | PPF-magic: "PPF20"                          |
+----------|--------|---------------------------------------------+
| 05       |   01   | Encoding method:                            |
|          |        | - If $00 then it is a PPF 1.0 patch         |
|          |        | - If $01 then it is a PPF 2.0 patch         |
+----------|--------|---------------------------------------------+
| 06-55    |   50   | Patch description                           |
+----------|--------|---------------------------------------------+
| 56-59    |   04   | Size of the file (e.g. CDRWin binfile) this |
|          |        | patch was made of. Used for identification  |
+----------|--------|---------------------------------------------+
| 60-1083  | 1024   | This is a binary block of 1024 byte taken   |
|          |        | from position $9320 of the file (e.g. CDRWin|
|          |        | binfile) this patch was made of. Used for   |
|          |        | identification.                             |
+----------|--------|---------------------------------------------+
| 1084-X   |   XX   | The patch itself. See below for structure! |
'----------+--------+---------------------------------------------'
@END_PPF20HEADER - total headersize = 1084 bytes.


2. The PPF 2.0 patch itself (encoding method #1)

@START_PPF20PATCH
FORMAT : xxxx,y,zzzz

         xxxx   = 4 byte file offset.

         y      = Number of bytes that will be changed.

         zzzz   = New data to be written ('y' number of bytes).

Example
~~~~~~~

Starting from file offset 0x0015F9D0 replace 3 bytes with 01,02,03
D0 F9 15 00 03 01 02 03

Be careful! Watch the endian format! If you own an Amiga and want
to do a PPF2-patcher for Amiga don't forget to swap the endian-format
of the offset to avoid seek errors!

@END_PPF20PATCH


3. The PPF 2.0 fileid area

@START_FILEID

The fileid area is used to store additional patch information of
the PPF 2.0 file. I implemented this following the AMIGA standard
of adding a fileid to e.g. .txt files. You can add a FILE_ID to a
PPF 2.0 patch by using the tool 'PPFdiz.exe' or "PPF-O-MATIC2"
included in this package. You don't have to add a FILE_ID to your
PPF 2.0 patch. It's only for your pleasure! :)

For developers: a file_id area begins with @BEGIN_FILE_ID.DIZ and
ends with @END_FILE_ID.DIZ (Amiga BBS standard).
Between @BEGIN_FILE_ID.DIZ and @END_FILE_ID.DIZ you will find
the fileid and followed after @END_FILE_ID.DIZ you will find an
integer (4 bytes long) with the length of the FILE_ID.DIZ!

A FILE_ID.DIZ file cannot be greater than 3072 bytes.

If you do a PPF 2.0 applier be sure to check for an existing FILE_ID
AREA, because it is located after the patch data!

@END_FILEID
*/


// based on source code of ApplyPPF v2.0 for Linux/Unix by Icarus/Paradox
int
ppf_apply (const char *mod, const char *ppfname)
{
  FILE *modfile, *ppffile;
  char desc[50 + 1], diz[MAX_ID_SIZE + 1], buffer[1024], ppfblock[1024],
       modname[FILENAME_MAX];
  int x, method, dizlen = 0, modlen, ppfsize, bytes_to_skip = 0, n_changes;
  unsigned int pos;

  strcpy (modname, mod);
  ucon64_file_handler (modname, NULL, 0);
  fcopy (mod, 0, fsizeof (mod), modname, "wb"); // no copy if one file

  if ((modfile = fopen (modname, "r+b")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], modname);
      exit (1);
    }
  if ((ppffile = fopen (ppfname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ppfname);
      exit (1);
    }

  // Is it a PPF File?
  fread (buffer, 3, 1, ppffile);
  if (strncmp ("PPF", buffer, 3))
    {
      fprintf (stderr, "ERROR: %s is not a valid PPF file\n", ppfname);
      exit (1);
    }

  // What encoding method? PPF 1.0 or PPF 2.0?
  fseek (ppffile, 5, SEEK_SET);
  method = fgetc (ppffile);
  if (method != 0 && method != 1)
    {
      fprintf (stderr, "ERROR: Unknown encoding method! Check for updates\n");
      exit (1);
    }

  ppfsize = fsizeof (ppfname);

  // Show PPF information
  fseek (ppffile, 6, SEEK_SET);                 // Read description line
  fread (desc, 50, 1, ppffile);
  desc[50] = 0;                                 // terminate string
  printf ("\n"                                  // print a newline between
          "Filename        : %s\n", ppfname);   //  backup message and PPF info
  printf ("Encoding method : %d (PPF %d.0)\n", method, method + 1);
  printf ("Description     : %s\n", desc);

  if (method == 0)                              // PPF 1.0
    {
      printf ("FILE_ID.DIZ     : No\n\n");
      x = 56;                                   // file pointer is at right position (56)
    }
  else // method == 1                           // PPF 2.0
    {
      fseek (ppffile, ppfsize - 8, SEEK_SET);
      fread (buffer, 4, 1, ppffile);

      // Is there a file id?
      if (strncmp (".DIZ", buffer, 4))
        printf ("FILE_ID.DIZ     : No\n\n");
      else
        {
          printf ("FILE_ID.DIZ     : Yes, showing...\n");
          fread (&dizlen, 4, 1, ppffile);
#ifdef  WORDS_BIGENDIAN
          dizlen = bswap_32 (dizlen);           // FILE_ID.DIZ size is in little-endian format
#endif
          fseek (ppffile, ppfsize - dizlen - (16 + 4), SEEK_SET);
          bytes_to_skip = dizlen + 18 + 16 + 4; // +4 for FILE_ID.DIZ size integer
          if (dizlen > MAX_ID_SIZE)
            dizlen = MAX_ID_SIZE;               // do this after setting bytes_to_skip!
          fread (diz, dizlen, 1, ppffile);
          diz[dizlen] = 0;                      // terminate string
          puts (diz);
        }

      // Do the file size check
      fseek (ppffile, 56, SEEK_SET);
      fread (&x, 4, 1, ppffile);
#ifdef  WORDS_BIGENDIAN
      x = bswap_32 (x);                         // file size is stored in little-endian format
#endif
      modlen = fsizeof (modname);
      if (x != modlen)
        {
          fprintf (stderr, "ERROR: The size of %s is not %d bytes\n", modname, x);
          exit (1);
        }

      // Do the binary block check
      fseek (ppffile, 60, SEEK_SET);
      fread (ppfblock, 1024, 1, ppffile);
      fseek (modfile, 0x9320, SEEK_SET);
      memset (buffer, 0, 1024);                 // one little hack that makes PPF
      fread (buffer, 1024, 1, modfile);         //  suitable for files < 38688 bytes
      if (memcmp (ppfblock, buffer, 1024))
        {
          fprintf (stderr, "ERROR: This patch does not belong to this image\n");
          exit (1);
        }

      fseek (ppffile, 1084, SEEK_SET);
      x = 1084;
    }

  // Patch the image
  printf ("Patching...\n");
  for (; x < ppfsize - bytes_to_skip; x += 4 + 1 + n_changes)
    {
      fread (&pos, 4, 1, ppffile);              // Get position for modfile
#ifdef  WORDS_BIGENDIAN
      pos = bswap_32 (pos);
#endif
      n_changes = fgetc (ppffile);              // How many bytes do we have to write?
      fread (buffer, n_changes, 1, ppffile);    // And this is what we have to write
      fseek (modfile, pos, SEEK_SET);           // Go to the right position in the modfile
      fwrite (buffer, n_changes, 1, modfile);   // Write n_changes bytes to that pos
    }

  printf ("Done\n");
  fclose (ppffile);
  fclose (modfile);

  printf (ucon64_msg[WROTE], modname);
  return 0;
}


// based on sourcecode of MakePPF v2.0 Linux/Unix by Icarus/Paradox
int
ppf_create (const char *orgname, const char *modname)
{
  FILE *orgfile, *modfile, *ppffile;
  char ppfname[FILENAME_MAX], buffer[MAX_ID_SIZE], obuf[512], mbuf[512];
#if 0
  char *fidname = "FILE_ID.DIZ";
#endif
  int x, osize, msize, blocksize, n_changes, total_changes = 0;
  unsigned int seekpos = 0, pos;

  osize = fsizeof (orgname);
  msize = fsizeof (modname);
#ifndef DIFF_FSIZE
  if (osize != msize)
    {
      fprintf (stderr, "ERROR: File sizes do not match\n");
      return -1;
    }
#endif

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
  strcpy (ppfname, modname);
  set_suffix (ppfname, ".ppf");
  ucon64_file_handler (ppfname, NULL, 0);
  if ((ppffile = fopen (ppfname, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], ppfname);
      exit (1);
    }

  // creating PPF 2.0 header
  fwrite ("PPF20", 5, 1, ppffile);              // magic
  fputc (1, ppffile);                           // encoding method
  memset (buffer, ' ', 50);
  fwrite (buffer, 50, 1, ppffile);              // description line
#ifdef  WORDS_BIGENDIAN
  x = bswap_32 (osize);
  fwrite (&x, 4, 1, ppffile);
#else
  fwrite (&osize, 4, 1, ppffile);               // orgfile size
#endif
  fseek (orgfile, 0x9320, SEEK_SET);
  memset (buffer, 0, 1024);                     // one little hack that makes PPF
  fread (buffer, 1024, 1, orgfile);             //  suitable for files < 38688 bytes
  fwrite (buffer, 1024, 1, ppffile);            // 1024 byte block

  printf ("Writing patch data, please wait...\n");
  // finding changes
  fseek (orgfile, 0, SEEK_SET);
  fseek (modfile, 0, SEEK_SET);
  while ((blocksize = fread (obuf, 1, 255, orgfile)))
    {
      blocksize = fread (mbuf, 1, blocksize, modfile);
#ifdef  DIFF_FSIZE
      if (blocksize == 0)
        break;
#endif
      pos = seekpos;
      x = 0;
      while (x != blocksize)
        {
          if (obuf[x] != mbuf[x])
            {
              pos = seekpos + x;
              n_changes = 0;
              do
                {
                  buffer[n_changes] = mbuf[x];
                  n_changes++;
                  x++;
                }
              while (x != blocksize && obuf[x] != mbuf[x]);
              total_changes += n_changes;
#ifdef  WORDS_BIGENDIAN
              pos = bswap_32 (pos);
#endif
              fwrite (&pos, 4, 1, ppffile);
              fputc (n_changes, ppffile);
              fwrite (buffer, n_changes, 1, ppffile);
            }
          else
            x++;
        }
      seekpos += blocksize;
    }

#ifdef  DIFF_FSIZE
  if (msize > osize)
    {
      pos = seekpos;
      while ((blocksize = fread (buffer, 1, 255, modfile)))
        {
          total_changes += blocksize;
#ifdef  WORDS_BIGENDIAN
          x = bswap_32 (pos);
          fwrite (&x, 4, 1, ppffile);
#else
          fwrite (&pos, 4, 1, ppffile);
#endif
          fputc (blocksize, ppffile);
          fwrite (buffer, blocksize, 1, ppffile);
          pos += blocksize;
        }
    }
  else if (msize < osize)
    printf ("WARNING: %s is smaller than %s\n"
            "         PPF can't store information about that fact\n",
            modname, orgname);
#endif

  fclose (orgfile);
  fclose (modfile);

  if (total_changes == 0)
    {
      printf ("%s and %s are identical\n"
              "Removing: %s\n", orgname, modname, ppfname);
      fclose (ppffile);
      remove (ppfname);
      return -1;
    }

#if 0
  if (fidname)
    {
      int fsize = fsizeof (fidname);
      if (fsize > MAX_ID_SIZE)
        fsize = MAX_ID_SIZE;                    // File id only up to 3072 bytes!
      printf ("Adding FILE_ID.DIZ (%s)...\n", fidname);
      ucon64_fread (buffer, 0, fsize, fidname);
      fwrite ("@BEGIN_FILE_ID.DIZ", 18, 1, ppffile);
      fwrite (buffer, fsize, 1, ppffile);
      fwrite ("@END_FILE_ID.DIZ", 16, 1, ppffile);
#ifdef  WORDS_BIGENDIAN
      fsize = bswap_32 (fsize);                 // Write file size in little-endian format
#endif
      fwrite (&fsize, 4, 1, ppffile);
    }
#endif
  fclose (ppffile);

  printf (ucon64_msg[WROTE], ppfname);
  return 0;
}


int
ppf_set_desc (const char *ppf, const char *description)
{
  char desc[50], ppfname[FILENAME_MAX];

  strcpy (ppfname, ppf);
  memset (desc, ' ', 50);
  strncpy (desc, description, strlen (description));
  ucon64_file_handler (ppfname, NULL, 0);
  fcopy (ppf, 0, fsizeof (ppf), ppfname, "wb"); // no copy if one file
  ucon64_fwrite (desc, 6, 50, ppfname, "r+b");

  printf (ucon64_msg[WROTE], ppfname);
  return 0;
}


int
ppf_set_fid (const char *ppf, const char *fidname)
{
  int fidsize, ppfsize, pos;
  char ppfname[FILENAME_MAX],
       fidbuf[MAX_ID_SIZE + 34 + 1] = "@BEGIN_FILE_ID.DIZ"; // +1 for string terminator

  strcpy (ppfname, ppf);
  ucon64_file_handler (ppfname, NULL, 0);
  fcopy (ppf, 0, fsizeof (ppf), ppfname, "wb"); // no copy if one file

  printf ("Adding FILE_ID.DIZ (%s)...\n", fidname);
  fidsize = ucon64_fread (fidbuf + 18, 0, MAX_ID_SIZE, fidname);
  memcpy (fidbuf + 18 + fidsize, "@END_FILE_ID.DIZ", 16);

  ppfsize = fsizeof (ppfname);
  pos = ucon64_find (ppfname, 0, ppfsize, "@BEGIN_FILE_ID.DIZ", 18,
    MEMCMP2_CASE | UCON64_FIND_QUIET);
  if (pos == -1)
    pos = ppfsize;
  truncate (ppfname, pos);

  ucon64_fwrite (fidbuf, pos, fidsize + 18 + 16, ppfname, "r+b");
  pos += fidsize + 18 + 16;
#ifdef  WORDS_BIGENDIAN
  fidsize = bswap_32 (fidsize);                 // Write file size in little-endian format
#endif
  ucon64_fwrite (&fidsize, pos, 4, ppfname, "r+b");

  printf (ucon64_msg[WROTE], ppfname);
  return 0;
}
