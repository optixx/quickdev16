/*#######################################################################################
Connect AVR to MMC/SD 

Copyright (C) 2004 Ulrich Radig

Bei Fragen und Verbesserungen wendet euch per EMail an

mail@ulrichradig.de

oder im Forum meiner Web Page : www.ulrichradig.de

Dieses Programm ist freie Software. Sie k�nnen es unter den Bedingungen der 
GNU General Public License, wie von der Free Software Foundation ver�ffentlicht, 
weitergeben und/oder modifizieren, entweder gem�� Version 2 der Lizenz oder 
(nach Ihrer Option) jeder sp�teren Version. 

Die Ver�ffentlichung dieses Programms erfolgt in der Hoffnung, 
da� es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
F�R EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License. 

Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
Programm erhalten haben. 
Falls nicht, schreiben Sie an die Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
#######################################################################################*/

#include <avr/io.h>

#include "mmc.h"

#include <util/delay.h>         

//############################################################################
//Routine zur Initialisierung der MMC/SD-Karte (SPI-MODE)
unsigned char mmc_init (void){

  unsigned char a;
  unsigned int Timeout = 0;
   
  //Konfiguration des Ports an der die MMC/SD-Karte angeschlossen wurde
  MMC_Direction_REG &=~(1<<SPI_DI);         //Setzen von Pin MMC_DI auf Input
  MMC_Direction_REG |= (1<<SPI_Clock);      //Setzen von Pin MMC_Clock auf Output
  MMC_Direction_REG |= (1<<SPI_DO);         //Setzen von Pin MMC_DO auf Output
  MMC_Direction_REG |= (1<<MMC_Chip_Select);//Setzen von Pin MMC_Chip_Select auf Output
  //MMC_Direction_REG |= (1<<SPI_SS);   
  MMC_Write |= (1<<MMC_Chip_Select);        //Setzt den Pin MMC_Chip_Select auf High Pegel

  for(a=0;a<200;a++){
	nop();
	nop();
	nop();
	};      //Wartet eine kurze Zeit
 
   //Aktiviren des SPI - Bus, Clock = Idel LOW
   //SPI Clock teilen durch 128
   SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1); //Enable SPI, SPI in Master Mode     
   
  _delay_ms(10);
  //Initialisiere MMC/SD-Karte in den SPI-Mode
  for (a = 0;a<0x0f;a++){ //Sendet min 74+ Clocks an die MMC/SD-Karte
    mmc_write_byte(0xff);
    _delay_us(100);
    
  }
   
  //Sendet Commando CMD0 an MMC/SD-Karte
  unsigned char CMD[] = {0x40,0x00,0x00,0x00,0x00,0x95};
  while(mmc_write_command (CMD) !=1){
      _delay_us(100);
 	if (Timeout++ > 200){
	 MMC_Disable();
     return(1); //Abbruch bei Commando1 (Return Code1)
     }
  }
  //Sendet Commando CMD1 an MMC/SD-Karte
  Timeout = 0;
  CMD[0] = 0x41;//Commando 1
  CMD[5] = 0xFF;
  while( mmc_write_command (CMD) !=0){
	if (Timeout++ > 400){
	  MMC_Disable();
      return(2); //Abbruch bei Commando2 (Return Code2)
      }
	}
 
  //SPI Bus auf max Geschwindigkeit
  SPCR &= ~((1<<SPR0) | (1<<SPR1));
  SPSR = SPSR|(1<<SPI2X); 
   
   //set MMC_Chip_Select to high (MMC/SD-Karte Inaktiv)
   MMC_Disable();
   return(0);
}


//############################################################################
//Sendet ein Commando an die MMC/SD-Karte
unsigned char mmc_write_command (unsigned char *cmd){
	unsigned char a;
   unsigned char tmp = 0xff;
   unsigned int Timeout = 0;

   //set MMC_Chip_Select to high (MMC/SD-Karte Inaktiv) 
   MMC_Disable();

   //sendet 8 Clock Impulse
   mmc_write_byte(0xFF);

   //set MMC_Chip_Select to low (MMC/SD-Karte Aktiv)
   MMC_Enable();

   //sendet 6 Byte Commando
   for (a = 0;a<0x06;a++){ 
      mmc_write_byte(*cmd++);
      }

   //Wartet auf ein gültige Antwort von der MMC/SD-Karte
   while (tmp == 0xff){
      tmp = mmc_read_byte();
      if (Timeout++ > 500){
         break; //Abbruch da die MMC/SD-Karte nicht Antwortet
         }
      }
   return(tmp);
}

//############################################################################
//Routine zum Empfangen eines Bytes von der MMC-Karte 
unsigned char mmc_read_byte (void){ 
  SPDR = 0xff;
  while(!(SPSR & (1<<SPIF))){}; 
  return (SPDR);
}

//############################################################################
//Routine zum Senden eines Bytes zur MMC-Karte
void mmc_write_byte (unsigned char Byte){
   SPDR = Byte;    //Sendet ein Byte
   while(!(SPSR & (1<<SPIF))){ ; //Wartet bis Byte gesendet wurde	
   }
}


//############################################################################
//Routine zum schreiben eines Blocks(512Byte) auf die MMC/SD-Karte
unsigned char mmc_write_sector (unsigned long addr,unsigned char *Buffer){
   unsigned char tmp;
	unsigned int a;
	unsigned char b;

   //Commando 24 zum schreiben eines Blocks auf die MMC/SD - Karte
   unsigned char cmd[] = {0x58,0x00,0x00,0x00,0x00,0xFF}; 
   
   /*Die Adressierung der MMC/SD-Karte wird in Bytes angegeben,
     addr wird von Blocks zu Bytes umgerechnet danach werden 
     diese in das Commando eingefuegt*/
     
   addr = addr << 9; //addr = addr * 512
   
   cmd[1] = ((addr & 0xFF000000) >>24 );
   cmd[2] = ((addr & 0x00FF0000) >>16 );
   cmd[3] = ((addr & 0x0000FF00) >>8 );

   //Sendet Commando cmd24 an MMC/SD-Karte (Write 1 Block/512 Bytes)
   tmp = mmc_write_command (cmd);

   if (tmp != 0)  return(tmp);
         
   //Wartet einen Moment und sendet einen Clock an die MMC/SD-Karte
   for (b=0;b<100;b++)
      {
      mmc_read_byte();
      }
   
   //Sendet Start Byte an MMC/SD-Karte
   mmc_write_byte(0xFE);   
   
   //Schreiben des Bolcks (512Bytes) auf MMC/SD-Karte
	a=511;				// do while konstrukt weils schneller geht
	tmp=*Buffer++;		// holt neues byte aus ram in register   
	do{   
		SPDR = tmp;    //Sendet ein Byte
		tmp=*Buffer++;	// holt schonmal neues aus ram in register
		while( !(SPSR & (1<<SPIF)) ){ ;}//Wartet bis Byte gesendet wurde	
      }while(a--);
   
   //CRC-Byte schreiben
   mmc_write_byte(0xFF); //Schreibt Dummy CRC
   mmc_write_byte(0xFF); //CRC Code wird nicht benutzt
   
   //Fehler beim schreiben? (Data Response XXX00101 = OK)
   if((mmc_read_byte()&0x1F) != 0x05) return(1);

   //Wartet auf MMC/SD-Karte Bussy
   while (mmc_read_byte() != 0xff){};
   
   //set MMC_Chip_Select to high (MMC/SD-Karte Inaktiv)
   MMC_Disable();
   
return(0);
}

//############################################################################
//Routine zum lesen des CID Registers von der MMC/SD-Karte (16Bytes)
void mmc_read_block(unsigned char *cmd,unsigned char *Buffer,unsigned int Bytes){   

  unsigned int a;

   //Sendet Commando cmd an MMC/SD-Karte
   if (mmc_write_command (cmd) != 0)
         {
          return;
         }

   //Wartet auf Start Byte von der MMC/SD-Karte (FEh/Start Byte)
   
   while (mmc_read_byte() != 0xfe){};

   //Lesen des Bolcks (normal 512Bytes) von MMC/SD-Karte
   for (a=0;a<Bytes;a++)
      {
      *Buffer++ = mmc_read_byte();
      }
   //CRC-Byte auslesen
   mmc_read_byte();//CRC - Byte wird nicht ausgewertet
   mmc_read_byte();//CRC - Byte wird nicht ausgewertet
   
   //set MMC_Chip_Select to high (MMC/SD-Karte Inaktiv)
   MMC_Disable();
   
   return;
}

//############################################################################
//Routine zum lesen eines Blocks(512Byte) von der MMC/SD-Karte
unsigned char mmc_read_sector (unsigned long addr,unsigned char *Buffer){   
   //Commando 16 zum lesen eines Blocks von der MMC/SD - Karte
   unsigned char cmd[] = {0x51,0x00,0x00,0x00,0x00,0xFF}; 
   
   /*Die Adressierung der MMC/SD-Karte wird in Bytes angegeben,
     addr wird von Blocks zu Bytes umgerechnet danach werden 
     diese in das Commando eingef�gt*/
     
   addr = addr << 9; //addr = addr * 512

   cmd[1] = ((addr & 0xFF000000) >>24 );
   cmd[2] = ((addr & 0x00FF0000) >>16 );
   cmd[3] = ((addr & 0x0000FF00) >>8 );

    mmc_read_block(cmd,Buffer,512);

   return(0);
}


/*
//############################################################################
//Routine zum lesen des CID Registers von der MMC/SD-Karte (16Bytes)
unsigned char mmc_read_cid (unsigned char *Buffer){
   //Commando zum lesen des CID Registers
   unsigned char cmd[] = {0x4A,0x00,0x00,0x00,0x00,0xFF}; 
   
   mmc_read_block(cmd,Buffer,16);

   return(0);
}

//############################################################################
//Routine zum lesen des CSD Registers von der MMC/SD-Karte (16Bytes)
unsigned char mmc_read_csd (unsigned char *Buffer){   
   //Commando zum lesen des CSD Registers
   unsigned char cmd[] = {0x49,0x00,0x00,0x00,0x00,0xFF};
   
   mmc_read_block(cmd,Buffer,16);

   return(0);
}
*/
