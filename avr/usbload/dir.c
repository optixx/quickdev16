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


#include "dir.h"
#include "debug.h"
#include "sram.h"
#include "config.h"

#include <string.h>


uint16_t positon = 0;

void dir_entry_start(){
    positon = 0;
}

void dir_entry_dump(uint32_t addr, dir_ent_t* ent){
    debug(DEBUG_FAT,"dir_entry_dump: addr=0x%06lx id=%li name=%s size=%li attr=%i\n", addr, ent->id, ent->file_name, 
                    ent->file_size, ent->file_attr);
}


void dir_entry_add(uint32_t id, uint8_t* file_name,uint32_t file_size,uint8_t file_attr){
    uint32_t addr;
    dir_ent_t ent;
    strncpy(ent.file_name,file_name,13);
    ent.id = id;
    ent.file_size = file_size;
    ent.file_attr = file_attr;
    addr = DIR_ENTRY_LOC + (positon << DIR_ENTRY_SIZE_SHIFT );
    sram_bulk_copy(addr, (uint8_t *) &ent, DIR_ENTRY_SIZE );
    dir_entry_dump(addr, &ent);
    positon++;
}

void dir_entry_header(uint16_t position, uint8_t * header){
    uint32_t addr;
    addr = DIR_ENTRY_LOC + ( position << DIR_ENTRY_SIZE_SHIFT ) + DIR_ENTRY_HEADER_OFF;
    sram_bulk_copy(addr, (uint8_t *) header, DIR_ENTRY_HEADER_SIZE);
}



