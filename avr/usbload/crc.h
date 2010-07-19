/*
 * =====================================================================================
 *
 * ________        .__        __    ________               ____  ________
 * \_____  \  __ __|__| ____ |  | __\______ \   _______  _/_   |/  _____/
 *  /  / \  \|  |  \  |/ ___\|  |/ / |    |  \_/ __ \  \/ /|   /   __  \
 * /   \_/.  \  |  /  \  \___|    <  |    `   \  ___/\   / |   \  |__\  \
 * \_____\ \_/____/|__|\___  >__|_ \/_______  /\___  >\_/  |___|\_____  /
 *        \__>             \/     \/        \/     \/                 \/
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
uint16_t do_crc(uint8_t * data, uint16_t size);
uint16_t do_crc_update(uint16_t crc, uint8_t * data, uint16_t size);
uint16_t crc_check_memory_range(uint32_t start_addr, uint32_t size,
                                uint8_t * buffer);
uint16_t crc_check_bulk_memory(uint32_t bottom_addr, uint32_t bank_size,
                               uint32_t top_addr);

#endif
