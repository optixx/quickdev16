#include "data.h"


word crc_update(char far * data, word size)
{
    word i;
    word j;
    word crc = 0;
    for (j = 0; j < size; j++) {
        crc = crc ^ ((word) data[j] << 8);
        for (i = 0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}


word crc_update_mem(unsigned long addr, word size)
{
    word i;
    word j;
    word crc = 0;
    for (j = 0; j < size; j++) {
        crc = crc ^ ((word) * (byte *) (addr + j) << 8);
        for (i = 0; i < 8; i++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}
