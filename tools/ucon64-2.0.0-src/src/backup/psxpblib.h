/*
 *
 * PSX Peripheral Bus Library v1.4 17/01/00 Richard Davies
 * <http://www.debaser.force9.co.uk/>
 * <mailto:richard@debaser.force9.co.uk>
 *
 * Revision History:
 * v1.4 - 17/01/00 Win32 / Win32 DLL support and bug fixes
 * v1.3 - 21/12/99 Linux support and bug fixes
 * v1.1 - 26/09/99 Minor Controller detection improvements.
 * v1.0 - 17/07/99 Initial release (based on PSXTest v1.1 by me).
 *
 * see psxpblib.txt for details.
 *
 */

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef USE_PARALLEL

// outportb() and inportb() are only present in uCON64 if USE_PARALLEL is defined
#define psx_outportb(P, B) outportb((unsigned short) (P), (unsigned char) (B))
#define psx_inportb(P) inportb((unsigned short) (P))

#ifdef  __MSDOS__
#include <stdlib.h>
#include <pc.h>
#endif

#define LPT1_BASE 0x378
#define LPT2_BASE 0x278
#define LPT3_BASE 0x3bc

#define LPT_D0  0x01            /* pin 2 */
#define LPT_D1  0x02            /* pin 3 */
#define LPT_D2  0x04            /* pin 4 */
#define LPT_D3  0x08            /* pin 5 */
#define LPT_D4  0x10            /* pin 6 */
#define LPT_D5  0x20            /* pin 7 */
#define LPT_D6  0x40            /* pin 8 */
#define LPT_D7  0x80            /* pin 9 */
#define LPT_AUT 0x08            /* pin 14 */
#define LPT_SEL 0x10            /* pin 13 */
#define LPT_PAP 0x20            /* pin 12 */
#define LPT_ACK 0x40            /* pin 10 */
#define LPT_ERR 0x02            /* pin 15 */
#define LPT_INI 0x04            /* pin 16 */
#define LPT_STR 0x08            /* pin 1 */

#define PSX_MAX_CONPORTS 8
#define PSX_MAX_TAPS     4
#define PSX_MAX_DELAY    10

#define PSX_MAX_DATA     30     /* maximum possible length of controller packet in bytes */

                                /* JAP code  EUR code    Name */

#define PSX_MOUSE        1      /* SCPH-1030 SCPH-????   Mouse */
#define PSX_NEGCON       2      /* SLPH-0001 SLEH-0003   Namco neGcon
                                   SLPH-0007             Nasca Pachinco Handle (untested); Twist = Twist, TW = B
                                   SLPH-0015             ? Volume Controller (untested); Rotation = Twist, A = Start, B = A
                                   Puchi Carat paddle controller (not working!); Rotation = Twist, A = Start, B = A
                                   SLPH-???? SLEH-0005   MadKatz Steering Wheel (twitchy) */
#define PSX_KONAMIGUN    3      /* SLPH-???? SLPH-????   Konami Lightgun (untested) */
#define PSX_DIGITAL      4      /* SCPH-1010 SCPH 1080 E Controller
                                   SCPH-1110 SCPH-????   Analog Joystick - Digital Mode
                                   SCPH-???? SCPH-1180 E Analog Controller - Digital Mode
                                   SCPH-1150 SCPH-1200 E Dual Shock Analog Controller - Digital Mode
                                   SLPH-???? SLEH-0011   Ascii Resident Evil Pad
                                   SLPH-???? SLEH-0004   Namco Arcade Stick (untested)
                                   Twin Stick
                                   Blaze Pro-Shock Arcade Stick */
#define PSX_ANALOG_GREEN 5      /* SCPH-1110 SCPH-????   Analog Joystick - Analog Mode (untested)
                                   SCPH-???? SCPH-1180 E Analog Controller - Analog Green Mode */
#define PSX_GUNCON       6      /* SLPH-???? SLEH-0007   Namco G-con45 */
#define PSX_ANALOG_RED   7      /* SCPH-1150 SCPH-1200 E Dual Shock Analog Controller - Analog Red Mode */
                                /* SCPH-???? SCPH-1180 E Analog Controller - Analog Red Mode */
#define PSX_JOGCON       14     /* SLPH-???? SLEH-0020   Namco Jogcon */
/*#define PSX_MULTITAP   17    SCPH-1070 SCPH-1070 E Multi tap */

#define PSX_MCB_STATE_OK       0x05
#define PSX_MCB_STATE_DELETED  0x0a
#define PSX_MCB_STATE_RESERVED 0x0f

#define PSX_MCB_LTYPE_NONE     0x00
#define PSX_MCB_LTYPE_FIRST    0x01
#define PSX_MCB_LTYPE_MIDDLE   0x02
#define PSX_MCB_LTYPE_LAST     0x03
#define PSX_MCB_LTYPE_RESERVED 0x0f

#define PSX_MCB_ICON_VALID     0x01



typedef struct
{
  unsigned char type;
  unsigned char length;
  unsigned char data[PSX_MAX_DATA];
} PSX_CON_BUF;

typedef struct
{
  unsigned char read;
  char filename[9];
  char code[11];
  char territory;               /* E, A or I */
  int bytes;
  unsigned char state;          /* PSX_MCB_STAT_* or unknown */
  unsigned char linktype;       /* PSX_MCB_LTYPE_* or unknowm */
  unsigned char next;           /* 0 to 14 */
} PSX_MCB_INFO_DIR;

typedef struct
{
  unsigned char read;
  char name[92];
  unsigned char blocks;
  unsigned char icon_valid;
  unsigned char icon_frames;
} PSX_MCB_INFO_DAT;

typedef struct
{
  unsigned char scanned;
  unsigned char read;
  char name[92];
  char filename[9];
  char code[11];
  char territory;
  unsigned char state;
  unsigned char blocks;
  int bytes;
  unsigned char linktype;
  unsigned char next;
  unsigned char icon_valid;
  unsigned char icon_frames;
} PSX_MCB_INFO;


/* sets clock for conport connected to parallel port base */
void psx_clk (int base, int conport, int on);
/* sets att for conport connected to parallel port base */
void psx_att (int base, int conport, int on);
/* sets command for conport connected to parallel port base */
void psx_cmd (int base, int conport, int on);
/* tests data for conport connected to parallel port base, returns 1 if high */
int psx_dat (int base, int conport);
/* tests ack for conport connected to parallel port base, returns 1 if high */
int psx_ack (int base, int conport);
/* wait for delay * (outportb() execution time) */
void psx_delay (int base, int delay);

/* send byte as a command to conport connected to parallel port base
 * assumes clock high and the attention of conport */
unsigned char psx_sendbyte (int base, int conport, int delay,
                            unsigned char byte, int wait);
/* get io permissions under linux*/
int psx_obtain_io_permission (int base);
/* sets clock high and gets the attention of conport, use before psx_sendbyte() */
void psx_sendinit (int base, int conport, int delay);
/* use after psx_sendbyte() */
void psx_sendclose (int base, int conport, int delay);
/* send string as a series of commands to conport connected to parallel port base */
void psx_sendstring (int base, int conport, int delay, int string[]);

/* tests for the presence of a controller on conport:tap connected to base
 * returns the type if present, otherwise -1 */
int psx_controller_detect (int base, int conport, int tap, int delay);
/* reads a controller on conport:tap connected to base returns the data
 * if present, otherwise -1 */
PSX_CON_BUF *psx_controller_read (int base, int conport, int tap, int delay);
/* sends force feedback/shock init command sequence to conport:tap on port base
 * (also initialises crash protection for some controllers) */
void psx_controller_vinit (int base, int conport, int tap, int delay);
/* sends the dual shock command sequence to conport:tap on port base */
void psx_controller_vshock (int base, int conport, int tap, int delay,
                            int shock, int rumble);

/* Reads a single frame (128 bytes) from Memory Card on conport base:tap */
unsigned char *psx_memcard_read_frame (int base, int conport, int tap, int delay,
                                       int frame);
/* Writes a single frame (128 bytes) to Memory Card on conport base:tap */
int psx_memcard_write_frame (int base, int conport, int tap, int delay,
                             int frame, unsigned char *data_f);
/* Reads a single block (64 frames) from Memory Card on conport base:tap */
unsigned char *psx_memcard_read_block (int base, int conport, int tap, int delay,
                                       int block);
/* Writes a single block (64 frames) to Memory Card on conport base:tap */
int psx_memcard_write_block (int base, int conport, int tap, int delay,
                             int block, unsigned char *data_b);

/* Reads the info associated with block from the directory */
PSX_MCB_INFO_DIR *psx_mcb_read_dir (int base, int conport, int tap, int delay,
                                    int block);
/* Prints the info associated with block from it's data */
PSX_MCB_INFO_DAT *psx_mcb_read_dat (int base, int conport, int tap, int delay,
                                    int block);
/* Merges the info associated with block from the directory and it's data */
PSX_MCB_INFO *psx_mcb_info_merge (PSX_MCB_INFO_DIR mcb_info_dir,
                                  PSX_MCB_INFO_DAT mcb_info_dat,
                                  PSX_MCB_INFO * mcb_info);
/* Reads the info associated with block from the directory and it's data */
PSX_MCB_INFO *psx_mcb_read_info (int base, int conport, int tap, int delay,
                                 int block);

#endif // USE_PARALLEL
