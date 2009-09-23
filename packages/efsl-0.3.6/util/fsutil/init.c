/*****************************************************************************\
*                     EFSL - Embedded Filesystems Library                     *
*                     -----------------------------------                     *
*                                                                             *
* Filename : init.c                                                           *
* Release  : 0.3 - devel                                                      *
* Description : This file contains the functions to initialise the library on *
*               linux and other POSIX systems, usage is in the fsutils        *
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

#include "include/init.h"

void option_init(EFSL_cmd_options *opt)
{
	opt->type=0;
	
	opt->local_opt.filename=NULL;
	
	opt->lfi_opt.imagename=NULL;
}

euint8 parse_cmd(EFSL_cmd_options *opt,char *cmd)
{
	euint8 *token,*vtoken;
	euint8 type=0;

	token=strtok(cmd,":");
	if(strcmp(token,"local")==0)type=IFTYPE_LOCAL;
	if(strcmp(token,"lfi")==0)type=IFTYPE_LFI;
	if(strcmp(token,"lnmi")==0)type=IFTYPE_NMI;
	if(!type){
		printf("Invalid type\n");
		return(1);
	}
	
	opt->type=type;
	
	switch(type){
		case IFTYPE_LOCAL:
			while((token=strtok(NULL,"="))!=NULL){
				vtoken=strtok(NULL,",");
				if(strcmp(token,"file")==0){\
					printf("LOCAL::file(%s)\n",vtoken);
					opt->local_opt.filename=malloc(strlen(vtoken)+1);
					strcpy(opt->local_opt.filename,vtoken);
				}else{
					printf("LOCAL::unrecognised option %s (%s)\n",token,vtoken);
				}
			}
			break;
		case IFTYPE_LFI:
			while((token=strtok(NULL,"="))!=NULL){
				vtoken=strtok(NULL,",");
				if(strcmp(token,"image")==0){
					opt->lfi_opt.imagename=malloc(strlen(vtoken)+1);
					strcpy(opt->lfi_opt.imagename,vtoken);
					printf("LFI::image(%s)\n",opt->lfi_opt.imagename);
				}else if(strcmp(token,"file")){
					opt->lfi_opt.filename=malloc(strlen(vtoken)+1);
					strcpy(opt->lfi_opt.filename,vtoken);
					printf("LFI::filename(%s)\n",opt->lfi_opt.filename);
				}else{
					printf("LFI::unrecognised option %s (%s)\n",token,vtoken);
				}
			}
			break;
		case IFTYPE_NMI:
			while((token=strtok(NULL,"="))!=NULL){
				vtoken=strtok(NULL,",");
			}
			break;
	}
	return(0);
}

euint8 Init_EFSL(efsl_storage    **s_src ,efsl_storage    **s_dst,
                 efsl_fs **f_src ,efsl_fs **f_dst,
                 File            **ef_src,File            **ef_dst,
                 FILE            **lf_src,FILE            **lf_dst,
                 int argc, char **argv)
{
	euint8 *arg,do_dest=0;
	EFSL_cmd_options *opt_src,*opt_dst;
	void *rhwi;
	
	efsl_storage_conf *sconf;
	efsl_fs_conf *fconf;

	if(argc<=1){
		printf("No arguments given\n");
		return(1);
	}
	
	sconf=malloc(sizeof(*sconf));
	fconf=malloc(sizeof(*fconf));
	
	/* Parse source argument
	 * from TYPE:opt1=val1,opt2=val2...
	 */
	printf("Source argument : %s\n",argv[1]);
	arg=malloc(strlen(argv[1])+1);
	strcpy(arg,argv[1]);
	opt_src=malloc(sizeof(*opt_src));
	option_init(opt_src);
	parse_cmd(opt_src,arg);
	free(arg);
		
	if(argc>=3){
		printf("Destination argument : %s\n",argv[2]);
		arg=malloc(strlen(argv[2]+1));
		strcpy(arg,argv[2]);
		opt_dst=malloc(sizeof(*opt_dst));
		option_init(opt_dst);
		parse_cmd(opt_dst,arg);
		free(arg);
		do_dest=1;
	}
	
	/* Options parsed, action time */
	/* Start off with source stuff */
	
	switch(opt_src->type){
		case IFTYPE_LOCAL:
				(*lf_src)=fopen(opt_src->local_opt.filename,"r");
				if(*lf_src==NULL){
					printf("Unable to open local file : %s\n",opt_src->local_opt.filename);
					return(1);
				}
			break;
		case IFTYPE_LFI:
				rhwi=malloc(sizeof(linuxFileInterface));
				((linuxFileInterface*)rhwi)->fileName=malloc(strlen(opt_src->lfi_opt.imagename)+1);
				strcpy(((linuxFileInterface*)rhwi)->fileName,opt_src->lfi_opt.imagename);
				(*s_src)=malloc(sizeof(**s_src));
				sconf->hwObject = rhwi;
				sconf->if_init_fptr = lf_init;
				sconf->if_read_fptr = lf_readBuf;
				sconf->if_write_fptr = lf_writeBuf;
				sconf->if_ioctl_fptr = lf_ioctl;
				sconf->ioman_bufmem = malloc(IOMAN_NUMBUFFER*512);
				if(efsl_initStorage( (*s_src) ,sconf)){
					printf("Unable to initialise source storage layer\n");
					return(1);
				}
				(*f_src)=malloc(sizeof(**f_src));
				fconf->storage=(*s_src);
				fconf->no_partitions=1; /* Always use full file as partition */
				if(efsl_initFs( (*f_src) ,fconf)){
					printf("Unable to initialise source filesystem layer\n");
					return(1);
				}
				(*ef_src)=malloc(sizeof(**ef_src));
				if( file_fopen(*ef_src,&(*f_src)->filesystem,opt_src->lfi_opt.filename,'r') ){
					printf("Unable to open sourcefile in image\n");
					return(1);
				}
			break;
	}
	
	if(do_dest){
		
		switch(opt_dst->type){
			case IFTYPE_LOCAL:
					if(opt_src->type == opt_dst->type &&
					(strcmp(opt_src->local_opt.filename,opt_dst->local_opt.filename))==0){
						printf("LOCAL:: Destination file is source file\n");
						return(1);
					}else{
						(*lf_dst)=fopen(opt_dst->local_opt.filename,"w");
						if((*lf_dst)==NULL){
							printf("Unable to open local file : %s\n",opt_dst->local_opt.filename);
							return(1);
						}
					}
				break;
			case IFTYPE_LFI:
					if(opt_src->type == opt_dst->type &&
					(strcmp(opt_src->lfi_opt.imagename,opt_dst->lfi_opt.imagename))==0){
						printf("LFI:: Source/Dest is the same!\n");
						(*s_dst)=(*s_src);
						(*f_dst)=(*f_src);
					}else{
						rhwi=malloc(sizeof(linuxFileInterface));
						((linuxFileInterface*)rhwi)->fileName=malloc(strlen(opt_dst->lfi_opt.imagename)+1);
						strcpy(((linuxFileInterface*)rhwi)->fileName,opt_dst->lfi_opt.imagename);
						(*s_dst)=malloc(sizeof(**s_dst));
						sconf->hwObject = rhwi;
						sconf->if_init_fptr = lf_init;
						sconf->if_read_fptr = lf_readBuf;
						sconf->if_write_fptr = lf_writeBuf;
						sconf->if_ioctl_fptr = lf_ioctl;
						sconf->ioman_bufmem = malloc(IOMAN_NUMBUFFER*512);
						if(efsl_initStorage( (*s_dst) ,sconf)){
							printf("Unable to initialise source storage layer\n");
							return(1);
						}
						(*f_dst)=malloc(sizeof(**f_src));
						fconf->storage=(*s_dst);
						fconf->no_partitions=1; /* Always use full file as partition */
						if(efsl_initFs( (*f_dst) ,fconf)){
							printf("Unable to initialise source filesystem layer\n");
							return(1);
						}
					}
					(*ef_dst)=malloc(sizeof(**ef_dst));
					if( file_fopen(*ef_dst,&(*f_dst)->filesystem,opt_dst->lfi_opt.filename,'w') ){
						printf("Unable to open destinationfile in image\n");
						return(1);
					}
				break;
		}
	}	
	
	return(0);
}

