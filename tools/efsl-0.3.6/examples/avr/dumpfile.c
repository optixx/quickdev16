/*****************************************************************************/
#include <efs.h>
#include <sd.h>
#include <atmega_spi.h>
/*****************************************************************************/

/*****************************************************************************/
void hang(void);
/*****************************************************************************/

void main(void)
{
	efsl_storage_conf storage_conf;
	efsl_fs_conf fs_conf;
	
	efsl_storage storage;
	efsl_fs fs;
	File file_r;

	atmegaSpiInterface spi_interface;
	SdSpiProtocol sd_protocol;
	
	char buf[512];
	unsigned short i,e;
		
	debug_init();

	DBG((TXT("\nHello :-)\n")));
	
	/* Init */
	spi_interface.pinSelect=0x01;
	
	sd_protocol.spiHwInterface=&spi_interface;
	sd_protocol.spiHwInit=(void *)atmega_spi_init;
	sd_protocol.spiSendByte=(void *)atmega_spi_send;

    storage_conf.hwObject=&sd_protocol;
    storage_conf.if_init_fptr=(void *)sd_Init;
    storage_conf.if_read_fptr=(void *)sd_readSector;
    storage_conf.if_write_fptr=(void *)sd_writeSector;
    storage_conf.if_ioctl_fptr=(void *)sd_ioctl;
    storage_conf.ioman_bufmem=0;

	fs_conf.no_partitions=0;
	fs_conf.storage=&storage;
	
	DBG((TXT("Let's go...\n")));
	
	if(efsl_initStorage(&storage,&storage_conf)){
		DBG((TXT("Error initializing storage: %d")));
		hang();
	}
	
	if(efsl_initFs(&fs,&fs_conf)){
		DBG((TXT("Unable to mount fs")));
		hang();
	}

	if(file_fopen(&file_r,&fs.filesystem,"orig.txt",'r')!=0){
		DBG((TXT("Could not open file\n")));
		hang();
	}
	DBG((TXT("File opened")));

	while((e=file_read(&file_r,512,buf))){
		for(i=0;i<e;i++){
			DBG((TXT("%c"),buf[i]));
		}
	}

	DBG((TXT("Done :-)")));
	
	hang();
}
/*****************************************************************************/

void hang(void)
{
	while((1))
		_NOP();
}
/*****************************************************************************/

