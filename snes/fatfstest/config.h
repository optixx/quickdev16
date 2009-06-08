
#define CMD_INIT        0x00
#define CMD_READ        0x01
#define CMD_WRITE       0x02
#define CMD_NONE        0xff

/*
 * Memory Map 
 */
#define MMIO_CMD        0x3010
#define MMIO_SECTOR01   0x3011
#define MMIO_SECTOR02   0x3012
#define MMIO_SECTOR03   0x3013
#define MMIO_SECTOR04   0x3014
#define MMIO_COUNT      0x3015
#define MMIO_RETVAL     0x3016


#define SHARED_SIZE     512
#define SHARED_ADDR     0x3d0000


#undef MMIO_DEBUG
