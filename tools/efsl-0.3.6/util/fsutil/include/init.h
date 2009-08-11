/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename : init.h                                                           *
* Description : This file contains the functions to initialise the library on *
*               linux and other POSIX systems, usage is in the fsutils        *
*                                                                             *
* This library is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU Lesser General Public                  *
* License as published by the Free Software Foundation; either                *
* version 2.1 of the License, or (at your option) any later version.          *
*                                                                             *
* This library is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU           *
* Lesser General Public License for more details.                             *
*                                                                             *
*                                                    (c)2004 Lennart Yseboodt *
*                                                    (c)2004 Michael De Nil   *
\*****************************************************************************/

#ifndef __FSUTIL_INIT_H_
#define __FSUTIL_INIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/string.h>
#include <stddef.h>
#include "config.h"
#include "efs.h"
#include "linuxfile.h"

#define IFTYPE_LOCAL 1
#define IFTYPE_LFI   2
#define IFTYPE_NMI   3

struct _local_options{
	euint8* filename;
};
typedef struct _local_options local_options;

struct _lfi_options{
	euint8* imagename;
	euint8* filename;
};
typedef struct _lfi_options lfi_options;

struct _EFSL_cmd_options{
	euint8 type;
	/*euint8* op_filename;*/
	local_options local_opt;
	lfi_options lfi_opt;
};
typedef struct _EFSL_cmd_options EFSL_cmd_options;

void option_init(EFSL_cmd_options *opt);

euint8 Init_EFSL(efsl_storage    **s_src ,efsl_storage    **s_dst,
                 efsl_fs **f_src ,efsl_fs **f_dst,
                 File            **ef_src,File            **ef_dst,
                 FILE            **lf_src,FILE            **lf_dst,
                 int argc, char **argv);

euint8 parse_cmd(EFSL_cmd_options *opt,char *cmd);

#endif
