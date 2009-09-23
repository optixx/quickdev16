/*
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        28.05.2004
 Description:    Dieses Programm dient als Beispiel zur Ansteuerung einer MMC/SD-Memory-Card.
				 Zum Zugriff auf eine MMC/SD-Karte, muß man nur die Datei mmc.c
				 in sein eigenes Programm einfügen.
*/

#include <stdio.h>
#include <string.h>	
#include <avr/io.h>	
#include <avr/eeprom.h>	
	
#include "mmc.h"
#include "fat.h"
#include "usart.h"

//Hauptprogramm
int main (void)
{
	//SYSCLK defined in usart.h
	//Initzialisierung der seriellen Schnittstelle
	usart_init(9600);

	//Initialisierung der MMC/SD-Karte
	usart_write("System Ready!\r\n");	
	while ( mmc_init() !=0) //ist der Rückgabewert ungleich NULL ist ein Fehler aufgetreten
		{
		usart_write("** Keine MMC/SD Karte gefunden!! **\n");	
		}
	usart_write("Karte gefunden!!\n");
	
	fat_init();//laden Cluster OFFSET und Size
	//Initialisierung der MMC/SD-Karte ENDE!

	unsigned char Buffer[512];
	unsigned int tmp;
	
	mmc_read_csd (Buffer);
	
	for (tmp = 0;tmp<16;tmp++)
		{
		usart_write("%x ",Buffer[tmp]);
		};


	//Ausgabe des Root Directory
	unsigned int Clustervar;
	unsigned char Dir_Attrib = 0;
	unsigned long Size = 0;
	usart_write("\r\nDirectory\r\n");
	for (char a = 1;a < 240;a++)
	{
		Clustervar = fat_read_dir_ent(0,a,&Size,&Dir_Attrib,Buffer);
			if (Clustervar == 0xffff)
			{
				break;
			}
		tmp = (Size & 0x0000FFFF);
		usart_write("Cluster = %4x DirA = %2x FileName = ",Clustervar,Dir_Attrib);
		usart_write("%s",Buffer);
		usart_write("\r\n");
	}
	usart_write("\r\nDirectory Ende\r\n");

	//Lade Cluster für das index.htm File in den Speicher 
	Clustervar = 0;//suche im Root Verzeichnis
	if (fat_search_file((unsigned char *)"mmc.txt",&Clustervar,&Size,&Dir_Attrib,Buffer) == 1)
		{
		usart_write("\nFile Found!!\r\n");
		//Lese File und gibt es auf der seriellen Schnittstelle aus
		for (int b = 0;b<52;b++)
			{
			fat_read_file (Clustervar,Buffer,b);
			for (int a = 0;a<512;a++)
				{
				usart_write("%c",Buffer[a]);
				}
			}
		}

	usart_write("FERTIG!!\r\n");
	//Hauptprogramm läuft ständig in einer schleife und macht nichts
	while (1)
		{
		}
return (1);
}

