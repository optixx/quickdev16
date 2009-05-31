
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#include "uart.h"
extern FILE uart_stdout;

int main(void)
{

    uart_init();
    stdout = &uart_stdout;
    printf("Init done\n");
	
    DDRD |= (1 << PD7);
	
	while(1){
	    
	    /*
	    PORTD |= (1 << PD7);
	    PORTD &= ~(1 << PD7);
	    */
	    PORTD ^= (1 << PD7);
	    
	}
	
	return 0 ;
	
}





