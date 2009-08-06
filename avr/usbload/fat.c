#include "fat.h"

uint8_t cluster_size;
uint16_t fat_offset;
uint16_t cluster_offset;
uint16_t volume_boot_record_addr;

void fat_init(uint8_t * Buffer)
{
    struct BootSec *bootp;
    mmc_read_sector(MASTER_BOOT_RECORD, Buffer);
    if (Buffer[510] == 0x55 && Buffer[511] == 0xAA) {
        FAT_DEBUG("MBR Signatur found!\r\n");
    }

    else {
        FAT_DEBUG("MBR Signatur not found!\r\n");
        while (1);
    }
    volume_boot_record_addr = Buffer[VBR_ADDR] + (Buffer[VBR_ADDR + 1] << 8);
    mmc_read_sector(volume_boot_record_addr, Buffer);
    if (Buffer[510] == 0x55 && Buffer[511] == 0xAA) {
        FAT_DEBUG("VBR Signatur found!\r\n");
    }

    else {
        FAT_DEBUG("VBR Signatur not found!\r\n");
        volume_boot_record_addr = MASTER_BOOT_RECORD;
        mmc_read_sector(MASTER_BOOT_RECORD, Buffer);
    }
    bootp = (struct BootSec *) Buffer;
    cluster_size = bootp->BPB_SecPerClus;
    fat_offset = bootp->BPB_RsvdSecCnt;
    cluster_offset = ((bootp->BPB_BytesPerSec * 32) / BlockSize);
    cluster_offset += fat_root_dir_addr(Buffer);
}

uint16_t fat_root_dir_addr(uint8_t * Buffer)
{
    struct BootSec *bootp;
    uint16_t FirstRootDirSecNum;


    mmc_read_sector(volume_boot_record_addr, Buffer);
    bootp = (struct BootSec *) Buffer;


    FirstRootDirSecNum = (bootp->BPB_RsvdSecCnt +
                          (bootp->BPB_NumFATs * bootp->BPB_FATSz16));
    FirstRootDirSecNum += volume_boot_record_addr;
    return (FirstRootDirSecNum);
}


uint16_t fat_read_dir_ent(uint16_t dir_cluster,
                          uint8_t Entry_Count,
                          uint32_t * Size,
                          uint8_t * Dir_Attrib, uint8_t * Buffer)
{
    uint8_t *pointer;
    uint16_t TMP_Entry_Count = 0;
    uint32_t Block = 0;
    struct DirEntry *dir;
    uint16_t blk;
    uint16_t a;
    uint8_t b;
    pointer = Buffer;
    if (dir_cluster == 0) {
        Block = fat_root_dir_addr(Buffer);
    }

    else {
        fat_load(dir_cluster, &Block, Buffer);
        Block = ((Block - 2) * cluster_size) + cluster_offset;
    }


    for (blk = Block;; blk++) {
        mmc_read_sector(blk, Buffer);
        for (a = 0; a < BlockSize; a = a + 32) {
            dir = (struct DirEntry *) &Buffer[a];
            if (dir->DIR_Name[0] == 0) {
                return (0xFFFF);
            }

            if ((dir->DIR_Attr != ATTR_LONG_NAME) &&
                (dir->DIR_Name[0] != DIR_ENTRY_IS_FREE)) {


                if (TMP_Entry_Count == Entry_Count) {


                    for (b = 0; b < 11; b++) {
                        if (dir->DIR_Name[b] != SPACE) {
                            if (b == 8) {
                                *pointer++ = '.';
                            }
                            *pointer++ = dir->DIR_Name[b];
                        }
                    }
                    *pointer++ = '\0';
                    *Dir_Attrib = dir->DIR_Attr;


                    *Size = dir->DIR_FileSize;


                    dir_cluster = dir->DIR_FstClusLO;


                    return (dir_cluster);
                }
                TMP_Entry_Count++;
            }
        }
    }
    return (0xFFFF);
}

void fat_load(uint16_t Cluster, uint32_t * Block, uint8_t * TMP_Buffer)
{
    uint16_t FAT_Block_Store = 0;
    uint16_t FAT_Byte_Addresse;
    uint16_t FAT_Block_Addresse;
    uint16_t a;

    for (a = 0;; a++) {
        if (a == *Block) {
            *Block = (0x0000FFFF & Cluster);
            return;
        }
        if (Cluster == 0xFFFF) {
            break;
        }

        FAT_Byte_Addresse = (Cluster * 2) % BlockSize;


        FAT_Block_Addresse =
            ((Cluster * 2) / BlockSize) + volume_boot_record_addr + fat_offset;

        if (FAT_Block_Addresse != FAT_Block_Store) {
            FAT_Block_Store = FAT_Block_Addresse;
            mmc_read_sector(FAT_Block_Addresse, TMP_Buffer);
        }
        Cluster =
            (TMP_Buffer[FAT_Byte_Addresse + 1] << 8) +
            TMP_Buffer[FAT_Byte_Addresse];
    }
    return;
}

void fat_read_file(uint16_t Cluster, uint8_t * Buffer, uint32_t BlockCount)
{

    uint32_t Block = (BlockCount / cluster_size);
    fat_load(Cluster, &Block, Buffer);
    Block = ((Block - 2) * cluster_size) + cluster_offset;
    Block += (BlockCount % cluster_size);
    mmc_read_sector(Block, Buffer);
    return;
}

void fat_write_file(uint16_t cluster, uint8_t * buffer, uint32_t blockCount)
{
    uint8_t tmp_buffer[513];
    uint32_t block = (blockCount / cluster_size);
    fat_load(cluster, &block, tmp_buffer);
    block = ((block - 2) * cluster_size) + cluster_offset;
    block += (blockCount % cluster_size);
    mmc_write_sector(block, buffer);
    return;
}

uint8_t fat_search_file(uint8_t * File_Name,
                        uint16_t * Cluster,
                        uint32_t * Size, uint8_t * Dir_Attrib, uint8_t * Buffer)
{
    uint16_t Dir_Cluster_Store = *Cluster;
    uint8_t a;
    for (a = 0; a < 100; a++) {
        *Cluster =
            fat_read_dir_ent(Dir_Cluster_Store, a, Size, Dir_Attrib, Buffer);
        if (*Cluster == 0xffff) {
            return (0);
        }
        if (strcasecmp((char *) File_Name, (char *) Buffer) == 0) {
            return (1);
        }
    }
    return (2);
}
