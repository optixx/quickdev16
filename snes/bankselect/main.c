#include "data.h";
#include "pad.h";
#include "event.h";
#include "myEvents.h";
#include "ressource.h";
#include "PPU.h"
#include "debug.h"
#include "crc.h"


#include <stdlib.h>

padStatus pad1;

void initInternalRegisters(void) {
	characterLocation[0] = 0x0000;
	characterLocation[1] = 0x0000;
	characterLocation[2] = 0x0000;
	characterLocation[3] = 0x0000;
	initDebugMap();
}

void preInit(void) {
	// For testing purpose ... 
	// Insert code here to be executed before register init
}

void main(void) {
	word i,j;
	word crc01;
	word crc02;
    padStatus pad1;       //012345678901234567890123456789012
	char line_header[32] = "BANK ADDR   VAL 123456789ABCDEF";
    char line[32] = "                             ";	
    char space; 
    unsigned long addr; 
	initInternalRegisters();

	*(byte*) 0x2105 = 0x01;	// MODE 1 value
	*(byte*) 0x212c = 0x01; // Plane 0 (bit one) enable register
	*(byte*) 0x212d = 0x00;	// All subPlane disable
	*(byte*) 0x2100 = 0x0f; // enable background

    enablePad();    
    enableDebugScreen();
    writeln(line_header,0);

	while(1){
		addr = 0x018000;
		for(j=1; j<16; j++) {
			int2hex((unsigned long)j,&line[0],4);
			int2hex((unsigned long)addr,&line[5],6);
			int2hex((unsigned long)*(byte*)addr,&line[12],4);
            writeln(line,j+1);
    		addr+= 0x010000;
        	pad1 = readPad((byte) 0);
        		while(!pad1.start) {
    			//waitForVBlank();
    			pad1 = readPad((byte) 0);
    		}
        }
    }
	while(1);
}

void IRQHandler(void) {
}

void NMIHandler(void) {
	//processEvents();
}
