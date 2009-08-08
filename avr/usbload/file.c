
#include <string.h>	
#include <stdlib.h>

#include "hardware.h"
#include "fat.h"
#include "file.h"

//*******************************************************************************************************************************
// 2 möglichkeiten beim öffnen, datei existiert oder muss angelegt werden
//*******************************************************************************************************************************
unsigned char ffopen(char name[]){		

  unsigned char file_flag=fat_loadFileDataFromDir(name);			//prüfung ob datei vorhanden und evetuelles laden des file struct						
 
  if( file_flag==0 ){				/** Datei existiert, anlegen nicht nötig! **/
	 fat_getClustersInRow( file.firstCluster,1 );						// cluster chain suchen
	 fat_loadSector( fat_clustToSec(file.firstCluster) );			// lät die ersten 512 bytes der datei auf puffer:sector.	
	 file.lastCluster=file.firstCluster;								// setzen des arbeis clusters								
	 return 1;
	 }
  #if (write==1)
  else{								/** Datei existiert nicht, also anlegen !	(nur wenn schreiben option an ist)**/
	 fat_getClustersInRow(3,0);									// leere cluster suchen	 
	 strcpy((char*)file.name,(char*)name);						// ---	füllen des file struct, zum abschließenden schreiben.
	 file.firstCluster=fat_secToClust(fat.startSectors);	// 		1. cluster der datei.
	 file.lastCluster=file.firstCluster;						//			letzter bekannter cluster der datei			
	 file.attrib=32;													// ---	file.row wird in der funktion fat_getFreeRowOfDir geschrieben !! 		 
	 fat_makeFileEntry((char *)file.name,file.attrib,file.firstCluster,0);		// DATEI ANLEGEN
	 fat.currentSectorNr=fat_clustToSec(file.firstCluster);	// setzen des ersten sektors	
	 return 2;
	 }
	#endif
}


//*******************************************************************************************************************************
// schließt die datei operation ab. eigentlich nur nötig wenn geschrieben wurde.
// es gibt 2 möglichkeiten : 
// 1. die datei wird geschlossen und man war innerhalb der datei größe, dann muss nur der daten sektor geschrieben werden.
// 2. die datei wird geschlossen und es wurde über die alte datei länge hinaus geschrieben.
// der zweite fall ist komplizierter, weil ermittelt werden muss wie viele sektoren neu beschrieben wurden um diese zu verketten
// und die neue datei länge zu ermitteln. abschließend wird entweder (fall 1) nur der daten sektor geschrieben, oder
// der aktuallisierte datei eintrag (von setClusterChain wird ggf der dirty sector puffer geschrieben)
//*******************************************************************************************************************************
unsigned char ffclose(void){
  #if (write==1) 
  /** 2 möglichkeiten beim schließen !!	(lesend spielt keine rolle) **/

  if( file.length < (file.seek+file.cntOfBytes) ){		/** 1.) es wurde über die alte datei größe hinaus geschrieben **/																					
	 unsigned int comp_cntOfBytes=file.cntOfBytes;		// sicher nötig wegen schleife...
	 while( comp_cntOfBytes < 512 ){							// sektor ist beschrieben worden, daher nötigenfalls mit 00 füllen			
		fat.sector[comp_cntOfBytes]=0x00;					// beschreibt ungenutzte bytes mit 0x00 	
		comp_cntOfBytes++;			
		}		
	 
	 char name[13];																	// zum sichern des dateinamens
	 unsigned long int save_length = file.cntOfBytes + file.seek;		// muss gesichert werden, wird sonst von der karte geladen und verändert !	  
 	 strcpy(name,(char *)file.name);												// muss gesichert werden, wird sonst von der karte geladen und verändert !
	 
	 fat_setClusterChain(fat_secToClust(fat.startSectors),fat.currentSectorNr - fat.startSectors);	// verketten der geschriebenen cluster				
	 fat_loadFileDataFromDir(name);							// läd sektor, des datei eintrags, und läd daten von karte auf struct file!			 
	 fat_makeRowDataEntry(file.row,name,32,file.firstCluster,save_length);	 	// macht eintrag im puffer				

	 fat_writeSector(fat.currentSectorNr);								// abschließendes schreiben !				
	 }	

  else if( fat.bufferDirty==1) fat_writeSector( fat.currentSectorNr );			/** 2.) nicht über alte datei länge hinaus **/
  #endif
  
  file.cntOfBytes=0;					// init der nötigen zähler
  file.seek=0;
  return(0);	
}



// *******************************************************************************************************************************
// offset byte wird übergeben. es wird durch die sektoren der datei gespult, bis der sektor mit dem offset byte erreicht ist,
// dann wird der sektor geladen und der zähler für die bytes eines sektors gesetzt. wenn das byte nicht in den sektoren ist,
// die vorgeladen wurden, müssen noch weitere sektoren der datei gesucht werden.
// *******************************************************************************************************************************
void ffseek(unsigned long int offset){		
  
  fat_getClustersInRow(file.firstCluster,1);			// suchen von anfang der cluster chain aus !
  unsigned long int sec=fat.startSectors;				// setzen des 1. sektors der neu gesuchten  								// sektor variable zum durchgehen durch die sektoren  

  do{ 	 													/** 3 möglichkeiten ab hier ! **/
	 if(offset>=512){											/** 1. offset ist in nächstem sektor **/
		sec++;													// da byte nicht in diesem sektor ist, muss hochgezählt werden
		offset-=512;											// ein sektor weniger in dem das byte sein kann
		file.seek+=512;										// file.seek update, damit bei ffclose() die richtige file.length herauskommt
		}
	 else break; 												/** 2. sektor in dem offset ist, ist gefunden ! **/

	 if ( sec == fat.endSectors ){						/** 3. es müssen mehr sektoren der datei gesucht werden  **/
		fat_getClustersInRow(fat_getNextCluster( file.lastCluster ),1 );	// nachladen von clustern in der chain
		sec=fat.startSectors;										// setzen des 1. sektors der neu gesuchten
		} 
				
	 }while(1);

  file.lastCluster=fat_secToClust(sec);	// letzter bekannter cluster der datei
  fat_loadSector(sec);  						// sektor mit offset byte laden
  file.cntOfBytes = offset % 512;			// setzen des lese zählers   
}




 #if (smallFileSys==0)

//***************************************************************************************PhysFS****************************************
// wechselt verzeichniss. start immer im root Dir.
// MUSS in das direktory gewechselt werden, in dem die datei zum lesen/schreiben ist !
//*******************************************************************************************************************************
unsigned char ffcd(char name[]){		
	return(fat_cd(name));
}


//*******************************************************************************************************************************
// zeigt reihen eines clusters an, wird für ffls benötigt !
// es wird ab dem start sektor start_sec, der dazugehörige cluster angezeigt. geprüft wird ob es ein richtiger 
// eintrag in der reihe ist (nicht gelöscht, nicht frei usw). die sektoren des clusters werden nachgeladen.
// die dateien werden mit namen und datei größe angezeigt.
//*******************************************************************************************************************************
void lsRowsOfClust (unsigned long int start_sec){

  unsigned char row;						// reihen
  unsigned char sec=0;					// sektoren 
  
  do{
	fat_loadSector(start_sec + sec);			// sektoren des clusters laden		
	for(row=0;row<16;row++){					// geht durch reihen des sektors	
	  fat_loadRowOfSector(row);				// reihe eines sektors (auf dem puffer) laden				
	  if((file.attrib==0x20||file.attrib==0x10) && (file.name[0]!=0xE5&&file.name[0]!=0x00)){	 
          printf("%s  %li\n",file.name, file.length);
		  }
	  }	
	}while(++sec<fat.secPerClust);
}


//*******************************************************************************************************************************
// zeigt inhalt eines direktory an.
// unterscheidung ob man sich im rootDir befindet nötig, weil bei fat16 im root dir eine bestimmt anzahl sektoren durchsucht
// werden müssen und bei fat32 ab einem start cluster ! ruft lsRowsOfClust auf um cluster/sektoren anzuzeigen.
//*******************************************************************************************************************************
unsigned char ffls(void){

  unsigned long int clust;											// cluster
  unsigned int s;														// fat16 root dir sektoren

  if(fat.dir==0 && fat.fatType==16){							// IM ROOTDIR.	fat16		
	for(s=0;s<(unsigned int)(fat.dataDirSec+2-fat.rootDir);s++){	// zählt durch RootDir sektoren (errechnet anzahl rootDir sektoren).	
	  lsRowsOfClust(fat.rootDir+s);								// zeigt reihen eines root dir clust an
	  }
	}

  else {		
	if(fat.dir==0 && fat.fatType==32)clust=fat.rootDir;	// IM ROOTDIR.	fat32		
	else clust=fat.dir;												// NICHT ROOT DIR																
	while( !((clust==0xfffffff&&fat.fatType==32)||(clust==0xffff&&fat.fatType==16)) ){	// prüft ob weitere sektoren zum lesen da sind (fat32||fat16)	
	  lsRowsOfClust(fat_clustToSec(clust));															// zeigt reihen des clusters an
	  clust=fat_getNextCluster(clust);																	// liest nächsten cluster des dir-eintrags
	  }
	}
  return(0);
}



//*******************************************************************************************************************************
// wechselt in das parent verzeichniss (ein verzeichniss zurück !)
// die variable fat.dir enthält den start cluster des direktory in dem man sich grade befindet, anhand diesem,
// kann der "." bzw ".." eintrag im ersten sektor des direktory ausgelesen und das parent direktory bestimmt werden.
//*******************************************************************************************************************************
unsigned char ffcdLower(void){

  if(fat.dir==0)return(1);							// im root dir, man kann nicht höher !

  fat_loadSector(fat_clustToSec(fat.dir));	// läd 1. sektor des aktuellen direktory.
  fat_loadRowOfSector(1);							// ".." eintrag (parent dir) ist 0 wenn parent == root
  fat.dir=file.firstCluster;						// dir setzen
	
  return(0);
}



// *******************************************************************************************************************************
// erstellt einen dir eintrag im aktuellen verzeichniss (geht nur wenn keine lese/schreib usw. vorgänge).
// prüft ob es den den dir-namen:name schon gibt, dann wird nichts angelegt.
// wenn ok, dann wird ein freier cluster gesucht, als ende markiert, der eintrag ins dir geschrieben.
// dann wird der cluster des dirs aufbereitet. der erste sektor des clusters enthält den "." und ".." eintrag.
// der "." hat den 1. cluster des eigenen dirs. der ".." eintrag ist der 1. cluster des parent dirs.
// ein dir wird immer mit 0x00 initialisiert ! also alle sektoren des clsters ( bis auf . und .. einträge)!
// *******************************************************************************************************************************
unsigned char ffmkdir(char name[]){
#ifndef __AVR_ATmega8__ 
    unsigned char i;
  if(0==fat_loadFileDataFromDir(name))return (1);	// prüft ob dirname im dir schon vorhanden, wenn ja, abbruch !			

  fat_getClustersInRow(2,0);				// holt neue freie cluster, ab cluster 2 ...  		  
  fat_setCluster(fat_secToClust(fat.startSectors),0x0fffffff);			// fat16/32 cluster chain ende setzen.	(neuer ordner in fat)

  fat_makeFileEntry(name,0x10,fat_secToClust(fat.startSectors),0); 	// macht dir eintrag im aktuellen verzeichniss (legt ordner im partent verzeichniss an)	

  // aufbereiten des neuen dir clusters !
  fat_markSector00();												// löschen des sektors (nur im puffer)./**/
//   fat_makeRowDataEntry(0,(char *)str_p(PSTR(".           ")),0x10,fat_secToClust(fat.currentSectorNr),0);	// macht "." eintrag des dirs
//   fat_makeRowDataEntry(1,(char *)str_p(PSTR("..          ")),0x10,fat.dir,0);											// macht ".." eintrag des dirs
  fat_makeRowDataEntry(0,".           ",0x10,fat_secToClust(fat.currentSectorNr),0);		// macht "." eintrag des dirs
  fat_makeRowDataEntry(1,"..          ",0x10,fat.dir,0);			// macht ".." eintrag des dirs
  fat_writeSector(fat_clustToSec(fat.startSectors));					// schreibt einträge auf karte !

  fat_markSector00();										// löschen des sektors (nur im puffer).  
  
  for(i=1;i<fat.secPerClust;i++){		// ein dir cluster muss mit 0x00 initialisiert werden !
	fat_writeSector(fat.startSectors+i);				// löschen des cluster (überschreibt mit 0x00), wichtig bei ffls!
	} 
#endif
  return(0);

}

#endif



#if (write==1)
//*******************************************************************************************************************************
// löscht datei aus aktuellem verzeichniss, wenn es die datei gibt.
// löscht dateien und ordner rekursiv !
//*******************************************************************************************************************************
unsigned char ffrm(char name[]){

 

  if(0==fat_loadFileDataFromDir(name)){	// datei/ordner gibt es
	if(file.attrib!=0x10){						// ist datei.	
	  fat.sector[file.row<<5]=0xE5;			// datei gelöscht markieren (file.row*32, damit man auf reihen anfang kommt)
	  if((file.row-1)>=0){						// gibt es einen eintrag davor ?
	  if(fat.sector[(file.row<<5)-21]==0x0f)fat.sector[(file.row<<5)-32]=0xE5;	// langer datei eintag auch gelöscht.
		}	
	  fat.bufferDirty=1;									// eintrag in puffer gemacht. 	  
	  if(0==fat_getNextCluster(file.firstCluster)) return(0);	// 1.cluster ist leer ?!?
	  fat_delClusterChain(file.firstCluster);		// löscht die zu der datei gehörige cluster-chain aus der fat.
	  return(0);
	  }
 
  //TODO noch nicht optimal. zu viele schreib vorgänge beim löschen von datei einträgen. max bis zu 16/sektor !
  else{									// ist ordner, rekursiv löschen !!
  #ifndef __AVR_ATmega8__			// mega8 zu klein für die funktionalität....
	unsigned long int parent;
	unsigned long int own;
	unsigned long int clustsOfDir;		// um durch die cluster chain eines dirs zu gehen.
	unsigned long int	cntSecOfClust=0;
	unsigned char i=0;

	fat.sector[file.row<<5]=0xE5;		// löscht dir eintrag
	if((file.row-1)>=0){			// gibt es einen eintrag davor (langer datei eintrag)?
	  if(fat.sector[(file.row<<5)-21]==0x0f)fat.sector[(file.row<<5)-32]=0xE5;	// langer datei eintag auch gelöscht
	  }
	fat.bufferDirty=1;					// puffer eintrag gemacht
		
	parent=fat.dir;									// der ordner in dem der zu löschende ordner ist.
	own=file.firstCluster;							// der 1. cluster des ordners der zu löschen ist.
	clustsOfDir=file.firstCluster;				// die "cluster" des zu löschenden ordners	

	do{													// geht durch cluster des dirs							
	  fat_loadSector(fat_clustToSec(clustsOfDir));		// sektor des dirs laden
	  do{												// geht durch sektoren des clusters
		do{	  											// geht durch reihen des sektors
		  fat_loadRowOfSector(i);

		  if(file.attrib!=0x10 && file.attrib!=0x00 && file.name[0]!=0xE5){	// ist kein ordner,noch nicht gelöscht, kein freier eintrag
			fat.sector[i<<5]=0xE5;									// erster eintrag der reihe als gelöscht markiert		
			fat.bufferDirty=1;										// puffer eintrag gemacht
			if(file.attrib==0x20){								// ist datei!
			  fat_delClusterChain(file.firstCluster);						// ist datei, dann cluster-chain der datei löschen
			  fat_loadSector(fat_clustToSec(clustsOfDir)+cntSecOfClust);	// sektor neu laden, weil löschen der chain, den puffer nutzt.			
			  }
			}

		  if(file.attrib==0x10&&file.name[0]=='.'){				// "." oder ".." eintrag erkannt, löschen !
			fat.sector[i<<5]=0xE5;									// eintrag als gelöscht markiert		
			fat.bufferDirty=1;										// puffer eintrag gemacht		  
			}

		  if(file.attrib==0x10&&file.name[0]!='.'&&file.name[0]!=0xe5&&file.name[0]!=0){	// ordner erkannt !
			fat.sector[i<<5]=0xE5;									// dir eintrag als gelöscht markiert		
			fat.bufferDirty=1;										// puffer eintrag gemacht		  
			fat_loadSector(fat_clustToSec(file.firstCluster));	// sektor des dirs laden		  
			clustsOfDir=file.firstCluster;						// eigenes dir ist file.firstCluster
			own=file.firstCluster;								// eigener start cluster/dir
			fat_loadRowOfSector(1);								// ".." laden um parent zu bestimmen
			parent=file.firstCluster;							// parent sichern.		  
			cntSecOfClust=0;									// init von gelesenen sektoren und reihen !
			i=0;
			continue;
			}

		  if(file.name[0]==0x00){								// ende des dirs erreicht, wenn nicht voll !!				  
			if(parent==fat.dir){								// erfolgreich alles gelöscht
			  fat_delClusterChain(own);							// cluster chain des ordners löschen
			  return(0);
			  }
			fat_delClusterChain(own);							// cluster chain des ordners löschen
			clustsOfDir=parent;									// parent ist jetzt wieder arbeisverzeichniss.		 
			own=parent;											// arbeitsverzeichniss setzten
			fat_loadSector(fat_clustToSec(own));				// sektor des dirs laden
			fat_loadRowOfSector(1);								// ".." laden um parent zu bestimmen
			parent=file.firstCluster;		  		  			// parent sichern.
			cntSecOfClust=0;									// init von gelesenen sektoren und reihen !
			i=0;
			continue;
			}
	  
		  i++;
		  }while(i<16);													// geht durch reihen des sektors.
	
		i=0;															// neuer sektor -> reihen von vorne.
		cntSecOfClust++;	  											
		fat_loadSector(fat_clustToSec(clustsOfDir)+cntSecOfClust);		// läd sektoren des clusters nach
		}while(cntSecOfClust<fat.secPerClust);							// geht durch sektoren des clusters.

	  cntSecOfClust=0;													// neuer cluster -> sektoren von vorne.
	  clustsOfDir=fat_getNextCluster(clustsOfDir);						// sucht neuen cluster der cluster-chain.	  
	  }while(!((clustsOfDir==0xfffffff&&fat.fatType==32)||(clustsOfDir==0xffff&&fat.fatType==16)));		// geht durch cluster des dirs.
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

  if(file.cntOfBytes==512){									/** EINEN SEKTOR GLESEN (ab hier 2 möglichkeiten !)		**/
	 file.cntOfBytes=0;											// byte zähler zurück setzen		 
	 if( fat.currentSectorNr == fat.endSectors-1 ){		/** 1.) nötig mehr sektoren der chain zu laden (mit ein bisschen glück nur alle 512*500=256000 bytes)**/																
		fat_getClustersInRow(fat_getNextCluster(fat_secToClust( fat.endSectors-1) ),1);	// nachladen von clustern in der chain
		fat.currentSectorNr=fat.startSectors-1;				// setzen des nächsten sektors um weiter lesen zu können..		
		}
	 fat_loadSector(fat.currentSectorNr+1);				/** 2.) die bekannten in einer reihe reichen noch.(nur alle 512 bytes)**/
	 }	  
  
  return fat.sector[file.cntOfBytes++];					// rückgabe, byte des sektors		(NACH rückgabe erhöhen von zähler ! )		
}


#if (write==1)
//*******************************************************************************************************************************
// schreibt 512 byte blöcke auf den puffer fat.sector. dann wird dieser auf die karte geschrieben. wenn genügend feie
// sektoren zum beschreiben bekannt sind, muss nicht in der fat nachgeschaut werden. sollten nicht genügend zusammenhängende 
// sektoren bekannt sein, werden die alten verkettet und neue gesucht (ist nötig sich den letzten bekannten zu merken -> file.lastCluster).
// es wird geprüft ob eine datei überschrieben wird (dann werden keine neuen sektoren verkettet), oder ob man über das
// dateiende hinaus schreibt. wichtige variabeln: fat.startSectors/fat.endSectors, fat.currentSectorNr und file.cntOfBytes
//*******************************************************************************************************************************
inline void ffwrite(unsigned char c){
	
  fat.sector[file.cntOfBytes++]=c;								// schreiben des chars auf den puffer sector und zähler erhöhen (pre-increment)  
  fat.bufferDirty=1;													// puffer dirty weil geschrieben und noch nicht auf karte.

  if( file.cntOfBytes==512 ){										/** SEKTOR GESCHRIEBEN ( 2 möglichkeiten ab hier !) **/
	 file.cntOfBytes=0;	  											// rücksetzen des sektor byte zählers	 
 	 mmc_write_sector(fat.currentSectorNr++,fat.sector);	/** 1.) vollen sektor auf karte schreiben und hochzählen von sektoren**/
 	 fat.bufferDirty=0;	 											// puffer jetzt clear, weil grade alles geschrieben.
	 file.seek+=512;													// position in der datei erhöhen, weil grade 512 bytes geschrieben

	 if( fat.currentSectorNr==fat.endSectors ){				/** 2.) es ist nötig, neue freie zu suchen und die alten zu verketten (mit ein bischen glück nur alle 512*500 bytes) **/			
  		unsigned char flag=1;
  		if( file.seek>file.length ){									// außerhalb der datei !!		
		  fat_setClusterChain(fat_secToClust(fat.startSectors),fat.endSectors-fat.startSectors);	// verketten der beschriebenen
  		  flag=0;
  		  }
		fat_getClustersInRow(file.lastCluster,flag);			// suchen der neuen sektoren.	
		fat.currentSectorNr=fat.startSectors;					// setzen des 1. sektors der neuen reihe zum schreiben.		  
		}	 
	 } 	
}


// *******************************************************************************************************************************
// schreibt string auf karte, siehe ffwrite()
// *******************************************************************************************************************************
inline void ffwrites(const char *s ){
    while (*s) ffwrite(*s++);
  }
#endif








