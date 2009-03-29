#include <stdio.h>
#include <unistd.h>
#include <usb.h>
#include <ftdi.h>

/*
ftdi_open_results[0] = "all fine";
ftdi_open_results[1] = "usb_find_busses() failed";
ftdi_open_results[2] = "usb_find_devices() failed";
ftdi_open_results[3] = "usb device not found";
ftdi_open_results[4] = "unable to open device";
ftdi_open_results[5] = "unable to claim device";
ftdi_open_results[6] = "reset failed";
ftdi_open_results[7] = "set baudrate failed";
*/

int put(struct ftdi_context *ftdic,unsigned char *buf){
	int f;
	//printf("Put %02x\n",buf[0]);
	f = ftdi_write_data(ftdic, buf, 1);
    if(f < 0) {
     	fprintf(stderr,"write failed for 0x%x, error %d\n",buf[0],f);
		return 1;
	}
}

#define CLK 0
#define RST 1
#define DELAY 0
#define ROMSIZE 0x10000

void dump_packet(unsigned int addr,unsigned int len,unsigned char  *packet){
	int i,j;
	int sum =0;
	for (i=0;i<len;i+=16) {
		sum = 0;
		for (j=0;j<16;j++) {
			sum +=packet[i+j];
		}
		if (!sum){
			//printf(".");
			continue;
		}
		printf("%08lx:", addr + i);
		for (j=0;j<16;j++) {
			printf(" %02x", packet[i+j]);
		}
		printf(" |");
		for (j=0;j<16;j++) {
			if (packet[i+j]>=33 && packet[i+j]<=126 )
				printf("%c", packet[i+j]);
			else
				printf(".");
		}
		printf("|\n");
	}
}


int counter_init(struct ftdi_context *ftdic){
 	unsigned  char buf[1];
	printf("Init\n");
	buf[0] = 0;
	
	/* clk hi */
	buf[0] = 1 << CLK;
	put(ftdic,buf);
	usleep(DELAY);
	
	
	buf[0] = 1 << CLK | 1 <<  RST ; 
	put(ftdic,buf);
	usleep(DELAY);
	
	buf[0] = 1 << CLK;
	put(ftdic,buf);
	usleep(DELAY);
	
}

int counter_clock(struct ftdi_context *ftdic){
 	unsigned  char buf[1];
	//printf("Start clock\n");
	buf[0] = 0; 
	put(ftdic,buf);
	usleep(DELAY);
	buf[0] = 1 << CLK ;
	put(ftdic,buf);
	//printf("End clock\n");
	//usleep(DELAY);
	return 0;
}

unsigned char data_read(struct ftdi_context *ftdic){
	int f;
	unsigned  char buf;
	f = ftdi_read_pins(ftdic,&buf);
    if(f < 0) {
     	fprintf(stderr,"read failed, error %d\n",f);
		return 0;
	}
	return buf;
}

int main(int argc, char **argv)
{
	struct ftdi_context ftdica,ftdicb;
	int f,i,r;

  	ftdi_init(&ftdica);
  	ftdi_init(&ftdicb);
	
	// Open A
	r = ftdi_set_interface(&ftdica, INTERFACE_A);
	printf("A: set interface A: %d\n",r);
 	f = ftdi_usb_open(&ftdica, 0x0403, 0x6010); 
	if(f < 0 && f != -5) {
    	fprintf(stderr, "A: unable to open ftdi device: %d\n",f);
    	exit(-1);
	}
	printf("A: ftdi open succeeded: %d\n",f);
	r= ftdi_enable_bitbang(&ftdica, 0x00);
	printf("A: enabling bitbang mode: %d\n",r);

	// Open B
	r = ftdi_set_interface(&ftdicb, INTERFACE_B);
	printf("B: set interface B: %d\n",r);
	f = ftdi_usb_open(&ftdicb, 0x0403, 0x6010); 
  	if(f < 0 && f != -5) {
    	fprintf(stderr, "B: unable to open ftdi device: %d\n",f);
    	exit(-1);
  	}
  	printf("B: ftdi open succeeded: %d\n",f);
  	r= ftdi_enable_bitbang(&ftdicb, 0xFF);
  	printf("B: enabling bitbang mode: %d\n",r);

	counter_init(&ftdicb);
	unsigned int addr = 0;
	
	for ( addr = 0; addr<=0xfffff; addr+=1){
		counter_clock(&ftdicb);
		//byte = data_read(&ftdica);
		if (addr%0xff==0)
			printf("0x%08x:\n",addr);
	} 
	exit(0);
	
	
	
	unsigned char byte;
	unsigned char *buffer;
	buffer = (unsigned char*)malloc(ROMSIZE);
	if (NULL == buffer){
    	fprintf(stderr, "Malloc failed",f);
    	exit(-1);
	}
	for ( addr = 0; addr<0x1000; addr+=1){
		counter_clock(&ftdicb);
		byte = data_read(&ftdica);
		printf("0x%08x: %x\n",addr,byte);
		buffer[addr] = byte;
	} 
	dump_packet(0x000,ROMSIZE,buffer);


 
	ftdi_disable_bitbang(&ftdica);
  	ftdi_usb_close(&ftdica);
  	ftdi_deinit(&ftdica);

  	ftdi_disable_bitbang(&ftdicb);
  	ftdi_usb_close(&ftdicb);
  	ftdi_deinit(&ftdicb);

}
