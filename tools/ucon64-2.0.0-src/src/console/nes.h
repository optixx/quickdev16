/*
nes.h - Nintendo Entertainment System support for uCON64

Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2002 - 2003 dbjh


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef NES_H
#define NES_H

typedef enum { INES = 1, UNIF, PASOFAMI, FFE, FDS, FAM } nes_file_t;

extern const st_getopt2_t nes_usage[];

/*
 iNES Format (.NES)
 ------------------

    Following the iNES header description (size = $10 bytes):

    +--------+------+------------------------------------------+
    | Offset | Size | Content(s)                               |
    +--------+------+------------------------------------------+
    |   0    |  3   | 'NES'                                    |
    |   3    |  1   | $1A                                      |
    |   4    |  1   | 16K PRG-ROM page count (size=x*0x4000)   |
    |   5    |  1   | 8K CHR-ROM page count  (size=y*0x2000)   |
    |   6    |  1   | ROM Control Byte #1                      |
    |        |      |   %####vTsM                              |
    |        |      |    |  ||||+- 0=Horizontal Mirroring      |
    |        |      |    |  ||||   1=Vertical Mirroring        |
    |        |      |    |  |||+-- 1=SaveRAM enabled ($6000)   |
    |        |      |    |  ||+--- 1=Trainer data (512 bytes)  |
    |        |      |    |  |+---- 1=Four-screen mirroring     |
    |        |      |    |  |      ($2000,$2400,$2800,$2C00)   |
    |        |      |    +--+----- Mapper # (lower 4-bits)     |
    |   7    |  1   | ROM Control Byte #2                      |
    |        |      |   %####00PU                              |
    |        |      |    |  |  |+- 1=VS Unisystem arcade       |
    |        |      |    |  |  +-- 1=Playchoice-10 arcade      |
    |        |      |    +--+----- Mapper # (upper 4-bits)     |
    |  8-15  |  8   | $00                                      |
    | 16-..  |      | Actual 16K PRG-ROM pages (in linear      |
    |  ...   |      | order). If a trainer exists, it precedes |
    |  ...   |      | the first PRG-ROM bank.                  |
    | ..-EOF |      | CHR-ROM pages (in ascending order).      |
    +--------+------+------------------------------------------+

 IMPORTANT: the iNES format DOES NOT support mappers greater than 255.
 There are a couple of Famicom mappers > 255. They use byte #7 (low 4 bits),
 which conflicts with VS Unisystem/PlayChoice-10 identification.
*/

#define INES_SIG_S      "NES\x1a"

// flags in st_ines_header_t.ctrl1
#define INES_MIRROR     0x01
#define INES_SRAM       0x02
#define INES_TRAINER    0x04
#define INES_4SCREEN    0x08

#define INES_HEADER_START 0
#define INES_HEADER_LEN (sizeof (st_ines_header_t))

typedef struct
{
  char signature[4];                            // 0x4e,0x45,0x53,0x1a (NES file signature)
  unsigned char prg_size;                       // # 16 kB banks
  unsigned char chr_size;                       // # 8 kB banks
  unsigned char ctrl1;
  unsigned char ctrl2;
  unsigned int reserved1;                       // 0
  unsigned int reserved2;                       // 0
} st_ines_header_t;


#define UNIF_SIG_S      "UNIF"
#define UNIF_REVISION   8                       // the "official" spec is at version 7 (10/08/2002)

// numeric values of id strings in little endian format, e.g.
//  UNIF_ID == 'F' << 24 | 'I' << 16 | 'N' << 8 | 'U'
#define UNIF_ID 0x46494E55
#define MAPR_ID 0x5250414D
#define READ_ID 0x44414552
#define NAME_ID 0x454D414E
#define TVCI_ID 0x49435654
#define DINF_ID 0x464E4944
#define CTRL_ID 0x4C525443
#define BATR_ID 0x52544142
#define VROR_ID 0x524F5256
#define MIRR_ID 0x5252494D

#define PCK0_ID 0x304B4350
#define PCK1_ID 0x314B4350
#define PCK2_ID 0x324B4350
#define PCK3_ID 0x334B4350
#define PCK4_ID 0x344B4350
#define PCK5_ID 0x354B4350
#define PCK6_ID 0x364B4350
#define PCK7_ID 0x374B4350
#define PCK8_ID 0x384B4350
#define PCK9_ID 0x394B4350
#define PCKA_ID 0x414B4350
#define PCKB_ID 0x424B4350
#define PCKC_ID 0x434B4350
#define PCKD_ID 0x444B4350
#define PCKE_ID 0x454B4350
#define PCKF_ID 0x464B4350

#define CCK0_ID 0x304B4343
#define CCK1_ID 0x314B4343
#define CCK2_ID 0x324B4343
#define CCK3_ID 0x334B4343
#define CCK4_ID 0x344B4343
#define CCK5_ID 0x354B4343
#define CCK6_ID 0x364B4343
#define CCK7_ID 0x374B4343
#define CCK8_ID 0x384B4343
#define CCK9_ID 0x394B4343
#define CCKA_ID 0x414B4343
#define CCKB_ID 0x424B4343
#define CCKC_ID 0x434B4343
#define CCKD_ID 0x444B4343
#define CCKE_ID 0x454B4343
#define CCKF_ID 0x464B4343

#define PRG0_ID 0x30475250
#define PRG1_ID 0x31475250
#define PRG2_ID 0x32475250
#define PRG3_ID 0x33475250
#define PRG4_ID 0x34475250
#define PRG5_ID 0x35475250
#define PRG6_ID 0x36475250
#define PRG7_ID 0x37475250
#define PRG8_ID 0x38475250
#define PRG9_ID 0x39475250
#define PRGA_ID 0x41475250
#define PRGB_ID 0x42475250
#define PRGC_ID 0x43475250
#define PRGD_ID 0x44475250
#define PRGE_ID 0x45475250
#define PRGF_ID 0x46475250

#define CHR0_ID 0x30524843
#define CHR1_ID 0x31524843
#define CHR2_ID 0x32524843
#define CHR3_ID 0x33524843
#define CHR4_ID 0x34524843
#define CHR5_ID 0x35524843
#define CHR6_ID 0x36524843
#define CHR7_ID 0x37524843
#define CHR8_ID 0x38524843
#define CHR9_ID 0x39524843
#define CHRA_ID 0x41524843
#define CHRB_ID 0x42524843
#define CHRC_ID 0x43524843
#define CHRD_ID 0x44524843
#define CHRE_ID 0x45524843
#define CHRF_ID 0x46524843

#if     UNIF_REVISION > 7
// UNIF revision 8 (if it ever comes out) will probably include this chunk type
#define WRTR_ID 0x52545257
#define WRTR_MARKER     ';'
#define WRTR_MARKER_S   ";"
#endif

#define BOARDNAME_MAXLEN        32              // chunk length, NOT string length
                                                //  string lentgh = chunk length - 1
#define UNIF_HEADER_START 0
#define UNIF_HEADER_LEN (sizeof (st_unif_header_t))

typedef struct
{
  char signature[4];                            // 0x55,0x4e,0x49,0x46 ("UNIF")
  unsigned int revision;                        // revision number
  unsigned char expansion[24];                  // reserved
} st_unif_header_t;

typedef struct
{
  unsigned int id;                              // chunk identification string
  unsigned int length;                          // data length, in little endian format
  void *data;
} st_unif_chunk_t;

// DINF chunk data
typedef struct
{
  char dumper_name[100];                        // name of the person who dumped the cart
  unsigned char day;                            // day of the month when cartridge was dumped
  unsigned char month;                          // month of the year when cartridge was dumped
  unsigned short year;                          // year during which the cartridge was dumped
  char dumper_agent[100];                       // name of the ROM-dumping means used (ASCII-Z string)
} st_dumper_info_t;


#define FDS_SIG_S       "FDS\x1A"
#define FDS_HEADER_START 0
#define FDS_HEADER_LEN  16


// the FAM define is a guess based on FAM2FDS, more info is needed about the
//  FAM format
#define FAM_HEADER_LEN  192


extern int nes_fds (void);
extern int nes_fdsl (st_rominfo_t *rominfo, char *output_str);
extern int nes_n (const char *name);
extern int nes_s (void);
extern int nes_pasofami (void);
extern int nes_ineshd (st_rominfo_t *rominfo);
extern int nes_ffe (st_rominfo_t *rominfo);
extern int nes_ines (void);
extern int nes_init (st_rominfo_t *rominfo);
extern int nes_unif (void);
extern int nes_j (unsigned char **mem_image);
extern int nes_dint (void);
extern nes_file_t nes_get_file_type (void);

#endif // NES_H
