


#ifndef _FILE_H

#define _FILE_H

  // **************************************************************************************************************************
  // funktionen

extern inline uint8_t ffread(void);       // liest byte-weise aus der datei (puffert immer 512 bytes zwischen)
extern inline void ffwrite(uint8_t c);    // schreibt ein byte in die geöffnete datei 
extern inline void ffwrites(const char *s);     // schreibt string auf karte

extern uint8_t ffopen(char name[]);       // kann immer nur 1 datei bearbeiten.
extern uint8_t ffclose(void);     // muss aufgerufen werden bevor neue datei bearbeitet wird.

extern void ffseek(uint32_t offset);   // setzt zeiger:bytesOfSec auf position in der geöffneten datei.
extern uint8_t ffcd(char name[]); // wechselt direktory
extern void ffls(void);         // zeigt direktory inhalt an
extern void ffls_smc(void);         // zeigt direktory inhalt an
extern uint8_t ffcdLower(void);   // geht ein direktory zurück, also cd.. (parent direktory)
extern uint8_t ffrm(char name[]); // löscht datei aus aktuellem verzeichniss.
extern void ffmkdir(char name[]);       // legt ordner in aktuellem verzeichniss an.
void lsRowsOfClust(uint32_t start_sec);        // zeigt reihen eines clusters an, ab start_sec
void fileUpdate(void);          // updatet datei eintrag auf karte

  // **************************************************************************************************************************//
  // #######################################################################################################################



#endif
