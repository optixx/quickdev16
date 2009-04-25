#include "data.h";
#include "pad.h";
#include "event.h";
#include "myEvents.h";
#include "ressource.h";
#include "PPU.h"
#include "debug.h"

#include <stdlib.h>

padStatus pad1;

event *currentScrollEvent;
word scrollValue;

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

	initInternalRegisters();

	// Screen map data @ VRAM location $1000
	setTileMapLocation(0x1000, (byte) 0x00, (byte) 0);
	//*(byte*) 0x2107 = 0x10;
	
	// Plane 0 Tile graphics @ $2000
	setCharacterLocation(0x2000, (byte) 0);
	//*(byte*) 0x210b = 0x02;

	//VRAMLoad((word) title_pic, 0x2000, 0x1BE0);
	//VRAMLoad((word) title_map, 0x1000, 0x0800);
	CGRAMLoad((word) title_pal, (byte) 0x00, (word) 256);
	
	// TODO sitwch to mode 0 for trying
	*(byte*) 0x2105 = 0x01;	// MODE 1 value

	*(byte*) 0x212c = 0x01; // Plane 0 (bit one) enable register
	*(byte*) 0x212d = 0x00;	// All subPlane disable

	*(byte*) 0x2100 = 0x0f; // enable background

	currentScrollEvent = NULL;
	scrollValue = 0;

	//initEvents();
	//enablePad();
	debug();
	//addEvent(&NMIReadPad, 1);
	
	// Loop forever
	while(1);
}

void IRQHandler(void) {
}

void NMIHandler(void) {
	//processEvents();
}
