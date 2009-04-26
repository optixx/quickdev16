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

void writeln(char *buffer,word y){
    char i;
    waitForVBlank();
	for(i=0; i<32; i++) {
		waitForVBlank();
		VRAMByteWrite((byte) (buffer[i]-32), (word) (0x4000+i+(y*0x20)));
	}
}

void enableDebugScreen(void){
	VRAMLoad((word) debugFont_pic, 0x5000, 2048);
	CGRAMLoad((word) debugFont_pal, (byte) 0x00, (word) 16);
	VRAMLoad((word) debugMap, 0x4000, 0x0800);
	setTileMapLocation(0x4000, (byte) 0x00, (byte) 0);
	setCharacterLocation(0x5000, (byte) 0);
	*(byte*) 0x2100 = 0x0f; // enable background
}