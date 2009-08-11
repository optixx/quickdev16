
#include <stdlib.h>
#include <avr/io.h>

#include "main.h"
#include "file.h"
#include "fat.h"						
#include "hardware.h"


//*****************************************************************************************************************

void main(void){

  uinit();							// uart initialisierung

 
  uputs((unsigned char*)"\nBoot");

  while (mmc_init() !=0){ 		//ist der Rückgabewert ungleich NULL ist ein Fehler aufgetreten	
	;	
	}  
	
  uputs((unsigned char*)"... ");		

  if(0==fat_initfat()){				//ist der Rückgabewert ungleich NULL ist ein Fehler aufgetreten	

	 uputs((unsigned char*)"Ok\n");		// wenn auf dem terminal "Boot... OK" zu lesen ist, ist init ok. jetzt kann man schreiben/lesen
	
    beispiele();
  }


}

// *****************************************************************************************************************
void beispiele(void){

  char datei[12]="TEST    TXT";		// hier muss platz für 11 zeichen sein (8.3), da fat_str diesen string benutzt !!
  fat_str(datei);						// wandelt "test.txt" in das fat format 8.3 der form: "TEST    TXT" muss immer dieses Format haben, auch ordner !!

  // 0.) ______________löschen von dateien/ordnern (ordner rekursiv)____________________________________________
  ffrm( datei );								// löschen der datei/ordner falls vorhanden

  // 1.) ______________anlegen und schreiben____________________________________________________________________
  // 	öffnet datei, wenn nicht vorhanden, legt ffopen datei an (rückgabewert = 1 datei existiert, also nur öffnen, 2 = angelegt).   
  ffopen( datei );						

  // schreibt string 
  ffwrites((char*)"Hallo Datei :)");
  // neue zeile in der datei
  ffwrite(0x0D);
  ffwrite(0x0A);

  // schließt datei
  ffclose();

  // 2.)________________ändern von vorhandenen daten in dateien__________________________________________________
  ffopen( datei );		// siehe oben...
  ffseek(12);				// spult in datei auf position 12 vor (fängt immer bei 0 an zu zählen !)
  ffwrite(';');			// schreibt dann ab position 12 (überschreibt daten der datei, hier nur 1 zeichen)
  ffclose();				// schließt datei  

  // 3.)________________lesen von dateien_________________________________________________________________________
  ffopen( datei );							// siehe oben...
  unsigned long int seek=file.length;	// eine variable setzen und runterzählen bis 0 geht am schnellsten !
  do{
	 uputc(ffread());							// liest ein zeichen und gibt es über uart aus !
	 }while(--seek);							// liest solange bytes da sind (von datei länge bis 0)
  ffclose();									// schließt datei

  uputc('\n');	// neue zeile weil neue unteraufgabe

  // 4.)________________anhängen von daten an datei_______________________________________________________________
  ffopen( datei);				// siehe oben...
  ffseek(file.length);			// spult in datei ans ende
  ffwrites((char*)"Dies ist ein Test...");	  // siehe oben
  // neue zeile in der datei
  ffwrite(0x0D);
  ffwrite(0x0A);
  ffclose();						// schließt datei  

  // 3.)________________lesen von dateien_________________________________________________________________________
  ffopen( datei );		// siehe oben...
  seek=file.length;		// eine variable setzen und runterzählen bis 0 geht am schnellsten !
  do{
	 uputc(ffread());		// liest ein zeichen und gibt es über uart aus !
	 }while(--seek);		// liest solange bytes da sind (von datei länge bis 0)
  ffclose();				// schließt datei
  
}

// *****************************************************************************************************************








