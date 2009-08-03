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
 
#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__


#define SHARED_MEM_SNES_ACK           0xa5
#define SHARED_MEM_SNES_RTS           0x5a


#define SHARED_MEM_CMD_BANK_COUNT     0
#define SHARED_MEM_CMD_BANK_CURRENT   1

#define SHARED_MEM_CMD_UPLOAD_START   3
#define SHARED_MEM_CMD_UPLOAD_END     4
#define SHARED_MEM_CMD_UPLOAD_PROGESS 5
#define SHARED_MEM_CMD_TERMINATE      6


#define SHARED_MEM_LOC_STATE            0x000000
#define SHARED_MEM_LOC_CMD              0x000001
#define SHARED_MEM_LOC_PAYLOAD          0x000002

#define SHARED_IRQ_LOC_LO              0x00fffe
#define SHARED_IRQ_LOC_HI              0x00ffff


#define SHARED_IRQ_HANDLER_LO           0x00
#define SHARED_IRQ_HANDLER_HI           0x10

void shared_memory_put(uint8_t cmd, uint8_t value);

#endif
