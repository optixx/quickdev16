
#include <avr/io.h>

#include "uart.h"


//***********************Funktionen**********************************
void uputc(unsigned char c){		//zeichen senden				

  while(!(UCSRA & (1<<UDRE))){;}	//buffer voll, solange warten bis platz f�r zeichen !!	
  UDR=c;									//zeichen schreiben		
}	

//*******************************************************************
void uputs (unsigned char *s){		//string senden
	
  while(*s) uputc(*s++);				//sendet zeichenkette, bis ende.. '\0'    
  
}


// *******************************************************************
unsigned char ugetc(void){			//zeichen holen				

  while (!(UCSRA & (1<<RXC))) {;}	// warten bis Zeichen verfuegbar
        
  return UDR;  		                // Zeichen aus UDR an Aufrufer zurueckgeben
}
/*

// ******************************************************************* 
void ugets(char* Buffer, unsigned char MaxLen){
  unsigned char NextChar;
  unsigned char StringLen = 0;
 
  NextChar = ugetc();         		// Warte auf und empfange das nächste Zeichen  
                                
  while( NextChar != '\n' && StringLen < MaxLen - 1 ) {		//string ende oder puffer voll
    *Buffer++ = NextChar;
    StringLen++;
    NextChar = ugetc();
  }
   
  *Buffer = '\0';					//string abschluss
}
*/


//*******************************************************************
void uinit (void){					//init usart Tx, 8n1 ,UDRE interupt enable

  UCSRC |= (3<<UCSZ0);    		// URSEL = 1 dann wird UCSRC benutzt sonst UBRRH ; UCSZ0=data register- 8bit; USBS stop bit 1 	
  //Baudrate (high und low byte)
  UBRRH = (unsigned char)(UBRR_VAL>>8);
  UBRRL = (unsigned char)UBRR_VAL;
  UCSRB |= (1<<TXEN)|(1<<RXEN);  	// UART TX,RX einschalten,	data register empty interrupt enable,
}

