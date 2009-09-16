/*
cd64.c - CD64 support for uCON64

Copyright (c) 2004 dbjh


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
#include <stdarg.h>
#include <ultra64/host/cd64lib.h>
#include "misc/misc.h"
#include "misc/parallel.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "cd64.h"


const st_getopt2_t cd64_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "CD64"/*"19XX UFO http://www.cd64.com"*/,
      NULL
    },
#if     defined USE_PARALLEL && defined USE_LIBCD64
    {
      "xcd64", 0, 0, UCON64_XCD64,
      NULL, "send/receive ROM to/from CD64; " OPTION_LONG_S "port=PORT\n"
      "receives automatically (64 Mbits) when ROM does not exist",
      &ucon64_wf[WF_OBJ_N64_DEFAULT_STOP_NO_ROM]
    },
    {
      "xcd64c", 1, 0, UCON64_XCD64C,
      "N", "receive N Mbits of ROM from CD64; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_N64_STOP_NO_ROM]
    },
    {
      "xcd64b", 0, 0, UCON64_XCD64B,
      NULL, "send boot emu to CD64; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_N64_DEFAULT_STOP]
    },
    {
      "xcd64s", 0, 0, UCON64_XCD64S,
      NULL, "send/receive SRAM to/from CD64; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM file does not exist",
      &ucon64_wf[WF_OBJ_N64_STOP_NO_ROM]
    },
    {
      "xcd64f", 0, 0, UCON64_XCD64F,
      NULL, "send/receive flash RAM to/from CD64; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when flash RAM file does not exist",
      &ucon64_wf[WF_OBJ_N64_STOP_NO_ROM]
    },
    {
      "xcd64e", 0, 0, UCON64_XCD64E,
      NULL, "send/receive EEPROM data to/from CD64; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when EEPROM file does not exist",
      &ucon64_wf[WF_OBJ_N64_STOP_NO_ROM]
    },
    {
      "xcd64m", 1, 0, UCON64_XCD64M,
      "INDEX", "send/receive memory pack data to/from CD64; " OPTION_LONG_S "port=PORT\n"
      "INDEX is ignored for CD64 BIOS protocol\n"
      "receives automatically when memory pack file does not exist",
      &ucon64_wf[WF_OBJ_N64_STOP_NO_ROM]
    },
    {
      "xcd64p", 1, 0, UCON64_XCD64P,
      "PROT", "use protocol PROT when communicating with CD64; " OPTION_LONG_S "port=PORT\n"
      "PROT=0 CD64 BIOS\n"
      "PROT=1 Ghemor\n"
      "PROT=2 UltraLink",
      &ucon64_wf[WF_OBJ_N64_SWITCH]
    },
#endif // USE_PARALLEL && USE_LIBCD64
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#if     defined USE_PARALLEL && defined USE_LIBCD64

static time_t cd64_starttime;


static void
cd64_progress (uint32_t current, uint32_t total)
{
  ucon64_gauge (cd64_starttime, current, total);
}


static int
cd64_notice_helper (FILE *file, const char *prefix, const char *format,
                    va_list argptr)
{
  int n_chars;

  fputs (prefix, file);
  n_chars = vfprintf (file, format, argptr);
  fputc ('\n', file);
  fflush (file);

  return n_chars;
}


static int
cd64_notice (const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = cd64_notice_helper (stdout, "NOTE (libcd64): ", format, argptr);
  va_end (argptr);

  return n_chars;
}


static int
cd64_notice2 (const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = cd64_notice_helper (stderr, "ERROR (libcd64): ", format, argptr);
  va_end (argptr);

  return n_chars;
}


static int
fread_wrapper (void *io_id, void *buffer, uint32_t size)
{
  return fread (buffer, 1, size, (FILE *) io_id);
}


static int
fwrite_wrapper (void *io_id, void *buffer, uint32_t size)
{
  return fwrite (buffer, 1, size, (FILE *) io_id);
}


static int32_t
ftell_wrapper (void *io_id)
{
  return (int32_t) ftell ((FILE *) io_id);
}


static int
fseek_wrapper (void *io_id, int32_t offset, int whence)
{
  return fseek ((FILE *) io_id, offset, whence);
}


static struct cd64_t *
cd64_init (void)
{
  struct cd64_t *cd64;
#ifdef  USE_PPDEV
  uint16_t port = strtol (&ucon64.parport_dev[strlen (ucon64.parport_dev) - 1], NULL, 10);
  method_t method = PPDEV;
#else
  uint16_t port = ucon64.parport;
  method_t method = RAWIO;
#endif
  int is_parallel = 1;

  if ((cd64 = (struct cd64_t *) calloc (1, sizeof (struct cd64_t))) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], sizeof (struct cd64_t));
      exit (1);
    }

#ifndef USE_PPDEV
  if (ucon64.parport == UCON64_UNKNOWN)
    {
      fputs ("ERROR: No port or invalid port specified\n"
             "TIP:   Specify one with --port or in the configuration file\n", stderr);
      exit (1);
    }
  if (port >= 0x300 && port <= 0x330)
    is_parallel = 0;
#endif

  cd64->notice_callback = cd64_notice;
  cd64->notice_callback2 = cd64_notice2;

  if (!cd64_create (cd64, method, port, (protocol_t) ucon64.io_mode, is_parallel))
    {
      fputs ("ERROR: Could not initialise libcd64\n", stderr);
      exit (1);
    }

  cd64->read_callback = fread_wrapper;          // actually f*2(), if zlib
  cd64->write_callback = fwrite_wrapper;        //  support is enabled
  cd64->tell_callback = ftell_wrapper;
  cd64->seek_callback = fseek_wrapper;
  cd64->progress_callback = cd64_progress;
  strcpy (cd64->io_driver_dir, ucon64.configdir);

  // parport_print_info() displays a reasonable message (even if we're using a
  //  comms link)
  parport_print_info ();

  if (!cd64->devopen (cd64))
    {
      fputs ("ERROR: Could not open I/O device for CD64\n", stderr);
      exit (1);
    }
#if     defined __unix__ && !defined __MSDOS__
  drop_privileges ();
#endif

  return cd64;
}


int
cd64_read_rom (const char *filename, int size)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "w+b")) == NULL) // cd64_download_cart() also
    {                                           //  reads from file
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_download_cart (cd64, file, size * MBIT, NULL);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_write_rom (const char *filename)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_upload_dram (cd64, file, ucon64.file_size, NULL, 1);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_write_bootemu (const char *filename)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_upload_bootemu (cd64, file, ucon64.file_size, NULL);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_read_sram (const char *filename)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_download_sram (cd64, file);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_write_sram (const char *filename)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_upload_sram (cd64, file);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_read_flashram (const char *filename)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_download_flashram (cd64, file);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_write_flashram (const char *filename)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_upload_flashram (cd64, file);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_read_eeprom (const char *filename)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_download_eeprom (cd64, file);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_write_eeprom (const char *filename)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  cd64_starttime = time (NULL);
  cd64_upload_eeprom (cd64, file);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_read_mempack (const char *filename, int index)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }

  if (ucon64.io_mode == CD64BIOS)
    index = -1;
  cd64_starttime = time (NULL);
  cd64_download_mempak (cd64, file, (int8_t) index);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}


int
cd64_write_mempack (const char *filename, int index)
{
  FILE *file;
  struct cd64_t *cd64 = cd64_init ();

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }

  if (ucon64.io_mode == CD64BIOS)
    index = -1;
  cd64_starttime = time (NULL);
  cd64_upload_mempak (cd64, file, (int8_t) index);

  cd64->devclose (cd64);
  fclose (file);
  free (cd64);

  return 0;
}

#endif // USE_PARALLEL && USE_LIBCD64
