/*
term.h - terminal functions

Copyright (c) 1999 - 2006       NoisyB
Copyright (c) 2001 - 2005, 2015 dbjh
Copyright (c) 2002 - 2004       Jan-Erik Karlsson (Amiga code)


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
#ifndef MISC_TERM_H
#define MISC_TERM_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


extern int term_open (void);
extern int term_close (void);

extern int term_w (void);
extern int term_h (void);
extern const char *term_up (void);
extern const char *term_clreoln (void);
extern const char *term_emph (void);
extern const char *term_norm (void);


/*
  ansi_init()     initialize ANSI output
  gauge()         simple gauge (uses ANSI if ansi_init() was successful)
                  if both color values are == -1, no color/ANSI will be used
  clear_line ()   clear the current line (79 spaces)
*/
extern int ansi_init (void);
extern void clear_line (void);
extern int gauge (int percent, int width, char char1, char char2, int color1, int color2);


/*
  Portability (conio.h, etc...)

  init_conio()         init console I/O
  deinit_conio()       stop console I/O
  getch()
  kbhit()
  vprintf2()
  printf2()
  fprintf2()
*/
#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined __APPLE__                       // Mac OS X actually
extern void init_conio (void);
extern void deinit_conio (void);
#define getch getchar                           // getchar() acts like DOS getch() after init_conio()
extern int kbhit (void);                        // may only be used after init_conio()!
#endif

#ifdef  __MSDOS__
#include <conio.h>                              // getch()
#include <pc.h>                                 // kbhit()
#endif

#ifdef  _WIN32
// Note that _WIN32 is defined by cl.exe while the other constants (like WIN32)
//  are defined in header files. MinGW's gcc.exe defines all constants.

#include <conio.h>                              // kbhit() & getch()
#include <sys/types.h>

#ifdef  USE_ANSI_COLOR
#include <stdarg.h>
#include <stdio.h>


extern int vprintf2 (const char *format, va_list argptr);
extern int printf2 (const char *format, ...);
extern int fprintf2 (FILE *file, const char *format, ...);

#define vprintf vprintf2
#define printf printf2
#define fprintf fprintf2
#endif // USE_ANSI_COLOR

#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#include <io.h>
#pragma warning(pop)


#define STDIN_FILENO (fileno (stdin))
#define STDOUT_FILENO (fileno (stdout))
#define STDERR_FILENO (fileno (stderr))
#endif

#define snprintf _snprintf
#endif // _WIN32


#ifdef  AMIGA
// The compiler used by Jan-Erik doesn't have snprintf(). - dbjh
#include "misc/snprintf.h"


extern int kbhit (void);
//#define getch getchar
// Gonna use my (Jan-Erik) fake one. Might work better and more like the real
//  getch().
#endif

#endif // MISC_TERM_H
