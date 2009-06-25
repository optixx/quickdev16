#ifndef DUMP_H
#define DUMP_H

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

void dump_packet(uint32_t addr,uint32_t len,uint8_t *packet);
void dump_memoryt(uint32_t bottom_addr, uint32_t top_addr);

#endif 
        