
#include <ctype.h>
#include <string.h>

#include "fat.h"
#include "file.h"
#include "mmc.h"
#include "config.h"

struct Fat fat;   					// wichtige daten/variablen der fat
struct File file;					// wichtige dateibezogene daten/variablen


#if (WRITE==1)

//***************************************************************************************************************
// schreibt sektor nummer:sec auf die karte (puffer:sector) !!
// setzt bufferFlag=0 da puffer nicht dirty sein kann nach schreiben !
//***************************************************************************************************************
unsigned char fat_writeSector(unsigned long int sec){	
 
	fat.bufferDirty=0;								// buffer kann nicht dirty sein weil wird geschrieben		
	return(mmc_write_sector(sec,fat.sector));		// schreiben von sektor puffer		
}

#endif


// ***************************************************************************************************************
// umrechnung cluster auf 1.sektor des clusters (möglicherweise mehrere sektoren/cluster) !
// ***************************************************************************************************************
unsigned long int fat_clustToSec(unsigned long int clust){

	return (fat.dataDirSec+2+((clust-2) * fat.secPerClust));					// errechnet den 1. sektor der sektoren des clusters
}

// ***************************************************************************************************************
// umrechnung sektor auf cluster (nicht die position im cluster selber!!)
// ***************************************************************************************************************
unsigned long int fat_secToClust(unsigned long int sec){
 
  return ((sec-fat.dataDirSec-2+2*fat.secPerClust)/fat.secPerClust);	// umkerhrfunktion von fat_clustToSec
}


//***************************************************************************************************************
// läd sektor:sec auf puffer:sector zum bearbeiten im ram !
// setzt currentSectorNr auf richtigen wert (also den sektor der gepuffert ist). es wird geprüft
// ob der gepufferte sektor geändert wurde, wenn ja muss erst geschrieben werden, um diese daten nicht zu verlieren !
//***************************************************************************************************************
unsigned char fat_loadSector(unsigned long int sec){
	
  if(sec!=fat.currentSectorNr){					// nachladen nötig			
	#if (WRITE==1)
	if(fat.bufferDirty==1) fat_writeSector(fat.currentSectorNr);	// puffer diry, also vorher schreiben		  		  
	#endif
	mmc_read_sector(sec,fat.sector);	// neuen sektor laden						  
	fat.currentSectorNr=sec;						// aktualisiert sektor nummer (nummer des gepufferten sektors)
	return(0);	
	}	
		
  else return(0);										// alles ok, daten sind schon da (sec==fat.currentSectorNr)	

  return(1);											// fehler	
}







// datei lesen funktionen:

// fat_loadSector -> fat_loadRowOfSector -> fat_loadFileDataFromCluster -> fat_loadFileDataFromDir -> fat_loadFileDataFromDir -> fat_cd   "daten chain"

//***************************************************************************************************************
// läd die reihe:row des gepufferten sektors auf das struct:file. dort stehen dann
// alle wichgigen daten wie: 1.cluster,länge bei dateien, name des eintrags, reihen nummer (im sektor), attribut use...
//***************************************************************************************************************
unsigned char fat_loadRowOfSector(unsigned int row){

	unsigned char i;
	row=row<<5;									// multipliziert mit 32 um immer auf zeilen anfang zu kommen (zeile 0=0,zeile 1=32,zeile 2=62 usw).

	void *vsector;								// void, damit man schoen umbiegen kann :)
	
	for(i=0;i<11;i++)
		file.name[i]=fat.sector[row+i];			// datei name, ersten 10 bytes vom 32 byte eintrag.

	file.attrib=fat.sector[row+11];							// datei attribut, byte 11.

	vsector=&fat.sector[row+26];							// low word von fist.cluster
	file.firstCluster=*(unsigned short*)vsector;
	
	vsector=&fat.sector[row+20];							// high word von first.cluster
	file.firstCluster|=(*(unsigned short*)vsector)<<16;

	vsector=&fat.sector[row+28];							// 4 byte von file.length
	file.length=*(unsigned long int*)vsector;

	return(0);				
}


//***************************************************************************************************************
// geht reihen weise durch sektoren des clusters mit dem startsektor:sec, und sucht nach der datei mit dem 
// namen:name. es werden die einzelnen sektoren nachgeladen auf puffer:sector vor dem bearbeiten.
// wird die datei in dem cluster gefunden ist return 0 , sonst return1.
//***************************************************************************************************************
unsigned char fat_loadFileDataFromCluster(unsigned long int sec , char name[]){
	
  unsigned char r;  
  unsigned char s=0;

  do{										// sektoren des clusters prüfen
	r=0;									// neuer sektor, dann reihen von 0 an.
	mmc_read_sector(sec+s,fat.sector);		// läd den sektor sec auf den puffer fat.sector
	fat.currentSectorNr=sec+s;				// setzen des aktuellen sektors
	  do{									// reihen des sektors prüfen
		fat_loadRowOfSector(r);				// zeile 0-15 auf struct file laden
		if(file.name[0]==0){				// wenn man auf erste 0 stößt müsste der rest auch leer sein!
			file.row=r;						// zeile sichern.
			return(1);
			}
		if(0==strncmp((char*)file.name,name,10)){		// zeile r ist gesuchte		
		  file.row=r;					// zeile sichern.
		  return(0);			
		  }
		r++;
	  }while(r<16);					// zählt zeilennummer (16(zeilen) * 32(spalten) == 512 bytes des sektors)
	s++;
	}while(s<fat.secPerClust);			// geht durch sektoren des clusters

	return (1);						// fehler (datei nicht gefunden, oder fehler beim lesen)
}


//***************************************************************************************************************
// wenn dir == 0 dann wird das root direktory durchsucht, wenn nicht wird der ordner cluster-chain gefolgt, um
// die datei zu finden. es wird das komplette directory in dem man sich befindet durchsucht.
// bei fat16 wird der rootDir berreich durchsucht, bei fat32 die cluster chain des rootDir.
//***************************************************************************************************************
unsigned char fat_loadFileDataFromDir(char name[]){ 

  unsigned int s;

  if(fat.dir==0 && fat.fatType==16){									// IM ROOTDIR.	fat16	
	for(s=0;s<(unsigned int)(fat.dataDirSec+2-fat.rootDir);s++){		// zählt durch RootDir sektoren (errechnet anzahl rootDir sektoren).
	  if(0==fat_loadFileDataFromCluster(fat.rootDir+s,name))return(0);// sucht die datei, wenn da, läd daten (1.cluster usw)
	  }
	}		
	
  else {		
	unsigned long int i;
	if(fat.dir==0 && fat.fatType==32)i=fat.rootDir;					// IM ROOTDIR.	fat32		
	else i=fat.dir;															// NICHT ROOTDIR																											
	while(!((i==0xfffffff&&fat.fatType==32)||(i==0xffff&&fat.fatType==16))){	// prüft ob weitere sektoren zum lesen da sind (fat32||fat16)
	  if(0==fat_loadFileDataFromCluster(fat_clustToSec(i),name))return(0);	// lät die daten der datei auf struct:file. datei gefunden (umrechnung auf absoluten sektor)
	  i=fat_getNextCluster(i);													// liest nächsten cluster des dir-eintrags (unterverzeichniss größer 16 einträge)
	  }
	}

  return(1);																	// datei/verzeichniss nicht gefunden
}


#if (SMALL_FILE_SYSTEM==0)

//***************************************************************************************************************
// start immer im root Dir. start in root dir (dir==0).
// es MUSS in das direktory gewechselt werden, in dem die datei zum lesen/anhängen ist (außer root, da startet mann)!
//***************************************************************************************************************
unsigned char fat_cd(char name[]){

  if(name[0]==0){								// ZUM ROOTDIR FAT16/32
	fat.dir=0;										// root dir 
	return(0);
	}

  if(0==fat_loadFileDataFromDir(name)){			// NICHT ROOTDIR	(fat16/32)
	fat.dir=file.firstCluster;						// zeigt auf 1.cluster des dir	(fat16/32)	
	return(0);
	}

  return(1);									// dir nicht gewechselt (nicht da?) !!
}

#endif





#if (WRITE==1)

// datei anlegen funktionen :

// ***************************************************************************************************************
// sucht leeren eintrag (zeile) im cluster mit dem startsektor:secStart.
// wird dort kein freier eintrag gefunden ist return (1).
// wird ein freier eintrag gefunden, ist die position der freien reihe auf file.row abzulesen und return (0).
// der sektor mit der freien reihe ist auf dem puffer:sector gepuffert.
// ****************************************************************************************************************
unsigned char fat_getFreeRowOfCluster(unsigned long secStart){
    
  unsigned int b;								// zum durchgenen der sektor bytes
  unsigned char s=0;							// sektoren des clusters.
  
  do{
	file.row=0;									// neuer sektor(oder 1.sektor), reihen von vorne.
	if(0==fat_loadSector(secStart+s)){			// laed sektor auf puffer fat.sector
	  for(b=0;b<512;b=b+32){					// zaehlt durch zeilen (0-15).
		if(fat.sector[b]==0x00||fat.sector[b]==0xE5)return(0);	// prueft auf freihen eintrag (leer oder geloescht == OK!).
		file.row++;								// zählt reihe hoch (nächste reihe im sektor).
		}
	  }
	s++;										// sektoren des clusters ++ weil einen geprüft.
	}while(s<fat.secPerClust);					// geht die sektoren des clusters durch (moeglicherweise auch nur 1. sektor).
  return (1);									// nicht gefunden in diesem cluster (== nicht OK!).
}


// ***************************************************************************************************************
// sucht leeren eintrag (zeile) im directory mit dem startcluster:dir.
// geht die cluster chain des direktories durch. dabei werden auch alle sektoren der cluster geprüft.
// wird dort kein freier eintrag gefunden, wird ein neuer leerer cluster gesucht, verkettet und der 
// 1. sektor des freien clusters geladen. die reihe wird auf den ersten eintrag gesetzt, da frei.
// anhand der reihe kann man nun den direktory eintrag vornehmen, und auf die karte schreiben.
// ****************************************************************************************************************
void fat_getFreeRowOfDir(unsigned long int dir){
  
  unsigned long int start=dir;
 
  // solange bis ende cluster chain.
  while( !((dir==0xfffffff&&fat.fatType==32)||(dir==0xffff&&fat.fatType==16)) ){		  
	if(0==fat_getFreeRowOfCluster(fat_clustToSec(dir)))return;	// freien eintrag in clustern, des dir gefunden !!
	start=dir;		
	dir=fat_getNextCluster(dir);	
	}									// wenn aus schleife raus, kein freier eintrag da -> neuer cluster nötig.

  dir=fat_secToClust(fat.startSectors);	// dir ist jetzt neuer cluster zum verketten !
  fat_setCluster(start,dir);			// cluster-chain mit neuem cluster verketten
  fat_setCluster(dir,0x0fffffff);		// cluster-chain ende markieren

  //es muessen neue gesucht werden, weil der bekannte aus file.c ja grade verkettet wurden. datei eintrag passte nicht mehr ins dir...
  fat_getFreeClustersInRow(2);							// neue freie cluster suchen, für datei.
  file.firstCluster=fat_secToClust(fat.startSectors);	// 1. cluster der datei
  file.lastCluster=fat_secToClust(fat.endSectors);		// letzter bekannter cluster der datei

  fat.currentSectorNr=fat_clustToSec(dir);	// setzen des richtigen sektors, also auf den 1. der neu verketteten

  unsigned int j=511;
  do{
    fat.sector[j]=0x00;						//schreibt puffer fat.sector voll mit 0x00==leer
  	}while(j--);

  unsigned char i=1;						// ab 1 weil der 1.sektor des clusters eh noch beschrieben wird...
  do{
	fat_writeSector(fat.currentSectorNr+i);	// löschen des cluster (überschreibt mit 0x00), wichtig bei ffls,
	i++;
	}while(i<fat.secPerClust);

  file.row=0;								// erste reihe frei, weil grad neuen cluster verkettet !
}


//***************************************************************************************************************
// erstellt 32 byte eintrag einer datei, oder verzeichnisses im puffer:sector.
// erstellt eintrag in reihe:row, mit namen:name usw... !!  
// muss noch auf die karte geschrieben werden ! nicht optimiert auf geschwindigkeit.
//***************************************************************************************************************
void fat_makeRowDataEntry(unsigned int row,char name[],unsigned char attrib,unsigned long int cluster,unsigned long int length){

  fat.bufferDirty=1;								// puffer beschrieben, also neue daten darin(vor lesen muss geschrieben werden)
  
  row=row<<5;										// multipliziert mit 32 um immer auf zeilen anfang zu kommen (zeile 0=0,zeile 1=32,zeile 2=62 ... zeile 15=480)	

  unsigned char i;								// byte zähler in reihe von sektor (32byte eintrag)
  unsigned char	*bytesOfSec=&fat.sector[row];	// zeiger auf sector bytes
  void *vsector;

  for(i=0;i<11;i++) *bytesOfSec++=name[i];				// namen schreiben

  *bytesOfSec++=attrib;									// attrib schreiben

  vsector=&fat.sector[row+12];
  *(unsigned long int*)vsector++=0x01010101;			// 	unnoetige felder beschreiben
  *(unsigned long int*)vsector=0x01010101;

  vsector=&fat.sector[row+20];
  *(unsigned short*)vsector=(cluster&0xffff0000)>>16;// low word	von cluster
  
  vsector=&fat.sector[row+22];
  *(unsigned long int*)vsector=0x01010101;			// unnoetige felder beschreiben
  
  vsector=&fat.sector[row+26];
  *(unsigned short*)vsector=(cluster&0x0000ffff);	// high word von cluster
  
  vsector=&fat.sector[row+28];
  *(unsigned long int*)vsector=length;				// laenge
}


//***************************************************************************************************************
// macht den datei eintrag im jetzigen verzeichniss (fat.dir).
// file.row enthält die reihen nummer des leeren eintrags, der vorher gesucht wurde, auf puffer:sector ist der gewünschte
// sektor gepuffert. für fat16 im root dir muss andere funktion genutzt werden, als fat_getFreeRowOfDir (durchsucht nur dirs).
// fat.rootDir enthält bei fat32 den start cluster des directory, bei fat16 den 1. sektor des rootDir bereichs!
//***************************************************************************************************************
void fat_makeFileEntry(char name[],unsigned char attrib,unsigned long int length){
  
  unsigned int s;													// zähler für root dir sektoren fat16
 
  if(fat.dir==0&&fat.fatType==32)fat_getFreeRowOfDir(fat.rootDir);	// IM ROOT DIR (fat32)

  else if(fat.dir==0 && fat.fatType==16){							// IM ROOT DIR (fat16)		
	for(s=0;s<(unsigned int)(fat.dataDirSec+2-fat.rootDir);s++){	// zählt durch RootDir sektoren (errechnet anzahl rootDir sektoren).								
	  if(0==fat_getFreeRowOfCluster(fat.rootDir+s))break;			// geht durch sektoren des root dir.
	  }
	}  

  else fat_getFreeRowOfDir(fat.dir);									// NICHT ROOT DIR fat32/fat16
		
  fat_makeRowDataEntry(file.row,name,attrib,file.firstCluster,length);	// schreibt file eintrag auf puffer
  fat_writeSector(fat.currentSectorNr);									// schreibt puffer auf karte
}

#endif





// fat funktionen:

//***************************************************************************************************************
// sucht nötige folge Cluster aus der fat ! 
// erster daten cluster = 2, ende einer cluster chain 0xFFFF (fat16) oder 0xFFFFFFF, 0xFFFFFF8 (fat32), 
// stelle des clusters in der fat, hat als wert, den nächsten cluster. (1:1 gemapt)!
//***************************************************************************************************************
unsigned long int fat_getNextCluster(unsigned long int oneCluster){	
  
  // FAT 16**************FAT 16	
  if(fat.fatType==16){																	
	unsigned long int i=oneCluster>>8;;			// (i=oneCluster/256)errechnet den sektor der fat in dem oneCluster ist (rundet immer ab)
	unsigned long int j=(oneCluster<<1)-(i<<9);	// (j=(oneCluster-256*i)*2 == 2*oneCluster-512*i)errechnet das low byte von oneCluster
			
	if(0==fat_loadSector(i+fat.fatSec)){			// ob neu laden nötig, wird von fat_loadSector geprüft
	  void *bytesOfSec=&fat.sector[j];	  			// zeiger auf puffer
	  return *(unsigned short*)bytesOfSec;		// da der ram auch little endian ist, kann einfach gecastet werden und gut :)
	  }	
	}

  // FAT 32**************FAT 32	
  else{												
	unsigned long int i=oneCluster>>7;			// (i=oneCluster/128)errechnet den sektor der fat in dem oneCluster ist (rundet immer ab)
	unsigned long int j=(oneCluster<<2)-(i<<9);	// (j=(oneCluster-128*i)*4 == oneCluster*4-512*i)errechnet das low byte von oneCluster
		
	if(0==fat_loadSector(i+fat.fatSec)){			// ob neu laden nötig wird von fat_loadSector geprüft
	  void *bytesOfSec=&fat.sector[j];				// zeiger auf puffer
	  return *(unsigned long int*)bytesOfSec;	// da der ram auch little endian ist, kann einfach gecastet werden und gut :)
	  }	
  }

  return(0);
}


//***************************************************************************************************************
// sucht verkettete cluster einer datei, die in einer reihe liegen. worst case: nur ein cluster.
// sieht in der fat ab dem cluster offsetCluster nach. sucht die anzahl von MAX_CLUSTERS_IN_ROW,
// am stück,falls möglich. prüft ob der cluster neben offsetCluster dazu gehört...
// setzt dann fat.endSectors und fat.startSectors. das -1 weil z.b. [95,98] = {95,96,97,98} = 4 sektoren
//***************************************************************************************************************
void fat_getFatChainClustersInRow(unsigned long int offsetCluster){

  unsigned int i=0;
  fat.startSectors=fat_clustToSec(offsetCluster);	// setzen des 1. sektors der datei
  fat.endSectors=fat.startSectors;
  do{
	 if( (offsetCluster+1+i)==fat_getNextCluster(offsetCluster+i) ) fat.endSectors+=fat.secPerClust; // zählen der zusammenhängenden sektoren					
	 else {
		file.lastCluster=offsetCluster+i;			// cluster daneben gehört nicht dazu, somit ist offset+i der letzte bekannte
		break;											
		}		
	 }while(i++<MAX_CLUSTERS_IN_ROW);	 
  fat.endSectors+=fat.secPerClust-1;
}


#if (WRITE==1)

//***************************************************************************************************************
// sucht freie zusammenhängende cluster aus der fat. maximal MAX_CLUSTERS_IN_ROW am stück.
// erst wir der erste frei cluster gesucht, ab offsetCluster(iklusive) und dann wird geschaut, ob der
// daneben auch frei ist. setzt dann fat.endSectors und fat.startSectors. das -1 weil z.b. [95,98] = {95,96,97,98} = 4 sektoren
//***************************************************************************************************************
void fat_getFreeClustersInRow(unsigned long int offsetCluster){

  unsigned int i=1; 															// variable für anzahl der zu suchenden sektoren
  while(fat_getNextCluster(offsetCluster)) offsetCluster++;		// suche des 1. freien clusters																					
		
  fat.startSectors=fat_clustToSec(offsetCluster);	// setzen des startsektors der freien sektoren (umrechnen von cluster zu sektoren)	 
  fat.endSectors=fat.startSectors;

  do{																// suche der nächsten freien
	 if(0==fat_getNextCluster(offsetCluster+i) ) fat.endSectors+=fat.secPerClust;	// zählen der zusammenhängenden sektoren			
	 else break;												// cluster daneben ist nicht frei				
	 }while(i++<MAX_CLUSTERS_IN_ROW);	

  fat.endSectors+=fat.secPerClust-1;					// -1 weil der erste sektor schon mit zählt z.b. [95,98] = 4 sektoren  
}

 
//***************************************************************************************************************
// verkettet ab startCluster bis einschließlich endCluster. verkettet auch den letzten bekannten mit den neu übergebenen !
// es ist wegen der fragmentierung der fat nötig, sich den letzten bekannten cluster zu merken, 
// damit man bei weiteren cluster in einer reihe die alten cluster noch dazu verketten kann (so sind lücken im verketten möglich).
//***************************************************************************************************************
void fat_setClusterChain(unsigned long int startCluster,unsigned int endCluster){

  fat_setCluster(file.lastCluster,startCluster);		// ende der chain setzen, bzw verketten der ketten
  
  while(startCluster!=endCluster){  
	 startCluster++;
	 fat_setCluster( startCluster-1 ,startCluster );	// verketten der cluster der neuen kette									
	 }
	 
  fat_setCluster(startCluster,0xfffffff);				// ende der chain setzen
  file.lastCluster=endCluster;							// ende cluster der kette updaten
  fat_writeSector(fat.currentSectorNr);					// schreiben des fat sektors auf die karte

}


//***************************************************************************************************************
// setzt den cluster inhalt. errechnet den sektor der fat in dem cluster ist, errechnet das low byte von
// cluster und setzt dann byteweise den inhalt:content.
// prüft ob buffer dirty (zu setztender cluster nicht in jetzt gepuffertem).
// prüfung erfolgt in fat_loadSector, dann wird alter vorher geschrieben, sonst gehen dort daten verloren !!
//***************************************************************************************************************
void fat_setCluster(unsigned long int cluster, unsigned long int content){  

	// FAT 16**************FAT 16	
	if(fat.fatType==16){					
		unsigned long int i=cluster>>8;			// (i=cluster/256)errechnet den sektor der fat in dem cluster ist (rundet immer ab)
		unsigned long int j=(cluster<<1)-(i<<9);	// (j=(cluster-256*i)*2 == 2*cluster-512*i)errechnet das low byte von cluster
		
		if(0==fat_loadSector(i+fat.fatSec)){		//	neu laden (fat_loadSector prüft ob schon gepuffert)
			void *bytesOfSec=&fat.sector[j];		// init des zeigers auf unterste adresse
			*(unsigned short*)bytesOfSec=content;// weil ram auch little endian geht das so :)
			fat.bufferDirty=1;						// zeigt an, dass im aktuellen sector geschrieben wurde
			}	
		}

	// FAT 32**************FAT 32	
	else{													
		unsigned long int i=cluster>>7;			// (i=cluster/128)errechnet den sektor der fat in dem cluster ist (rundet immer ab)
		unsigned long int j=(cluster<<2)-(i<<9);	//(j=(cluster-128*i)*4 == cluster*4-512*i)errechnet das low byte von cluster		
		
		if(0==fat_loadSector(i+fat.fatSec)){		//	neu laden (fat_loadSector prüft ob schon gepuffert)
			void *bytesOfSec=&fat.sector[j];		// init des zeigers auf unterste adresse
			*(unsigned long int*)bytesOfSec=content;		// weil ram auch little endian geht das so :)
			fat.bufferDirty=1;						// zeigt an, dass im aktuellen sector geschrieben wurde
			}	
		}		
}


//***************************************************************************************************************
// löscht cluster chain, beginnend ab dem startCluster.
// sucht cluster, setzt inhalt usw.. abschließend noch den cluster-chain ende markierten cluster löschen.
//***************************************************************************************************************
void fat_delClusterChain(unsigned long int startCluster){

  unsigned long int nextCluster=startCluster;		// tmp variable, wegen verketteter cluster..  
	
  do{
	 startCluster=nextCluster;
	 nextCluster=fat_getNextCluster(startCluster);
	 fat_setCluster(startCluster,0x00000000);  	
  }while(!((nextCluster==0xfffffff&&fat.fatType==32)||(nextCluster==0xffff&&fat.fatType==16)));	
  fat_writeSector(fat.currentSectorNr);
}
 
#endif


//***************************************************************************************************************
// Initialisiert die Fat(16/32) daten, wie: root directory sektor, daten sektor, fat sektor...
// siehe auch Fatgen103.pdf. ist NICHT auf performance optimiert!
// byte/sector, byte/cluster, anzahl der fats, sector/fat ... (halt alle wichtigen daten zum lesen ders datei systems!)
//*****************************************************************<**********************************************
unsigned char fat_loadFatData(unsigned long int sec){

										// offset,size
  unsigned int  		rootEntCnt;		// 17,2				größe die eine fat belegt
  unsigned int  		fatSz16;		// 22,2				sectors occupied by one fat16
  unsigned long int 	fatSz32;		// 36,4				sectors occupied by one fat32

  void *vsector;

  if(0==mmc_read_sector(sec,fat.sector)){	// lesen von fat sector und bestimmen der wichtigen berreiche 

	fat.bufferDirty = 0;				// init wert des flags
		
	fat.secPerClust=fat.sector[13];		// fat.secPerClust, 13 only (power of 2)

	vsector=&fat.sector[14];
	fat.fatSec=*(unsigned short*)vsector;

	vsector=&fat.sector[17];
	rootEntCnt=*(unsigned short*)vsector;

	vsector=&fat.sector[22];
	fatSz16=*(unsigned short*)vsector;

	fat.rootDir	 = (((rootEntCnt <<5) + 512) /512)-1;	// ist 0 bei fat 32, sonst der root dir sektor

	if(fat.rootDir==0){									// FAT32 spezifisch (die prüfung so, ist nicht spezifikation konform !).

		vsector=&fat.sector[36];
		fatSz32=*(unsigned long int *)vsector;

		vsector=&fat.sector[44];
		fat.rootDir=*(unsigned long int *)vsector;

		fat.dataDirSec = fat.fatSec + (fatSz32 * fat.sector[16]);	// data sector (beginnt mit cluster 2)
		fat.fatType=32;									// fat typ
	  }

	else{												// FAT16	spezifisch
		fat.dataDirSec = fat.fatSec + (fatSz16 * fat.sector[16]) + fat.rootDir;		// data sektor (beginnt mit cluster 2)
		fat.rootDir=fat.dataDirSec-fat.rootDir;			// root dir sektor, da nicht im datenbereich (cluster)	
		fat.rootDir+=sec;								// addiert den startsektor auf 	"
		fat.fatType=16;									// fat typ
		}		

	fat.fatSec+=sec;									// addiert den startsektor auf
	fat.dataDirSec+=sec;								// addiert den startsektor auf (umrechnung von absolut auf real)
	fat.dataDirSec-=2;									// zeigt auf 1. cluster
	fat.dir=0;											// dir auf '0'==root dir, sonst 1.Cluster des dir
	return (0);
	}


  return (1);			// sector nicht gelesen, fat nicht initialisiert!!
}


//************************************************************************************************<<***************
// int fat sucht den 1. cluster des dateisystems (fat16/32) auch VBR genannt,  
//************************************************************************************************<<***************
unsigned char fat_initfat(void){					
  
   unsigned long int secOfFirstPartition=0;					// ist 1. sektor der 1. partition aus dem MBR  
  
	if(0==mmc_read_sector(0,fat.sector)){

	  void *vsector=&fat.sector[454];								//startsektor bestimmen
	  secOfFirstPartition=*(unsigned long int*)vsector;

	  //prüfung ob man schon im VBR gelesen hat (0x6964654d = "Medi")
	  if(secOfFirstPartition==0x6964654d)  return fat_loadFatData(0);  //ist superfloppy

	  else {return fat_loadFatData(secOfFirstPartition);}				 // ist partitioniert...
	 }

  return (1);
}


#if (SMALL_FILE_SYSTEM==0)

// *****************************************************************************************************************
// bereitet str so auf, dass man es auf die mmc/sd karte schreiben kann.
// wandelt z.b. "t.txt" -> "T        TXT" oder "main.c" in "MAIN    C  " => also immer 8.3 und upper case letter
// VORSICHT es werden keine Prüfungen gemacht !
// *****************************************************************************************************************
char * fat_str(char *str){

  unsigned char i;
  unsigned char j=0;
  unsigned char c;
  char tmp[12];					// tmp string zum einfacheren umwandeln
  
  strcpy(tmp,str);

  for(i=0;i<11;i++)str[i]=' ';	// als vorbereitung alles mit leerzeichen füllen
  str[11]='\0';

  i=0;

  do{
	c=toupper(tmp[j]);
	if(c=='\0')return str;
	if(c!='.')str[i]=c;
	else i=7;
	i++;
	j++;
	}while(i<12);

  return str;
  
}

#endif

