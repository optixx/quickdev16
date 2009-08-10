
#ifndef _FAT_H

#define _FAT_H

  // **************************************************************************************************************************
  // WICHTIGE SCHLATER: -> hier kann die code größe angepasst werden, zu lasten der funktionalität ! 
#define SMALL_FILE_SYSTEM 0     // wenn 1 dann ist kleines file system, wenn 0 dann komplette file unterstützung !
#define WRITE 1                 // wenn 1 dann ist write an, wenn 0 dann read only !
#define OVER_WRITE 1            // wenn 1 dann kann ffwrite dateien überschreiben (nicht performant), wenn 0 dann nur normales schreiben !
#define MAX_CLUSTERS_IN_ROW	256     // gibt an wie viele cluster am stück ohne fat lookup geschrieben bzw gelesen werden können, wenn 
                                        // die fat nicht fragmentiert ist !

  // 1. fat_getFreeRowOfCluster -> fat_getFreeRowOfDir -> fat_makeRowDataEntry -> fat_makeFileEntry -> fat_writeSector "eintrag gemacht !!"
  // 2. fat_loadSector -> fat_loadRowOfSector -> fat_loadFileDataFromCluster -> fat_loadFileDataFromDir (-> fat_cd) "daten chain"

  // **************************************************************************************************************************
  // funktionen

extern uint32_t fat_clustToSec(uint32_t);     // rechnet cluster zu 1. sektor des clusters um
extern uint32_t fat_secToClust(uint32_t sec); // rechnet sektor zu cluster um!
extern uint32_t fat_getNextCluster(uint32_t oneCluster);      // fat auf nächsten, verketteten cluster durchsuchen 
extern uint8_t fat_initfat(void); // initalisierung (durchsucht MBR oder nicht) 
extern uint8_t fat_writeSector(uint32_t sec);    // schreibt sektor auf karte 
extern void fat_setCluster(uint32_t cluster, uint32_t content);       // setzt cluster inhalt in der fat
extern void fat_delClusterChain(uint32_t startCluster);        // löscht cluster-chain in der fat
extern void fat_getFreeRowOfDir(uint32_t dir); // durchsucht directory nach feiem eintrag
extern void fat_makeFileEntry(char name[], uint8_t attrib,
                              uint32_t length);
extern uint8_t fat_loadSector(uint32_t sec);     // läd übergebenen absoluten sektor
extern uint8_t fat_loadFileDataFromDir(char name[]);      // durchsucht das aktuelle directory 
extern uint8_t fat_cd(char *);    // wechselt directory (start im rootDir)
extern uint8_t fat_loadFatData(uint32_t);        // läd fat daten 
extern uint8_t fat_getFreeRowOfCluster(unsigned long secStart);   // durchsucht cluster nach freiem eintrag
extern void fat_getFreeClustersInRow(uint32_t offsetCluster);  // sucht zusammenhängende freie cluster aus der fat
extern void fat_getFatChainClustersInRow(uint32_t offsetCluster);      // sucht fat-chain cluster die zusammenhängen
extern void fat_makeRowDataEntry(uint16_t row, char name[],
                                 uint8_t attrib,
                                 uint32_t cluster,
                                 uint32_t length);
extern uint8_t fat_loadRowOfSector(uint16_t); // läd reihe des geladen sektors auf struct:file
extern uint8_t fat_loadFileDataFromCluster(uint32_t sec, char name[]);   // durchsucht die reihen des geladenen sektors 
extern void fat_setClusterChain(uint32_t newOffsetCluster,
                                uint16_t length);
extern char *fat_str(char *str);        // wandelt einen string so, dass er der fat konvention entspricht !


        // **************************************************************************************************************************
  // variablen

extern struct Fat {             // fat daten (1.cluster, root-dir, dir usw.)
    uint8_t sector[512];  // der puffer für sektoren !
    uint8_t bufferDirty;  // puffer wurde beschrieben, sector muss geschrieben werden bevor er neu geladen wird
    uint32_t currentSectorNr;  // aktuell geladener Sektor (in sector) //beschleunigt wenn z.b 2* 512 byte puffer vorhanden, oder
                                        // bei fat operationen im gleichen sektor
    uint32_t dir;      // Direktory zeiger rootDir=='0' sonst(1.Cluster des dir; start auf root)
    uint32_t rootDir;  // Sektor(f16)/Cluster(f32) nr root directory
    uint32_t dataDirSec;       // Sektor nr data area 
    uint32_t fatSec;   // Sektor nr fat area
    uint32_t startSectors;     // der erste sektor in einer reihe (freie oder verkettete)
    uint32_t endSectors;
    uint8_t secPerClust;  // anzahl der sektoren pro cluster
    uint8_t fatType;      // fat16 oder fat32 (16 oder 32)
} fat;

extern struct File {            // datei infos
    uint16_t cntOfBytes;    // -nicht direkt aus dem dateisystem- zäht geschriebene bytes eines sektors
    uint8_t name[13];     // 0,10 datei Name.ext (8.3 = max 11)(MUSS uint8_t weil E5)
    uint8_t attrib;       // 11,1 datei Attribut: 8=value name, 32=datei, 16=Verzeichniss, 15=linux kleingeschrieben eintrag
    uint8_t row;          // reihe im sektor in der die datei infos stehen (reihe 0-15)
    uint32_t firstCluster;     // 20,2 /26,2 datei 1.cluster hi,low(möglicherweise der einzige) (4-byte)
    uint32_t length;   // 28,4 datei Länge (4-byte)
    uint32_t lastCluster;      // -nicht direkt aus dem dateisystem- letzter cluster der ersten kette
    uint32_t seek;     // schreib position in der datei
} file;

#endif
