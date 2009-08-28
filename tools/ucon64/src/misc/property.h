/*
property.h - configfile handling

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
#ifndef MISC_PROPERTY_H
#define MISC_PROPERTY_H

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  __cplusplus
extern "C" {
#endif


typedef struct
{
  const char *name;                             // property name
  const char *value;                            // property value
  const char *comment;                          // property comment, # COMMENT
} st_property_t;                                // NAME=VALUE


/*
  get_property()       get value of propname from filename or return value
                         of env with name like propname or return def
  get_property_int()   like get_property() but returns an integer which is 0
                         if the value of propname was 0, [Nn] or [Nn][Oo]
                         and an integer or at least 1 for every other case
  get_property_fname() like get_property() but specifically for filenames,
                         i.e., it runs realpath2() on the filename and fixes
                         the characters if necessary (Cygwin)
  set_property()       set propname with value in filename
  set_property_array() set an array of properties (st_property_t) at once
  DELETE_PROPERTY()    like set_property but when value of propname is NULL
                         the whole property will disappear from filename
*/
extern char *get_property (const char *filename, const char *propname, char *value,
                           const char *def);
extern int get_property_int (const char *filename, const char *propname);
extern char *get_property_fname (const char *filename, const char *propname,
                                 char *buffer, const char *def);
extern int set_property (const char *filename, const char *propname, const char *value, const char *comment);
extern int set_property_array (const char *filename, const st_property_t *prop);
#define DELETE_PROPERTY(a, b) (set_property(a, b, NULL, NULL))

#ifdef  __cplusplus
}
#endif
#endif // MISC_PROPERTY_H
