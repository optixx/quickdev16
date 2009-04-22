/*
property.c - configfile handling

Copyright (c) 2004 NoisyB <noisyb@gmx.net>


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
#include <string.h>
#include <sys/stat.h>                           // for struct stat
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#include "file.h"                               // realpath2()
#include "property.h"
#include "misc.h"                               // getenv2()


// Shouldn't we include string.h? - dbjh
#ifndef  _WIN32
#define stricmp strcasecmp
#endif

#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768

#define PROPERTY_SEPARATOR '='
#define PROPERTY_SEPARATOR_S "="
#define PROPERTY_COMMENT '#'
#define PROPERTY_COMMENT_S "#"


char *
get_property (const char *filename, const char *propname, char *buffer,
              const char *def)
{
  char line[MAXBUFSIZE], *p = NULL;
  FILE *fh;
  int prop_found = 0, i, whitespace_len;

  if ((fh = fopen (filename, "r")) != 0)        // opening the file in text mode
    {                                           //  avoids trouble under DOS
      while (fgets (line, sizeof line, fh) != NULL)
        {
          whitespace_len = strspn (line, "\t ");
          p = line + whitespace_len;            // ignore leading whitespace
          if (*p == '#' || *p == '\n' || *p == '\r')
            continue;                           // text after # is comment
          if ((p = strpbrk (line, "#\r\n")))    // strip *any* returns
            *p = 0;

          p = strchr (line, PROPERTY_SEPARATOR);
          // if no divider was found the propname must be a bool config entry
          //  (present or not present)
          if (p)
            *p = 0;                             // note that this "cuts" _line_
          // strip trailing whitespace from property name part of line
          for (i = strlen (line) - 1;
               i >= 0 && (line[i] == '\t' || line[i] == ' ');
               i--)
            ;
          line[i + 1] = 0;

          if (!stricmp (line + whitespace_len, propname))
            {
              if (p)
                {
                  p++;
                  // strip leading whitespace from value
                  strcpy (buffer, p + strspn (p, "\t "));
                  // strip trailing whitespace from value
                  for (i = strlen (buffer) - 1;
                       i >= 0 && (buffer[i] == '\t' || buffer[i] == ' ');
                       i--)
                    ;
                  buffer[i + 1] = 0;
                }
              prop_found = 1;
              break;                            // an environment variable
            }                                   //  might override this
        }
      fclose (fh);
    }

  p = getenv2 (propname);
  if (*p == 0)                                  // getenv2() never returns NULL
    {
      if (!prop_found)
        {
          if (def)
            strcpy (buffer, def);
          else
            buffer = NULL;                      // buffer won't be changed
        }                                       //  after this func (=ok)
    }
  else
    strcpy (buffer, p);
  return buffer;
}


int
get_property_int (const char *filename, const char *propname)
{
  char buf[MAXBUFSIZE];                         // MAXBUFSIZE is enough for a *very* large number
                                                // and people who might not get the idea that 
                                                // get_property_int() is ONLY about numbers
  int value = 0;

  get_property (filename, propname, buf, NULL);

  if (*buf)
    switch (tolower (*buf))
      {
      case '0':                                 // 0
      case 'n':                                 // [Nn]o
        return 0;
      }

  value = strtol (buf, NULL, 10);
  return value ? value : 1;                     // if buf was only text like 'Yes'
}                                               //  we'll return at least 1


char *
get_property_fname (const char *filename, const char *propname, char *buffer,
                    const char *def)
// get a filename from file with name filename, expand it and fix characters
{
  char tmp[FILENAME_MAX];

  get_property (filename, propname, tmp, def);
#ifdef  __CYGWIN__
  fix_character_set (tmp);
#endif
  return realpath2 (tmp, buffer);
}


int
set_property (const char *filename, const char *propname, const char *value,
              const char *comment)
{
  int found = 0, result = 0, file_size = 0, i;
  char line[MAXBUFSIZE], line2[MAXBUFSIZE], *str = NULL, *p = NULL;
  FILE *fh;
  struct stat fstate;

  if (stat (filename, &fstate) != 0)
    file_size = fstate.st_size;

  if (!(str = (char *) malloc (file_size + MAXBUFSIZE)))
    return -1;
  *str = 0;

  if ((fh = fopen (filename, "r")) != 0)        // opening the file in text mode
    {                                           //  avoids trouble under DOS
      while (fgets (line, sizeof line, fh) != NULL)
        {
          strcpy (line2, line);
          if ((p = strpbrk (line2, PROPERTY_SEPARATOR_S "#\r\n")))
            *p = 0;                             // note that this "cuts" _line2_
          for (i = strlen (line2) - 1;
               i >= 0 && (line2[i] == '\t' || line2[i] == ' ');
               i--)
            ;
          line2[i + 1] = 0;

          if (!stricmp (line2 + strspn (line2, "\t "), propname))
            {
              found = 1;
              if (value == NULL)
                continue;

              sprintf (line, "%s" PROPERTY_SEPARATOR_S "%s\n", propname, value);
            }
          strcat (str, line);
        }
      fclose (fh);
    }

  if (!found && value)
    {
      if (comment)
        {
          strcat (str, PROPERTY_COMMENT_S "\n" PROPERTY_COMMENT_S " ");

          for (p = strchr (str, 0); *comment; comment++)
            switch (*comment)
              {
              case '\r':
                break;
              case '\n':
                strcat (str, "\n" PROPERTY_COMMENT_S " ");
                break;

              default:
                p = strchr (str, 0);
                *p = *comment;
                *(++p) = 0;
                break;
              }

          strcat (str, "\n" PROPERTY_COMMENT_S "\n");
        }

      sprintf (line, "%s" PROPERTY_SEPARATOR_S "%s\n", propname, value);
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
      result = set_property (filename, prop[i].name, prop[i].value, prop[i].comment);

      if (result == -1) // failed
        break;
    }

  return result;
}
