/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : efs.c                                                            *
* Release  : 0.3 - devel                                                      *
* Description : This should become the wrapper around efs. It will contain    *
*               functions like efs_init etc.                                  *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2006 Lennart Yseboodt *
*                                                    (c)2006 Michael De Nil   *
\*****************************************************************************/

#ifndef __EFS_H__
#define __EFS_H__

/*****************************************************************************/
#include "types.h"
#include "config.h"
#include "extract.h"
#include "sextract.h"
#include "interface.h"
#include "disc.h"
#include "partition.h"

/* WARNING !!!!
 * These includes DO NOT BELONG HERE,
 * remove them when the VFS layer is implemented !!!!
 */
#include "fs.h"
#include "file.h"
#include "time.h"
#include "ui.h" 

/*****************************************************************************/
#define LINUX_FILE_CONFIG	0x00
#define AVR_SD_CONFIG		0x01
/*****************************************************************************/


struct _efsl_storage {
	Interface interface;
	IOManager ioman;
	Disc disc;
};
typedef struct _efsl_storage efsl_storage;

struct _efsl_storage_conf {
	
	void *hwObject;
	esint8 (*if_init_fptr)(void*);
	esint8 (*if_read_fptr)(void*,euint32,euint8*);
	esint8 (*if_write_fptr)(void*,euint32,euint8*);
	esint8 (*if_ioctl_fptr)(void*,euint16,void*);
	void *ioman_bufmem;
};
typedef	struct _efsl_storage_conf efsl_storage_conf;

struct _efsl_fs {
	efsl_storage *storage;
	Partition partition;
	FileSystem filesystem;
};
typedef struct _efsl_fs efsl_fs;

struct _efsl_fs_conf {
	efsl_storage *storage;
	euint8 no_partitions;
};
typedef struct _efsl_fs_conf efsl_fs_conf;


/*****************************************************************************/
esint8 efsl_initStorage(efsl_storage *efsl_storage,efsl_storage_conf *config);
esint8 efsl_initFs(efsl_fs *efsl_filesystem,efsl_fs_conf *config);



#endif

