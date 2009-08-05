/*
 * ####################################################################################### Connect ARM to MMC/SD   Copyright (C) 2004
 * Ulrich Radig #######################################################################################
 */

#ifndef _MMC_H_
#define _MMC_H_

#include <avr/io.h>

    // #define SPI_Mode 1 //1 = Hardware SPI | 0 = Software SPI
//#define SPI_Mode			1

#define MMC_Write			PORTB   // Port an der die MMC/SD-Karte angeschlossen ist also des SPI
#define MMC_Read			PINB
#define MMC_Direction_REG	DDRB

#define MMC_CS  PB4
#define MMC_DO  PB6
#define MMC_DI  PB5
#define MMC_CLK PB7
//#define SPI_SS  PC4             // Nicht Benutz muﬂ aber definiert werden

    // Prototypes
extern unsigned char mmc_read_byte(void);
extern void mmc_write_byte(unsigned char);
extern void mmc_read_block(unsigned char *, unsigned char *, unsigned in);
extern unsigned char mmc_init(void);
extern unsigned char mmc_read_sector(unsigned long, unsigned char *);
extern unsigned char mmc_write_sector(unsigned long, unsigned char *);
extern unsigned char mmc_write_command(unsigned char *);
extern unsigned char mmc_read_csd(unsigned char *);
extern unsigned char mmc_read_cid(unsigned char *);

    // set MMC_Chip_Select to high (MMC/SD-Karte Inaktiv)
#define MMC_Disable() MMC_Write|= (1<<MMC_CS);

    // set MMC_Chip_Select to low (MMC/SD-Karte Aktiv)
#define MMC_Enable() MMC_Write&=~(1<<MMC_CS);

#define nop()  __asm__ __volatile__ ("nop" ::)

#endif                          // _MMC_H_
