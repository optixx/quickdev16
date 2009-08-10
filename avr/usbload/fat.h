
#ifndef _FAT_H

  #define _FAT_H

  // **************************************************************************************************************************
  // WICHTIGE SCHLATER: -> hier kann die code größe angepasst werden, zu lasten der funktionalität ! 
  #define SMALL_FILE_SYSTEM 0	// wenn 1 dann ist kleines file system, wenn 0 dann komplette file unterstützung !
  #define WRITE 0 				// wenn 1 dann ist write an, wenn 0 dann read only !
  #define OVER_WRITE 0			// wenn 1 dann kann ffwrite dateien überschreiben (nicht performant), wenn 0 dann nur normales schreiben !
  #define MAX_CLUSTERS_IN_ROW	256 // gibt an wie viele cluster am stück ohne fat lookup geschrieben bzw gelesen werden können, wenn die fat nicht fragmentiert ist !
 
  // 1. fat_getFreeRowOfCluster -> fat_getFreeRowOfDir -> fat_makeRowDataEntry -> fat_makeFileEntry -> fat_writeSector  "eintrag gemacht !!"
  // 2. fat_loadSector -> fat_loadRowOfSector -> fat_loadFileDataFromCluster -> fat_loadFileDataFromDir (-> fat_cd)   "daten chain"

  // **************************************************************************************************************************
  // funktionen

  extern unsigned long int 	fat_clustToSec(unsigned long int);		// rechnet cluster zu 1. sektor des clusters um
  extern unsigned long int 	fat_secToClust(unsigned long int sec);		// rechnet sektor zu cluster um!
  extern unsigned long int 	fat_getNextCluster(unsigned long int oneCluster);	// fat auf nächsten, verketteten cluster durchsuchen 
  extern unsigned char	fat_initfat(void);							// initalisierung (durchsucht MBR oder nicht)   
  extern unsigned char 	fat_writeSector(unsigned long int sec);		// schreibt sektor auf karte  
  extern void				fat_setCluster(unsigned long int cluster, unsigned long int content); // setzt cluster inhalt in der fat
  extern void 				fat_delClusterChain(unsigned long int startCluster);	// löscht cluster-chain in der fat
  extern void			 	fat_getFreeRowOfDir(unsigned long int dir);	// durchsucht directory nach feiem eintrag
  extern void 				fat_makeFileEntry(char name[],unsigned char attrib,unsigned long int length);
  extern unsigned char 	fat_loadSector(unsigned long int sec);		// läd übergebenen absoluten sektor
  extern unsigned char 	fat_loadFileDataFromDir(char name[]);		// durchsucht das aktuelle directory 
  extern unsigned char	fat_cd(char *);								// wechselt directory (start im rootDir)
  extern unsigned char	fat_loadFatData(unsigned long int);			// läd fat daten   
  extern unsigned char 	fat_getFreeRowOfCluster(unsigned long secStart);// durchsucht cluster nach freiem eintrag
  extern void			 	fat_getFreeClustersInRow(unsigned long int offsetCluster);	// sucht zusammenhängende freie cluster aus der fat
  extern void				fat_getFatChainClustersInRow(unsigned long int offsetCluster);	// sucht fat-chain cluster die zusammenhängen
  extern void				fat_makeRowDataEntry(unsigned int row,char name[],unsigned char attrib,unsigned long int cluster,unsigned long int length);  
  extern unsigned char	fat_loadRowOfSector(unsigned int);			// läd reihe des geladen sektors auf struct:file
  extern unsigned char	fat_loadFileDataFromCluster(unsigned long int sec , char name[]);	// durchsucht die reihen des geladenen sektors 
  extern void			 	fat_setClusterChain(unsigned long int newOffsetCluster,unsigned int length);
  extern char 				*fat_str(char *str);						// wandelt einen string so, dass er der fat konvention entspricht !
  

	// **************************************************************************************************************************
  // variablen

  extern struct Fat{						// fat daten (1.cluster, root-dir, dir usw.)
	unsigned char 		sector[512];	// der puffer für sektoren !
	unsigned char 		bufferDirty;	// puffer wurde beschrieben, sector muss geschrieben werden bevor er neu geladen wird
	unsigned long int 	currentSectorNr;// aktuell geladener Sektor (in sector)  //beschleunigt wenn z.b 2* 512 byte puffer vorhanden, oder bei fat operationen im gleichen sektor
	unsigned long int 	dir; 		  	// Direktory zeiger rootDir=='0' sonst(1.Cluster des dir; start auf root)
	unsigned long int  	rootDir;		// Sektor(f16)/Cluster(f32) nr root directory
	unsigned long int  	dataDirSec;		// Sektor nr data area 
	unsigned long int  	fatSec;	 		// Sektor nr fat area
	unsigned long int   startSectors;	// der erste sektor in einer reihe (freie oder verkettete)
	unsigned long int 	endSectors;	
	unsigned char 		secPerClust;	// anzahl der sektoren pro cluster
	unsigned char  		fatType;		// fat16 oder fat32 (16 oder 32)
	}fat;  

  extern struct File{					// datei infos
	unsigned int 			cntOfBytes;		// -nicht direkt aus dem dateisystem- zäht geschriebene bytes eines sektors
	unsigned char 		name[13];		// 0,10			datei Name.ext (8.3 = max 11)(MUSS unsigned char weil E5)
	unsigned char 		attrib;			// 11,1			datei Attribut: 8=value name, 32=datei, 16=Verzeichniss, 15=linux kleingeschrieben eintrag
	unsigned char 		row;			// reihe im sektor in der die datei infos stehen (reihe 0-15)
	unsigned long int 	firstCluster;	// 20,2 /26,2	datei 1.cluster hi,low(möglicherweise der einzige)	(4-byte)
	unsigned long int 	length;			// 28,4			datei Länge (4-byte)
	unsigned long int 	lastCluster;	// -nicht direkt aus dem dateisystem- letzter cluster der ersten kette
	unsigned long int 	seek;			// schreib position in der datei
	}file;	

#endif


 



