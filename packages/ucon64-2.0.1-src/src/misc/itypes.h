/*
itypes.h - integer types

Copyright (c) 1999 - 2004 NoisyB
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
#ifndef MISC_ITYPES_H
#define MISC_ITYPES_H

#ifdef  HAVE_CONFIG_H
#include "config.h"                             // HAVE_INTTYPES_H
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef  HAVE_INTTYPES_H
#include <inttypes.h>
#else                                           // __MSDOS__, _WIN32 (VC++)
#ifndef OWN_INTTYPES
#define OWN_INTTYPES                            // signal that these are defined
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short int uint16_t;
typedef signed short int int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
#ifndef _WIN32
typedef unsigned long long int uint64_t;
typedef signed long long int int64_t;
#else
typedef unsigned __int64 uint64_t;
typedef signed __int64 int64_t;
#endif
#endif                                          // OWN_INTTYPES
#endif

#ifdef  __cplusplus
}
#endif
#endif // MISC_ITYPES_H
