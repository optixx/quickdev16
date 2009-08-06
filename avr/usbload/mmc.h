#ifndef _MMC_H_
#define _MMC_H_

#include <avr/io.h>


#define MMC_WRITE			PORTB   // Port an der die MMC/SD-Karte angeschlossen ist also des SPI
#define MMC_READ			PINB
#define MMC_REG	DDRB

#define MMC_CS  PB4
#define MMC_DO  PB6
#define MMC_DI  PB5
#define MMC_CLK PB7
extern uint8_t mmc_read_byte(void);
extern void mmc_write_byte(uint8_t);
extern void mmc_read_block(uint8_t *, uint8_t *, unsigned in);
extern uint8_t mmc_init(void);
extern uint8_t mmc_read_sector(uint32_t, uint8_t *);
extern uint8_t mmc_write_sector(uint32_t, uint8_t *);
extern uint8_t mmc_write_command(uint8_t *);
extern uint8_t mmc_read_csd(uint8_t *);
extern uint8_t mmc_read_cid(uint8_t *);

#define mmc_disable() MMC_WRITE|= (1<<MMC_CS);

#define mmc_enable() MMC_WRITE&=~(1<<MMC_CS);

#define nop()  __asm__ __volatile__ ("nop" ::)

#endif
