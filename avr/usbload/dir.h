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


#ifndef __DIR_H__
#define __DIR_H__

#include <stdlib.h>
#include <stdint.h>

#define DIR_ENTRY_LOC           0x010000
#define DIR_ENTRY_SIZE          64
#define DIR_ENTRY_SIZE_SHIFT    6
#define DIR_ENTRY_HEADER_SIZE   44
#define DIR_ENTRY_HEADER_OFF    20

typedef struct {             
    uint16_t    id;                 // 1
    uint8_t     file_name[13];      // 8.3  = 12 + 1 = 12
    uint32_t    file_size;          // 4
    uint8_t     file_attr;          // 1
    uint8_t     snes_header[44];    // 44
} dir_ent_t;                        // 64

void dir_entry_start();
void dir_entry_add_(uint16_t id, uint8_t file_name,uint32_t file_size,uint8_t file_attr);
void dir_entry_header(uint16_t position, uint8_t * header);

#endif
