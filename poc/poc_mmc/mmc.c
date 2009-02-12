/*#######################################################################################
Connect AVR to MMC/SD 

Copyright (C) 2004 Ulrich Radig

Bei Fragen und Verbesserungen wendet euch per EMail an

mail@ulrichradig.de

oder im Forum meiner Web Page : www.ulrichradig.de

Dieses Programm ist freie Software. Sie können es unter den Bedingungen der 
GNU General Public License, wie von der Free Software Foundation veröffentlicht, 
weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder 
(nach Ihrer Option) jeder späteren Version. 

Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, 
daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License. 

Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
Programm erhalten haben. 
Falls nicht, schreiben Sie an die Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
#######################################################################################*/

#include "mmc.h"
#include <util/delay.h>

//############################################################################
//Routine zur Initialisierung der MMC/SD-Karte (SPI-MODE)
unsigned char mmc_init ()
//############################################################################
{
	unsigned int Timeout = 0,i;
	
	//Konfiguration des Ports an der die MMC/SD-Karte angeschlossen wurde
	DDRC  |= ( (1<<MMC_DO) | (1<<MMC_CS) | (1<<MMC_CLK) );
        DDRC  &=  ~(1<<MMC_DI);
        PORTC |= ( (1<<MMC_DO) | (1<<MMC_DI) | (1<<MMC_CS) );

	//Wartet eine kurze Zeit
	_delay_ms(10);

	
	//Initialisiere MMC/SD-Karte in den SPI-Mode
	for(i=0; i<250; i++)
	{
                PORTC ^= (1<<MMC_CLK);
                _delay_us(4);
        }
        
	PORTC &= ~(1<<MMC_CLK);
        _delay_us(10);

	 PORTC &= ~(1<<MMC_CS);
        _delay_us(3);

	
	//Sendet Commando CMD0 an MMC/SD-Karte
	unsigned char CMD[] = {0x40,0x00,0x00,0x00,0x00,0x95};
	
	while(mmc_write_command (CMD) !=1)
	{
		if (Timeout++ > 20)
		{
			MMC_Disable();
			return(1); //Abbruch bei Commando1 (Return Code1)
		}
	}
	
	//Sendet Commando CMD1 an MMC/SD-Karte
	Timeout = 0;
	
	CMD[0] = 0x41;//Commando 1
	CMD[5] = 0xFF;
	
	while( mmc_write_command (CMD) !=0)
	{
		if (Timeout++ > 800)
			{
			MMC_Disable();
			return(9); //Abbruch bei Commando2 (Return Code2)
			}
	}
	return(0);
}

//############################################################################
//Sendet ein Commando an die MMC/SD-Karte
unsigned char mmc_write_command (unsigned char *cmd)
//############################################################################
{
	unsigned char tmp = 0xff;
	unsigned int Timeout = 0;


	//sendet 6 Byte Commando
	for (unsigned char a = 0;a<0x06;a++) //sendet 6 Byte Commando zur MMC/SD-Karte
	{
		mmc_write_byte(*cmd++);
	}

	//Wartet auf ein gültige Antwort von der MMC/SD-Karte
	while (tmp == 0xff)	
	{
		tmp = mmc_read_byte();

		if (Timeout++ > 50)
			{
			break; //Abbruch da die MMC/SD-Karte nicht Antwortet
			}
		}
	return(tmp);
}

//############################################################################
//Routine zum Empfangen eines Bytes von der MMC-Karte 
unsigned char mmc_read_byte (void)
//############################################################################
{
	uint8_t Byte=0,j;
	
	for(j=0; j<8; j++){
        	Byte = (Byte<<1);
		
		PORTC |= (1<<MMC_CLK);
                _delay_us(4);

                if(PINC & (1<<MMC_DI)){
                	Byte |= 1;
                }
                else{
                	Byte &= ~1;
                }

                PORTC &= ~(1<<MMC_CLK);
                _delay_us(4);
	}
	
	return (Byte);
}


//############################################################################
//Routine zum Senden eines Bytes zur MMC-Karte
void mmc_write_byte (unsigned char Byte)
//############################################################################
{
	uint8_t i;

	for(i=0; i<8; i++){
        	if(Byte & 0x80){
                	PORTC |=  (1<<MMC_DO);
                }
                else{
                	PORTC &= ~(1<<MMC_DO);
                }

                Byte = (Byte<<1);
                PORTC |=  (1<<MMC_CLK);
                _delay_us(4);
                PORTC &= ~(1<<MMC_CLK);
                _delay_us(4);
	}
	PORTC |= (1<<MMC_DO);
}

//############################################################################
//Routine zum schreiben eines Blocks(512Byte) auf die MMC/SD-Karte
unsigned char mmc_write_sector (unsigned long addr,unsigned char *Buffer)
//############################################################################
{
	unsigned char tmp;
	//Commando 24 zum schreiben eines Blocks auf die MMC/SD - Karte
	unsigned char cmd[] = {0x58,0x00,0x00,0x00,0x00,0xFF}; 
	
	/*Die Adressierung der MMC/SD-Karte wird in Bytes angegeben,
	  addr wird von Blocks zu Bytes umgerechnet danach werden 
	  diese in das Commando eingefügt*/
	  
	addr = addr << 9; //addr = addr * 512
	
	cmd[1] = ((addr & 0xFF000000) >>24 );
	cmd[2] = ((addr & 0x00FF0000) >>16 );
	cmd[3] = ((addr & 0x0000FF00) >>8 );

	//Sendet Commando cmd24 an MMC/SD-Karte (Write 1 Block/512 Bytes)
	tmp = mmc_write_command (cmd);
	if (tmp != 0)
		{
		return(tmp);
		}
			
	//Wartet einen Moment und sendet einen Clock an die MMC/SD-Karte
	for (unsigned char a=0;a<100;a++)
		{
		mmc_read_byte();
		}
	
	//Sendet Start Byte an MMC/SD-Karte
	mmc_write_byte(0xFE);	
	
	//Schreiben des Bolcks (512Bytes) auf MMC/SD-Karte
	for (unsigned int a=0;a<512;a++)
		{
		mmc_write_byte(*Buffer++);
		}
	
	//CRC-Byte schreiben
	mmc_write_byte(0xFF); //Schreibt Dummy CRC
	mmc_write_byte(0xFF); //CRC Code wird nicht benutzt
	
	//Fehler beim schreiben? (Data Response XXX00101 = OK)
	if((mmc_read_byte()&0x1F) != 0x05) return(1);

	//Wartet auf MMC/SD-Karte Bussy
	while (mmc_read_byte() != 0xff){};
	
	
return(0);
}

//############################################################################
//Routine zum lesen des CID Registers von der MMC/SD-Karte (16Bytes)
void mmc_read_block(unsigned char *cmd,unsigned char *Buffer,unsigned int Bytes)
//############################################################################
{	
	//Sendet Commando cmd an MMC/SD-Karte
	if (mmc_write_command (cmd) != 0)
			{
			 return;
			}

	//Wartet auf Start Byte von der MMC/SD-Karte (FEh/Start Byte)
	
	while (mmc_read_byte() != 0xfe){};

	//Lesen des Bolcks (normal 512Bytes) von MMC/SD-Karte
	for (unsigned int a=0;a<Bytes;a++)
		{
		*Buffer++ = mmc_read_byte();
		}
	//CRC-Byte auslesen
	mmc_read_byte();//CRC - Byte wird nicht ausgewertet
	mmc_read_byte();//CRC - Byte wird nicht ausgewertet
	
	
	return;
}

//############################################################################
//Routine zum lesen eines Blocks(512Byte) von der MMC/SD-Karte
unsigned char mmc_read_sector (unsigned long addr,unsigned char *Buffer)
//############################################################################
{	
	//Commando 16 zum lesen eines Blocks von der MMC/SD - Karte
	unsigned char cmd[] = {0x51,0x00,0x00,0x00,0x00,0xFF}; 
	/*Die Adressierung der MMC/SD-Karte wird in Bytes angegeben,
	  addr wird von Blocks zu Bytes umgerechnet danach werden 
	  diese in das Commando eingefügt*/
	  
	addr = addr << 9; //addr = addr * 512

	cmd[1] = ((addr & 0xFF000000) >>24 );
	cmd[2] = ((addr & 0x00FF0000) >>16 );
	cmd[3] = ((addr & 0x0000FF00) >>8 );

	mmc_read_block(cmd,Buffer,512);

	return(0);
}

//############################################################################
//Routine zum lesen des CID Registers von der MMC/SD-Karte (16Bytes)
unsigned char mmc_read_cid (unsigned char *Buffer)
//############################################################################
{
	//Commando zum lesen des CID Registers
	unsigned char cmd[] = {0x4A,0x00,0x00,0x00,0x00,0xFF}; 
	
	mmc_read_block(cmd,Buffer,16);

	return(0);
}

//############################################################################
//Routine zum lesen des CSD Registers von der MMC/SD-Karte (16Bytes)
unsigned char mmc_read_csd (unsigned char *Buffer)
//############################################################################
{	
	//Commando zum lesen des CSD Registers
	unsigned char cmd[] = {0x49,0x00,0x00,0x00,0x00,0xFF};
	
	mmc_read_block(cmd,Buffer,16);
	return(0);
}
