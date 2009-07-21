/*
 * =====================================================================================
 *
 *            .d8888b  88888b.   .d88b.  .d8888b  888d888 8888b.  88888b.d88b.
 *            88K      888 "88b d8P  Y8b 88K      888P"      "88b 888 "888 "88b
 *            "Y8888b. 888  888 88888888 "Y8888b. 888    .d888888 888  888  888
 *                 X88 888  888 Y8b.          X88 888    888  888 888  888  888
 *             88888P' 888  888  "Y8888   88888P' 888    "Y888888 888  888  888
 *
 *                                  www.optixx.org
 *
 *
 *        Version:  1.0
 *        Created:  07/21/2009 03:32:16 PM
 *         Author:  david@optixx.org
 *
 * =====================================================================================
 */




#ifndef __CRC_H__
#define __CRC_H__

#include <stdlib.h>
#include <stdint.h> 


uint16_t crc_xmodem_update(uint16_t crc, uint8_t data);
uint16_t do_crc(uint8_t * data,uint16_t size);
uint16_t do_crc_update(uint16_t crc,uint8_t * data,uint16_t size);
void crc_check_memory(uint32_t bottom_addr,uint32_t top_addr,uint32_t bank_size,uint8_t *buffer);
uint16_t crc_check_memory_range(uint32_t start_addr, uint32_t size,uint8_t *buffer);
uint16_t crc_check_bulk_memory(uint32_t bottom_addr, uint32_t bank_size,uint32_t top_addr);

#endif
