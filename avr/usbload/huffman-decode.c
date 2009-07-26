/* avr-huffman-decode.c */
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

#include "huffman-decode.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef DEBUG
  #undef DEBUG
#endif

#define DEBUG 1

#if DEBUG
  #include <avr/pgmspace.h>
#endif

#define V_NODE (-2)
#define V_EOF  (-1)

#define PREFIX_SIZE_B 32 

#define ALLOC_ERROR {}

#undef BLOCK_ALLOC 1

typedef struct {
	int16_t value;
	void*   left;
	void*   right;
} node_t;

#if HUFFMAN_USE_ADDR_16
void huffman_dec_init(huffman_dec_ctx_t* ctx, uint16_t(*rb_func)(uint16_t)){
#else
void huffman_dec_init(huffman_dec_ctx_t* ctx, uint16_t(*rb_func)(uint32_t)){
#endif
	ctx->tree = NULL;
	ctx->addr = 0;
	ctx->read_byte = rb_func;
	ctx->rbuffer_index = 8;
}

#if HUFFMAN_USE_ADDR_16
void huffman_dec_set_addr(huffman_dec_ctx_t* ctx, uint16_t addr){
#else
void huffman_dec_set_addr(huffman_dec_ctx_t* ctx, uint32_t addr){
#endif
	ctx->addr = addr;
}

static inline void prefix_increment(uint8_t* prefix){
	uint8_t i;
	for(i=0; i<PREFIX_SIZE_B; ++i){
		prefix[i] += 1;
		if(prefix[i]!=0)
			return;
	}
}

static inline void prefix_shiftleft(uint8_t* prefix){
	uint8_t i;
	uint8_t c[2]={0,0};
	uint8_t ci=0;	
	for(i=0; i<PREFIX_SIZE_B; ++i){
		c[ci] = (prefix[i])>>7;				
		prefix[i]<<=1;
		ci ^= 1;
		prefix[i]|=c[ci];
	}
}

static inline void set_last_to_eof(node_t* start){
	node_t* current = start;
	while(current->value==V_NODE){
		current=current->right;
	}
	current->value=V_EOF;
}

#if DEBUG
void print_tree(node_t* node){
	if(node->value==V_NODE){
        printf("\n%p --> node->left=%p node->right=%p",node,node->left, node->right);
		print_tree(node->left);
		print_tree(node->right);
	}else{
        printf("\n%p => %i",node,node->value);
	}
}
#endif

uint8_t build_tree(huffman_dec_ctx_t* ctx){
	uint16_t treesize;
	uint16_t treeindex=1;
	int8_t i,t;
	if(ctx->read_byte(ctx->addr++)!=0xC0)
		return 1;
	if(((treesize=ctx->read_byte(ctx->addr++))&0xFE)!=0xDE)
		return 1;
	treesize = (treesize&1)<<8;
	treesize += ctx->read_byte(ctx->addr++);
	if(treesize>0x1ff)
		return 2;
#if BLOCK_ALLOC	
	ctx->tree = calloc(2*treesize-1, sizeof(node_t));
#else
	ctx->tree = calloc(1, sizeof(node_t));
#endif
	((node_t*)(ctx->tree))->value = V_NODE;
	uint16_t depth=0;
	uint16_t count=0;
	uint16_t  v;
	uint8_t prefix[PREFIX_SIZE_B];
	uint8_t cdepth=0;	
	node_t* current=ctx->tree;
	current->value = V_NODE;
	memset(prefix, 0, PREFIX_SIZE_B);
	do{
		while(count==0){
			depth++;
			count= ctx->read_byte(ctx->addr++);
			if(count==255)
				count += ctx->read_byte(ctx->addr++);
		}
		v = ctx->read_byte(ctx->addr++);
		if(v>0xff)
			return 3;
		--count;
		for(;cdepth<depth;++cdepth){
			prefix_shiftleft(prefix);
		}
#if DEBUG
		printf("\n value %x => ",v);
#endif
		current=ctx->tree;
		for(i=depth-1; i>=0; --i){
			t=(prefix[i/8])&(1<<(i%8));
			if(t==0){
#if DEBUG
				printf("0");
#endif
				if(current->left==NULL){
#if BLOCK_ALLOC
					current->left=&(((node_t*)(ctx->tree))[treeindex++]);
#else
					current->left=calloc(1, sizeof(node_t));
#endif
					((node_t*)(current->left))->value = V_NODE;
				}
				current = current->left;
			} else {
#if DEBUG
				printf("1");
#endif
				if(current->right==NULL){
#if BLOCK_ALLOC
					current->right=&(((node_t*)(ctx->tree))[treeindex++]);
#else
					current->right=calloc(1, sizeof(node_t));
#endif
					((node_t*)(current->right))->value=V_NODE;
				}
				current = current->right;
			}
		}
#if !BLOCK_ALLOC	
		if(current==NULL)
			ALLOC_ERROR
#endif
		current->value=v;
		prefix_increment(prefix);
	}while(!(prefix[depth/8]&(1<<(depth%8))));		
#if DEBUG
	print_tree(ctx->tree);
#endif 
	set_last_to_eof(ctx->tree);
	return 0;
}

void free_tree(node_t* node){
#if !BLOCK_ALLOC
	if(node->value==V_NODE){
		free_tree(node->left);
		free_tree(node->right);
	}
#endif	
	free(node);
	
}

static uint8_t read_bit(huffman_dec_ctx_t* ctx){
	uint16_t x;
	uint8_t t;
	if(ctx->rbuffer_index==8){
		x=ctx->read_byte(ctx->addr);
		ctx->addr++;
		if(t>0xff)
			return 0xFF;
		ctx->rbuffer = (uint8_t)x;
		ctx->rbuffer_index=0;
	}
	t=(ctx->rbuffer)>>7;
	ctx->rbuffer<<=1;
	ctx->rbuffer_index++;
	return t;
}

uint16_t huffman_dec_byte(huffman_dec_ctx_t* ctx){
	node_t* current=ctx->tree;
	uint8_t t;
	if(current==NULL){
#if DEBUG		
		printf("\nbuild tree");
#endif
		t=build_tree(ctx);
		if(t!=0){
#if DEBUG
			printf("\n!!! building tree failed !!!\r\n");
#endif
			return 0xFFFF;
		}
#if DEBUG
		printf("\ntree build successful");
#endif
		current=ctx->tree;
	}
	while(current->value==V_NODE){
		t=read_bit(ctx);
		if(t==0xFF)
			goto eof_detected;
		if(t==0){
			current=current->left;
		} else {
			current=current->right;
		}
	}
	if(current->value!=V_EOF){
		return current->value;
	}
	eof_detected:
		free_tree(ctx->tree);	
		ctx->tree = NULL;
		return 0xFFFF;
}
