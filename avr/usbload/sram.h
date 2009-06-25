#ifndef __SRAM_H__
#define __SRAM_H__

#include <stdlib.h>
#include <stdint.h> 
#include <avr/io.h>



/* ---------------------------- PORT A ---------------------------- */

#define AVR_DATA_PORT           PORTA
#define AVR_DATA_DIR            DDRA
#define AVR_DATA_PIN            PINA

#define avr_data_in()           ((AVR_DATA_DIR = 0x00),\
                                (AVR_DATA_PORT = 0x00))

#define avr_data_out()          (AVR_DATA_DIR = 0xff)

#define LED_PORT	            PORTC
#define LED_DIR		            DDRC
#define LED_PIN		            PC7

#define led_on()	            ((LED_PORT &=~ (1 << LED_PIN)),\
                                (LED_DIR &=~ (1 << LED_PIN)))
#define led_off()	            ((LED_PORT &=~ (1 << LED_PIN)),\
                                (LED_DIR |= (1 << LED_PIN)))

/* ---------------------------- PORT B ---------------------------- */

#define AVR_PORT                PORTB
#define AVR_DIR                 DDRB
#define AVR_RD_PORT             PORTB
#define AVR_RD_DIR              DDRB
#define AVR_RD_PIN              PB2

#define avr_rd_hi()             (AVR_RD_PORT |= (1 << AVR_RD_PIN))
#define avr_rd_lo()             (AVR_RD_PORT &= ~(1 << AVR_RD_PIN))

#define AVR_WR_PORT             PORTB
#define AVR_WR_DIR              DDRB
#define AVR_WR_PIN              PB1

#define avr_wr_hi()             (AVR_WR_PORT |= (1 << AVR_WR_PIN))
#define avr_wr_lo()             (AVR_WR_PORT &= ~(1 << AVR_WR_PIN))

#define AVR_CS_PORT             PORTB
#define AVR_CS_DIR              DDRB
#define AVR_CS_PIN              PB0

#define avr_cs_hi()             (AVR_CS_PORT |= (1 << AVR_CS_PIN))
#define avr_cs_lo()             (AVR_CS_PORT &= ~(1 << AVR_CS_PIN))

#define SNES_IRQ_PORT           PORTB
#define SNES_IRQ_DIR            DDRB
#define SNES_IRQ_PIN            PB3

#define snes_irq_off()          (SNES_IRQ_PORT |= (1 << SNES_IRQ_PIN))
#define snes_irq_on()           (SNES_IRQ_PORT &= ~(1 << SNES_IRQ_PIN))



/* ---------------------------- PORT C ---------------------------- */

#define AVR_ADDR_PORT	    	PORTC
#define AVR_ADDR_DIR	    	DDRC
#define AVR_ADDR_LATCH_PORT	    PORTC
#define AVR_ADDR_LATCH_DIR	    DDRC
#define AVR_ADDR_LATCH_PIN	    PC6

#define avr_addr_latch_hi()       (AVR_ADDR_LATCH_PORT |= (1 << AVR_ADDR_LATCH_PIN)))
#define avr_addr_latch_lo()       (AVR_ADDR_LATCH_PORT &= ~(1 << AVR_ADDR_LATCH_PIN)))

#define AVR_ADDR_SCK_PORT	    PORTC
#define AVR_ADDR_SCK_DIR	    DDRC
#define AVR_ADDR_SCK_PIN	    PC5

#define avr_addr_sck_hi()       (AVR_ADDR_SCK_PORT |= (1 << AVR_ADDR_SCK_PIN)))
#define avr_addr_sck_lo()       (AVR_ADDR_SCK_PORT &= ~(1 << AVR_ADDR_SCK_PIN)))

#define AVR_ADDR_SER_PORT	    PORTC
#define AVR_ADDR_SER_DIR	    DDRC
#define AVR_ADDR_SER_PIN	    PC4

#define avr_addr_ser_hi()       (AVR_ADDR_SER_PORT |= (1 << AVR_ADDR_SER_PIN)))
#define avr_addr_ser_lo()       (AVR_ADDR_SER_PORT &= ~(1 << AVR_ADDR_SER_PIN)))

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

/* ---------------------------- PORT D ---------------------------- */

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
void sram_set(uint32_t addr, uint32_t len, uint8_t value);
void sram_copy(uint32_t addr,uint8_t *src, uint32_t len);
void sram_read_buffer(uint32_t addr,uint8_t *dst, uint32_t len);

void sram_bulk_read_start(uint32_t addr);
inline void sram_bulk_read_next(void);
inline void sram_bulk_read_end(void);
uint8_t sram_bulk_read(void);

void sram_bulk_write_start(uint32_t addr);
inline void sram_bulk_write_next(void);
inline void sram_bulk_write_end(void);
void sram_bulk_write(uint8_t data);

void sram_bulk_copy(uint32_t addr, uint8_t * src, uint32_t len);
void sram_bulk_read_buffer(uint32_t addr, uint8_t * dst, uint32_t len);
void sram_bulk_set(uint32_t addr, uint32_t len,uint8_t value);

#endif
