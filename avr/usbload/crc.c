#include <stdlib.h>
#include <stdint.h>

#include "crc.h"
#include "uart.h"

extern FILE uart_stdout;

uint16_t crc_xmodem_update(uint16_t crc, uint8_t data)
{
    int i;
    crc = crc ^ ((uint16_t) data << 8);
    for (i = 0; i < 8; i++) {
        if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
    }
    return crc;
}

uint16_t do_crc(uint8_t * data, uint16_t size)
{
    uint16_t crc = 0;
    uint16_t i;
    for (i = 0; i < size; i++) {
        crc = crc_xmodem_update(crc, data[i]);
    }
    return crc;
}


uint16_t do_crc_update(uint16_t crc, uint8_t * data, uint16_t size)
{
    uint16_t i;
    for (i = 0; i < size; i++)
        crc = crc_xmodem_update(crc, data[i]);
    return crc;
}
