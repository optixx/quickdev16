/* huffman-encode.c */
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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define DEBUG 0
#define XDEBUG 0

typedef struct{
	uint8_t  depth;
	uint16_t value;
	uint32_t weight;
	void*    left;
	void*    right;
} node_t;

typedef struct{
	uint16_t value;
	unsigned depth;
	void*    encoding;
} item_t;

typedef struct{
	uint8_t initialized;
	item_t* item;
	void*   parent;
	void*   left;
	void*   right;
} node2_t;

uint32_t histogram[256];
node_t*  pool[256+1];
unsigned poolsize;
unsigned item_count;
node_t*  tree;
node_t*  treenodes;
unsigned treeindex=0;
item_t*  itemlist;
unsigned itemindex=0;
node2_t* node2list=NULL;
unsigned node2list_index=0;
item_t*  valueencode[256];
item_t*  eof_encoding;

void reset_histogram(void){
	memset(histogram, 0, 256*sizeof(uint32_t));
}

void build_histogram(char* fname){
	FILE* f;
	int t;
	f = fopen(fname, "r");
	while((t=fgetc(f))!=EOF){
		histogram[(uint8_t)t&0xFF]++;	
	}
	fclose(f);
}


void build_pool(void){
	unsigned i,j;	
	memset(pool, 0, 256*sizeof(node_t*));
	for(i=0,j=0;i<256;++i){
		if(histogram[i]==0)
			continue;
		pool[j] = malloc(sizeof(node_t));
		if(pool[j]==NULL){
			fprintf(stderr,"out of memory error (%d)!\n", __LINE__);
			exit(-1);		
		}
		pool[j]->depth  = 0;
		pool[j]->value  = i;
		pool[j]->left   = NULL;
		pool[j]->right  = NULL;
		pool[j]->weight = histogram[i];
		j++;
	}
	pool[j] = malloc(sizeof(node_t));
	if(pool[j]==NULL){
		fprintf(stderr,"out of memory error (%d)!\n", __LINE__);
		exit(-1);		
	}
	pool[j]->depth  = 0;
	pool[j]->value  = 0xFFFF;
	pool[j]->left   = NULL;
	pool[j]->right  = NULL;
	pool[j]->weight = 1;
	j++;
	poolsize = j;
	item_count = j;
}

void find_lightest2(unsigned* a, unsigned* b, unsigned* depth){
	unsigned ia=0;
	unsigned ib=0;
	uint32_t wa, wb;
	unsigned i;
	wa = wb = 0xFFFFFFFF;
	for(i=0; i<poolsize; ++i){
		if(pool[i]==NULL)
			continue;		
		if(wa>=pool[i]->weight){
			wb = wa;
			ib = ia;
			wa = pool[i]->weight;
			ia = i;		
		}else{
			if(wb>pool[i]->weight){
				wb = pool[i]->weight;
				ib = i;		
			}		
		}
	}
	if(wb == 0xFFFFFFFF || wa == 0xFFFFFFFF){
		fprintf(stderr, "Error while searching!\n");
		exit(-2);
	}
	if(pool[ia]->depth <= pool[ib]->depth){
		*a = ia;
		*b = ib;
		*depth = pool[ib]->depth;
	} else {
		*a = ib;
		*b = ia;
		*depth = pool[ia]->depth;
	}
}

void init_tree(void){
	treenodes = malloc((poolsize-1)*sizeof(node_t));
	if(treenodes==NULL){
		fprintf(stderr,"out of memory error (%d)!\n", __LINE__);
		exit(-1);		
	}
#if XDEBUG	
	printf("treenodes := %p\n", (void*)treenodes);
#endif
}


void update_tree(void){
	if(poolsize<2)
		return;
	unsigned a,b, depth;
	find_lightest2(&a,&b,&depth);
#if XDEBUG
	printf("joining %d and %d\n", a,b);
#endif
	treenodes[treeindex].depth  = depth+1;
	treenodes[treeindex].weight = pool[a]->weight + pool[b]->weight;
	treenodes[treeindex].left   = (pool[a]);
	treenodes[treeindex].right  = (pool[b]);
#if XDEBUG
	printf("  idx = %d\n    self   = %p\n    depth  = %d\n    weight = %d\n"
	       "    left   = %p\n    right  = %p\n",
	       treeindex, (void*)&(treenodes[treeindex]), treenodes[treeindex].depth,
		   treenodes[treeindex].weight, treenodes[treeindex].left, 
		   treenodes[treeindex].right); 
#endif
	pool[a] = &(treenodes[treeindex]);
	pool[b] = pool[poolsize-1];
	pool[poolsize-1] = NULL;
	--poolsize;
	++treeindex;
}

void build_tree(void){
	while(poolsize>1){
		update_tree();
	}
	tree = &(treenodes[treeindex-1]);
}


void free_leaf(node_t* node){
	if(node->depth==0){
		free(node);
	}else{
		free_leaf(node->left);
		free_leaf(node->right);
	}
}

void free_tree(void){
	free_leaf(tree);
	free(treenodes); 
	tree = NULL;
}

void init_itemlist(void){
	itemlist = calloc((item_count),sizeof(item_t));
	if(itemlist==NULL){
		fprintf(stderr,"out of memory error (%d)!\n", __LINE__);
		exit(-1);		
	}	
}

void update_itemlist(node_t* node, unsigned depth){
	if(node->depth==0){
		itemlist[itemindex].value = node->value;
		itemlist[itemindex].depth = depth;
		++itemindex;
	} else {
		update_itemlist(node->left,  depth+1);
		update_itemlist(node->right, depth+1);		
	}
}

void build_itemlist(void){
	update_itemlist(tree, 0);
}

int item_compare_depth(const void* a, const void* b){
	if(((item_t*)a)->value==0xFFFF)
		return  1;
	if(((item_t*)b)->value==0xFFFF)
		return -1;	
	return ((item_t*)a)->depth - ((item_t*)b)->depth;
}

void sort_itemlist(void){
	qsort(itemlist, item_count, sizeof(item_t), item_compare_depth);
}

#define PREFIX_SIZE_B 32

void prefix_increment(uint8_t* prefix){
	uint8_t i;
	for(i=0; i<PREFIX_SIZE_B; ++i){
		prefix[i] += 1;
		if(prefix[i]!=0)
			return;
	}
}

void prefix_shiftleft(uint8_t* prefix){
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

void gen_itemencoding(void){
	uint8_t prefix[PREFIX_SIZE_B];
	memset(prefix, 0, PREFIX_SIZE_B);
	unsigned depth=0;
	unsigned depth_B=0;
	unsigned i,j;	
	for(i=0; i<item_count; ++i){
		if(depth!=itemlist[i].depth){
			for(j=depth; j<itemlist[i].depth; ++j)
				prefix_shiftleft(prefix);
			depth   = itemlist[i].depth;
			depth_B = (depth+7)/8;
		}
		itemlist[i].encoding=malloc(depth_B);
		if(itemlist[i].encoding==NULL){
			fprintf(stderr,"out of memory error (%d)!\n", __LINE__);
			exit(-1);		
		}
		memcpy(itemlist[i].encoding, prefix, depth_B);
		prefix_increment(prefix);			
	}
}

void add_item2(item_t* item){
	int       i;
	uint8_t   t;
	node2_t*  current;
	current = node2list;
	char c;
	c = item->value;
//	printf(" %2.2X (%c) => ", c, (c>32&&c<128)?c:' ');
	for(i=item->depth-1; i>=0; --i){
		t = (((uint8_t*)(item->encoding))[i/8])&(1<<(i%8));
		if(current->initialized==0){
			current->initialized=1;
			current->parent = NULL;
			current->item   = NULL;
			current->left   = NULL;
			current->right  = NULL; 
			node2list_index++;
		}
		if(t==0){
//			putchar('0');
			if(current->left){
				current = current->left;
			} else {
				current->left = &(node2list[node2list_index++]);
				((node2_t*)(current->left))->parent = current;
				current = current->left;
				current->initialized=1;
				current->item=NULL;
			}
		} else {
//			putchar('1');
			if(current->right){
				current = current->right;
			} else {
				current->right = &(node2list[node2list_index++]);
				((node2_t*)(current->right))->parent = current;
				current = current->right;
				current->initialized=1;
				current->item=NULL;
			}
		}
	}
	current->item = item;
	current->left = NULL;
	current->right = NULL;
//	printf("\n");
}

void gen_tree2(void){
	unsigned i;	
	node2list = calloc(2*item_count-1, sizeof(node2_t));
#if XDEBUG
	printf("item_count = %d\n", item_count);
#endif
	if(node2list==NULL){
		fprintf(stderr,"out of memory error (%d)!\n", __LINE__);
		exit(-1);		
	}
	for(i=0;i<item_count;++i){
		add_item2(&(itemlist[i]));
	}
}

void bit_writer(FILE* f, uint8_t bit, uint8_t flush){
	static uint8_t buffer;
	static uint8_t buffer_index=0;
	if(flush){
		for(;buffer_index<8;++buffer_index)
			buffer<<=1;
	}
	if(buffer_index==8){	
		fputc(buffer, f);
		buffer_index=0;
	}
	
	buffer<<=1;
	buffer|=bit?1:0;
	buffer_index++;
}

void encoding_writer(FILE* f, uint8_t* data, uint16_t length){
	int i;
	if(data==NULL)
		bit_writer(f, 0, 1);
	for(i=length-1;i>=0;--i){
		bit_writer(f,data[i/8]&(1<<(i%8)),0);
	} 	
}

void build_valueencode(void){
	unsigned i;
	memset(valueencode, 0, 256*sizeof(void*));
	for(i=0; i<item_count-1; ++i){
		valueencode[itemlist[i].value] = &(itemlist[i]);
	}
	eof_encoding = &(itemlist[i]);
}

void write_tree(FILE* f){
	unsigned i,j;
	unsigned last=0;
	unsigned last_depth=1;
	fputc(0xc0, f);
	fputc(0xde + (item_count>>8), f);	
	fputc(item_count,f);
	for(i=0; i<item_count; ++i){
		if(itemlist[i].depth!=last_depth){
			if(i-last>=255){
				fputc(255, f);
				fputc(i-last-255, f);
			} else {
				fputc(i-last, f);
			}
			for(j=last; j<i; ++j){
				fputc(itemlist[j].value, f);
			}
			for(j=last_depth+1; j<itemlist[i].depth; ++j)
				fputc(0, f);
			last=i;
			last_depth=itemlist[i].depth;
			
		}
	}
	if(i-last>=255){
		fputc(255, f);
		fputc(i-last-255, f);
	} else {
		fputc(i-last, f);
	}
	for(j=last; j<i; ++j){
		fputc(itemlist[j].value, f);
	}
}

void compress_file(FILE* fin, FILE* fout){
	int t;
	while((t=fgetc(fin))!=EOF){
		t = (uint8_t)t;
		if(valueencode[t]==NULL){
			fprintf(stderr,"no encoding for %2.2X (%c) found!\n",t,(t>32&&t<128)?t:' ');
			exit(-3);
		}
		encoding_writer(fout, valueencode[t]->encoding, valueencode[t]->depth);		
	};		
	encoding_writer(fout, eof_encoding->encoding, eof_encoding->depth);
	encoding_writer(fout, NULL, 0);
}


void compress_huffman(char * filename_in, char * filename_out){
	FILE* fin; 
	FILE* fout;
	reset_histogram();
	build_histogram(filename_in);
	build_pool();
	init_tree();
	build_tree();
	init_itemlist();
	build_itemlist();
	free_tree();
	sort_itemlist();	
	gen_itemencoding();			
	gen_tree2();
	build_valueencode();
	fin  = fopen(filename_in, "r");
	fout = fopen(filename_out, "w");
	write_tree(fout);
	compress_file(fin, fout);
	fclose(fin);
	fclose(fout);
}

int main(int argc, char** argv){
	char filename_out[strlen(argv[1])+20];
	strcpy(filename_out, argv[1]);
	strcat(filename_out, ".hfm");
    compress_huffman(argv[1],filename_out);
	return 0;
}


