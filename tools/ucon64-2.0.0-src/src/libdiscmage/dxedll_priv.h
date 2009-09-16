/*
dxedll_priv.h - DXE support (code/data private to DXE)

Copyright (c) 2002 - 2003 dbjh


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef DXEDLL_PRIV_H
#define DXEDLL_PRIV_H

#include "dxedll_pub.h"

#ifdef __cplusplus
extern "C" {
#endif

extern st_symbol_t import_export;

// zlib functions
#if 0
#define gzopen import_export.gzopen
#define gzclose import_export.gzclose
#define gzwrite import_export.gzwrite
#define gzgets import_export.gzgets
#define gzeof import_export.gzeof
#define gzseek import_export.gzseek
#define gzputc import_export.gzputc
#define gzread import_export.gzread
#define gzgetc import_export.gzgetc
#define gzrewind import_export.gzrewind
#define gztell import_export.gztell

// unzip functions
#define unzOpen import_export.unzOpen
#define unzOpenCurrentFile import_export.unzOpenCurrentFile
#define unzGoToFirstFile import_export.unzGoToFirstFile
#define unzClose import_export.unzClose
#define unzGetGlobalInfo import_export.unzGetGlobalInfo
#define unzGoToNextFile import_export.unzGoToNextFile
#define unzCloseCurrentFile import_export.unzCloseCurrentFile
#define unzeof import_export.unzeof
#define unzReadCurrentFile import_export.unzReadCurrentFile
#define unztell import_export.unztell
#define unzGetCurrentFileInfo import_export.unzGetCurrentFileInfo
#endif

// variables
#define __dj_stdin import_export.__dj_stdin
#define __dj_stdout import_export.__dj_stdout
#define __dj_stderr import_export.__dj_stderr
#define __dj_ctype_flags import_export.__dj_ctype_flags
#define __dj_ctype_tolower import_export.__dj_ctype_tolower
#define __dj_ctype_toupper import_export.__dj_ctype_toupper
//#define errno import_export.errno

#ifdef __cplusplus
}
#endif

#endif // DXEDLL_PRIV_H
