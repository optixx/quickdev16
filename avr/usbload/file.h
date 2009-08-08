


#ifndef _FILE_H

  #define _FILE_H 
 
  //#######################################################################################################################
  // funktionen

  extern inline unsigned char ffread(void);			// liest byte-weise aus der datei (puffert immer 512 bytes zwischen)
  extern inline void ffwrite(unsigned char c);		// schreibt ein byte in die geöffnete datei  
  extern inline void ffwrites(const char *s );		// schreibt string auf karte

  extern unsigned char ffopen(char name[]);			// kann immer nur 1 datei bearbeiten.
  extern unsigned char ffclose(void);					// muss aufgerufen werden bevor neue datei bearbeitet wird.

  extern void ffseek(unsigned long int offset);		// setzt zeiger:bytesOfSec auf position in der geöffneten datei.
  extern unsigned char ffcd(char name[]);				// wechselt direktory
  extern unsigned char ffls(void);				 		// zeigt direktory inhalt an
  extern unsigned char ffcdLower(void);				// geht ein direktory zurück, also cd.. (parent direktory)
  extern unsigned char ffrm(char name[]);				// löscht datei aus aktuellem verzeichniss.
  extern unsigned char ffmkdir(char name[]);			// legt ordner in aktuellem verzeichniss an.    
  void 	lsRowsOfClust (unsigned long int start_sec);	// zeigt reihen eines clusters an, ab start_sec
  
  //#######################################################################################################################
  
	  
  
#endif




