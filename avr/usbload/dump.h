#ifndef __DUMP_H__
#define __DUMP_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

void dump_memory(uint32_t bottom_addr, uint32_t top_addr);

void dump_packet(uint32_t addr,uint32_t len,uint8_t *packet);

#endif 
        