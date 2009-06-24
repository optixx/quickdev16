#include <stdlib.h>
#include <stdint.h> 
#include <avr/io.h>


#define ROMSIZE      4
#define BLOCKS 		(ROMSIZE << 8)
#define MEMSIZE 	0x80000



#define LED_PORT	            PORTC
#define LED_DIR		            DDRC
#define LED_PIN		            PC7

#define led_on()	            ((LED_PORT &=~ (1 << LED_PIN)),\
                                (LED_DIR &=~ (1 << LED_PIN)))
#define led_off()	            ((LED_PORT &=~ (1 << LED_PIN)),\
                                (LED_DIR |= (1 << LED_PIN)))
/* Port C*/
#define AVR_ADDR_PORT	    	PORTC
#define AVR_ADDR_DIR	    	DDRC
#define AVR_ADDR_LATCH_PORT	    PORTC
#define AVR_ADDR_LATCH_DIR	    DDRC
#define AVR_ADDR_LATCH_PIN	    PC6

#define AVR_ADDR_SCK_PORT	    PORTC
#define AVR_ADDR_SCK_DIR	    DDRC
#define AVR_ADDR_SCK_PIN	    PC5

#define AVR_ADDR_SER_PORT	    PORTC
#define AVR_ADDR_SER_DIR	    DDRC
#define AVR_ADDR_SER_PIN	    PC4

#define AVR_ADDR_LOAD_PORT	    PORTC
#define AVR_ADDR_LOAD_DIR	    DDRC
#define AVR_ADDR_LOAD_PIN	    PC2

#define counter_load()	        ((AVR_ADDR_LOAD_PORT &= ~(1 << AVR_ADDR_LOAD_PIN)),\
                                (AVR_ADDR_LOAD_PORT |= (1 << AVR_ADDR_LOAD_PIN)))

#define AVR_ADDR_DOWN_PORT	    PORTC
#define AVR_ADDR_DOWN_DIR	    DDRC
#define AVR_ADDR_DOWN_PIN	    PC1

#define counter_down()	        ((AVR_ADDR_DOWN_PORT &= ~(1 << AVR_ADDR_DOWN_PIN)),\
                                (AVR_ADDR_DOWN_PORT |= (1 << AVR_ADDR_DOWN_PIN)))

#define AVR_ADDR_UP_PORT	    PORTC
#define AVR_ADDR_UP_DIR	        DDRC
#define AVR_ADDR_UP_PIN	        PC0

#define counter_up()	        ((AVR_ADDR_UP_PORT &= ~(1 << AVR_ADDR_UP_PIN)),\
                                (AVR_ADDR_UP_PORT |= (1 << AVR_ADDR_UP_PIN)))

#define SNES_WR_PORT	        PORTC
#define SNES_WR_DIR	            DDRC
#define SNES_WR_PIN	            PC3

/* Port B*/
#define AVR_PORT	            PORTB
#define AVR_DIR	                DDRB
#define AVR_RD_PORT	            PORTB
#define AVR_RD_DIR	            DDRB
#define AVR_RD_PIN	            PB2

#define AVR_WR_PORT	            PORTB
#define AVR_WR_DIR	            DDRB
#define AVR_WR_PIN	            PB1

#define AVR_CS_PORT	            PORTB
#define AVR_CS_DIR	            DDRB
#define AVR_CS_PIN	            PB0

#define SNES_IRQ_PORT	        PORTB
#define SNES_IRQ_DIR	        DDRB
#define SNES_IRQ_PIN	        PB3

#define snes_irq_off()	        (SNES_IRQ_PORT |= (1 << SNES_IRQ_PIN))
#define snes_irq_on()	        (SNES_IRQ_PORT &= ~(1 << SNES_IRQ_PIN))


/* Port A*/
#define AVR_DATA_PORT	        PORTA
#define AVR_DATA_DIR	        DDRA
#define AVR_DATA_PIN	        PINA

#define avr_data_in()	        ((AVR_DATA_DIR = 0x00),\
                                (AVR_DATA_PORT = 0x00))

#define avr_data_out()	        (AVR_DATA_DIR = 0xff)

/* Port D*/

#define AVR_SNES_PORT	        PORTD
#define AVR_SNES_DIR	        DDRD
#define AVR_SNES_SW_PORT	    PORTD
#define AVR_SNES_SW_DIR	        DDRD
#define AVR_SNES_SW_PIN	        PD5

#define avr_bus_active()	    ((AVR_SNES_SW_PORT &= ~(1 << AVR_SNES_SW_PIN)),\
                                (HI_LOROM_SW_PORT |= (1 << HI_LOROM_SW_PIN)))

#define snes_bus_active()	    (AVR_SNES_SW_PORT |= (1 << AVR_SNES_SW_PIN))

#define HI_LOROM_SW_PORT	    PORTD
#define HI_LOROM_SW_DIR	        DDRD
#define HI_LOROM_SW_PIN	        PD6

#define snes_hirom()	        (HI_LOROM_SW_PORT &= ~(1 << HI_LOROM_SW_PIN))
#define snes_lorom()	        (HI_LOROM_SW_PORT |= (1 << HI_LOROM_SW_PIN))


#define SNES_WR_EN_PORT	        PORTD
#define SNES_WR_EN_DIR	        DDRD
#define SNES_WR_EN_PIN	        PD7

#define snes_wr_disable()	    (SNES_WR_EN_PORT &= ~(1 << SNES_WR_EN_PIN))
#define snes_wr_enable()	    (SNES_WR_EN_PORT |= (1 << SNES_WR_EN_PIN))







void system_init(void);
void sreg_set(uint32_t addr);

uint8_t sram_read(uint32_t addr);
void sram_write(uint32_t addr, uint8_t data);
void sram_clear(uint32_t addr, uint32_t len);
void sram_copy(uint32_t addr,uint8_t *src, uint32_t len);
void sram_read_buffer(uint32_t addr,uint8_t *dst, uint32_t len);
uint8_t sram_check(uint8_t *buffer, uint32_t len);
