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


void wait(void)
{
    enablePad();
    pad1 = readPad((byte) 0);
    while (!pad1.A) {
        waitForVBlank();
        pad1 = readPad((byte) 0);
    }
}


void main(void) {
	word x,y;
    unsigned long addr; 
    
    initInternalRegisters();
    *(byte *) 0x2105 = 0x01;    // MODE 1 value
    *(byte *) 0x212c = 0x01;    // Plane 0 (bit one) enable register
    *(byte *) 0x212d = 0x00;    // All subPlane disable
    *(byte *) 0x2100 = 0x0f;    // enable background

    debug_enable();
    
    
    addr = 0x21400;
    x = 0;
    y = 0;
    //printfs(26,"PRESS A");
    //wait();
    //clears();
    for (addr = 0x2100 ; addr < 0x21C0; addr+=8){
    	waitForVBlank();
        printfs(y,"%lX: %02X %02X %02X %02X %02X %02X %02X %02X",addr,
        *(byte *) addr,*(byte *) (addr +1),*(byte *) (addr+2),*(byte *) (addr+3),
        *(byte *) (addr+4),*(byte *) (addr +5),*(byte *) (addr+6),*(byte *) (addr+7));
        y++;
    }
    while(1);
    printfs(26,"PRESS A");
    //wait();
    clears();
    y = 0;
    for (addr = 0x4100 ; addr < 0x41C0; addr+=8){
    	waitForVBlank();
        printfs(y,"%lX: %02X %02X %02X %02X %02X %02X %02X %02X",addr,
        *(byte *) addr,*(byte *) (addr +1),*(byte *) (addr+2),*(byte *) (addr+3),
        *(byte *) (addr+4),*(byte *) (addr +5),*(byte *) (addr+6),*(byte *) (addr+7));
        y++;
    }
    printfs(26,"PRESS A");
    wait();
    clears();
    
    

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
