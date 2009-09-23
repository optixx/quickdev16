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


/*****************************************************************************/
#include "efs.h"
/*****************************************************************************/

/* This stuff must be rewritten and causes compile errors while the datastructures are in flux */

#if 0
esint8 efsl_initStorage(efsl_storage *efsl_storage,efsl_storage_conf *config)
{
	if(if_init(&(efsl_storage->interface),
				config->hwObject,
				config->if_init_fptr,
				config->if_read_fptr,
				config->if_write_fptr,
				config->if_ioctl_fptr)
	){
		return(-1);
	}
	
	if(ioman_init(&(efsl_storage->ioman),
		&(efsl_storage->interface),
		config->ioman_bufmem)
	){
			return(-1);
	}
	
	disc_initDisc(&(efsl_storage->disc),&(efsl_storage->ioman));
	
	return(0);
}
/*****************************************************************************/

esint8 efsl_initFs(efsl_fs *efsl_filesystem,efsl_fs_conf *config)
{
	efsl_filesystem->storage=config->storage;
	
	if(config->no_partitions){
		efsl_filesystem->storage->disc.partitions[0].type=0x0B;
		efsl_filesystem->storage->disc.partitions[0].LBA_begin=0;
		efsl_filesystem->storage->disc.partitions[0].numSectors=
			efsl_filesystem->storage->interface.sectorCount;
	}
	
	part_initPartition(
		&(efsl_filesystem->partition),
		&(efsl_filesystem->storage->disc)
	);
	
	if(fs_initFs(&(efsl_filesystem->filesystem),&(efsl_filesystem->partition))){
		return(-1);
	}
	
	return(0);
}
/*****************************************************************************/
#endif

