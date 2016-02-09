/*
misc.c - miscellaneous functions

Copyright (c) 1999 - 2005       NoisyB
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
#ifdef  HAVE_CONFIG_H
#include "config.h"                             // USE_ZLIB
#endif
#include <ctype.h>
#include <stdarg.h>                             // va_arg()
#include <stdlib.h>

#ifdef  __MSDOS__
#include <dos.h>                                // delay(), milliseconds
#elif   defined __unix__
#include <unistd.h>                             // usleep(), microseconds
#elif   defined __BEOS__
#include <OS.h>                                 // snooze(), microseconds
// Include OS.h before misc.h, because OS.h includes StorageDefs.h which
//  includes param.h which unconditionally defines MIN and MAX.
#elif   defined AMIGA
#include <fcntl.h>
#include <unistd.h>
#include <dos/dos.h>
#include <dos/var.h>
#include <dos/dostags.h>
#include <libraries/lowlevel.h>                 // GetKey()
#include <proto/dos.h>
#include <proto/lowlevel.h>
#elif   defined _WIN32
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4255) // 'function' : no function prototype given: converting '()' to '(void)'
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <windows.h>                            // Sleep(), milliseconds
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
#endif

#include "misc/archive.h"
#include "misc/file.h"
#include "misc/itypes.h"
#include "misc/misc.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768


typedef struct st_func_node
{
  void (*func) (void);
  struct st_func_node *next;
} st_func_node_t;

static st_func_node_t func_list = { NULL, NULL };
static int func_list_locked = 0;


int
misc_digits (unsigned long v)
{
  int ret = 1;

  if ( v >= 100000000 ) { ret += 8; v /= 100000000; }
  if ( v >=     10000 ) { ret += 4; v /=     10000; }
  if ( v >=       100 ) { ret += 2; v /=       100; }
  if ( v >=        10 ) { ret += 1;                 }

  return ret;
}


int
bytes_per_second (time_t start_time, int nbytes)
{
  int curr = (int) difftime (time (0), start_time);

  if (curr < 1)
    curr = 1;                                   // "round up" to at least 1 sec (no division
                                                //  by zero below)
  return nbytes / curr;                         // # bytes/second (average transfer speed)
}


int
misc_percent (int pos, int len)
{
  if (len < 1)
    len = 1;

  return (int) ((((int64_t) 100) * pos) / len);
}


void
wait2 (int nmillis)
{
#ifdef  __MSDOS__
  delay (nmillis);
#elif   defined __unix__ || defined __APPLE__   // Mac OS X actually
  usleep (nmillis * 1000);
#elif   defined __BEOS__
  snooze (nmillis * 1000);
#elif   defined AMIGA
  Delay (nmillis * 1000);
#elif   defined _WIN32
  Sleep (nmillis);
#else
#ifdef  __GNUC__
#warning Please provide a wait2() implementation
#else
#pragma message ("Please provide a wait2() implementation")
#endif
  volatile int n;
  for (n = 0; n < nmillis * 65536; n++)
    ;
#endif
}


void
dumper (FILE *output, const void *buffer, size_t bufferlen, int virtual_start,
        unsigned int flags)
// Do NOT use DUMPER_PRINT in uCON64 code - dbjh
{
#define DUMPER_REPLACER ('.')
  size_t pos;
  char buf[17];
  const unsigned char *p = (const unsigned char *) buffer;

  memset (buf, 0, sizeof (buf));
  for (pos = 0; pos < bufferlen; pos++, p++)
    if (flags & DUMPER_PRINT)
      {
//        fprintf (output, (flags & DUMPER_DEC_COUNT ? "%010d  " : "%08x  "),
//          (int) (pos + virtual_start));
        int c = isprint (*p) ||
#ifdef USE_ANSI_COLOR
                *p == 0x1b || // ESC
#endif
                isspace (*p) ? *p : DUMPER_REPLACER;
        fputc (c, output);
      }
    else if (flags & DUMPER_DUAL)
      {
        if (!(pos & 3))
          fprintf (output, (flags & DUMPER_DEC_COUNT ? "%010d  " : "%08x  "),
            (int) (pos + virtual_start));

        fprintf (output, "%02x  %08d  ",
                         *p,
                         ((*p >> 7) & 1) * 10000000 +
                         ((*p >> 6) & 1) * 1000000 +
                         ((*p >> 5) & 1) * 100000 +
                         ((*p >> 4) & 1) * 10000 +
                         ((*p >> 3) & 1) * 1000 +
                         ((*p >> 2) & 1) * 100 +
                         ((*p >> 1) & 1) * 10 +
                         (*p & 1));

        *(buf + (pos & 3)) = isprint (*p) ? *p : DUMPER_REPLACER;
        if (!((pos + 1) & 3))
          fprintf (output, "%s\n", buf);
      }
    else if (flags & DUMPER_CODE)
      {
        fprintf (output, "0x%02x, ", *p);

        if (!((pos + 1) & 7))
          fprintf (output, (flags & DUMPER_DEC_COUNT ? "// (%d) 0x%x\n" : "// 0x%x (%d)\n"),
            (int) (pos + virtual_start + 1),
            (int) (pos + virtual_start + 1));
      }
    else // if (flags & DUMPER_HEX) // default
      {
        if (!(pos & 15))
          fprintf (output, (flags & DUMPER_DEC_COUNT ? "%08d  " : "%08x  "),
            (int) (pos + virtual_start));

        fprintf (output, (pos + 1) & 3 ? "%02x " : "%02x  ", *p);

        *(buf + (pos & 15)) = isprint (*p) ? *p : DUMPER_REPLACER;
        if (!((pos + 1) & 15))
          fprintf (output, "%s\n", buf);
      }

  if (flags & DUMPER_PRINT)
    return;
  else if (flags & DUMPER_DUAL)
    {
      if (pos & 3)
        {
          *(buf + (pos & 3)) = 0;
          fprintf (output, "%s\n", buf);
        }
    }
  else if (flags & DUMPER_CODE)
    return;
  else // if (flags & DUMPER_HEX) // default
    {
      if (pos & 15)
        {
          *(buf + (pos & 15)) = 0;
          fprintf (output, "%s\n", buf);
        }
    }
}


int
change_mem (char *buf, int bufsize, char *searchstr, int strsize, char wc,
            char esc, char *newstr, int newsize, int offset, ...)
// convenience wrapper for change_mem2()
{
  va_list argptr;
  int i, n_esc = 0, retval;
  st_cm_set_t *sets;

  for (i = 0; i < strsize; i++)
    if (searchstr[i] == esc)
      n_esc++;

  sets = (st_cm_set_t *) malloc (n_esc * sizeof (st_cm_set_t));
  va_start (argptr, offset);
  for (i = 0; i < n_esc; i++)
    {
      sets[i].data = va_arg (argptr, char *);   // get next set of characters
      sets[i].size = va_arg (argptr, int);      // get set size
    }
  va_end (argptr);
  retval = change_mem2 (buf, bufsize, searchstr, strsize, wc, esc, newstr,
                        newsize, offset, sets);
  free (sets);
  return retval;
}


int
change_mem2 (char *buf, int bufsize, char *searchstr, int strsize, char wc,
             char esc, char *newstr, int newsize, int offset, st_cm_set_t *sets)
/*
  Search for all occurrences of string searchstr in buf and replace newsize
  bytes in buf by copying string newstr to the end of the found search string
  in buf plus offset.
  If searchstr contains wildcard characters wc, then n wildcard characters in
  searchstr match any n characters in buf.
  If searchstr contains escape characters esc, sets must point to an array of
  sets. sets must contain as many elements as there are escape characters in
  searchstr. searchstr matches for an escape character if one of the characters
  in sets[i]->data matches.
  Note that searchstr is not necessarily a C string; it may contain one or more
  zero bytes as strsize indicates the length.
  offset is the relative offset from the last character in searchstring and may
  have a negative value.
  The return value is the number of times a match was found.
  This function was written to patch SNES ROM dumps. It does basically the same
  as the old uCON does, with one exception, the line with:
    bufpos -= n_wc;

  As stated in the comment, this causes the search to restart at the first
  wildcard character of the sequence of wildcards that was most recently
  skipped if the current character in buf didn't match the current character
  in searchstr. This makes change_mem() behave a bit more intuitive. For
  example
    char str[] = "f foobar means...";
    change_mem2 (str, strlen (str), "f**bar", 6, '*', '!', "XXXXXXXX", 8, 2, NULL);
  finds and changes "foobar means..." into "foobar XXXXXXXX", while with uCON's
  algorithm it would not (but does the job good enough for patching SNES ROMs).

  One example of using sets:
    char str[] = "fu-bar     is the same as foobar    ";
    st_cm_set_t sets[] = {{"o-", 2}, {"uo", 2}};
    change_mem2 (str, strlen (str), "f!!", 3, '*', '!', "fighter", 7, 1, sets);
  This changes str into "fu-fighter is the same as foofighter".
*/
{
  char *set;
  int bufpos, strpos = 0, pos_1st_esc = -1, setsize, i, n_wc, n_matches = 0,
      setindex = 0;
  const char *overflow_msg =
               "WARNING: The combination of buffer position (%d), offset (%d) and\n"
               "         replacement size (%d) would cause a buffer overflow -- ignoring\n"
               "         match\n";

  for (bufpos = 0; bufpos < bufsize; bufpos++)
    {
      if (strpos == 0 && searchstr[0] != esc && searchstr[0] != wc)
        while (bufpos < bufsize && searchstr[0] != buf[bufpos])
          bufpos++;

      // handle escape character in searchstr
      while (searchstr[strpos] == esc && bufpos < bufsize)
        {
          if (strpos == pos_1st_esc)
            setindex = 0;                       // reset argument pointer
          if (pos_1st_esc == -1)
            pos_1st_esc = strpos;

          set = sets[setindex].data;            // get next set of characters
          setsize = sets[setindex].size;        // get set size
          setindex++;
          i = 0;
          // see if buf[bufpos] matches with any character in current set
          while (i < setsize && buf[bufpos] != set[i])
            i++;
          if (i == setsize)
            break;                              // buf[bufpos] didn't match with any char

          if (strpos == strsize - 1)            // check if we are at the end of searchstr
            {
              if (bufpos + offset >= 0 && bufpos + offset + newsize <= bufsize)
                {
                  memcpy (buf + bufpos + offset, newstr, newsize);
                  n_matches++;
                }
              else
                printf (overflow_msg, bufpos, offset, newsize);
              break;
            }

          strpos++;
          bufpos++;
        }
      if (searchstr[strpos] == esc)
        {
          strpos = 0;
          continue;
        }

      // skip wildcards in searchstr
      n_wc = 0;
      while (searchstr[strpos] == wc && bufpos < bufsize)
        {
          if (strpos == strsize - 1)            // check if at end of searchstr
            {
              if (bufpos + offset >= 0 && bufpos + offset + newsize <= bufsize)
                {
                  memcpy (buf + bufpos + offset, newstr, newsize);
                  n_matches++;
                }
              else
                printf (overflow_msg, bufpos, offset, newsize);
              break;
            }

          strpos++;
          bufpos++;
          n_wc++;
        }
      if (bufpos == bufsize)
        break;
      if (searchstr[strpos] == wc)
        {
          strpos = 0;
          continue;
        }

      if (searchstr[strpos] == esc)
        {
          bufpos--;                             // current char has to be checked, but `for'
          continue;                             //  increments bufpos
        }

      // no escape char, no wildcard -> normal character
      if (searchstr[strpos] == buf[bufpos])
        {
          if (strpos == strsize - 1)            // check if at end of searchstr
            {
              if (bufpos + offset >= 0 && bufpos + offset + newsize <= bufsize)
                {
                  memcpy (buf + bufpos + offset, newstr, newsize);
                  n_matches++;
                }
              else
                printf (overflow_msg, bufpos, offset, newsize);
              strpos = 0;
            }
          else
            strpos++;
        }
      else
        {
          bufpos -= n_wc;                       // scan the most recent wildcards too if
          if (strpos > 0)                       //  the character didn't match
            {
              bufpos--;                         // current char has to be checked, but `for'
              strpos = 0;                       //  increments bufpos
            }
        }
    }

  return n_matches;
}


int
build_cm_patterns (st_cm_pattern_t **patterns, const char *filename, int verbose)
/*
  This function goes a bit over the top what memory allocation technique
  concerns, but at least it's stable.
  Note the important difference between (*patterns)[0].n_sets and
  patterns[0]->n_sets (not especially that member). I (dbjh) am too ashamed to
  tell how long it took me to finally realise that...
*/
{
  char src_name[FILENAME_MAX], line[MAXBUFSIZE], buffer[MAXBUFSIZE],
       *token, *last, *ptr;
  int line_num = 0, n_sets, n_codes = 0, n, currentsize1, requiredsize1,
      currentsize2, requiredsize2, currentsize3, requiredsize3;
  FILE *srcfile;

  strcpy (src_name, filename);
  if (access (src_name, F_OK | R_OK))
    return -1;                                  // NOT an error, it's optional

  if ((srcfile = fopen (src_name, "r")) == NULL) // open in text mode
    {
      fprintf (stderr, "ERROR: Cannot open \"%s\" for reading\n", src_name);
      return -1;
    }

  *patterns = NULL;
  currentsize1 = requiredsize1 = 0;
  while (fgets (line, sizeof line, srcfile) != NULL)
    {
      line_num++;
      n_sets = 0;

      ptr = line + strspn (line, "\t ");
      if (*ptr == '#' || *ptr == '\n' || *ptr == '\r')
        continue;
      if ((ptr = strpbrk (line, "\n\r#")) != NULL) // text after # is comment
        *ptr = 0;

      requiredsize1 += sizeof (st_cm_pattern_t);
      if (requiredsize1 > currentsize1)
        {
          currentsize1 = requiredsize1 + 10 * sizeof (st_cm_pattern_t);
          if ((*patterns = (st_cm_pattern_t *) realloc (*patterns, currentsize1)) == NULL)
            {
              fprintf (stderr, "ERROR: Not enough memory for buffer (%d bytes)\n", currentsize1);
              return -1;
            }
        }

      (*patterns)[n_codes].search = NULL;
      currentsize2 = 0;
      requiredsize2 = 1;                        // for string terminator
      n = 0;
      strcpy (buffer, line);
      token = strtok (buffer, ":");
      token = strtok (token, " ");
//      printf ("token: \"%s\"\n", token);
      last = token;
      // token is never NULL here (yes, tested with empty files and such)
      do
        {
          requiredsize2++;
          if (requiredsize2 > currentsize2)
            {
              currentsize2 = requiredsize2 + 10;
              if (((*patterns)[n_codes].search =
                   (char *) realloc ((*patterns)[n_codes].search, currentsize2)) == NULL)
                {
                  fprintf (stderr, "ERROR: Not enough memory for buffer (%d bytes)\n", currentsize2);
                  free (*patterns);
                  *patterns = NULL;
                  return -1;
                }
            }
          (*patterns)[n_codes].search[n] = (unsigned char) strtol (token, NULL, 16);
          n++;
        }
      while ((token = strtok (NULL, " ")) != NULL);
      (*patterns)[n_codes].search_size = n;     // size in bytes

      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %d is invalid, no wildcard value is specified\n",
                  line_num);
          continue;
        }
      (*patterns)[n_codes].wildcard = (char) strtol (token, NULL, 16);

      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %d is invalid, no escape value is specified\n",
                  line_num);
          continue;
        }
      (*patterns)[n_codes].escape = (char) strtol (token, NULL, 16);

      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %d is invalid, no replacement is specified\n", line_num);
          continue;
        }
      (*patterns)[n_codes].replace = NULL;
      currentsize2 = 0;
      requiredsize2 = 1;                        // for string terminator
      n = 0;
      do
        {
          requiredsize2++;
          if (requiredsize2 > currentsize2)
            {
              currentsize2 = requiredsize2 + 10;
              if (((*patterns)[n_codes].replace =
                   (char *) realloc ((*patterns)[n_codes].replace, currentsize2)) == NULL)
                {
                  fprintf (stderr, "ERROR: Not enough memory for buffer (%d bytes)\n", currentsize2);
                  free ((*patterns)[n_codes].search);
                  free (*patterns);
                  *patterns = NULL;
                  return -1;
                }
            }
          (*patterns)[n_codes].replace[n] = (unsigned char) strtol (token, NULL, 16);
          n++;
        }
      while ((token = strtok (NULL, " ")) != NULL);
      (*patterns)[n_codes].replace_size = n;  // size in bytes

      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      token = strtok (token, " ");
      last = token;
      if (!token)
        {
          printf ("WARNING: Line %d is invalid, no offset is specified\n", line_num);
          continue;
        }
      (*patterns)[n_codes].offset = strtol (token, NULL, 10); // yes, offset is decimal

      if (verbose)
        {
          printf ("line:         %d\n"
                  "searchstring: ",
                  line_num);
          for (n = 0; n < (*patterns)[n_codes].search_size; n++)
            printf ("%02x ", (unsigned char) (*patterns)[n_codes].search[n]);
          printf ("(%d)\n"
                  "wildcard:     %02x\n"
                  "escape:       %02x\n"
                  "replacement:  ",
                  (*patterns)[n_codes].search_size,
                  (unsigned char) (*patterns)[n_codes].wildcard,
                  (unsigned char) (*patterns)[n_codes].escape);
          for (n = 0; n < (*patterns)[n_codes].replace_size; n++)
            printf ("%02x ", (unsigned char) (*patterns)[n_codes].replace[n]);
          printf ("(%d)\n"
                  "offset:       %d\n",
                  (*patterns)[n_codes].replace_size,
                  (*patterns)[n_codes].offset);
        }

      (*patterns)[n_codes].sets = NULL;
      currentsize2 = 0;
      requiredsize2 = 1;                        // for string terminator
      strcpy (buffer, line);
      token = strtok (last, ":");
      token = strtok (NULL, ":");
      last = token;
      while (token)
        {
          requiredsize2 += sizeof (st_cm_set_t);
          if (requiredsize2 > currentsize2)
            {
              currentsize2 = requiredsize2 + 10 * sizeof (st_cm_set_t);
              if (((*patterns)[n_codes].sets =
                   (st_cm_set_t *) realloc ((*patterns)[n_codes].sets, currentsize2)) == NULL)
                {
                  fprintf (stderr, "ERROR: Not enough memory for buffer (%d bytes)\n", currentsize2);
                  free ((*patterns)[n_codes].replace);
                  free ((*patterns)[n_codes].search);
                  free (*patterns);
                  *patterns = NULL;
                  return -1;
                }
            }

          (*patterns)[n_codes].sets[n_sets].data = NULL;
          currentsize3 = 0;
          requiredsize3 = 1;                    // for string terminator
          n = 0;
          token = strtok (token, " ");
          do
            {
              requiredsize3++;
              if (requiredsize3 > currentsize3)
                {
                  currentsize3 = requiredsize3 + 10;
                  if (((*patterns)[n_codes].sets[n_sets].data =
                       (char *) realloc ((*patterns)[n_codes].sets[n_sets].data, currentsize3)) == NULL)
                    {
                      fprintf (stderr, "ERROR: Not enough memory for buffer (%d bytes)\n", currentsize3);
                      free ((*patterns)[n_codes].sets);
                      free ((*patterns)[n_codes].replace);
                      free ((*patterns)[n_codes].search);
                      free (*patterns);
                      *patterns = NULL;
                      return -1;
                    }
                }
              (*patterns)[n_codes].sets[n_sets].data[n] =
                (unsigned char) strtol (token, NULL, 16);
              n++;
            }
          while ((token = strtok (NULL, " ")) != NULL);
          (*patterns)[n_codes].sets[n_sets].size = n;

          if (verbose)
            {
              printf ("set:          ");
              for (n = 0; n < (*patterns)[n_codes].sets[n_sets].size; n++)
                printf ("%02x ", (unsigned char) (*patterns)[n_codes].sets[n_sets].data[n]);
              printf ("(%d)\n", (*patterns)[n_codes].sets[n_sets].size);
            }

          strcpy (buffer, line);
          token = strtok (last, ":");
          token = strtok (NULL, ":");
          last = token;

          n_sets++;
        }
      (*patterns)[n_codes].n_sets = n_sets;

      n_codes++;

      if (verbose)
        fputc ('\n', stdout);
    }
  fclose (srcfile);
  return n_codes;
}


void
cleanup_cm_patterns (st_cm_pattern_t **patterns, int n_patterns)
{
  int n, m;
  for (n = 0; n < n_patterns; n++)
    {
      free ((*patterns)[n].search);
      (*patterns)[n].search = NULL;
      free ((*patterns)[n].replace);
      (*patterns)[n].replace = NULL;
      for (m = 0; m < (*patterns)[n].n_sets; m++)
        {
          free ((*patterns)[n].sets[m].data);
          (*patterns)[n].sets[m].data = NULL;
        }
      free ((*patterns)[n].sets);
      (*patterns)[n].sets = NULL;
    }
  free (*patterns);
  *patterns = NULL;
}


char *
getenv2 (const char *variable)
/*
  getenv() suitable for enviroments w/o HOME, TMP or TEMP variables.
  The caller should copy the returned string to its own memory, because this
  function will overwrite that memory on the next call.
  Note that this function never returns NULL.
*/
{
  char *tmp;
  static char value[MAXBUFSIZE];
#ifdef  __MSDOS__
/*
  On DOS and Windows the environment variables are not stored in a case
  sensitive manner. The run-time system of DJGPP acts as if they are stored in
  upper case. Its getenv() however *is* case sensitive. We fix this by changing
  all characters of the search string (variable) to upper case.

  Note that in Cygwin's Bash environment variables *are* stored in a case
  sensitive manner.
*/
  char tmp2[MAXBUFSIZE];

  strcpy (tmp2, variable);
  variable = strupr (tmp2);                     // DON'T copy the string into variable
#endif                                          //  (variable itself is local)

  *value = 0;

  if (!strcmp (variable, "HOME"))
    {
      if ((tmp = getenv ("UCON64_HOME")) != NULL)
        strcpy (value, tmp);
      else if ((tmp = getenv ("HOME")) != NULL)
        strcpy (value, tmp);
      else if ((tmp = getenv ("USERPROFILE")) != NULL)
        strcpy (value, tmp);
      else if ((tmp = getenv ("HOMEDRIVE")) != NULL)
        {
          strcpy (value, tmp);
          tmp = getenv ("HOMEPATH");
          strcat (value, tmp ? tmp : DIR_SEPARATOR_S);
        }
      else
        /*
          Don't just use C:\\ on DOS, the user might not have write access
          there (Windows NT DOS-box). Besides, it would make uCON64 behave
          differently on DOS than on the other platforms.
          Returning the current directory when none of the above environment
          variables are set can be seen as a feature. A frontend could execute
          uCON64 with an environment without any of the environment variables
          set, so that the directory from where uCON64 starts will be used.
        */
        {
          char c;
          getcwd (value, FILENAME_MAX);
          c = (char) toupper ((int) *value);
          // if current dir is root dir strip problematic ending slash (DJGPP)
          if (c >= 'A' && c <= 'Z' &&
              value[1] == ':' && value[2] == '/' && value[3] == 0)
            value[2] = 0;
        }
    }
  else if ((tmp = getenv (variable)) != NULL)
    strcpy (value, tmp);
  else
    {
      if (!strcmp (variable, "TEMP") || !strcmp (variable, "TMP"))
        {
#if     defined __MSDOS__ || defined __CYGWIN__
          /*
            DJGPP and (yet another) Cygwin quirck
            A trailing backslash is used to check for a directory. Normally
            DJGPP's run-time system is able to handle forward slashes in paths,
            but access() won't differentiate between files and dirs if a
            forward slash is used. Cygwin's run-time system seems to handle
            paths with forward slashes quite different from paths with
            backslashes. This trick seems to work only if a backslash is used.
          */
          if (access ("\\tmp\\", R_OK | W_OK) == 0)
#else
          // trailing file separator to force it to be a directory
          if (access (DIR_SEPARATOR_S "tmp" DIR_SEPARATOR_S, R_OK | W_OK) == 0)
#endif
            strcpy (value, DIR_SEPARATOR_S "tmp");
          else
            getcwd (value, FILENAME_MAX);
        }
    }

#ifdef  __CYGWIN__
  /*
    Under certain circumstances Cygwin's run-time system returns "/" as value
    of HOME while that var has not been set. To specify a root dir a path like
    /cygdrive/<drive letter> or simply a drive letter should be used.
  */
  if (!strcmp (variable, "HOME") && !strcmp (value, "/"))
    getcwd (value, FILENAME_MAX);
#endif

  return value;
}


#if     defined __unix__ && !defined __MSDOS__
int
drop_privileges (void)
{
  uid_t uid;
  gid_t gid;

  uid = getuid ();
  if (setuid (uid) == -1)
    {
      fputs ("ERROR: Could not set uid\n", stderr);
      return 1;
    }
  gid = getgid ();                              // This shouldn't be necessary
  if (setgid (gid) == -1)                       //  if `make install' was used,
    {                                           //  but just in case (root did
      fputs ("ERROR: Could not set gid\n", stderr); //  `chmod +s')
      return 1;
    }

  return 0;
}
#endif


int
register_func (void (*func) (void))
{
  st_func_node_t *new_node;

  if ((new_node = (st_func_node_t *) malloc (sizeof (st_func_node_t))) == NULL)
    return -1;

  new_node->func = func;
  new_node->next = func_list.next;
  func_list.next = new_node;
  return 0;
}


int
unregister_func (void (*func) (void))
{
  st_func_node_t *func_node = &func_list, *prev_node = &func_list;

  while (func_node->next != NULL && func_node->func != func)
    {
      prev_node = func_node;
      func_node = func_node->next;
    }
  if (func_node->func != func)
    return -1;

  if (!func_list_locked)
    {
      prev_node->next = func_node->next;
      free (func_node);
      return 0;
    }
  else
    return -1;
}


void
handle_registered_funcs (void)
{
  st_func_node_t *func_node = &func_list;

  func_list_locked = 1;
  while (func_node->next != NULL)
    {
      func_node = func_node->next;              // first node's func is NULL
      if (func_node->func != NULL)
        func_node->func ();
    }
  func_list_locked = 0;
}


#ifdef  _WIN32
int
truncate (const char *path, off_t size)
{
  int retval;
  HANDLE file = CreateFile (path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return -1;

  SetFilePointer (file, size, 0, FILE_BEGIN);
  retval = SetEndOfFile (file);                 // returns nonzero on success
  CloseHandle (file);

  return retval ? 0 : -1;                       // truncate() returns zero on success
}


int
sync (void)
{
  _commit (fileno (stdout));
  _commit (fileno (stderr));
  fflush (NULL);                                // flushes all streams opened for output
  return 0;
}


#if     defined __MINGW32__ && defined DLL
// Ugly hack in order to fix something in zlib
FILE *
fdopen (int fd, const char *mode)
{
  return _fdopen (fd, mode);
}
#endif


#elif   defined AMIGA                           // _WIN32
int
truncate (const char *path, off_t size)
{
  BPTR fh;
  ULONG newsize;

  if (!(fh = Open (path, MODE_OLDFILE)))
    return -1;

  newsize = SetFileSize (fh, size, OFFSET_BEGINNING);
  Close (fh);

  return newsize == (ULONG) size ? 0 : -1;      // truncate() returns zero on success
}


int
chmod (const char *path, mode_t mode)
{
  if (!SetProtection ((STRPTR) path,
                      ((mode & S_IRUSR ? 0 : FIBF_READ) |
                       (mode & S_IWUSR ? 0 : FIBF_WRITE | FIBF_DELETE) |
                       (mode & S_IXUSR ? 0 : FIBF_EXECUTE) |
                       (mode & S_IRGRP ? FIBF_GRP_READ : 0) |
                       (mode & S_IWGRP ? FIBF_GRP_WRITE | FIBF_GRP_DELETE : 0) |
                       (mode & S_IXGRP ? FIBF_GRP_EXECUTE : 0) |
                       (mode & S_IROTH ? FIBF_OTR_READ : 0) |
                       (mode & S_IWOTH ? FIBF_OTR_WRITE | FIBF_OTR_DELETE : 0) |
                       (mode & S_IXOTH ? FIBF_OTR_EXECUTE : 0))))
    return -1;
  else
    return 0;
}


void
sync (void)
{
}


int
readlink (const char *path, char *buf, int bufsize)
{
  (void) path;                                  // warning remover
  (void) buf;                                   // idem
  (void) bufsize;                               // idem
  // always return -1 as if anything passed to it isn't a soft link
  return -1;
}


// custom _popen() and _pclose(), because the standard ones (named popen() and
//  pclose()) are buggy
FILE *
_popen (const char *path, const char *mode)
{
  int fd;
  BPTR fh;
  long fhflags;
  char *apipe = malloc (strlen (path) + 7);
  if (!apipe)
    return NULL;

  sprintf (apipe, "PIPE:%08lx.%08lx", (ULONG) FindTask (NULL), (ULONG) time (0));

  if (*mode == 'w')
    fhflags = MODE_NEWFILE;
  else
    fhflags = MODE_OLDFILE;

  if (fh = Open (apipe, fhflags))
    {
      switch (SystemTags(path, SYS_Input, Input(), SYS_Output, fh, SYS_Asynch,
                TRUE, SYS_UserShell, TRUE, NP_CloseInput, FALSE, TAG_END))
        {
        case 0:
          return fopen (apipe, mode);
          break;
        case -1:
          Close (fh);
          return 0;
          break;
        default:
          return 0;
          break;
        }
    }
  return 0;
}


int
_pclose (FILE *stream)
{
  if (stream)
    return fclose (stream);
  else
    return -1;
}
#endif                                          // AMIGA
