#include <stdlib.h>
#include <stdint.h> 


uint16_t crc_xmodem_update (uint16_t crc, uint8_t data);
uint16_t do_crc(uint8_t * data,uint16_t size);
uint16_t do_crc_update(uint16_t crc,uint8_t * data,uint16_t size);
