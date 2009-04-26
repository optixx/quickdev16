#include "data.h"

byte tileMapLocation[4];
word characterLocation[4];

void waitForVBlank(void) {
	byte Status;
	do {
		Status = *(byte*)0x4210;
	} while (!(Status & 0x80));
}

void setTileMapLocation(word vramDst, byte screenProp, byte bgNumber) {
	tileMapLocation[bgNumber] = ((vramDst >> 8) & 0xfc) | ( screenProp & 0x03 );
	*(byte*) (0x2107+bgNumber) = tileMapLocation[bgNumber];
}

void restoreTileMapLocation(byte bgNumber) {
	*(byte*) (0x2107+bgNumber) = tileMapLocation[bgNumber];
}

void setCharacterLocation(word vramDst, byte bgNumber) {
	characterLocation[bgNumber] = vramDst;
	if(bgNumber < 2) {
		*(byte*) 0x210b = (characterLocation[1] >> 8 & 0xf0 ) + (characterLocation[0] >> 12);
	} else {
		*(byte*) 0x210c = (characterLocation[3] >> 8 & 0xf0 ) + (characterLocation[2] >> 12);
	}
}

void restoreCharacterLocation(byte bgNumber) {
	setCharacterLocation(characterLocation[bgNumber], bgNumber);
}

void VRAMByteWrite(byte value, word vramDst) {
	*(byte*)0x2115 = 0x80;	
	*(word*)0x2116 = vramDst;

	*(byte*)0x2118 = value;
}

void VRAMLoad(word src, word vramDst, word size) {
	// set address in VRam for read or write ($2116) + block size transfer ($2115)
	*(byte*)0x2115 = 0x80;
	*(word*)0x2116 = vramDst;	

	*(word*)0x4300 = 0x1801;	// set DMA control register (1 word inc) 
								// and destination ($21xx xx -> 0x18)
	*(word*)0x4302 = src;		// DMA channel x source address offset 
								// (low $4302 and high $4303 optimisation)
	*(byte*)0x4304 = 0x01; 		// DMA channel x source address bank
	*(word*)0x4305 = size;		// DMA channel x transfer size 
								// (low $4305 and high $4306 optimisation)

	// Turn on DMA transfer for this channel
	waitForVBlank();
	*(byte*)0x2100 = 0x80;
	*(byte*)0x420b = 0x01;
	*(byte*)0x2100 = 0x00;
}

void CGRAMLoad(word src, byte cgramDst, word size) {
	
	// set address in VRam for read or write + block size
	*(byte*)0x2121 = cgramDst;

	*(word*)0x4300 = 0x2200;	// set DMA control register (1 byte inc) 
								// and destination ($21xx xx -> 022)
	*(word*)0x4302 = src;		// DMA channel x source address offset
								// (low $4302 and high $4303 optimisation)
	*(byte*)0x4304 = 0x01; 		// DMA channel x source address bank
	*(word*)0x4305 = size;		// DMA channel x transfer size
								// (low $4305 and high $4306 optimisation)

	// Turn on DMA transfer for this channel
	waitForVBlank();
	*(byte*)0x2100 = 0x80;
	*(byte*)0x420b = 0x01;
	*(byte*)0x2100 = 0x00;
}