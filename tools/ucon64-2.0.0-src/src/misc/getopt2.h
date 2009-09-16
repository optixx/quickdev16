/*
getopt2.h - getopt1() extension

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
#ifndef MISC_GETOPT2_H
#define MISC_GETOPT2_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif
#include "getopt.h"                       // getopt2 needs struct option from getopt1


/*
  Extended getopt1(), usage and workflow handling

  getopt2_usage()       render usage output from st_getopt2_t array
  getopt2_parse_usage() parse usage output into st_getopt2_t array (for dev)
  getopt2_long()        turn st_getopt2_t into struct option for getopt1_long()
  getopt2_long_only()   turn st_getopt2_t into struct option for getopt1_long_only()
  getopt2_short()       turn st_getopt2_t into short options string for getopt1_*()
  getopt2_get_index_by_val() return single st_getopt2_t by val

  OPTION        option marker (default: '-')
  OPTION_S      option marker as string (default: "-")
  OPTION_LONG_S long option marker as string (default: "--")
  OPTARG        optarg separator (default: '=')
  OPTARG_S      optarg separator as string (default: "=")
                <imperative>
                  you will use THESE everywhere and you will NEVER change them
                </imperative>
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
#ifdef  DEBUG
extern void getopt2_parse_usage (const char *usage_output);
#endif
extern int getopt2_long (struct option *long_option, const st_getopt2_t *option, int n);
extern int getopt2_long_only (struct option *long_option, const st_getopt2_t *option, int n);
extern int getopt2_short (char *short_option, const st_getopt2_t *option, int n);
extern const st_getopt2_t *getopt2_get_index_by_val (const st_getopt2_t *option, int val);

#ifdef  __cplusplus
}
#endif
#endif // MISC_GETOPT2_H
