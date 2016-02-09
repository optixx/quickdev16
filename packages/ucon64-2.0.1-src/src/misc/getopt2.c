/*
getopt2.c - getopt1() extension

Copyright (c) 2004 - 2005 NoisyB
Copyright (c) 2005        dbjh


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
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <stdio.h>
#include <string.h>
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <sys/stat.h>
#ifdef  _MSC_VER
#pragma warning(pop)
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
#include <windows.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#include <io.h>
#pragma warning(pop)
#define S_ISDIR(mode) ((mode) & _S_IFDIR ? 1 : 0)
#define F_OK 00
#endif
#endif
#include "misc/file.h"
#include "misc/getopt2.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768


const st_getopt2_t *
getopt2_get_index_by_val (const st_getopt2_t *option, int val)
{
  int i = 0;

  for (; option[i].name || option[i].help; i++)
    if (option[i].name && // it IS an option
        option[i].val == val)
      return &option[i];

  return NULL;
}


#ifdef  DEBUG
void
getopt2_sanity_check_output (st_getopt2_t *p)
{
  printf ("{\"%s\", %d, 0, %d, \"%s\", \"%s\", %d}, // console: %d workflow: %d\n",
    p->name,
    p->has_arg,
    p->val,
    p->arg_name,
    p->help ? "usage" : p->help, // i (nb) mean it
//    p->help,
    0,
    p->object ? ((st_ucon64_obj_t *) p->object)->console : 0,
    p->object ? ((st_ucon64_obj_t *) p->object)->flags : 0);
}


void
getopt2_sanity_check (const st_getopt2_t *option)
{
  int x, y = 0;

  for (x = 0; option[x].name || option[x].help; x++)
    if (option[x].name)
      for (y = 0; option[y].name || option[y].help; y++)
        if (option[y].name)
          if (!strcmp (option[x].name, option[y].name))
            if (option[x].val != option[y].val ||
                option[x].has_arg != option[y].has_arg)
              {
                fprintf (stderr, "ERROR: getopt2_sanity_check(): found dupe %s%s with different has_arg, or val\n",
                  option[x].name[1] ? OPTION_LONG_S : OPTION_S, option[x].name);
              }
}


void
getopt2_parse_usage (const char *usage_output)
// parse usage output into st_getopt2_t array (for development)
{
  int i = 0, count = 0;
  char buf[MAXBUFSIZE], *s = NULL, *d = NULL;
  FILE *fh = fopen (usage_output, "r");

  if (!fh)
    return;

  while (fgets (buf, MAXBUFSIZE, fh))
    {
      st_getopt2_t usage;
      int value = 0;

      if (*buf == '\n')
        continue;

      memset (&usage, 0, sizeof (st_getopt2_t));

#ifdef  DEBUG
      printf (buf);
#endif
      s = d = buf;
      d = strstr (s, " " OPTION_S);
      if (d && (d - s) < 10)
        {
          s = (d + strspn (++d, OPTION_S));

          for (i = 0; s[i] && s[i] != ' '; i++)
            if (s[i] == OPTARG)
              {
                value = 1;
                d = strtok (s, OPTARG_S);
                break;
              }

          if (!value)
            d = strtok (s, " ");

          if (d)
            usage.name = d;

          if (value)                            // parse =VALUE
            {
              d = strtok (NULL, " ");

              if (d)
                usage.arg_name = d;
            }
        }


      if (usage.name)
        {
          printf ("{\"%s\", ", usage.name);

          if (usage.arg_name)
            printf ("1, \"%s\", ", usage.arg_name);
          else
            printf ("0, NULL, ");

          printf ("\"%s\", NULL},", strtrimr (strtriml (strtok (NULL, "\n"))));

        }
      else
        printf ("{NULL, 0, NULL, \"%s\", NULL},", strtrimr (strtriml (strtok (s, "\n"))));

      count++;
      if (!(count % 10))
        printf ("         // %d", count);
      fputc ('\n', stdout);
    }
}


void
getopt2_usage_code (const st_getopt2_t *usage)
{
  int i = 0;
  char buf[MAXBUFSIZE];

#ifdef  DEBUG
  getopt2_sanity_check (usage);
#endif

  for (; usage[i].name || usage[i].help; i++)
    {
      printf ("{\n  %s%s%s, %d, 0, %d, // %d\n  %s%s%s, %s%s%s,\n  (void *) %d\n},\n",
        usage[i].name ? "\"" : "",
        usage[i].name ? usage[i].name : "NULL",
        usage[i].name ? "\"" : "",
        usage[i].has_arg,
        usage[i].val,
        i,
        usage[i].arg_name ? "\"" : "",
        usage[i].arg_name ? usage[i].arg_name : "NULL",
        usage[i].arg_name ? "\"" : "",
        usage[i].help ? "\"" : "",
        usage[i].help ? string_code (buf, usage[i].help) : "NULL",
        usage[i].help ? "\"" : "",
        (int) usage[i].object);
    }
}
#endif // DEBUG


void
getopt2_usage (const st_getopt2_t *usage)
{
  int i = 0;
  char buf[MAXBUFSIZE];

  for (i = 0; usage[i].name || usage[i].help; i++)
    if (usage[i].help) // hidden options ARE allowed
      {
        if (usage[i].name)
          {
            sprintf (buf, "%s%s%s%s%s%s ",
              // long or short name?
              (usage[i].name[1] ? "  " OPTION_LONG_S : "   " OPTION_S),
              usage[i].name,
              usage[i].has_arg == 2 ? "[" : "", // == 2 arg is optional
              usage[i].arg_name ? OPTARG_S : "",
              usage[i].arg_name ? usage[i].arg_name : "",
              usage[i].has_arg == 2 ? "]" : ""); // == 2 arg is optional

            if (strlen (buf) < 16)
              {
                strcat (buf, "                             ");
                buf[16] = 0;
              }
            fputs (buf, stdout);
          }

        if (usage[i].help)
          {
            char c, *p = buf, *p2 = NULL;

            strcpy (buf, usage[i].help);

            if (usage[i].name)
              for (; (p2 = strchr (p, '\n')) != NULL; p = p2 + 1)
                {
                  c = p2[1];
                  p2[1] = 0;
                  fputs (p, stdout);
                  fputs ("                  ", stdout);
                  p2[1] = c;
                }

            fputs (p, stdout);
            fputc ('\n', stdout);
          }
      }
}


static int
getopt2_long_internal (struct option *long_option, const st_getopt2_t *option,
                       int n, int long_only)
{
  int i = 0, j = 0, x = 0;

#ifdef  DEBUG
  getopt2_sanity_check (option);
#endif

  memset (long_option, 0, sizeof (struct option) * n);

  for (; option[i].name || option[i].help; i++)
    if (option[i].name) // IS option
      if (long_only || option[i].name[1]) // IS long
                                          // if (long_only) also one char options are long
        {
          for (j = 0; j < i; j++)
            if (option[j].name)
              if (!strcmp (option[i].name, option[j].name))
                break; // no dupes

          if (j == i && x < n)
            {
#ifdef  _MSC_VER
              (char *)
#endif
              long_option[x].name =
#ifdef  _MSC_VER
                                    (char *)
#endif
                                    option[i].name;
              long_option[x].has_arg = option[i].has_arg;
              long_option[x].flag = option[i].flag;
              long_option[x++].val = option[i].val;
            }
      }

  return x < n ? x + 1 : 0;
}


int
getopt2_long (struct option *long_option, const st_getopt2_t *option, int n)
{
  return getopt2_long_internal (long_option, option, n, 0);
}


int
getopt2_long_only (struct option *long_option, const st_getopt2_t *option, int n)
{
  return getopt2_long_internal (long_option, option, n, 1);
}


int
getopt2_short (char *short_option, const st_getopt2_t *option, int n)
{
  int i = 0;
  char *p = short_option;

#ifdef  DEBUG
  getopt2_sanity_check (option);
#endif

  *p = 0;
  for (; option[i].name || option[i].help; i++)
    if ((int) strlen (short_option) + 3 < n && option[i].name) // IS option
      if (!option[i].name[1]) // IS short
        if (!strchr (short_option, option[i].name[0])) // no dupes
          {
            *p++ = option[i].name[0];
            switch (option[i].has_arg)
              {
              case 2:
                *p++ = ':';
              case 1:                           // falling through
                *p++ = ':';
              case 0:
                break;
#ifdef  DEBUG
              default:
                fprintf (stderr, "ERROR: getopt2_short(): unexpected has_arg value (%d)\n", option[i].has_arg);
#endif // DEBUG
              }
            *p = 0;
          }
#ifdef  DEBUG
  printf ("%s\n", short_option);
  fflush (stdout);
#endif

  return (int) strlen (short_option) + 3 < n ? (int) strlen (short_option) : 0;
}


static int
getopt2_file_recursion (const char *fname, int (*callback_func) (const char *),
                        int *calls, int flags)
{
  char path[FILENAME_MAX];
  struct stat fstate;

  if (strlen (fname) >= FILENAME_MAX - 2)
    return 0;

  realpath2 (fname, path);

  /*
    Try to get file status information only if the file with name fname exists.
    If the file does not exist I set st_mode to 0 instead of __S_IFREG, because I
    don't know if the latter is portable. - dbjh
  */
  if (access (path, F_OK) == 0)
    {
      if (stat (path, &fstate) != 0)
        return 0;
    }
  else
    fstate.st_mode = 0;

  /*
    We test whether fname is a directory, because we handle directories
    differently. The callback function should test whether its argument is a
    regular file, a character special file, a block special file, a FIFO
    special file, a symbolic link or a socket. If the flags
    GETOPT2_FILE_FILES_ONLY, GETOPT2_FILE_RECURSIVE and
    GETOPT2_FILE_RECURSIVE_ONCE were not used by the calling function, it may
    also have to test whether the argument is a directory.
  */
  if (S_ISDIR (fstate.st_mode) ?
        !(flags & (GETOPT2_FILE_FILES_ONLY |
                   GETOPT2_FILE_RECURSIVE |
                   GETOPT2_FILE_RECURSIVE_ONCE)) :
        1)                                      // everything else: call callback
    {
      int result = 0; 

#ifdef  DEBUG
      printf ("callback_func() == %s\n", path);
      fflush (stdout);
#endif

      result = callback_func (path);

      if (!result)
        (*calls)++;

      return result;
    }

  if (S_ISDIR (fstate.st_mode) &&
      (flags & (GETOPT2_FILE_RECURSIVE | GETOPT2_FILE_RECURSIVE_ONCE)))
    {
      int result = 0; 
#ifndef _WIN32
      struct dirent *ep;
      DIR *dp;
#else
      char search_pattern[FILENAME_MAX];
      WIN32_FIND_DATA find_data;
      HANDLE dp;
#endif
      char buf[FILENAME_MAX], *p;

#if     defined __MSDOS__ || defined _WIN32 || defined __CYGWIN__
      char c = (char) toupper ((int) path[0]);
      if (path[strlen (path) - 1] == DIR_SEPARATOR ||
          (c >= 'A' && c <= 'Z' && path[1] == ':' && path[2] == 0))
#else
      if (path[strlen (path) - 1] == DIR_SEPARATOR)
#endif
        p = "";
      else
        p = DIR_SEPARATOR_S;

#ifndef _WIN32
      if ((dp = opendir (path)))
        {
          while ((ep = readdir (dp)))
            if (strcmp (ep->d_name, ".") != 0 &&
                strcmp (ep->d_name, "..") != 0)
              {
                sprintf (buf, "%s%s%s", path, p, ep->d_name);
                result = getopt2_file_recursion (buf, callback_func, calls,
                           flags & ~GETOPT2_FILE_RECURSIVE_ONCE);
                if (result != 0)
                  break;
              }
          closedir (dp);
        }
#else
      sprintf (search_pattern, "%s%s*", path, p);
      if ((dp = FindFirstFile (search_pattern, &find_data)) != INVALID_HANDLE_VALUE)
        {
          do
            if (strcmp (find_data.cFileName, ".") != 0 &&
                strcmp (find_data.cFileName, "..") != 0)
              {
                sprintf (buf, "%s%s%s", path, p, find_data.cFileName);
                result = getopt2_file_recursion (buf, callback_func, calls,
                           flags & ~GETOPT2_FILE_RECURSIVE_ONCE);
                if (result != 0)
                  break;
              }
          while (FindNextFile (dp, &find_data));
          FindClose (dp);
        }
#endif
    }

  return 0;
}


int
getopt2_file (int argc, char **argv, int (*callback_func) (const char *), int flags)
{
  int x = optind, calls = 0, result = 0;

  for (; x < argc; x++)
    {
      result = getopt2_file_recursion (argv[x], callback_func, &calls, flags);
      if (result != 0)
        break;
    }

  return calls;
}


#if defined TEST && TEST
// compile with -DTEST to build an executable

enum
{
  OPTION_HELP = 1,
  OPTION_A,
  OPTION_BBB,
  OPTION_C,
  OPTION_D
};


#define MAX_OPTIONS 256


int
main (int argc, char **argv)
{
  const char *str = "test";
  int c, digit_optind = 0;
  st_getopt2_t option[] =
    {
      {
        "help", 0, 0, OPTION_HELP,
        NULL, "show this output and exit",
        NULL
      },
      {
        "a", 0, 0, OPTION_A,
        NULL, "option a",
        (void *) str
      },
      {
        "bbb", 0, 0, OPTION_BBB,
        NULL, "option bbb",
        NULL
      },
      {
        "c", 1, 0, OPTION_C,
        "ARG", "option c with required ARG",
        NULL
      },
      {
        "d", 2, 0, OPTION_D,
        "ARG", "option d with optional ARG",
        NULL
      },
      {NULL, 0, 0, 0, NULL, NULL, NULL}
    };
  struct option option_long[MAX_OPTIONS];
  char option_short[MAX_OPTIONS*3];

  // turn st_getopt2_t into struct option
  getopt2_long (option_long, option, MAX_OPTIONS);
  getopt2_short (option_short, option, MAX_OPTIONS);

//  printf ("option_short: \"%s\"\n", option_short);

  optind = 0;
  while ((c = getopt_long (argc, argv, option_short, option_long, NULL)) != -1)
    {
      if (c == '?') // getopt() returns 0x3f ('?') when an unknown option was given
        {
          printf ("Try '%s " OPTION_LONG_S "help' for more information.\n",
            argv[0]);
          exit (1);
        }

      if (c == -1)
        break;

      switch (c)
        {
        case OPTION_A:
          printf ("option a with object '%s'\n", (const char *) getopt2_get_index_by_val (option, OPTION_A)->object);
          break;

        case OPTION_BBB:
          printf ("option bbb\n");
          break;

        case OPTION_C:
          printf ("option c with required value '%s'\n", optarg);
          break;

        case OPTION_D:
          printf ("option d with optional value '%s'\n", optarg);
          break;

        case OPTION_HELP:
          getopt2_usage (option);
          break;

        default:
          printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }

  if (optind < argc)
    {
      printf ("non-option ARGV-elements: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      printf ("\n");
    }

  exit (0);
}
#endif // TEST
