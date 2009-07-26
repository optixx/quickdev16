/* avr-huffman-decode.h */
/*
    This file is part of the AVR-Huffman.
    Copyright (C) 2009  Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
