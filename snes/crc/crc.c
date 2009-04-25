#include "data.h"


word crc_update (byte *data, word size)
{
	word i;
    word j;
	word crc=0;
	for (j=0; j<size; j++){
		crc = crc ^ ((word)data[j]<< 8);
		for (i=0; i<8; i++){
        	if (crc & 0x8000)
            	crc = (crc << 1) ^ 0x1021;
        	else
            	crc <<= 1;
    	}
	}
	return crc;
}


word crc_update_mem (word addr, word size)
{
	word i;
    word j;
	word crc=0;
    word v;
	for (j=0; j<size; j++){
        v = addr;
		crc = crc ^ ((word)v<< 8);
		for (i=0; i<8; i++){
        	if (crc & 0x8000)
            	crc = (crc << 1) ^ 0x1021;
        	else
            	crc <<= 1;
    	}
	}
	return crc;
}