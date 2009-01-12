 
#include <avr/io.h>  
#include <util/delay.h>
#include <avr/eeprom.h> 

#include <stdio.h>

#include "uart.h"


#define debug(x, fmt) printf("%s:%u: %s=" fmt, __FILE__, __LINE__, #x, x)

extern FILE uart_stdout;

uint16_t ee_data01 __attribute__((section(".eeprom"))) = 0x01;
uint16_t ee_data02 __attribute__((section(".eeprom"))) = 0x02;
uint16_t ee_data03 __attribute__((section(".eeprom"))) = 0x03;
uint16_t ee_data04 __attribute__((section(".eeprom"))) = 0x04;


#define DATAPORT    PORTA
#define ADDRPORTLO  PORTA
#define ADDRPORTHI  PORTC
#define CRTLPORT    PORTD


#define LATCH_LE_PIN    2
#define LATCH_OE_PIN    3
#define SRAM_OE_PIN     4
#define SRAM_CE_PIN     5
#define SRAM_WE_PIN     6


// LE high -> forward imput 
// LE low  -> latch input
#define LATCH_LEHI      CRTLPORT != _BV(LATCH_LE_PIN);     
#define LATCH_LELO      CRTLPORT &=~_BV(LATCH_LE_PIN);
// OE high -> normal logic level
// OE low  -> high impendance
#define LATCH_OEHI      CRTLPORT != _BV(LATCH_OE_PIN);
#define LATCH_OELO      CRTLPORT &=~_BV(LATCH_OE_PIN);
// OE high output disable
// OE low output enable
#define SRAM_OEHI       CRTLPORT != _BV(SRAM_OE_PIN);
#define SRAM_OELO       CRTLPORT &=~_BV(SRAM_OE_PIN);
// CE high chip disable
// CE low chip enable
#define SRAM_CEHI       CRTLPORT != _BV(SRAM_CE_PIN);
#define SRAM_CELO       CRTLPORT &=~_BV(SRAM_CE_PIN);
// WE high write disable
// WE low write enable
#define SRAM_WEHI       CRTLPORT != _BV(SRAM_WE_PIN);
#define SRAM_WELO       CRTLPORT &=~_BV(SRAM_WE_PIN);




int sram_write(uint16_t addr, uint8_t data)
{

    uint8_t addr_lo = addr &  8;
    uint8_t addr_hi = addr >> 8;
    
    SRAM_OEHI;
    SRAM_CELO;
    SRAM_WELO;

    LATCH_OEHI; 
    LATCH_LEHI
    DATAPORT = data;
    LATCH_LELO

    ADDRPORTLO = addr_lo;
    ADDRPORTHI = addr_hi;

    SRAM_CEHI;
    
    SRAM_CELO;

    return 0;
}


int sram_read(uint16_t addr, uint8_t * data)
{

    uint8_t addr_lo = addr &  8;
    uint8_t addr_hi = addr >> 8;
    
    SRAM_OELO;
    SRAM_CELO;
    SRAM_WEHI;

    
    LATCH_OEHI;
    LATCH_LELO;

    ADDRPORTLO = addr_lo;
    ADDRPORTHI = addr_hi;

    SRAM_CEHI;
    
    SRAM_CELO;

    LATCH_LEHI;
    *data = DATAPORT;

    SRAM_OEHI;
    return 0;
}



int main (void) {            // (2)
 
    DDRB  = 0xff;             // (3)
    PORTB = 0xff;             // (4)
    uint8_t i = 0; 
    uint8_t j = 7; 
    uart_init();
    stdout = &uart_stdout;
    
    while(1) {                // (5a)
        PORTB  |=  (1<< j);
        j++;
        if ( j == 8 ) j = 0; 
        PORTB  &= ~(1 << j );  // Toggle PB0 z.B. angeschlossene LED
        /* 
           Die maximale Zeit pro Funktionsaufruf ist begrenzt auf 
           262.14 ms / F_CPU in MHz (im Beispiel: 
           262.1 / 3.6864 = max. 71 ms) 
           16 * 62.5ms (+ Zeit für Schleife) = ca. eine Sekunde warten
        */
        debug(j,"%x \n"); 
        for (i=1; i<=70; i++)         
            _delay_ms(15);    
    }
    return 0;                 // (6)
}

