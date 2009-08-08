/*#######################################################################################
Connect ARM to MMC/SD 

Copyright (C) 2004 Ulrich Radig
#######################################################################################*/


#ifndef _MMC_H
  #define _MMC_H  

  #define MMC_Write  		PORTB	//Port an der die MMC/SD-Karte angeschlossen ist also des SPI 
  #define MMC_Read 			PINB
  #define MMC_Direction_REG	DDRB	


  #define SPI_DI   		6  //Port Pin an dem Data Output der MMC/SD-Karte angeschlossen ist 
  #define SPI_DO    		5  //Port Pin an dem Data Input der MMC/SD-Karte angeschlossen ist
  #define SPI_Clock  		7  //Port Pin an dem die Clock der MMC/SD-Karte angeschlossen ist (clk)
  #define MMC_Chip_Select 4 //Port Pin an dem Chip Select der MMC/SD-Karte angeschlossen ist 
  #define SPI_SS			4  //Nicht Benutz mu� aber definiert werden


  extern  unsigned char mmc_read_byte(void);
  extern  void 			mmc_write_byte(unsigned char);
  extern  void 			mmc_read_block(unsigned char *,unsigned char *,unsigned in);
  extern  unsigned char mmc_init(void);
  extern  unsigned char mmc_read_sector (unsigned long,unsigned char *);
  extern  unsigned char mmc_write_sector (unsigned long,unsigned char *);
  extern  unsigned char mmc_write_command (unsigned char *);
  
  #define MMC_Disable() MMC_Write|= (1<<MMC_Chip_Select);

  #define MMC_Enable() MMC_Write&=~(1<<MMC_Chip_Select);

 #define nop()  __asm__ __volatile__ ("nop" ::)
#endif
