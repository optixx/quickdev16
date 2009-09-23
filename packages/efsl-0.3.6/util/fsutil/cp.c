/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : cp.c                                                             *
* Release  : 0.3 - devel                                                      *
* Description : This file is part of the linux utilities, main purpose        *
*               is to test and debug the library                              *
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

#include <stdio.h>
#include <stdlib.h>
#include "efs.h"
#include "linuxfile.h"
#include "include/init.h"

int main(int argc, char** argv)
{
	euint8 data_left=1,*buffer;
	euint32 bcount;

	/* SOURCE */
	FILE *in  = NULL;
	File *ein = NULL;
	efsl_storage *storage_in       = NULL;
	efsl_fs *filesystem_in = NULL;
	
	/* DESTINATION */
	FILE *out  = NULL;
	File *eout = NULL;
	efsl_storage *storage_out       = NULL;
	efsl_fs *filesystem_out = NULL;
	
	if(Init_EFSL(&storage_in,&storage_out,&filesystem_in,&filesystem_out,&ein,&eout,&in,&out,argc,argv)){
		printf("Error in init\n");
		return(-1);
	}
	
	buffer = malloc(4096);
	
	while(data_left){
		/* READ */
		if(ein==NULL){
			bcount=fread(buffer,1,4096,in);
		}else{
			bcount=file_read(ein,4096,buffer);
		}
		if(bcount!=4096)data_left=0;

		/* WRITE */
		if(eout==NULL){
			if(fwrite(buffer,bcount,1,out)!=1){
				printf("Error writing to local file\n");
				return(-1);
			}
		}else{
			if(file_write(eout,bcount,buffer)!=bcount){
				printf("Error writing EFSL file\n");
				return(-1);
			}
		}
	}
	
	if(in!=NULL)fclose(in);
	if(out!=NULL)fclose(out);
	if(ein!=NULL)file_fclose(ein);
	if(eout!=NULL)file_fclose(eout);
	if(filesystem_in!=NULL)fs_umount(&(filesystem_in->filesystem));
	if(filesystem_out!=NULL)fs_umount(&(filesystem_out->filesystem));
	
	return(0);
}
