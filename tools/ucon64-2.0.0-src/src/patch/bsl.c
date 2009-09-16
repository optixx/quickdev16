/*
bsl.c - Baseline patcher support for uCON64

Copyright (c) ???? - ???? The White Knight
Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2003        dbjh


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
#include "misc/misc.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "bsl.h"


const st_getopt2_t bsl_usage[] =
  {
    {
      "b", 0, 0, UCON64_B,
      NULL, "apply Baseline/BSL PATCH to ROM",
      &ucon64_wf[WF_OBJ_ALL_STOP]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


int
bsl_apply (const char *mod, const char *bslname)
{
  FILE *modfile, *bslfile;
  unsigned char byte;
  char buf[4096], modname[FILENAME_MAX];
  int data, nbytes, offset;

  strcpy (modname, mod);
  ucon64_file_handler (modname, NULL, 0);
  fcopy (mod, 0, fsizeof (mod), modname, "wb"); // no copy if one file

  if ((modfile = fopen (modname, "r+b")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], modname);
      return -1;
    }
  if ((bslfile = fopen (bslname, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], bslname);
      return -1;
    }

  printf ("Applying BSL/Baseline patch...\n");

  while (!feof (bslfile))                       // we could use 1, but feof() makes it fail-safe
    {
      fscanf (bslfile, "%d\n", &offset);
      fscanf (bslfile, "%d\n", &data);
      if ((offset == -1) && (data == -1))
        break;

      fseek (modfile, offset, SEEK_SET);
      fputc (data, modfile);
    }

  fscanf (bslfile, "%d\n", &offset);
  fscanf (bslfile, "%d\n", &nbytes);
  fseek (modfile, offset, SEEK_SET);
  if (nbytes > 0)
    {
      while (nbytes > 4096)
        {
          fread (buf, 4096, 1, bslfile);
          fwrite (buf, 4096, 1, modfile);
          nbytes -= 4096;
        }
      while (nbytes-- >= 0)                     // yes, one byte more than the
        {                                       //  _value_ read from the BSL file
          byte = fgetc (bslfile);
          fputc (byte, modfile);
        }
    }

  printf ("Patching complete\n\n");
  printf (ucon64_msg[WROTE], modname);
  printf ("\n"
          "NOTE: Sometimes you have to add/strip a 512 bytes header when you patch a ROM\n"
          "      This means you must modify for example a SNES ROM with -swc or -stp or\n"
          "      the patch will not work\n");

  fclose (bslfile);
  fclose (modfile);

  return 0;
}
