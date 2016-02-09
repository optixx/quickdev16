/*
property.c - configfile handling

Copyright (c) 2004 - 2005 NoisyB


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
#include <sys/stat.h>                           // for struct stat
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#include "misc/archive.h"
#include "misc/file.h"                          // realpath2()
#include "misc/misc.h"                          // getenv2()
#include "misc/property.h"
#include "misc/string.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif
#define MAXBUFSIZE 32768


int
property_check (const char *filename, int version, int verbose)
{
  char buf[MAXBUFSIZE];
  const char *p = NULL;
  int result = 0;

  if (access (filename, F_OK) != 0)
    {
      FILE *fh = NULL;

      if (verbose) 
        {
          fprintf (stderr, "NOTE: %s not found: creating...", filename);
          fflush (stderr);
        }

      if ((fh = fopen (filename, "w")) == NULL) // opening the file in text mode
        {                                       //  avoids trouble on DOS
          printf ("FAILED\n\n");
          return -1;
        }
      fclose (fh);                              // we'll use set_property() from now
    }
  else
    {
      p = get_property (filename, "version", PROPERTY_MODE_TEXT);
      if (strtol (p ? p : "0", NULL, 10) >= version)
        return 0;                               // OK

      strcpy (buf, filename);
      set_suffix (buf, ".old");

      if (verbose)
        {
          fprintf (stderr, "NOTE: updating config: will be renamed to %s...", buf);
          fflush (stderr);
        }

      rename (filename, buf);
    }

  // store new version
  sprintf (buf, "%d", version);
  result = set_property (filename, "version", buf, "configfile version (do NOT edit)");

  if (result > 0)
    {
      if (verbose)
        fprintf (stderr, "OK\n\n");
    }
  else
    {
      if (verbose)
        fprintf (stderr, "FAILED\n\n");

      // remove the crap
      remove (filename);
    }

  if (verbose)
    fflush (stderr);

  return 1;
}


const char *
get_property_from_string (char *str, const char *propname, const char prop_sep,
                          const char comment_sep)
{
  static char value_s[MAXBUFSIZE];
  char str_end[8], *p = NULL, buf[MAXBUFSIZE];
  int len = strlen (str);

  if (len >= MAXBUFSIZE)
    len = MAXBUFSIZE - 1;
  memcpy (buf, str, len);
  buf[len] = 0;

  p = strtriml (buf);
  if (*p == comment_sep || *p == '\n' || *p == '\r')
    return NULL;                                // text after comment_sep is comment

  sprintf (str_end, "%c\r\n", comment_sep);
  if ((p = strpbrk (buf, str_end)) != NULL)     // strip *any* returns and comments
    *p = 0;

  p = strchr (buf, prop_sep);
  if (p)
    {
      *p = 0;                                   // note that this "cuts" _buf_ ...
      p++;
    }
  strtriml (strtrimr (buf));

  if (!stricmp (buf, propname))                 // ...because we do _not_ use strnicmp()
    {
      // if no divider was found the propname must be a bool config entry
      //  (present or not present)
      if (p)
        {
          strncpy (value_s, p, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
          strtriml (strtrimr (value_s));
        }
      else
        strcpy (value_s, "1");
    }
  else
    return NULL;

  return value_s;
}


const char *
get_property (const char *filename, const char *propname, int mode)
{
  char line[MAXBUFSIZE], *p = NULL;
  FILE *fh;
  const char *value_s = NULL;

  if ((fh = fopen (filename, "r")) != 0)        // opening the file in text mode
    {                                           //  avoids trouble on DOS
      while (fgets (line, sizeof line, fh) != NULL)
        if ((value_s = get_property_from_string (line, propname,
               PROPERTY_SEPARATOR, PROPERTY_COMMENT)) != NULL)
          break;

      fclose (fh);
    }

  p = getenv2 (propname);
  if (*p != 0)                                  // getenv2() never returns NULL
    value_s = p;

  if (value_s)
    if (mode == PROPERTY_MODE_FILENAME)
      {
        static char tmp[FILENAME_MAX];

        realpath2 (value_s, tmp);
        value_s = tmp;
    }

  return value_s;
}


int
get_property_int (const char *filename, const char *propname)
{
  const char *value_s = NULL;                   // MAXBUFSIZE is enough for a *very* large number
  int value = 0;                                //  and people who might not get the idea that
                                                //  get_property_int() is ONLY about numbers
  value_s = get_property (filename, propname, PROPERTY_MODE_TEXT);

  if (!value_s)
    value_s = "0";

  if (*value_s)
    switch (tolower ((int) *value_s))
      {
        case '0':                               // 0
        case 'n':                               // [Nn]o
          return 0;
      }

  return value ? value : 1;                     // if value_s was only text like 'Yes'
}                                               //  we'll return at least 1


int
set_property (const char *filename, const char *propname,
              const char *value_s, const char *comment_s)
{
  int found = 0, result = 0, file_size = 0;
  char line[MAXBUFSIZE], line2[MAXBUFSIZE], *str = NULL, *p = NULL,
       line_end[6];
  FILE *fh;
  struct stat fstate;

  if (stat (filename, &fstate) != 0)
    file_size = fstate.st_size;

  if ((str = (char *) malloc (file_size + MAXBUFSIZE)) == NULL)
    return -1;

  sprintf (line_end, "%c\r\n", PROPERTY_COMMENT);

  *str = 0;
  if ((fh = fopen (filename, "r")) != 0)        // opening the file in text mode
    {                                           //  avoids trouble on DOS
      // update existing properties
      while (fgets (line, sizeof line, fh) != NULL)
        {
          strcpy (line2, line);
          if ((p = strpbrk (line2, line_end)) != NULL)
            *p = 0;                             // note that this "cuts" _line2_
          p = strchr (line2, PROPERTY_SEPARATOR);
          if (p)
            *p = 0;

          strtriml (strtrimr (line2));

          if (!stricmp (line2, propname))
            {
              found = 1;
              if (value_s)
                sprintf (line, "%s%c%s\n", propname, PROPERTY_SEPARATOR, value_s ? value_s : "1");
            }
          strcat (str, line);
        }
      fclose (fh);
    }

  // completely new properties are added at the bottom
  if (!found && value_s)
    {
      if (comment_s)
        {
          sprintf (strchr (str, 0), "%c\n%c ", PROPERTY_COMMENT, PROPERTY_COMMENT);

          for (p = strchr (str, 0); *comment_s; comment_s++)
            switch (*comment_s)
              {
              case '\r':
                break;
              case '\n':
                sprintf (strchr (str, 0), "\n%c ", PROPERTY_COMMENT);
                break;

              default:
                p = strchr (str, 0);
                *p = *comment_s;
                *(++p) = 0;
                break;
              }

          sprintf (strchr (str, 0), "\n%c\n", PROPERTY_COMMENT);
        }

      sprintf (line, "%s%c%s\n", propname, PROPERTY_SEPARATOR, value_s);
      strcat (str, line);
    }

  if ((fh = fopen (filename, "w")) == NULL)     // open in text mode
    return -1;
  result = fwrite (str, 1, strlen (str), fh);
  fclose (fh);

  return result;
}


int
set_property_array (const char *filename, const st_property_t *prop)
{
  int i = 0, result = 0;

  for (; prop[i].name; i++)
    {
      result = set_property (filename, prop[i].name, prop[i].value_s,
                             prop[i].comment_s);

      if (result == -1)                         // failed
        break;
    }

  return result;
}
