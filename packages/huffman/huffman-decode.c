/* huffman-decode.c */
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

#define V_NODE (-2)
#define V_EOF  (-1)

#define PREFIX_SIZE_B 32 

#define ALLOC_ERROR {fprintf(stderr,"failed to alloc memory in %s @ %d !\n",__FILE__, __LINE__); exit(-1);}

#define DEBUG 0

typedef struct {
	int16_t value;
	void*   left;
	void*   right;
} node_t;

node_t* tree=NULL;
uint16_t tree_index;

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

uint8_t append_tree(int16_t value, uint16_t depth){
	static uint8_t prefix[PREFIX_SIZE_B];
	static uint8_t cdepth=0;	
	node_t* current=tree;
	int8_t i,t;
	for(;cdepth<depth;++cdepth){
		prefix_shiftleft(prefix);
	}
	for(i=depth-1; i>=0; --i){
		t=(prefix[i/8])&(1<<(i%8));
		if(t==0){
			if(current->left==NULL){
			//	current->left=calloc(1, sizeof(node_t));
				current->left=&(tree[tree_index++]);
				((node_t*)(current->left))->value = V_NODE;
			}
			current = current->left;
		} else {
			if(current->right==NULL){
			//	current->right=calloc(1, sizeof(node_t));
				current->right=&(tree[tree_index++]);
				((node_t*)(current->right))->value = V_NODE;
			}
			current = current->right;
		}
	}
	if(current==NULL)
		ALLOC_ERROR
	current->value=value;
	prefix_increment(prefix);
	return prefix[depth/8]&(1<<(depth%8));
}

void set_last_to_eof(void){
	node_t* current=tree;
	while(current->value==V_NODE)
		current=current->right;
	current->value=V_EOF;
}

void build_tree(FILE* f){
	uint16_t treesize;
	uint16_t x1, x2;
	x1 =fgetc(f);
	x2 =fgetc(f);
	if(x1!=0xc0 || (x2&0xFE)!=0xde){
		fprintf(stderr,"No magic values found!\n");
	}
	treesize= fgetc(f);
	if(treesize>0xff){
		fprintf(stderr,"You are trying to uncompress an empty file!\n");
		exit(-3);
	}
	treesize = 2*(((x2&1)<<8)+treesize)-1;
	//printf("Using treesize=%i with size of %i\n", treesize, sizeof(node_t));
	tree = calloc(treesize, sizeof(node_t));
	if(tree==NULL)
		ALLOC_ERROR
	tree->value = V_NODE;
	tree_index=1;
	uint16_t depth=0;
	uint16_t count=0;
	uint8_t  v;
	do{
		while(count==0){
			depth++;
			count= fgetc(f);
			if(count==255)
				count += fgetc(f);
		}
		v = fgetc(f);
		--count;
	}while(!append_tree(v, depth));		
	set_last_to_eof();
}


void free_tree(node_t* node){
	free(node);
}

FILE* outfile;
FILE* infile;

void write_char(uint8_t v){
	fputc(v, outfile);
}

uint8_t read_bit(void){
	static uint8_t buffer;
	static uint8_t index=0;
	uint8_t t;
	if(index==0){
		buffer = fgetc(infile);
		if(feof(infile))
			return 0xFF;
	}
	t=buffer>>7;
	buffer<<=1;
	index = (index+1)%8;
	return t;
}

uint16_t decompress_byte(void){
	node_t* current=tree;
	uint8_t t;
	while(current->value==V_NODE){
		t=read_bit();
		if(t==0xFF)
			return EOF;
		if(t==0){
			current=current->left;
		} else {
			current=current->right;
		}
		if(current==NULL){
			fprintf(stderr, "Tree damaged!\n");
			exit(-4);
		}
	}
	return current->value;
}

void decompress(void){
	uint16_t t;
	for(;;){
		t=decompress_byte();
		if(t==(uint16_t)EOF)
			return;
		write_char(t);	
	}
}

int main(int argc, char** argv){
	int i;
	FILE* fin;
	FILE* fg;
//	outfile=stdout;
	for(i=1;i<argc;++i){
		fin = fopen(argv[i], "r");
		fg  = fopen("graphout.dot", "w");
		outfile = fopen("decompress.out", "w");
		puts("building tree ...");
		build_tree(fin);
		fclose(fg);
		infile=fin;
		puts("decompression ...");
		decompress();
		fclose(fin);
		fclose(outfile);
		free_tree(tree);
	}
	return 0;
}







