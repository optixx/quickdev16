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


#define SHARED_MEM_SWITCH_IRQ               0
#define SHARED_MEM_SWITCH_DELAY             20

#define SHARED_MEM_TX_SNES_ACK              0xa5
#define SHARED_MEM_TX_SNES_RTS              0x5a

#define SHARED_MEM_TX_CMD_BANK_COUNT        0x00
#define SHARED_MEM_TX_CMD_BANK_CURRENT      0x01

#define SHARED_MEM_TX_CMD_UPLOAD_START      0x03
#define SHARED_MEM_TX_CMD_UPLOAD_END        0x04
#define SHARED_MEM_TX_CMD_UPLOAD_PROGESS    0x05
#define SHARED_MEM_TX_CMD_TERMINATE         0x06

#define SHARED_MEM_TX_LOC_STATE             0x000000
#define SHARED_MEM_TX_LOC_SIZE              0x000100
#define SHARED_MEM_TX_LOC_CMD               0x000001
#define SHARED_MEM_TX_LOC_PAYLOAD           0x000002

#define SHARED_MEM_RX_AVR_ACK               0xa5
#define SHARED_MEM_RX_AVR_RTS               0x5a

#define SHARED_MEM_RX_CMD_PRINFT            0x00
#define SHARED_MEM_RX_CMD_FILESEL           0x01

#define SHARED_MEM_RX_LOC_STATE             0x001000
#define SHARED_MEM_RX_LOC_SIZE              0x000400
#define SHARED_MEM_RX_LOC_CMD               0x001001
#define SHARED_MEM_RX_LOC_LEN               0x001002
#define SHARED_MEM_RX_LOC_PAYLOAD           0x001003

#define SHARED_IRQ_LOC_LO                   0x00fffe
#define SHARED_IRQ_LOC_HI                   0x00ffff

/* Use COP IRQ LOC for hooked IRQ handler */
#define SHARED_IRQ_HANDLER_LO               0x0ffe4
#define SHARED_IRQ_HANDLER_HI               0x0ffe5



uint8_t shared_memory_scratchpad_region_save_helper(uint32_t addr);
void shared_memory_scratchpad_region_tx_save();
void shared_memory_scratchpad_region_tx_restore();
void shared_memory_scratchpad_region_rx_save();
void shared_memory_scratchpad_region_rx_restore();
void shared_memory_write(uint8_t cmd, uint8_t value);
int shared_memory_read(uint8_t *cmd, uint8_t *len,uint8_t *buffer);

#endif
