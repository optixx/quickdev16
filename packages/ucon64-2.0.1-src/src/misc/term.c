/*
term.c - terminal functions

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
// gcc term.c -o term -ltermcap -DUSE_TERMCAP
// st_term_t and term_*() functions taken and modified from LAME frontend/console.c
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef  USE_TERMCAP
#include <termcap.h>
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  _WIN32
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4255) // 'function' : no function prototype given: converting '()' to '(void)'
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <io.h>                                 // isatty() (MinGW)
#include <windows.h>                            // HANDLE, SetConsoleTextAttribute()
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#endif // _WIN32
#ifdef  DJGPP
#include <dpmi.h>                               // needed for __dpmi_int() by ansi_init()
#endif
#include "misc/term.h"
#include "ucon64_defines.h"


#ifdef  __CYGWIN__                              // on Cygwin (gcc for Windows) we
#define USE_POLL                                //  need poll() for kbhit(). poll()
#include <sys/poll.h>                           //  is available on Linux, not on
#endif                                          //  BeOS. DOS already has kbhit()

#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined __APPLE__                       // Mac OS X actually
#include <termios.h>
typedef struct termios tty_t;
#endif


typedef struct
{
#if     defined _WIN32 && !defined __CYGWIN__
  HANDLE Console_Handle;
#endif
  int w;                        // display width
  int h;                        // display height
  char up[10];
  char clreoln[10];
  char emph[10];
  char norm[10];
} st_term_t;


static st_term_t term;


int
term_open (void)
{
  st_term_t *t = &term;
#ifdef  USE_TERMCAP
  char *tp;
  char tc[10];
  int val;
#endif

  // defaults
  memset (t, 0, sizeof (st_term_t));
  t->w = 80;
  t->h = 25;
#if     defined _WIN32 && !defined __CYGWIN__
  t->Console_Handle = GetStdHandle (STD_ERROR_HANDLE);
#endif
  strcpy (t->up, "\033[A");

#ifdef  USE_TERMCAP
  // try to catch additional information about special termsole sequences
#if 0
  if (!(term_name = getenv ("TERM")))
    return -1;
  if (tgetent (term_buff, term_name) != 1)
    return -1;
#endif

  val = tgetnum ("co");
  if (val >= 40 && val <= 512)
    t->w = val;

  val = tgetnum ("li");
  if (val >= 16 && val <= 256)
    t->h = val;

  *(tp = tc) = 0;
  tp = tgetstr ("up", &tp);
  if (tp)
    strcpy (t->up, tp);

  *(tp = tc) = 0;
  tp = tgetstr ("ce", &tp);
  if (tp)
    strcpy (t->clreoln, tp);

  *(tp = tc) = 0;
  tp = tgetstr ("md", &tp);
  if (tp)
    strcpy (t->emph, tp);

  *(tp = tc) = 0;
  tp = tgetstr ("me", &tp);
  if (tp)
    strcpy (t->norm, tp);
#endif  // USE_TERMCAP

  return 0;
}


int
term_close (void)
{
  st_term_t *t = &term;

  if (t)
    {
      memset (t, 0, sizeof (st_term_t));
//      free (t);
    }

  return 0;
}


int
term_w (void)
{
  st_term_t *t = &term;
  return t->w;
}


int
term_h (void)
{
  st_term_t *t = &term;
  return t->h;
}


const char *
term_up (void)
{
  st_term_t *t = &term;
  return t->up;
}


const char *
term_clreoln (void)
{
  st_term_t *t = &term;
  return t->clreoln;
}


const char *
term_emph (void)
{
  st_term_t *t = &term;
  return t->emph;
}


const char *
term_norm (void)
{
  st_term_t *t = &term;
  return t->norm;
}


static int misc_ansi_color = 0;

#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined __APPLE__                       // Mac OS X actually
static void set_tty (tty_t *param);
#endif


#if     defined _WIN32 && defined USE_ANSI_COLOR
int
vprintf2 (const char *format, va_list argptr)
// Cheap hack to get the Visual C++ and MinGW ports support "ANSI colors".
//  Cheap, because it only supports the ANSI escape sequences uCON64 uses.
{
#undef  printf
#undef  fprintf
  int n_chars = 0, n_ctrl = 0, n_print, done = 0;
  char output[MAXBUFSIZE], *ptr, *ptr2;
  HANDLE stdout_handle;
  CONSOLE_SCREEN_BUFFER_INFO info;
  WORD org_attr, new_attr = 0;

  n_chars = _vsnprintf (output, MAXBUFSIZE, format, argptr);
  if (n_chars == -1)
    {
      fprintf (stderr, "INTERNAL ERROR: Output buffer in vprintf2() is too small (%d bytes).\n"
                       "                Please send a bug report\n", MAXBUFSIZE);
      exit (1);
    }
  output[MAXBUFSIZE - 1] = 0;

  if ((ptr = strchr (output, 0x1b)) == NULL)
    fputs (output, stdout);
  else
    {
      stdout_handle = GetStdHandle (STD_OUTPUT_HANDLE);
      GetConsoleScreenBufferInfo (stdout_handle, &info);
      org_attr = info.wAttributes;

      if (ptr > output)
        {
          *ptr = 0;
          fputs (output, stdout);
          *ptr = 0x1b;
        }
      while (!done)
        {
          if (memcmp (ptr, "\x1b[0m", 4) == 0)
            {
              new_attr = org_attr;
              n_ctrl = 4;
            }
          else if (memcmp (ptr, "\x1b[01;31m", 8) == 0)
            {
              new_attr = FOREGROUND_INTENSITY | FOREGROUND_RED;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[01;32m", 8) == 0)
            {
              new_attr = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[01;33m", 8) == 0) // bright yellow
            {
              new_attr = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[31;41m", 8) == 0)
            {
              new_attr = FOREGROUND_RED | BACKGROUND_RED;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[32;42m", 8) == 0)
            {
              new_attr = FOREGROUND_GREEN | BACKGROUND_GREEN;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[30;41m", 8) == 0) // 30 = foreground black
            {
              new_attr = BACKGROUND_RED;
              n_ctrl = 8;
            }
          else if (memcmp (ptr, "\x1b[30;42m", 8) == 0)
            {
              new_attr = BACKGROUND_GREEN;
              n_ctrl = 8;
            }
          else if (*ptr == 0x1b)
            {
              new_attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
              SetConsoleTextAttribute (stdout_handle, new_attr);
              printf ("\n"
                      "INTERNAL WARNING: vprintf2() encountered an unsupported ANSI escape sequence\n"
                      "                  Please send a bug report\n");
              n_ctrl = 0;
            }
          SetConsoleTextAttribute (stdout_handle, new_attr);

          ptr2 = strchr (ptr + 1, 0x1b);
          if (ptr2)
            n_print = ptr2 - ptr;
          else
            {
              n_print = strlen (ptr);
              done = 1;
            }

          ptr[n_print] = 0;
          ptr += n_ctrl;
          fputs (ptr, stdout);
          (ptr - n_ctrl)[n_print] = 0x1b;
          ptr = ptr2;
        }
    }
  return n_chars;
#define printf  printf2
#define fprintf fprintf2
}


int
printf2 (const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = vprintf2 (format, argptr);
  va_end (argptr);
  return n_chars;
}


int
fprintf2 (FILE *file, const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  if (file != stdout)
    n_chars = vfprintf (file, format, argptr);
  else
    n_chars = vprintf2 (format, argptr);
  va_end (argptr);
  return n_chars;
}
#endif // defined _WIN32 && defined USE_ANSI_COLOR


void
clear_line (void)
/*
  This function is used to fix a problem when using the MinGW or Visual C++ port
  on Windows 98 (probably Windows 95 too) while ANSI.SYS is not loaded.
  If a line contains colors, printed with printf() or fprintf() (actually
  printf2() or fprintf2()), it cannot be cleared by printing spaces on the same
  line. A solution is using SetConsoleTextAttribute(). The problem doesn't occur
  if ANSI.SYS is loaded. It also doesn't occur on Windows XP, even if ANSI.SYS
  isn't loaded. We print 79 spaces (not 80), because in command.com the cursor
  advances to the next line if we print something on the 80th column (in 80
  column mode). This doesn't happen in xterm.
*/
{
#if     !defined _WIN32 || !defined USE_ANSI_COLOR
  fputs ("\r                                                                               \r", stdout);
#else
  WORD org_attr;
  CONSOLE_SCREEN_BUFFER_INFO info;
  HANDLE stdout_handle = GetStdHandle (STD_OUTPUT_HANDLE);
  GetConsoleScreenBufferInfo (stdout_handle, &info);
  org_attr = info.wAttributes;
  SetConsoleTextAttribute (stdout_handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  fputs ("\r                                                                               \r", stdout);
  SetConsoleTextAttribute (stdout_handle, org_attr);
#endif
}


int
ansi_init (void)
{
  int result = isatty (STDOUT_FILENO);

#ifdef  DJGPP
  if (result)
    {
      // Don't use __MSDOS__, because __dpmi_regs and __dpmi_int are DJGPP specific
      __dpmi_regs reg;

      reg.x.ax = 0x1a00;                        // DOS 4.0+ ANSI.SYS installation check
      __dpmi_int (0x2f, &reg);
      if (reg.h.al != 0xff)                     // AL == 0xff if installed
        result = 0;
    }
#endif

  misc_ansi_color = result;

  return result;
}


int
gauge (int percent, int width, char char1, char char2, int color1, int color2)
{
  int x = 0;
  char buf[1024 + 32];                          // 32 == ANSI code buffer

  if (!width || percent < 0 || percent > 100)
    return -1;

  if (width > 1024)
    width = 1024;

  x = (width * percent) / 100;

  memset (buf, char1, x);
  buf[x] = 0;

  if (x < width) // percent < 100
    { 
      if (color1 != -1 && color2 != -1)
        sprintf (&buf[x], "\x1b[3%d;4%dm", color1, color1);

      memset (strchr (buf, 0), char2, width - x);
    }

  if (color1 != -1 && color2 != -1) // ANSI?
    {
      buf[width + 8] = 0;                       // 8 == ANSI code length
      fprintf (stdout, "\x1b[3%d;4%dm%s\x1b[0m", color2, color2, buf);
    }
  else // no ANSI
    {
      buf[width] = 0;
      fputs (buf, stdout);
    }

  return 0;
}


#if     (defined __unix__ && !defined __MSDOS__) || defined __BEOS__ || \
        defined __APPLE__                       // Mac OS X actually
static int oldtty_set = 0, stdin_tty = 1;       // 1 => stdin is a tty, 0 => it's not
static tty_t oldtty, newtty;


void
set_tty (tty_t *param)
{
  if (stdin_tty && tcsetattr (STDIN_FILENO, TCSANOW, param) == -1)
    {
      fprintf (stderr, "ERROR: Could not set tty parameters\n");
      exit (100);
    }
}


/*
  This code compiles with DJGPP, but is not neccesary. Our kbhit() conflicts
  with DJGPP's one, so it won't be used for that function. Perhaps it works
  for making getchar() behave like getch(), but that's a bit pointless.
*/
void
init_conio (void)
{
  if (!isatty (STDIN_FILENO))
    {
      stdin_tty = 0;
      return;                                   // rest is nonsense if not a tty
    }

  if (tcgetattr (STDIN_FILENO, &oldtty) == -1)
    {
      fprintf (stderr, "ERROR: Could not get tty parameters\n");
      exit (101);
    }
  oldtty_set = 1;

#if 0
  if (register_func (deinit_conio) == -1)
    {
      fprintf (stderr, "ERROR: Could not register function with register_func()\n");
      exit (102);
    }
#endif

  newtty = oldtty;
  newtty.c_lflag &= ~(ICANON | ECHO);
  newtty.c_lflag |= ISIG;
  newtty.c_cc[VMIN] = 1;                        // if VMIN != 0, read calls
  newtty.c_cc[VTIME] = 0;                       //  block (wait for input)

  set_tty (&newtty);
}


void
deinit_conio (void)
{
  if (oldtty_set)
    {
      tcsetattr (STDIN_FILENO, TCSAFLUSH, &oldtty);
      oldtty_set = 0;
    }
}


#if     defined __CYGWIN__ && !defined USE_POLL
#warning kbhit() does not work properly in Cygwin executable if USE_POLL is not defined
#endif
// this kbhit() conflicts with DJGPP's one
int
kbhit (void)
{
#ifdef  USE_POLL
  struct pollfd fd;

  fd.fd = STDIN_FILENO;
  fd.events = POLLIN;
  fd.revents = 0;

  return poll (&fd, 1, 0) > 0;
#else
  tty_t tmptty = newtty;
  int ch, key_pressed;

  tmptty.c_cc[VMIN] = 0;                        // doesn't work as expected on
  set_tty (&tmptty);                            //  Cygwin (define USE_POLL)

  if ((ch = fgetc (stdin)) != EOF)
    {
      key_pressed = 1;
      ungetc (ch, stdin);
    }
  else
    key_pressed = 0;

  set_tty (&newtty);

  return key_pressed;
#endif
}
#elif   defined AMIGA                           // (__unix__ && !__MSDOS__) ||
int                                             //  __BEOS__ ||__APPLE__
kbhit (void)
{
  return GetKey () != 0xff ? 1 : 0;
}


int
getch (void)
{
  BPTR con_fileh;
  int temp;

  con_fileh = Input ();
  // put the console into RAW mode which makes getchar() behave like getch()?
  if (con_fileh)
    SetMode (con_fileh, 1);
  temp = getchar ();
  // put the console out of RAW mode (might make sense)
  if (con_fileh)
    SetMode (con_fileh, 0);

  return temp;
}
#endif                                          // AMIGA



//#define TEST
#ifdef  TEST
int
main (int argc, char **argv)
{
  term_open ();
  printf ("%s%s%s%s%s%s", term_up (), term_up (), term_up (), term_up (), term_up (), term_up ());
}
#endif  // TEST
