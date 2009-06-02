#include "data.h";
#include "pad.h";
#include "event.h";
#include "myEvents.h";
#include "ressource.h";
#include "PPU.h"
#include "debug.h"


#include <stdlib.h>

padStatus pad1;

void initInternalRegisters(void) {
	characterLocation[0] = 0x0000;
	characterLocation[1] = 0x0000;
	characterLocation[2] = 0x0000;
	characterLocation[3] = 0x0000;
	debug_init();
}

void preInit(void) {
	// For testing purpose ... 
	// Insert code here to be executed before register init
}

void main(void) {
	word i,j;
	word crc01;
	word crc02;
	padStatus pad1;
	char line_header[32] = "OK2";
	initInternalRegisters();

	*(byte*) 0x2105 = 0x01;	// MODE 1 value
	*(byte*) 0x212c = 0x01; // Plane 0 (bit one) enable register
	*(byte*) 0x212d = 0x00;	// All subPlane disable
	*(byte*) 0x2100 = 0x0f; // enable background

    debug_enable();
    printfs(0,"FATFS TEST %i",10);
    printfc("Test me\n");
    printfc("Test me %i\n",10);

  
	while(1){
		while(!pad1.start) {
			waitForVBlank();
			pad1 = readPad((byte) 0);
		}
	}	
	while(1);
}

void IRQHandler(void) {
}

void NMIHandler(void) {
	//processEvents();
}
