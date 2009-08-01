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
 *
 * =====================================================================================
 */

#ifndef AVR_HUFFMAN_DECODE_H_
#define AVR_HUFFMAN_DECODE_H_

#include <stdint.h>

#define HUFFMAN_USE_ADDR_16 1

typedef struct {
	void* tree;
	uint8_t  rbuffer;
	uint8_t  rbuffer_index;
#if HUFFMAN_USE_ADDR_16
	uint16_t(*read_byte)(uint16_t addr);
	uint16_t addr;
#else
	uint16_t(*read_byte)(uint32_t addr);
	uint32_t addr;
#endif
} huffman_dec_ctx_t;

#if HUFFMAN_USE_ADDR_16
void huffman_dec_init(huffman_dec_ctx_t* ctx, uint16_t(*rb_func)(uint16_t));
void huffman_dec_set_addr(huffman_dec_ctx_t* ctx,uint16_t addr);
#else
void huffman_dec_init(huffman_dec_ctx_t* ctx, uint16_t(*rb_func)(uint32_t));
void huffman_dec_set_addr(huffman_dec_ctx_t* ctx,uint32_t addr);
#endif


uint16_t huffman_dec_byte(huffman_dec_ctx_t* ctx);

#endif /* AVR_HUFFMAN_DECODE_H_ */
