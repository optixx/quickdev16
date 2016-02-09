/*
getopt2.h - getopt1() extension

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
#ifndef MISC_GETOPT2_H
#define MISC_GETOPT2_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef  __CYGWIN__
#include "misc/getopt.h"                        // getopt2 needs struct option from getopt1
#else
#include <getopt.h>                             // Cygwin's unistd.h unconditionally includes getopt.h
#endif

#ifdef  __cplusplus
extern "C" {
#endif

/*
  Extended getopt1(), usage and workflow handling

  getopt2_usage()       render usage output from st_getopt2_t array
  getopt2_long()        turn st_getopt2_t into struct option for getopt1_long()
  getopt2_long_only()   turn st_getopt2_t into struct option for getopt1_long_only()
  getopt2_short()       turn st_getopt2_t into short options string for getopt1_*()
  getopt2_get_index_by_val() return single st_getopt2_t by val

                some useful defines:
  OPTION        option marker (default: '-')
  OPTION_S      option marker as string (default: "-")
  OPTION_LONG_S long option marker as string (default: "--")
  OPTARG        optarg separator (default: '=')
  OPTARG_S      optarg separator as string (default: "=")
                <imperative>
                  you will use THESE everywhere and you will NEVER change them
                </imperative>

  getopt2_file()        runs callback_func with the realpath() of file/dir as string
                        flags:
  0                           pass all files/dirs with their realpath()
  GETOPT2_FILE_FILES_ONLY     pass only files with their realpath()
  GETOPT2_FILE_RECURSIVE      pass all files/dirs with their realpath()'s recursively
  GETOPT2_FILE_RECURSIVE_ONCE like GETOPT2_FILE_RECURSIVE, but only one level deep
  (GETOPT2_FILE_FILES_ONLY|GETOPT2_FILE_RECURSIVE)
                           pass only files with their realpath()'s recursively

  callback_func()       getopt2_file() expects the callback_func to return the following
                          values:
                          0 == OK, 1 == skip the rest/break, -1 == failure/break

  Debugging and Development

  getopt2_sanity_check() check the whole st_getopt2_t array for dupes/errors/etc...
  getopt2_sanity_check_output() called by getopt2_sanity_check()

  getopt2_parse_usage() parse usage output into st_getopt2_t array
  getopt2_usage_code()  turn a st_getopt2_t array into C code
*/
#define OPTION '-'
#define OPTION_S "-"
#define OPTION_LONG_S "--"
#define OPTARG '='
#define OPTARG_S "="

typedef struct
{
  const char *name;           // see getopt()
  int has_arg;                // see getopt()
  int *flag;                  // see getopt()
  int val;                    // see getopt()
  const char *arg_name;       // name of the options arg as it should be
                              // displayed in the --help output
                              // "--name=arg_name" if has_arg == 1
                              // "--name[=arg_name]" if has_arg == 2
  const char *help;           // --help, -h, -? output for the current option
  void *object;               // could be used for workflow objects
} st_getopt2_t;

extern void getopt2_usage (const st_getopt2_t *option);
extern int getopt2_long (struct option *long_option,
                         const st_getopt2_t *option, int n);
extern int getopt2_long_only (struct option *long_option,
                              const st_getopt2_t *option, int n);
extern int getopt2_short (char *short_option, const st_getopt2_t *option,
                          int n);
extern const st_getopt2_t *getopt2_get_index_by_val (const st_getopt2_t *option,
                                                     int val);

#define GETOPT2_FILE_FILES_ONLY     1
#define GETOPT2_FILE_RECURSIVE      (1 << 1)
#define GETOPT2_FILE_RECURSIVE_ONCE (1 << 2)
extern int getopt2_file (int argc, char **argv,
                         int (* callback_func) (const char *), int flags);


#ifdef  DEBUG
extern void getopt2_sanity_check (const st_getopt2_t *option);
extern void getopt2_sanity_check_output (const st_getopt2_t *option);

extern void getopt2_parse_usage (const char *usage_output);
extern void getopt2_usage_code (const st_getopt2_t *usage);
#endif

#ifdef  __cplusplus
}
#endif

#endif // MISC_GETOPT2_H
