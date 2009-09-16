/*
string.c - some string functions

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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "string.h"


#ifdef  _MSC_VER
// Visual C++ doesn't allow inline in C source code
#define inline __inline
#endif


static inline int
is_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  /*
    Casting to unsigned char * is necessary to avoid differences between the
    different compilers' run-time environments. At least for isprint(). Without
    the cast the isprint() of (older versions of) DJGPP, MinGW, Cygwin and
    Visual C++ returns nonzero values for ASCII characters > 126.
  */
  for (; len >= 0; p++, len--)
    if (!func (*(unsigned char *) p))
      return 0;

  return 1;
}


static inline char *
to_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = func (*p);

  return s;
}


char *
strupr (char *s)
{
  return to_func (s, strlen (s), toupper);
}


char *
strlwr (char *s)
{
  return to_func (s, strlen (s), tolower);
}


char *
strcasestr2 (const char *str, const char *search)
{
  if (!(*search))
    return (char *) str;
#if 0
  else
    {
      int len = strlen (search);

      for (; *str; str++)
        if (!strnicmp (str, search, len))
          return (char *) str;
    }

  return NULL;
#else
  return (char *) memmem2 (str, strlen (str), search, strlen (search), MEMMEM2_CASE);
#endif
}


char *
strtrimr (char *str)
{
  int i = strlen (str) - 1;

  while (isspace ((int) str[i]) && (i >= 0))
    str[i--] = 0;

  return str;
}


char *
strtriml (char *str)
{
  int i = 0, j;

  j = strlen (str) - 1;

  while (isspace ((int) str[i]) && (i <= j))
    i++;

  if (0 < i)
    strcpy (str, &str[i]);

  return str;
}


#ifdef  DEBUG
static int
strarg_debug (int argc, char **argv)
{
  int pos;
  fprintf (stderr, "argc:     %d\n", argc);
  for (pos = 0; pos < argc; pos++)
    fprintf (stderr, "argv[%d]:  %s\n", pos, argv[pos]);

  fflush (stderr);
}
#endif


int
strarg (char **argv, char *str, const char *separator_s, int max_args)
{
  int argc = 0;

  if (str)
    if (*str)
      for (; (argv[argc] = (char *) strtok (!argc ? str : NULL, separator_s)) &&
           (argc < (max_args - 1)); argc++)
        ;

#ifdef  DEBUG
  strarg_debug (argc, argv);
#endif

  return argc;
}


int
memcmp2 (const void *buffer, const void *search, size_t searchlen, unsigned int flags)
{
  size_t i = 0;
  const unsigned char *b = (const unsigned char *) buffer,
                      *s = (const unsigned char *) search;

  if (!flags)
    return memcmp (buffer, search, searchlen);

  if (flags & MEMMEM2_REL)
    {
      searchlen--;
      if (searchlen < 1)
        return -1;
    }

  for (i = 0; i < searchlen; i++)
    {
      if (flags & MEMMEM2_WCARD (0))
        if (*(s + i) == (flags & 0xff))
          continue;

      if (flags & MEMMEM2_REL)
        {
          if ((*(b + i) - *(b + i + 1)) != (*(s + i) - *(s + i + 1)))
            break;
        }
      else
        {
          if (flags & MEMMEM2_CASE && isalpha (*(s + i)))
            {
              if (tolower (*(b + i)) != tolower (*(s + i)))
                break;
            }
          else
            if (*(b + i) != *(s + i))
              break;
        }
    }

  return i == searchlen ? 0 : -1;
}


const void *
memmem2 (const void *buffer, size_t bufferlen,
         const void *search, size_t searchlen, unsigned int flags)
{
  size_t i;

  if (bufferlen >= searchlen)
    for (i = 0; i <= bufferlen - searchlen; i++)
      if (!memcmp2 ((const unsigned char *) buffer + i, search, searchlen, flags))
        return (const unsigned char *) buffer + i;

  return NULL;
}


#if 0
int
memwcmp (const void *buffer, const void *search, unsigned int searchlen, int wildcard)
{
  unsigned int n;

  for (n = 0; n < searchlen; n++)
    if (((unsigned char *) search)[n] != wildcard &&
        ((unsigned char *) buffer)[n] != ((unsigned char *) search)[n])
      return -1;

  return 0;
}
#endif
