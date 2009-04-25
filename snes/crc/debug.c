#include "data.h"
#include "pad.h"
#include "PPU.h"
#include "ressource.h"
#include "crc.h"

word debugMap[0x400];

void initDebugMap(void) {
	word i;
	for(i=0; i<0x400; i++) {
		debugMap[i] = 0x00;
	}
}


void int2hex(word i, char *buf)
{
    word a;
    for (a = 0; a < 4; ++a) {
        buf[a] = (i >> (4 * (2 * 2 - 1 - a))) & 0xf;
        if (buf[a] < 10)
            buf[a] += '0';
        else
            buf[a] += 'A' - 10;
    }
    buf[a] = 0;
}

void debug(void) {
	word i,j;
	word crc01;
	word crc02;
	padStatus pad1;
	char line_header[32] = "BANK CRC  ADDR 123456789ABCDEF";
    char line[32] = "                             ";	
	char test_buffer[] = "da";
	void *pointer;
	
	VRAMLoad((word) debugFont_pic, 0x5000, 2048);
	CGRAMLoad((word) debugFont_pal, (byte) 0x00, (word) 16);
	VRAMLoad((word) debugMap, 0x4000, 0x0800);
	setTileMapLocation(0x4000, (byte) 0x00, (byte) 0);
	setCharacterLocation(0x5000, (byte) 0);
	*(byte*) 0x2100 = 0x0f; // enable background

    j=0;
    waitForVBlank();
	for(i=0; i<32; i++) {
		waitForVBlank();
		VRAMByteWrite((byte) (line_header[i]-32), (word) (0x4000+i+(j*0x20)));
	}

	while(1){
		pointer = (void*)0x8000;
		crc02 = crc_update(test_buffer,2);
		//crc01 = crc_update(pointer,255);
		for(j=0; j<8; j++) {
    		crc01 = crc_update(pointer,0x8000);
			int2hex(j,&line[0]);
			int2hex(crc01,&line[5]);
			int2hex((word)pointer,&line[10]);
            waitForVBlank();
			for(i=0; i<32; i++) {
				waitForVBlank();
				VRAMByteWrite((byte) (line[i]-32), (word) (0x4000+i+((j+1)*0x20)));
			}
            //pointer+=0x010000;
		}
		while(!pad1.start) {
			waitForVBlank();
			pad1 = readPad((byte) 0);
		}
	}	
	
}

#pragma section CODE=BANK3,offset $3:0000 
char far dummy[128];

