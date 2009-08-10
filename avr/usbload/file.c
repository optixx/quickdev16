#include <string.h>	
#include <stdlib.h>
#include <stdio.h>

#include "mmc.h"
#include "fat.h"
#include "file.h"
#include "config.h"

//*******************************************************************************************************************************
// 2 möglichkeiten beim öffnen, datei existiert(return 1) oder muss angelegt werden(return 2)
// zuerst wird geprüft ob es die datei im verzeichniss gibt. danach wird entschieden, ob die datei geöffnet wird oder angelegt.
// -beim offnen werden die bekannten cluster gesucht maximal MAX_CLUSTERS_IN_ROW in reihe. dann wird der 1. sektor der datei auf
// den puffer fat.sector geladen. jetzt kann man ffread lesen...
// -beim anlegen werden freie cluster gesucht, maximal MAX_CLUSTERS_IN_ROW in reihe. dann wird das struct file gefüllt.
// danach wird der dateieintrag gemacht(auf karte). dort wird auch geprüft ob genügend platz im aktuellen verzeichniss existiert.
// möglicherweise wird der 1. cluster der datei nochmal geändert. jetzt ist der erste frei sektor bekannt und es kann geschrieben werden.
//*******************************************************************************************************************************
unsigned char ffopen(char name[]){		

  unsigned char file_flag=fat_loadFileDataFromDir(name);		//prüfung ob datei vorhanden und evetuelles laden des file struct

  if( file_flag==0 ){				/** Datei existiert, anlegen nicht nötig! **/
	 fat_getFatChainClustersInRow( file.firstCluster );			// verkettete cluster aus der fat-chain suchen.
	 fat_loadSector( fat_clustToSec(file.firstCluster) );		// lät die ersten 512 bytes der datei auf puffer:sector.	
	 file.lastCluster=fat_secToClust(fat.endSectors);			// letzter bekannter cluster der datei	 
	 return 1;
	 }
  #if (WRITE==1)					// anlegen ist schreiben !
  else{							/** Datei existiert nicht, also anlegen !	(nur wenn schreiben option an ist)**/
	 fat_getFreeClustersInRow(2);								// leere cluster suchen, ab cluster 2.
	 strcpy((char*)file.name,(char*)name);					// ---	füllen des file struct, zum abschließenden schreiben.
	 file.firstCluster=fat_secToClust(fat.startSectors);		// 		1. cluster der datei
	 file.lastCluster=file.firstCluster;//fat_secToClust(fat.endSectors);			// 		letzter bekannter cluster der datei
	 file.attrib=32;											// ---	file.row wird in der funktion fat_getFreeRowOfDir geschrieben !!
	 file.length=0;												// damit da nix drin steht ^^
	 fat_makeFileEntry((char *)file.name,file.attrib,0);		// DATEI ANLEGEN auf karte
	 fat.currentSectorNr=fat_clustToSec(file.firstCluster);		// setzen des ersten sektors
	 return 2;
	 }
	#endif
}

//*******************************************************************************************************************************
// schließt die datei operation ab. eigentlich nur nötig wenn geschrieben wurde. es gibt 2 möglichkeiten :
// 1. die datei wird geschlossen und es wurde über die alte datei länge hinaus geschrieben.
// 2. die datei wird geschlossen und man war innerhalb der datei größe, dann muss nur der aktuelle sektor geschrieben werden.
// der erste fall ist komplizierter, weil ermittelt werden muss wie viele sektoren neu beschrieben wurden um diese zu verketten
// und die neue datei länge muss ermitt weden. abschließend wird entweder (fall 2) nur der aktuelle sektor geschrieben, oder
// der aktuallisierte datei eintrag und die cluster (diese werden verkettet, siehe fileUpdate() ).
//*******************************************************************************************************************************
unsigned char ffclose(void){

  #if (WRITE==1) 		/** 2 möglichkeiten beim schließen !!	(lesend spielt keine rolle, nichts muss geupdatet werden) **/

	if( file.length < (file.seek+file.cntOfBytes) )	fileUpdate();			/** 1.) es wurde über die alte datei größe hinaus geschrieben **/

	else if( fat.bufferDirty==1) fat_writeSector( fat.currentSectorNr );	/** 2.) nicht über alte datei länge hinaus **/

  #endif
  
  file.cntOfBytes=0;					// init werte der nötigen zähler
  file.seek=0;
  return(0);	
}

// *******************************************************************************************************************************
// updatet datei eintrag auf der karte und verkettet die dazugehörigen fat cluster.
// füllt den aktuell beschriebenen sektor mit 0x00, da sonst die datei nicht richtig angezeigt wird.
// darf nur während schreibe operationen aufgerufen werden !
// *******************************************************************************************************************************
#if (WRITE==1)

void fileUpdate(void){

  unsigned int comp_cntOfBytes=file.cntOfBytes;		// sicher nötig wegen schleife...
  while( comp_cntOfBytes < 512 ){						// sektor ist beschrieben worden, daher nötigenfalls mit 00 füllen				
	 fat.sector[comp_cntOfBytes]=0x00;					// beschreibt ungenutzte bytes mit 0x00 	
	 comp_cntOfBytes++;			
	 }	 
  char name[13];														// zum sichern des dateinamens
  unsigned long int save_length = file.cntOfBytes + file.seek;		// muss gesichert werden, wird sonst von der karte geladen und verändert !	  
  strcpy(name,(char *)file.name);										// muss gesichert werden, wird sonst von der karte geladen und verändert !
	 
  fat_setClusterChain(fat_secToClust(fat.startSectors),fat_secToClust(fat.currentSectorNr));	// verketten der geschriebenen cluster				
  fat_loadFileDataFromDir(name);							// läd sektor, des datei eintrags, und läd daten von karte auf struct file!			 
  fat_makeRowDataEntry(file.row,name,32,file.firstCluster,save_length);	 	// macht eintrag im puffer				

  fat_writeSector(fat.currentSectorNr);					
}

#endif


// *******************************************************************************************************************************
// offset byte wird übergeben. es wird durch die sektoren der datei gespult (gerechnet), bis der sektor mit dem offset byte erreicht
// ist, dann wird der sektor geladen und der zähler für die bytes eines sektors gesetzt. wenn das byte nicht in den sektoren ist,
// die "vorgesucht" wurden, müssen noch weitere sektoren der datei gesucht werden (sec > fat.endSectors).
// *******************************************************************************************************************************
void ffseek(unsigned long int offset){	

#if (WRITE==1)
	
  #if (OVER_WRITE==1)  									// man  muss den dateieintrag updaten, um die daten zu retten !!
	if( file.seek > file.length ) fileUpdate();			// fat verketten und datei update auf der karte !
  #endif
#endif

  fat_getFatChainClustersInRow(file.firstCluster);		// suchen von anfang der cluster chain aus !
  unsigned long int sec=fat.startSectors;			// sektor variable zum durchgehen durch die sektoren  
  file.seek=0;											// weil auch von anfang an der chain gesucht wird mit 0 initialisiert

  while(offset>=512){ 	 								/** suchen des sektors in dem offset ist  **/	 
	 sec++;												// da byte nicht in diesem sektor ist, muss hochgezählt werden
	 offset-=512;										// ein sektor weniger in dem das byte sein kann
	 file.seek+=512;									// file.seek update, damit bei ffclose() die richtige file.length herauskommt
	 if ( sec > fat.endSectors ){						// es müssen mehr sektoren der datei gesucht werden  
		fat_getFatChainClustersInRow(fat_getNextCluster( file.lastCluster ) );	// nachladen von clustern in der chain		
		sec=fat.startSectors;							// setzen des 1. sektors der neu geladenen, zum weitersuchen !
		} 				
	 }
  file.lastCluster=fat_secToClust(fat.endSectors);		// letzter bekannter cluster der datei
  fat_loadSector(sec);  								// sektor mit offset byte laden
  file.cntOfBytes = offset;								// setzen des lese zählers   
}




#if (SMALL_FILE_SYSTEM==0)

// *******************************************************************************************************************************
// wechselt verzeichniss. start immer im root Dir.
// MUSS in das direktory gewechselt werden, in dem die datei zum lesen/schreiben ist !
// *******************************************************************************************************************************
unsigned char ffcd(char name[]){		
	return(fat_cd(name));
}


// *******************************************************************************************************************************
// zeigt reihen eines clusters an, wird für ffls benötigt !
// es wird ab dem start sektor start_sec, der dazugehörige cluster angezeigt. geprüft wird ob es ein richtiger 
// eintrag in der reihe ist (nicht gelöscht, nicht frei usw). die sektoren des clusters werden nachgeladen.
// die dateien werden mit namen und datei größe angezeigt.
// *******************************************************************************************************************************
void lsRowsOfClust (unsigned long int start_sec){

  unsigned char row;						// reihen
  unsigned char sec=0;					// sektoren 
  do{
	fat_loadSector(start_sec + sec);			// sektoren des clusters laden		
	for(row=0;row<16;row++){					// geht durch reihen des sektors	
	  fat_loadRowOfSector(row);				// reihe eines sektors (auf dem puffer) laden				
	  if( (file.attrib==0x20||file.attrib==0x10) && (file.name[0]!=0xE5 && file.name[0]!=0x00) ){	 		  
          printf("Name:%s Size:%li\n",file.name,file.length );
		  }
	  }	
	}while(++sec<fat.secPerClust);
}


// *******************************************************************************************************************************
// zeigt inhalt eines direktory an.
// unterscheidung ob man sich im rootDir befindet nötig, weil bei fat16 im root dir eine bestimmt anzahl sektoren durchsucht
// werden müssen und bei fat32 ab einem start cluster ! ruft lsRowsOfClust auf um cluster/sektoren anzuzeigen.
// *******************************************************************************************************************************
void ffls(void){

  unsigned long int clust;									// cluster
  unsigned int s;												// fat16 root dir sektoren

  if(fat.dir==0 && fat.fatType==16){							// IM ROOTDIR.	fat16		
	for(s=0;s<(unsigned int)(fat.dataDirSec+2-fat.rootDir);s++){	// zählt durch RootDir sektoren (errechnet anzahl rootDir sektoren).	
	  lsRowsOfClust(fat.rootDir+s);								// zeigt reihen eines root dir clust an
	  }
	}

  else {		
	if(fat.dir==0 && fat.fatType==32)clust=fat.rootDir;			// IM ROOTDIR.	fat32
	else clust=fat.dir;										// NICHT ROOT DIR
	while( !((clust==0xfffffff&&fat.fatType==32)||(clust==0xffff&&fat.fatType==16)) ){	// prüft ob weitere sektoren zum lesen da sind (fat32||fat16)	
	  lsRowsOfClust(fat_clustToSec(clust));												// zeigt reihen des clusters an
	  clust=fat_getNextCluster(clust);													// liest nächsten cluster des dir-eintrags
	  }
	}  
}



//*******************************************************************************************************************************
// wechselt in das parent verzeichniss (ein verzeichniss zurück !)
// die variable fat.dir enthält den start cluster des direktory in dem man sich grade befindet, anhand diesem,
// kann der "." bzw ".." eintrag im ersten sektor des direktory ausgelesen und das parent direktory bestimmt werden.
//*******************************************************************************************************************************
unsigned char ffcdLower(void){

  if(fat.dir==0)return(1);				// im root dir, man kann nicht höher !

  fat_loadSector(fat_clustToSec(fat.dir));	// läd 1. sektor des aktuellen direktory.
  fat_loadRowOfSector(1);					// ".." eintrag (parent dir) ist 0 wenn parent == root
  fat.dir=file.firstCluster;				// dir setzen
	
  return(0);
}


#ifndef __AVR_ATmega8__ 

// *******************************************************************************************************************************
// erstellt einen dir eintrag im aktuellen verzeichniss.
// prüft ob es den den dir-namen schon gibt, dann wird nichts angelegt.
// wenn ok, dann wird ein freier cluster gesucht, als ende markiert, der eintrag ins dir geschrieben.
// dann wird der cluster des dirs aufbereitet. der erste sektor des clusters enthält den "." und ".." eintrag.
// der "." hat den 1. cluster des eigenen dirs. der ".." eintrag ist der 1. cluster des parent dirs.
// ein dir wird immer mit 0x00 initialisiert ! also alle einträge der sektoren des clusters ( bis auf . und .. einträge)!
// *******************************************************************************************************************************
#if (WRITE==1)

void ffmkdir(char name[]){

  unsigned char i;
  unsigned int j;

  if(0==fat_loadFileDataFromDir(name))return ;	// prüft ob dirname im dir schon vorhanden, wenn ja, abbruch !

  // cluster in fat setzen, und ordner eintrg im aktuellen verzeichniss machen.
  fat_getFreeClustersInRow(2);										// holt neue freie cluster, ab cluster 2 ...
  fat_setCluster(fat_secToClust(fat.startSectors),0x0fffffff);		// fat16/32 cluster chain ende setzen.	(neuer ordner in fat)
  file.firstCluster=fat_secToClust(fat.startSectors);				// dammit fat_makeFileEntry den cluster richtig setzen kann
  fat_makeFileEntry(name,0x10,0); 			// macht dir eintrag im aktuellen verzeichniss (legt ordner im partent verzeichniss an)

  // aufbereiten des puffers
  j=511;
  do{
	fat.sector[j]=0x00;						//schreibt puffer fat.sector voll mit 0x00==leer
	}while(j--);

  // aufbereiten des clusters
  for(i=1;i<fat.secPerClust;i++){			// ein dir cluster muss mit 0x00 initialisiert werden !
	fat_writeSector(fat.startSectors+i);	// löschen des cluster (überschreibt mit 0x00), wichtig bei ffls!
	} 

  // aufbereiten des neuen dir sektors mit "." und ".." eintraegen !
  fat_makeRowDataEntry(0,".           ",0x10,file.firstCluster,0);	// macht "." eintrag des dirs
  fat_makeRowDataEntry(1,"..          ",0x10,fat.dir,0);			// macht ".." eintrag des dirs
  fat_writeSector(fat_clustToSec(file.firstCluster));				// schreibt einträge auf karte !  
}
#endif //WRITE 
#endif		// ffmkdir wegen atmega8
#endif



#if (WRITE==1)

//*******************************************************************************************************************************
// löscht datei/ordner aus aktuellem verzeichniss, wenn es die datei gibt.
// löscht dateien und ordner rekursiv !
//*******************************************************************************************************************************
unsigned char ffrm(char name[]){ 

  if(0==fat_loadFileDataFromDir(name)){	// datei oder ordner ist vorhanden, nur dann lösch operation

	if(file.attrib!=0x10){				// ist datei.
	  fat.sector[file.row<<5]=0xE5;		// datei gelöscht markieren (file.row*32, damit man auf reihen anfang kommt)
	  if((file.row-1)>=0){				// gibt es einen eintrag davor ?
		  if(fat.sector[(file.row<<5)-21]==0x0f)fat.sector[(file.row<<5)-32]=0xE5;	// langer datei eintag auch gelöscht.
		  }
	  fat.bufferDirty=1;				// eintrag in puffer gemacht.
	  if(0==fat_getNextCluster(file.firstCluster)) return(0);	// 1.cluster ist leer ?!?
	  fat_delClusterChain(file.firstCluster);		// löscht die zu der datei gehörige cluster-chain aus der fat.
	  return(0);
	  }
 
  //TODO noch nicht optimal. zu viele schreib vorgänge beim löschen von datei einträgen. max bis zu 16/sektor !
  else{							// ist ordner, dann rekursiv löschen !!
	#ifndef __AVR_ATmega8__			// mega8 zu klein für die funktionalität....
	unsigned long int parent;
	unsigned long int own;
	unsigned long int clustsOfDir;		// um durch die cluster chain eines dirs zu gehen.
	unsigned char cntSecOfClust=0;
	unsigned char i=0;

	fat.sector[file.row<<5]=0xE5;			// löscht dir eintrag
		if((file.row-1)>=0){						// gibt es einen eintrag davor (langer datei eintrag)?
		  if(fat.sector[(file.row<<5)-21]==0x0f)fat.sector[(file.row<<5)-32]=0xE5;	// langer datei eintag auch gelöscht
		  }
		fat.bufferDirty=1;									// puffer eintrag gemacht

		parent=fat.dir;										// der ordner in dem der zu löschende ordner ist.
		own=file.firstCluster;								// der 1. cluster des ordners der zu löschen ist.
		clustsOfDir=file.firstCluster;						// die "cluster" des zu löschenden ordners

		do{													// geht durch cluster des dirs
		  fat_loadSector(fat_clustToSec(clustsOfDir));		// sektor des dirs laden
		  do{												// geht durch sektoren des clusters
			do{	  											// geht durch reihen des sektors
			  fat_loadRowOfSector(i);

			  if(file.attrib!=0x10 && file.attrib!=0x00 && file.name[0]!=0xE5){	// ist kein ordner,noch nicht gelöscht, kein freier eintrag
				fat.sector[i<<5]=0xE5;						// erster eintrag der reihe als gelöscht markiert
				fat.bufferDirty=1;							// puffer eintrag gemacht
				if(file.attrib==0x20){						// ist datei!
				  fat_delClusterChain(file.firstCluster);	// ist datei, dann cluster-chain der datei löschen
				  fat_loadSector(fat_clustToSec(clustsOfDir)+cntSecOfClust);	// sektor neu laden, weil löschen der chain, den puffer nutzt.
				  }
				}

			  if(file.attrib==0x10 && file.name[0]=='.'){	// "." oder ".." eintrag erkannt, löschen !
				fat.sector[i<<5]=0xE5;						// eintrag als gelöscht markiert
				fat.bufferDirty=1;							// puffer eintrag gemacht
				}

			  if(file.attrib==0x10 && file.name[0]!='.' && file.name[0]!=0xE5 && file.name[0]!=0){	// ordner erkannt !
				fat.sector[i<<5]=0xE5;						// dir eintrag als gelöscht markiert
				fat.bufferDirty=1;							// puffer eintrag gemacht
				fat_loadSector(fat_clustToSec(file.firstCluster));	// sektor des dirs laden
				clustsOfDir=file.firstCluster;				// eigenes dir ist file.firstCluster
				own=file.firstCluster;						// eigener start cluster/dir
				fat_loadRowOfSector(1);						// ".." laden um parent zu bestimmen
				parent=file.firstCluster;					// parent sichern.
				cntSecOfClust=0;							// init von gelesenen sektoren und reihen !
				i=0;
				continue;
				}

			  if(file.name[0]==0x00){						// ende des dirs erreicht, wenn nicht voll !!
				if(parent==fat.dir){						// erfolgreich alles gelöscht
				  fat_delClusterChain(own);					// cluster chain des ordners löschen
				  return(0);
				  }
				fat_delClusterChain(own);					// cluster chain des ordners löschen
				clustsOfDir=parent;							// parent ist jetzt wieder arbeisverzeichniss.
				own=parent;									// arbeitsverzeichniss setzten
				fat_loadSector(fat_clustToSec(own));		// sektor des dirs laden
				fat_loadRowOfSector(1);						// ".." laden um parent zu bestimmen
				parent=file.firstCluster;	  		  		// parent sichern.
				cntSecOfClust=0;							// init von gelesenen sektoren und reihen !
				i=0;
				continue;
				}

			  i++;
			  }while(i<16);								// geht durch reihen des sektors.
		
			i=0;											// neuer sektor -> reihen von vorne.
			cntSecOfClust++;
			fat_loadSector(fat_clustToSec(clustsOfDir)+cntSecOfClust);		// läd sektoren des clusters nach
			}while(cntSecOfClust<fat.secPerClust);							// geht durch sektoren des clusters.
	
		  cntSecOfClust=0;													// neuer cluster -> sektoren von vorne.
		  clustsOfDir=fat_getNextCluster(clustsOfDir);						// sucht neuen cluster der cluster-chain.
		  }while( !((clustsOfDir==0xfffffff && fat.fatType==32) || (clustsOfDir==0xffff && fat.fatType==16)) );		// geht durch cluster des dirs.
		  fat_delClusterChain(own);				// hier landet man, wenn der ordner voll war (auf cluster "ebene")!!
		  #endif
	  }
	
  }
  

  return(1);				// fehler, nicht gefunden?
}

#endif


//*******************************************************************************************************************************
// liest 512 bytes aus dem puffer fat.sector. dann werden neue 512 bytes der datei geladen, sind nicht genügend verkettete
// sektoren in einer reihe bekannt, wird in der fat nachgeschaut. dann werden weiter bekannte nachgeladen...
//*******************************************************************************************************************************
inline unsigned char ffread(void){  	 

  if(file.cntOfBytes==512){								/** EINEN SEKTOR GLESEN (ab hier 2 möglichkeiten !)		**/
	 file.cntOfBytes=0;											// byte zähler zurück setzen		 
	 if( fat.currentSectorNr == fat.endSectors ){		/** 1.) nötig mehr sektoren der chain zu laden (mit ein bisschen glück nur alle 512*MAX_CLUSTERS_IN_ROW bytes)**/
		fat_getFatChainClustersInRow(fat_getNextCluster(fat_secToClust( fat.endSectors) ));	// nachladen von clustern in der chain
		fat.currentSectorNr=fat.startSectors-1;					// setzen des nächsten sektors um weiter lesen zu können..
		}
	 fat_loadSector(fat.currentSectorNr+1);				/** 2.) die bekannten in einer reihe reichen noch.(nur alle 512 bytes)**/
	 }	  
  
  return fat.sector[file.cntOfBytes++];						// rückgabe, byte des sektors		(NACH rückgabe erhöhen von zähler ! )
}


#if (WRITE==1)
#if (OVER_WRITE==0)			// nicht überschreibende write funktion

//*******************************************************************************************************************************
// schreibt 512 byte blöcke auf den puffer fat.sector. dann wird dieser auf die karte geschrieben. wenn genügend feie
// sektoren zum beschreiben bekannt sind(datenmenge zu groß), muss nicht in der fat nachgeschaut werden. sollten nicht genügend
// zusammenhängende sektoren bekannt sein, werden die alten verkettet und neue gesucht. es ist nötig sich den letzten bekannten
// einer kette zu merken -> file.lastCluster, um auch nicht zusammenhängende cluster verketten zu können (fat_setClusterChain macht das)!
//*******************************************************************************************************************************
inline void ffwrite(unsigned char c){
	
  fat.sector[file.cntOfBytes++]=c;								// schreiben des chars auf den puffer sector und zähler erhöhen (pre-increment)  
  fat.bufferDirty=1;											// puffer dirty weil geschrieben und noch nicht auf karte.

  if( file.cntOfBytes==512 ){								/** SEKTOR VOLL ( 2 möglichkeiten ab hier !) **/
	 file.cntOfBytes=0;	  											// rücksetzen des sektor byte zählers	 
 	 mmc_write_sector(fat.currentSectorNr,fat.sector);		/** 1.) vollen sektor auf karte schreiben **/
 	 fat.bufferDirty=0;	 											// puffer jetzt clear, weil grade alles geschrieben.
	 file.seek+=512;												// position in der datei erhöhen, weil grade 512 bytes geschrieben
	 if( fat.currentSectorNr==fat.endSectors ){				/** 2.) es ist nötig, neue freie zu suchen und die alten zu verketten (mit ein bischen glück nur alle 512*MAX_CLUSTERS_IN_ROW bytes) **/
		fat_setClusterChain( fat_secToClust(fat.startSectors) , fat_secToClust(fat.endSectors) );	// verketten der beschriebenen
  		fat_getFreeClustersInRow(file.lastCluster);					// suchen von leeren sektoren.
		fat.currentSectorNr=fat.startSectors-1;						// setzen des 1. sektors der neuen reihe zum schreiben.
		}	 	 
	 fat.currentSectorNr++;											// nächsten sektor zum beschreiben.
	 } 	
}

#endif

#if (OVER_WRITE==1)			// überschreibende write funktion, nicht performant, weil immer auch noch ein sektor geladen werden muss

//*******************************************************************************************************************************
// schreibt 512 byte blöcke auf den puffer fat.sector. dann wird dieser auf die karte geschrieben. wenn genügend feie
// sektoren zum beschreiben bekannt sind, muss nicht in der fat nachgeschaut werden. sollten nicht genügend zusammenhängende 
// sektoren bekannt sein(datenmenge zu groß), werden die alten verkettet und neue gesucht. es ist nötig sich den letzten bekannten einer
// kette zu merken -> file.lastCluster, um auch nicht zusammenhängende cluster verketten zu können (fat_setClusterChain macht das)!
// es ist beim überschreiben nötig, die schon beschriebenen sektoren der datei zu laden, damit man die richtigen daten
// hat. das ist blöd, weil so ein daten overhead von 50% entsteht. da lesen aber schneller als schreiben geht, verliert man nicht 50% an geschwindigkeit.
//*******************************************************************************************************************************
inline void ffwrite(unsigned char c){
	
  fat.sector[file.cntOfBytes++]=c;								// schreiben des chars auf den puffer sector und zähler erhöhen (pre-increment)  
  fat.bufferDirty=1;											// puffer dirty weil geschrieben und noch nicht auf karte.

  if( file.cntOfBytes==512 ){								/** SEKTOR VOLL ( 2 möglichkeiten ab hier !) **/
	 file.cntOfBytes=0;	  											// rücksetzen des sektor byte zählers. 
	 mmc_write_sector(fat.currentSectorNr,fat.sector);		/** 1.) vollen sektor auf karte schreiben**/
 	 fat.bufferDirty=0;	 											// puffer jetzt clear, weil grade alles geschrieben.
	 file.seek+=512;												// position in der datei erhöhen, weil grade 512 bytes geschrieben.
	 if( fat.currentSectorNr==fat.endSectors ){				/** 2.) es ist nötig, neue freie zu suchen und die alten zu verketten (mit ein bischen glück nur alle 512*MAX_CLUSTERS_IN_ROW bytes) **/
  		if( file.seek > file.length ){								// außerhalb der datei !!
		  fat_setClusterChain( fat_secToClust(fat.startSectors) , fat_secToClust(fat.endSectors) );	// verketten der beschriebenen.  		
		  fat_getFreeClustersInRow( file.lastCluster );				// neue leere sektoren benötigt, also suchen.
  		  }
		else {
		  fat_getFatChainClustersInRow( fat_getNextCluster(file.lastCluster) );		// noch innerhalb der datei, deshlab verkettete suchen.
		  }
		fat.currentSectorNr=fat.startSectors-1;						// setzen des 1. sektors der neuen reihe zum schreiben.
		}
	 fat.currentSectorNr++;											// nächsten sektor zum beschreiben.
	 mmc_read_sector(fat.currentSectorNr,fat.sector);				// wegen überschreiben, muss der zu beschreibende sektor geladen werden...
	 } 	
}

#endif

// *******************************************************************************************************************************
// schreibt string auf karte, siehe ffwrite()
// *******************************************************************************************************************************
inline void ffwrites(const char *s ){
    while (*s) ffwrite(*s++);
  }

#endif








