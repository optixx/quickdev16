#include <stdlib.h>
#include <stdint.h> 


uint16_t crc_xmodem_update (uint16_t crc, uint8_t data);
uint16_t do_crc(uint8_t * data,uint16_t size);
uint16_t do_crc_update(uint16_t crc,uint8_t * data,uint16_t size);
void crc_check_memory(uint32_t top_addr,uint8_t *buffer);
uint16_t crc_check_memory_range(uint32_t start_addr, uint32_t size,uint8_t *buffer);
