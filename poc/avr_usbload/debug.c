#include <stdlib.h>
#include <stdint.h> 

#include "debug.h"
#include "uart.h"


extern FILE uart_stdout;

void dump_packet(uint32_t addr,uint32_t len,uint8_t *packet){
	uint16_t i,j;
	uint16_t sum = 0;
	uint8_t clear=0;
	
	for (i=0;i<len;i+=16) {
		
		sum = 0;
		for (j=0;j<16;j++) {
			sum +=packet[i+j];
		}
		if (!sum){
			clear=1;
			continue;
		}
		if (clear){
			printf("*\n");
			clear = 0;
		}	
		printf("%08lx:", addr + i);
		for (j=0;j<16;j++) {
			printf(" %02x", packet[i+j]);
		}
		printf(" |");
		for (j=0;j<16;j++) {
			if (packet[i+j]>=33 && packet[i+j]<=126 )
				printf("%c", packet[i+j]);
			else
				printf(".");
		}
		printf("|\n");
	}
}
