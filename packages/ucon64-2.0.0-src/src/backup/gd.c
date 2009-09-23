/*
gd.c - Game Doctor support for uCON64

Copyright (c) 2002 - 2003 John Weidman
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
#include <ctype.h>
#include <time.h>
#include "misc/string.h"
#include "misc/misc.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "gd.h"
#include "console/snes.h"                       // for snes_make_gd_names() &
#include "misc/parallel.h"                      //  snes_get_snes_hirom()


const st_getopt2_t gd_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Game Doctor SF3(SF6/SF7)/Professor SF(SF II)"/*"19XX Bung Enterprises Ltd http://www.bung.com.hk"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xgd3", 0, 0, UCON64_XGD3, // supports split files
      NULL, "send ROM to Game Doctor SF3/SF6/SF7; " OPTION_LONG_S "port=PORT\n"
      "this option uses the Game Doctor SF3 protocol",
      &ucon64_wf[WF_OBJ_SNES_DEFAULT_STOP_NO_ROM]
    },
    {
      "xgd6", 0, 0, UCON64_XGD6,
#if 1 // dumping is not yet supported
      NULL, "send ROM to Game Doctor SF6/SF7; " OPTION_LONG_S "port=PORT\n" // supports split files
#else
      NULL, "send/receive ROM to/from Game Doctor SF6/SF7; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when ROM does not exist\n"
#endif
      "this option uses the Game Doctor SF6 protocol",
      &ucon64_wf[WF_OBJ_SNES_DEFAULT_STOP_NO_ROM]
    },
    {
      "xgd3s", 0, 0, UCON64_XGD3S,
      NULL, "send SRAM to Game Doctor SF3/SF6/SF7; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_SNES_STOP_NO_ROM]
    },
    // --xgd3r should remain hidden until receiving works
    {
      "xgd3r", 0, 0, UCON64_XGD3R,
      NULL, NULL,
      &ucon64_wf[WF_OBJ_SNES_STOP_NO_ROM]
    },
    {
      "xgd6s", 0, 0, UCON64_XGD6S,
      NULL, "send/receive SRAM to/from Game Doctor SF6/SF7; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_SNES_STOP_NO_ROM]
    },
    {
      "xgd6r", 0, 0, UCON64_XGD6R,
      NULL, "send/receive saver (RTS) data to/from Game Doctor SF6/SF7;\n" OPTION_LONG_S "port=PORT\n"
      "receives automatically when saver file does not exist",
      &ucon64_wf[WF_OBJ_SNES_STOP_NO_ROM]
    },
#endif // USE_PARALLEL
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef  USE_PARALLEL

#define BUFFERSIZE 8192
#define GD_OK 0
#define GD_ERROR 1
#define GD3_PROLOG_STRING "DSF3"
#define GD6_READ_PROLOG_STRING "GD6R"           // GD reading, PC writing
#define GD6_WRITE_PROLOG_STRING "GD6W"          // GD writing, PC reading
#define GD6_TIMEOUT_ATTEMPTS 0x4000
#define GD6_RX_SYNC_TIMEOUT_ATTEMPTS 0x2000
#define GD6_SYNC_RETRIES 16

#ifdef  _MSC_VER
// Visual C++ doesn't allow inline in C source code
#define inline __inline
#endif


static void init_io (unsigned int port);
static void deinit_io (void);
static void io_error (void);
static void gd_checkabort (int status);
static void remove_destfile (void);
static int gd3_send_prolog_byte (unsigned char data);
static int gd3_send_prolog_bytes (unsigned char *data, int len);
static void gd3_send_byte (unsigned char data);
static int gd3_send_bytes (unsigned char *data, int len);
static int gd6_sync_hardware (void);
static int gd6_sync_receive_start (void);
static inline int gd6_send_byte_helper (unsigned char data, unsigned int timeout);
static int gd6_send_prolog_byte (unsigned char data);
static int gd6_send_prolog_bytes (unsigned char *data, int len);
static int gd6_send_bytes (unsigned char *data, int len);
static int gd6_receive_bytes (unsigned char *buffer, int len);
static int gd_send_unit_prolog (int header, unsigned size);
static int gd_write_rom (const char *filename, unsigned int parport,
                         st_rominfo_t *rominfo, const char *prolog_str);
static int gd_write_sram (const char *filename, unsigned int parport,
                          const char *prolog_str);
static int gd_write_saver (const char *filename, unsigned int parport,
                           const char *prolog_str);


typedef struct st_gd3_memory_unit
{
  char name[12];                                // Exact size is 11 chars but I'll
//  unsigned char *data;                        //  add one extra for string terminator
  unsigned int size;                            // Usually either 0x100000 or 0x80000
} st_gd3_memory_unit_t;

static int (*gd_send_prolog_byte) (unsigned char data);
static int (*gd_send_prolog_bytes) (unsigned char *data, int len);
static int (*gd_send_bytes) (unsigned char *data, int len);
static st_gd3_memory_unit_t gd3_dram_unit[GD3_MAX_UNITS];
static int gd_port, gd_bytessend, gd_fsize, gd_name_i = 0;
static time_t gd_starttime;
static char **gd_names;
static unsigned char gd6_send_toggle;
static const char *gd_destfname = NULL;
static FILE *gd_destfile;


void
init_io (unsigned int port)
/*
  - sets global `gd_port'. Then the send/receive functions don't need to pass
    `port' to all the I/O functions.
  - calls init_conio(). Necessary for kbhit() and DOS-like behaviour of getch().
*/
{
  gd_port = port;
#if 0 // we want to support non-standard parallel port addresses
  if (gd_port != 0x3bc && gd_port != 0x378 && gd_port != 0x278)
    {
      fputs ("ERROR: PORT must be 0x3bc, 0x378 or 0x278\n", stderr);
      exit (1);
    }
#endif

#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  init_conio ();
#endif

  parport_print_info ();
}


void
deinit_io (void)
{
// Put possible transfer cleanup stuff here
#if     (defined __unix__ || defined __BEOS__) && !defined __MSDOS__
  deinit_conio ();
#endif
}


void
io_error (void)
// This function could be changed to take a string argument that describes the
//  error. Or take an integer code that we can interpret here.
{
  fflush (stdout);
  fputs ("ERROR: Communication with Game Doctor failed\n", stderr);
  fflush (stderr);
  // calling fflush() seems to be necessary under Msys in order to make the
  //  error message be displayed before the "Removing: <filename>" message
  exit (1);
}


void
gd_checkabort (int status)
{
  if (((!ucon64.frontend) ? kbhit () : 0) && getch () == 'q')
    {
      puts ("\nProgram aborted");
      exit (status);
    }
}


static void
remove_destfile (void)
{
  if (gd_destfname)
    {
      printf ("Removing: %s\n", gd_destfname);
      fclose (gd_destfile);
      remove (gd_destfname);
      gd_destfname = NULL;
    }
}


int
gd3_send_prolog_byte (unsigned char data)
/*
  Prolog specific data output routine
  We could probably get away with using the general routine but the
  transfer program I (JW) traced to analyze the protocol did this for
  the bytes used to set up the transfer so here it is.
*/
{
  // Wait until SF3 is not busy
  do
    {
      if ((inportb ((unsigned short) (gd_port + PARPORT_STATUS)) & 0x08) == 0)
        return GD_ERROR;
    }
  while ((inportb ((unsigned short) (gd_port + PARPORT_STATUS)) & 0x80) == 0);

  outportb ((unsigned short) gd_port, data);    // set data
  outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 5); // Clock data out to SF3
  outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 4);

  return GD_OK;
}


int
gd3_send_prolog_bytes (unsigned char *data, int len)
{
  int i;

  for (i = 0; i < len; i++)
    if (gd3_send_prolog_byte (data[i]) == GD_ERROR)
      return GD_ERROR;
  return GD_OK;
}


int
gd_send_unit_prolog (int header, unsigned size)
{
  if (gd_send_prolog_byte (0x00) == GD_ERROR)
    return GD_ERROR;
  if (gd_send_prolog_byte ((unsigned char) ((header != 0) ? 0x02 : 0x00)) == GD_ERROR)
    return GD_ERROR;
  if (gd_send_prolog_byte ((unsigned char) (size >> 16)) == GD_ERROR) // 0x10 = 8 Mbit
    return GD_ERROR;
  if (gd_send_prolog_byte (0x00) == GD_ERROR)
    return GD_ERROR;
  return GD_OK;
}


void
gd3_send_byte (unsigned char data)
/*
  General data output routine
  Use this routine for sending ROM data bytes to the Game Doctor SF3 (SF6/SF7
  too).
*/
{
  // Wait until SF3 is not busy
  while ((inportb ((unsigned short) (gd_port + PARPORT_STATUS)) & 0x80) == 0)
    ;

  outportb ((unsigned short) gd_port, data);    // set data
  outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 5); // Clock data out to SF3
  outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 4);
}


int
gd3_send_bytes (unsigned char *data, int len)
{
  int i;

  for (i = 0; i < len; i++)
    {
      gd3_send_byte (data[i]);
      gd_bytessend++;
      if ((gd_bytessend - GD_HEADER_LEN) % 8192 == 0)
        {
          ucon64_gauge (gd_starttime, gd_bytessend, gd_fsize);
          gd_checkabort (2);                    // 2 to return something other than 1
        }
    }
  return GD_OK;
}


int
gd6_sync_hardware (void)
// Sets the SF7 up for an SF6/SF7 protocol transfer
{
  int timeout, retries;
  volatile int delay;

  for (retries = GD6_SYNC_RETRIES; retries > 0; retries--)
    {
      timeout = GD6_TIMEOUT_ATTEMPTS;

      outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 4);
      outportb ((unsigned short) gd_port, 0);
      outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 4);

      for (delay = 0x1000; delay > 0; delay--)  // A delay may not be necessary here
        ;

      outportb ((unsigned short) gd_port, 0xaa);
      outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 0);

      while ((inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x08) == 0)
        if (--timeout == 0)
          break;
      if (timeout == 0)
        continue;

      outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 4);

      while ((inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x08) != 0)
        if (--timeout == 0)
          break;
      if (timeout == 0)
        continue;

      outportb ((unsigned short) gd_port, 0x55);
      outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 0);

      while ((inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x08) == 0)
        if (--timeout == 0)
          break;
      if (timeout == 0)
        continue;

      outportb ((unsigned short) (gd_port + PARPORT_CONTROL), 4);

      while ((inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x08) != 0)
        if (--timeout == 0)
          break;
      if (timeout == 0)
        continue;

      return GD_OK;
    }
  return GD_ERROR;
}


int
gd6_sync_receive_start (void)
// Sync with the start of the received data
{
  int timeout = GD6_RX_SYNC_TIMEOUT_ATTEMPTS;

  while (1)
    {
      if (((inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x03) == 0x03) ||
          ((inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x03) == 0))
        break;

      if (--timeout == 0)
        return GD_ERROR;
    }

  outportb ((unsigned short) gd_port, 0);

  timeout = GD6_RX_SYNC_TIMEOUT_ATTEMPTS;
  while ((inportb ((unsigned short) (gd_port + PARPORT_STATUS)) & 0x80) != 0)
    if (--timeout == 0)
      return GD_ERROR;

  return GD_OK;
}


inline int
gd6_send_byte_helper (unsigned char data, unsigned int timeout)
{
  while ((inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x02) != gd6_send_toggle)
    if (--timeout == 0)
      return GD_ERROR;

  gd6_send_toggle = ~gd6_send_toggle & 0x02;
  outportb ((unsigned short) gd_port, data);
  outportb ((unsigned short) (gd_port + PARPORT_CONTROL), (unsigned char) (4 | (gd6_send_toggle >> 1)));

  return GD_OK;
}


int
gd6_send_prolog_byte (unsigned char data)
{
  unsigned int timeout = 0x100000;

  gd6_send_toggle = (inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x01) << 1;
  return gd6_send_byte_helper (data, timeout);
}


int
gd6_send_prolog_bytes (unsigned char *data, int len)
{
  int i;
  unsigned int timeout = 0x1e00000;

  gd6_send_toggle = (inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x01) << 1;
  for (i = 0; i < len; i++)
    if (gd6_send_byte_helper (data[i], timeout) != GD_OK)
      return GD_ERROR;
  return GD_OK;
}


int
gd6_send_bytes (unsigned char *data, int len)
{
  int i;
  unsigned int timeout = 0x1e0000;

  gd6_send_toggle = (inportb ((unsigned short) (gd_port + PARPORT_CONTROL)) & 0x01) << 1;
  for (i = 0; i < len; i++)
    {
      if (gd6_send_byte_helper (data[i], timeout) != GD_OK)
        return GD_ERROR;

      gd_bytessend++;
      if ((gd_bytessend - GD_HEADER_LEN) % 8192 == 0)
        {
          ucon64_gauge (gd_starttime, gd_bytessend, gd_fsize);
          gd_checkabort (2);                    // 2 to return something other than 1
        }
    }
  return GD_OK;
}


int
gd6_receive_bytes (unsigned char *buffer, int len)
{
  int i;
  unsigned char nibble1, nibble2;
  unsigned int timeout = 0x1e0000;

  outportb ((unsigned short) gd_port, 0x80);    // Signal the SF6/SF7 to send the next nibble
  for (i = 0; i < len; i++)
    {
      while ((inportb ((unsigned short) (gd_port + PARPORT_STATUS)) & 0x80) == 0)
        if (--timeout == 0)
          return GD_ERROR;

      nibble1 = (inportb ((unsigned short) (gd_port + PARPORT_STATUS)) >> 3) & 0x0f;
      outportb ((unsigned short) gd_port, 0x00); // Signal the SF6/SF7 to send the next nibble

      while ((inportb ((unsigned short) (gd_port + PARPORT_STATUS)) & 0x80) != 0)
        if (--timeout == 0)
          return GD_ERROR;

      nibble2 = (inportb ((unsigned short) (gd_port + PARPORT_STATUS)) << 1) & 0xf0;
      buffer[i] = nibble2 | nibble1;
      outportb ((unsigned short) gd_port, 0x80);
    }
  return GD_OK;
}


int
gd_add_filename (const char *filename)
{
  char buf[FILENAME_MAX], *p;

  if (gd_name_i < GD3_MAX_UNITS)
    {
      strcpy (buf, filename);
      p = strrchr (buf, '.');
      if (p)
        *p = 0;
      strncpy (gd_names[gd_name_i], basename2 (buf), 11);
      gd_names[gd_name_i][11] = 0;
      gd_name_i++;
    }
  return 0;
}


int
gd3_read_rom (const char *filename, unsigned int parport)
{
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  return fputs ("ERROR: The function for dumping a cartridge is not yet implemented for the SF3\n", stderr);
}


int
gd6_read_rom (const char *filename, unsigned int parport)
{
#if 0
  FILE *file;
  unsigned char *buffer;

  init_io (parport);
  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
#else
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  return fputs ("ERROR: The function for dumping a cartridge is not yet implemented for the SF6\n", stderr);
#endif
}


int
gd3_write_rom (const char *filename, unsigned int parport, st_rominfo_t *rominfo)
{
  gd_send_prolog_byte = gd3_send_prolog_byte;   // for gd_send_unit_prolog()
  gd_send_prolog_bytes = gd3_send_prolog_bytes;
  gd_send_bytes = gd3_send_bytes;

  return gd_write_rom (filename, parport, rominfo, GD3_PROLOG_STRING);
}


int
gd6_write_rom (const char *filename, unsigned int parport, st_rominfo_t *rominfo)
{
  gd_send_prolog_byte = gd6_send_prolog_byte;   // for gd_send_unit_prolog()
  gd_send_prolog_bytes = gd6_send_prolog_bytes;
  gd_send_bytes = gd6_send_bytes;

  return gd_write_rom (filename, parport, rominfo, GD6_READ_PROLOG_STRING);
}


/*
  Note: On most Game Doctor's the way you enter link mode to be able to upload
        the ROM to the unit is to hold down the R key on the controller while
        resetting the SNES. You will see the Game Doctor menu has a message that
        says "LINKING..."
*/
int
gd_write_rom (const char *filename, unsigned int parport, st_rominfo_t *rominfo,
              const char *prolog_str)
{
  FILE *file = NULL;
  unsigned char *buffer;
  char *names[GD3_MAX_UNITS], names_mem[GD3_MAX_UNITS][12],
       *filenames[GD3_MAX_UNITS], dir[FILENAME_MAX];
  int num_units, i, send_header, x, split = 1, hirom = snes_get_snes_hirom();

  init_io (parport);

  // We don't want to malloc() ridiculously small chunks (of 12 bytes)
  for (i = 0; i < GD3_MAX_UNITS; i++)
    names[i] = names_mem[i];

  gd_names = (char **) names;
  ucon64_testsplit_callback = gd_add_filename;
  num_units = ucon64.split = ucon64_testsplit (filename); // this will call gd_add_filename()
  ucon64_testsplit_callback = NULL;
  if (!ucon64.split)
    {
      split = 0;
      num_units = snes_make_gd_names (filename, rominfo, (char **) names);
    }

  dirname2 (filename, dir);
  gd_fsize = 0;
  for (i = 0; i < num_units; i++)
    {
      // No suffix is necessary but the name entry must be upper case and MUST
      //  be 11 characters long, padded at the end with spaces if necessary.
      memset (gd3_dram_unit[i].name, ' ', 11);  // "pad" with spaces
      gd3_dram_unit[i].name[11] = 0;            // terminate string so we can print it (debug)
      // Use memcpy() instead of strcpy() so that the string terminator in
      //  names[i] won't be copied.
      memcpy (gd3_dram_unit[i].name, strupr (names[i]), strlen (names[i]));

      x = strlen (dir) + strlen (names[i]) + 6; // file sep., suffix, ASCII-z => 6
      if ((filenames[i] = (char *) malloc (x)) == NULL)
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], x);
          exit (1);
        }
      sprintf (filenames[i], "%s" FILE_SEPARATOR_S "%s.078", dir, names[i]); // should match with what code of -s does

      if (split)
        {
          x = fsizeof (filenames[i]);
          gd_fsize += x;
          gd3_dram_unit[i].size = x;
          if (i == 0)                           // Correct for header of first file
            gd3_dram_unit[i].size -= GD_HEADER_LEN;
        }
      else
        {
          if (!gd_fsize)                        // Don't call fsizeof() more
            gd_fsize = fsizeof (filename);      //  often than necessary
          if (hirom)
            gd3_dram_unit[i].size = (gd_fsize - GD_HEADER_LEN) / num_units;
          else
            {
              if ((i + 1) * 8 * MBIT <= gd_fsize - GD_HEADER_LEN)
                gd3_dram_unit[i].size = 8 * MBIT;
              else
                gd3_dram_unit[i].size = gd_fsize - GD_HEADER_LEN - i * 8 * MBIT;
            }
        }
    }

  if ((buffer = (unsigned char *) malloc (8 * MBIT)) == NULL)
    { // a DRAM unit can hold 8 Mbit at maximum
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], 8 * MBIT);
      exit (1);
    }

  printf ("Send: %d Bytes (%.4f Mb)\n", gd_fsize, (float) gd_fsize / MBIT);

  // Put this just before the real transfer begins or else the ETA won't be
  //  correct.
  gd_starttime = time (NULL);

  // Send the ROM to the hardware
  if (memcmp (prolog_str, GD6_READ_PROLOG_STRING, 4) == 0)
    if (gd6_sync_hardware () == GD_ERROR)
      io_error ();
  memcpy (buffer, prolog_str, 4);
  buffer[4] = num_units;
  if (gd_send_prolog_bytes (buffer, 5) == GD_ERROR)
    io_error ();

  puts ("Press q to abort\n");
  for (i = 0; i < num_units; i++)
    {
#ifdef  DEBUG
      printf ("\nfilename (%d): \"%s\", ", split, (split ? (char *) filenames[i] : filename));
      printf ("name: \"%s\", size: %d\n", gd3_dram_unit[i].name, gd3_dram_unit[i].size);
#endif
      if (split)
        {
          if ((file = fopen (filenames[i], "rb")) == NULL)
            {
              fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filenames[i]);
              exit (1);
            }
        }
      else
        if (file == NULL)                     // don't open the file more than once
          if ((file = fopen (filename, "rb")) == NULL)
            {
              fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
              exit (1);
            }

      send_header = i == 0 ? 1 : 0;
      if (gd_send_unit_prolog (send_header, gd3_dram_unit[i].size) == GD_ERROR)
        io_error ();
      if (gd_send_prolog_bytes ((unsigned char *) gd3_dram_unit[i].name, 11) == GD_ERROR)
        io_error ();
      if (send_header)
        {
          // Send the Game Doctor 512 byte header
          fread (buffer, 1, GD_HEADER_LEN, file);
          if (gd_send_prolog_bytes (buffer, GD_HEADER_LEN) == GD_ERROR)
            io_error ();
          gd_bytessend += GD_HEADER_LEN;
        }
      if (split == 0)                           // Not pre-split -- have to split it ourselves
        {
          if (hirom)
            fseek (file, i * gd3_dram_unit[0].size + GD_HEADER_LEN, SEEK_SET);
          else
            fseek (file, i * 8 * MBIT + GD_HEADER_LEN, SEEK_SET);
        }
      fread (buffer, 1, gd3_dram_unit[i].size, file);
      if (gd_send_bytes (buffer, gd3_dram_unit[i].size) == GD_ERROR)
        io_error ();

      if (split || i == num_units - 1)
        fclose (file);
    }

  for (i = 0; i < num_units; i++)
    free (filenames[i]);
  free (buffer);
  deinit_io ();

  return 0;
}


int
gd3_read_sram (const char *filename, unsigned int parport)
{
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  return fputs ("ERROR: The function for dumping SRAM is not yet implemented for the SF3\n", stderr);
}


int
gd6_read_sram (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer, gdfilename[12];
  int len, bytesreceived = 0, transfer_size;
  time_t starttime;

  init_io (parport);

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  // Be nice to the user and automatically remove the file on an error (or abortion)
  gd_destfname = filename;
  gd_destfile = file;
  register_func (remove_destfile);

  if (gd6_sync_hardware () == GD_ERROR)
    io_error ();
  if (gd6_send_prolog_bytes ((unsigned char *) GD6_WRITE_PROLOG_STRING, 4) == GD_ERROR)
    io_error ();

  /*
    The BRAM (SRAM) filename doesn't have to exactly match any game loaded in
    the SF7. It needs to match any valid Game Doctor file name AND have an
    extension of .B## (where # is a digit from 0-9)
  */
  strcpy ((char *) gdfilename, "SF16497 B00"); // TODO: We might need to make a GD file name from the real one
  if (gd6_send_prolog_bytes (gdfilename, 11) == GD_ERROR)
    io_error ();

  if (gd6_sync_receive_start () == GD_ERROR)
    io_error ();

  if (gd6_receive_bytes (buffer, 16) == GD_ERROR)
    io_error ();

  transfer_size = buffer[1] | (buffer[2] << 8) | (buffer[3] << 16) | (buffer[4] << 24);
  if (transfer_size != 0x8000)
    {
      fprintf (stderr, "ERROR: SRAM transfer size from Game Doctor != 0x8000 bytes\n");
      exit (1);
    }

  printf ("Receive: %d Bytes\n", transfer_size);
  puts ("Press q to abort\n");

  starttime = time (NULL);
  while (bytesreceived < transfer_size)
    {
      if (transfer_size - bytesreceived >= BUFFERSIZE)
        len = BUFFERSIZE;
      else
        len = transfer_size - bytesreceived;

      if (gd6_receive_bytes (buffer, len) == GD_ERROR)
        io_error ();
      fwrite (buffer, 1, len, file);

      bytesreceived += len;
      ucon64_gauge (starttime, bytesreceived, 32 * 1024);
      gd_checkabort (2);
    }

  unregister_func (remove_destfile);
  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
}


int
gd3_write_sram (const char *filename, unsigned int parport)
{
  gd_send_prolog_bytes = gd3_send_prolog_bytes;
  gd_send_bytes = gd3_send_bytes;

  return gd_write_sram (filename, parport, GD3_PROLOG_STRING);
}


int
gd6_write_sram (const char *filename, unsigned int parport)
{
  gd_send_prolog_bytes = gd6_send_prolog_bytes;
  gd_send_bytes = gd6_send_bytes;

  return gd_write_sram (filename, parport, GD6_READ_PROLOG_STRING);
}


int
gd_write_sram (const char *filename, unsigned int parport, const char *prolog_str)
{
  FILE *file;
  unsigned char *buffer, gdfilename[12];
  int bytesread, bytessend = 0, size, header_size;
  time_t starttime;

  init_io (parport);

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  size = fsizeof (filename);                    // GD SRAM is 4*8 KB, emu SRAM often not

  if (size == 0x8000)
    header_size = 0;
  else if (size == 0x8200)
    {
      header_size = 0x200;
      size = 0x8000;
    }
  else
    {
      fputs ("ERROR: GD SRAM file size must be 32768 or 33280 bytes\n", stderr);
      exit (1);
    }

  printf ("Send: %d Bytes\n", size);
  fseek (file, header_size, SEEK_SET);          // skip the header

  if (memcmp (prolog_str, GD6_READ_PROLOG_STRING, 4) == 0)
    if (gd6_sync_hardware () == GD_ERROR)
      io_error ();
  memcpy (buffer, prolog_str, 4);
  buffer[4] = 1;
  if (gd_send_prolog_bytes (buffer, 5) == GD_ERROR)
    io_error ();

  buffer[0] = 0x00;
  buffer[1] = 0x80;
  buffer[2] = 0x00;
  buffer[3] = 0x00;
  if (gd_send_prolog_bytes (buffer, 4) == GD_ERROR)
    io_error ();

  /*
    The BRAM (SRAM) filename doesn't have to exactly match any game loaded in
    the SF7. It needs to match any valid Game Doctor file name AND have an
    extension of .B## (where # is a digit from 0-9)
  */
  strcpy ((char *) gdfilename, "SF8123  B00"); // TODO: We might need to make a GD file name from the real one
  if (gd_send_prolog_bytes (gdfilename, 11) == GD_ERROR)
    io_error ();

  puts ("Press q to abort\n");                  // print here, NOT before first GD I/O,
                                                //  because if we get here q works ;-)
  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      if (gd_send_bytes (buffer, bytesread) == GD_ERROR)
        io_error ();

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      gd_checkabort (2);
    }

  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
}


int
gd3_read_saver (const char *filename, unsigned int parport)
{
  (void) filename;                              // warning remover
  (void) parport;                               // warning remover
  return fputs ("ERROR: The function for dumping saver data is not yet implemented for the SF3\n", stderr);
}


int
gd6_read_saver (const char *filename, unsigned int parport)
{
  FILE *file;
  unsigned char *buffer, gdfilename[12];
  int len, bytesreceived = 0, transfer_size;
  time_t starttime;

  init_io (parport);

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  // Be nice to the user and automatically remove the file on an error (or abortion)
  gd_destfname = filename;
  gd_destfile = file;
  register_func (remove_destfile);

  if (gd6_sync_hardware () == GD_ERROR)
    io_error ();
  if (gd6_send_prolog_bytes ((unsigned char *) GD6_WRITE_PROLOG_STRING, 4) == GD_ERROR)
    io_error ();

  /*
    TODO: Graceful handling of an abort because of a name error?
          Currently we fail with a generic error.

    TODO: We could make a GD file name from the real one but a valid dummy name
          seems to work OK here. The user must have the proper game selected in
          the SF7 menu even if the real name is used.
  */
  strcpy ((char *) gdfilename, "SF16497 S00");
  if (gd6_send_prolog_bytes (gdfilename, 11) == GD_ERROR)
    io_error ();

  if (gd6_sync_receive_start () == GD_ERROR)
    io_error ();

  if (gd6_receive_bytes (buffer, 16) == GD_ERROR)
    io_error ();

  transfer_size = buffer[1] | (buffer[2] << 8) | (buffer[3] << 16) | (buffer[4] << 24);
  if (transfer_size != 0x38000)
    {
      fputs ("ERROR: Saver transfer size from Game Doctor != 0x38000 bytes\n", stderr);
      exit (1);
    }

  printf ("Receive: %d Bytes\n", transfer_size);
  puts ("Press q to abort\n");

  starttime = time (NULL);
  while (bytesreceived < transfer_size)
    {
      if (transfer_size - bytesreceived >= BUFFERSIZE)
        len = BUFFERSIZE;
      else
        len = transfer_size - bytesreceived;

      if (gd6_receive_bytes (buffer, len) == GD_ERROR)
        io_error ();
      fwrite (buffer, 1, len, file);

      bytesreceived += len;
      ucon64_gauge (starttime, bytesreceived, 32 * 1024);
      gd_checkabort (2);
    }

  unregister_func (remove_destfile);
  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
}


int
gd3_write_saver (const char *filename, unsigned int parport)
{
  gd_send_prolog_bytes = gd3_send_prolog_bytes;
  gd_send_bytes = gd3_send_bytes;

  return gd_write_saver (filename, parport, GD3_PROLOG_STRING);
}


int
gd6_write_saver (const char *filename, unsigned int parport)
{
  gd_send_prolog_bytes = gd6_send_prolog_bytes;
  gd_send_bytes = gd6_send_bytes;

  return gd_write_saver (filename, parport, GD6_READ_PROLOG_STRING);
}


int
gd_write_saver (const char *filename, unsigned int parport, const char *prolog_str)
{
  FILE *file;
  unsigned char *buffer, gdfilename[12];
  const char *p;
  int bytesread, bytessend = 0, size, fn_length;
  time_t starttime;

  init_io (parport);

  /*
    Check that filename is a valid Game Doctor saver filename.
    It should start with SF, followed by the game ID, followed by the extension.
    The extension is of the form .S## (where # is a digit from 0-9).
    E.g., SF16123.S00
    The saver base filename must match the base name of the game (loaded in the
    Game Doctor) that you are loading the saver data for.
  */

  // Strip the path out of filename for use in the GD
  p = basename2 (filename);
  fn_length = strlen (p);

  if (fn_length < 6 || fn_length > 11 // 7 ("base") + 1 (period) + 3 (extension)
      || toupper (p[0]) != 'S' || toupper (p[1]) != 'F'
      || p[fn_length - 4] != '.' || toupper (p[fn_length - 3]) != 'S')
    {
      fprintf (stderr, "ERROR: Filename (%s) is not a saver filename (SF*.S##)\n", p);
      exit (1);
    }

  if ((file = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], filename);
      exit (1);
    }
  if ((buffer = (unsigned char *) malloc (BUFFERSIZE)) == NULL)
    {
      fprintf (stderr, ucon64_msg[FILE_BUFFER_ERROR], BUFFERSIZE);
      exit (1);
    }

  size = fsizeof (filename);
  if (size != 0x38000)                  // GD saver size is always 0x38000 bytes -- no header
    {
      fputs ("ERROR: GD saver file size must be 229376 bytes\n", stderr);
      exit (1);
    }

  // Make a GD file name from the real one
  memset (gdfilename, ' ', 11);                 // "pad" with spaces
  gdfilename[11] = 0;                           // terminate string
  memcpy (gdfilename, p, fn_length - 4);        // copy name except extension
  memcpy (&gdfilename[8], "S00", 3);            // copy extension S00
  strupr ((char *) gdfilename);

  printf ("Send: %d Bytes\n", size);
  fseek (file, 0, SEEK_SET);

  if (memcmp (prolog_str, GD6_READ_PROLOG_STRING, 4) == 0)
    if (gd6_sync_hardware () == GD_ERROR)
      io_error ();
  memcpy (buffer, prolog_str, 4);
  buffer[4] = 1;
  if (gd_send_prolog_bytes (buffer, 5) == GD_ERROR)
    io_error ();

  // Transfer 0x38000 bytes
  buffer[0] = 0x00;
  buffer[1] = 0x80;
  buffer[2] = 0x03;
  buffer[3] = 0x00;
  if (gd_send_prolog_bytes (buffer, 4) == GD_ERROR)
    io_error ();

  if (gd_send_prolog_bytes (gdfilename, 11) == GD_ERROR)
    io_error ();

  puts ("Press q to abort\n");                  // print here, NOT before first GD I/O,
                                                //  because if we get here q works ;-)
  starttime = time (NULL);
  while ((bytesread = fread (buffer, 1, BUFFERSIZE, file)))
    {
      if (gd_send_bytes (buffer, bytesread) == GD_ERROR)
        io_error ();

      bytessend += bytesread;
      ucon64_gauge (starttime, bytessend, size);
      gd_checkabort (2);
    }

  free (buffer);
  fclose (file);
  deinit_io ();

  return 0;
}

#endif // USE_PARALLEL
