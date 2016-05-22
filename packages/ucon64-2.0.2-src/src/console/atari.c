/*
atari.c - Atari 2600/5200/7800 support for uCON64

Copyright (c) 2004 - 2005 NoisyB (noisyb@gmx.net)

Inspired by code from makewav v4.1 and MakeBin v1.0, written by Bob Colbert
  <rcolbert1@home.com>
atari_init() uses some code from Stella - An Atari 2600 VCS Emulator
  Copyright (c) 1995-2005 Bradford W. Mott


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
#include "misc/archive.h"
#include "misc/file.h"
#include "misc/string.h"
#include "misc/term.h"
#include "ucon64_misc.h"
#include "console/atari.h"
#include "backup/cc2.h"
#include "backup/spsc.h"
#include "backup/yoko.h"


static st_ucon64_obj_t atari_obj[] =
  {
    {UCON64_ATA, WF_SWITCH}
  };

const st_getopt2_t atari_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Atari VCS 2600/Atari 5200 SuperSystem/Atari CX7800/Atari 2600 Jr",
      /*"1977/1982/1984/1986 Atari"*/
      NULL
    },
    {
      UCON64_ATA_S, 0, 0, UCON64_ATA,
      NULL, "force recognition",
      &atari_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#define ATARI_ROM_SIZE 0x20000
#define ATARI_ROM_HEADER_START 0
#define ATARI_ROM_HEADER_LEN 0


typedef struct
{
  int bsm;
  unsigned char ctrl_byte;
  unsigned char start_hi;
  unsigned char start_low;
  unsigned char speed_hi;
  unsigned char speed_low;
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
  int game_page_count;
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
  unsigned char empty_page[1024];
  unsigned char page_list[24];
  unsigned char multi_byte;
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
} st_atari_rominfo_t;
#ifdef  _MSC_VER
#pragma warning(pop)
#endif


static st_atari_rominfo_t atari_rominfo;


enum {
  BSM_2K = 1,
  BSM_AR,
  BSM_CV,
  BSM_4K,
  BSM_F8SC,
  BSM_F8,
  BSM_FE,
  BSM_3F,
  BSM_DPC,
  BSM_E0,
  BSM_FANR,
  BSM_FASC,
  BSM_FA,
  BSM_F6SC,
  BSM_F6,
  BSM_E7,
  BSM_E7NR,
  BSM_F4SC,
  BSM_F4,
  BSM_MB,
  BSM_MC,
  BSM_UA
};


typedef struct
{
  int bsm; // id
  char *bsm_s;
  uint32_t fsize;
  unsigned char ctrl_byte;
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
  int start_page;
#ifdef  _MSC_VER
#pragma warning(pop)
#endif
} st_atari_bsmode_t;


static st_atari_bsmode_t bsmodes[] = {
  {BSM_2K,   "2K",   0x800,   0xca, 7},
  {BSM_CV,   "CV",   0x800,   0xea, 7},
  {BSM_4K,   "4K",   0x1000,  0xc8, 15},
  {BSM_F8SC, "F8SC", 0x2000,  0xe6, 15},
  {BSM_F8,   "F8",   0x2000,  0xc6, 15},
  {BSM_FE,   "FE",   0x2000,  0xcc, 15},
  {BSM_3F,   "3F",   0x2000,  0xce, 31},
  {BSM_E0,   "E0",   0x2000,  0xc1, 31},
  {BSM_FANR, "FANR", 0x2000,  0xc0, 15},
  {BSM_UA,   "UA",   0x2000,  0,    0}, // ctrl_byte? start_page?
  {BSM_AR,   "AR",   0x2100,  0,    0}, // ctrl_byte? start_page?
  {BSM_DPC,  "DPC",  0x2800,  0,    0}, // ctrl_byte? start_page?
  {BSM_DPC,  "DPC",  0x28ff,  0,    0}, // ctrl_byte? start_page?
  {BSM_FA,   "FA",   0x3000,  0xe0, 15},
  {BSM_F6SC, "F6SC", 0x4000,  0xe4, 15},
  {BSM_F6,   "F6",   0x4000,  0xc4, 15},
  {BSM_E7,   "E7",   0x4000,  0xe3, 63},
  {BSM_E7NR, "E7NR", 0x4000,  0xc3, 63},
  {BSM_F4SC, "F4SC", 0x8000,  0xe2, 15},
  {BSM_F4,   "F4",   0x8000,  0xc2, 15},
  {BSM_MB,   "MB",   0x10000, 0xc9, 15},
  {BSM_MC,   "MC",   0x20000, 0,    0}, // ctrl_byte? start_page?
  {0,        NULL,   0,       0,    0}
};


typedef struct
{
  uint32_t crc;
  const char *md5;
  int bsmode;
} st_atari_game_bsmode_t;


st_atari_game_bsmode_t atari_bsmodes[] = {
  // CommaVid
  {0x34ae2945, NULL, BSM_CV},                   // Magicard (CommaVid)
//  {0x30eb4f7a, NULL, BSM_CV},                   // Video Life (4K)
//  {0x9afa761f, NULL, BSM_CV},                   // Magicard (Life)
  {0x266bd1b6, NULL, BSM_CV},                   // Video Life (CommaVid)
  // Parker Brothers
  {0x2843d776, NULL, BSM_E0},                   // Frogger II - Threedeep!
  {0x690ada72, NULL, BSM_E0},                   // Gyruss [b]
  {0x525ee7e9, NULL, BSM_E0},                   // Gyruss
  {0x95da4070, NULL, BSM_E0},                   // James Bond 007 [b]
  {0x3216c1bb, NULL, BSM_E0},                   // James Bond 007
  {0xae4114d8, NULL, BSM_E0},                   // Montezuma's Revenge
  {0x00e44527, NULL, BSM_E0},                   // Mr. Do!'s Castle
  {0xf723b8a6, NULL, BSM_E0},                   // Popeye
  {0xe44c244e, NULL, BSM_E0},                   // Q-bert's Qubes [a]
  {0xb8f2dca6, NULL, BSM_E0},                   // Q-bert's Qubes
  {0xe77f6742, NULL, BSM_E0},                   // Star Wars - Death Star Battle (Parker Bros)
  {0xce09fcd4, NULL, BSM_E0},                   // Star Wars - The Arcade Game (Parker Bros)
  {0xdd85f0e7, NULL, BSM_E0},                   // Super Cobra [b]
  {0x8d372730, NULL, BSM_E0},                   // Super Cobra
  {0xd9088807, NULL, BSM_E0},                   // Tooth Protectors (DSD-Camelot)
  {0x7eed7362, NULL, BSM_E0},                   // Tutankham
  {0xc87fc312, NULL, BSM_E0},                   // Popeye_(eks)
  {0xef3ec01e, NULL, BSM_E0},                   // Super Cobra_(eks)
  {0x84a101d4, NULL, BSM_E0},                   // Star Wars - Death Star Battle_(eks)
  {0x2fc06cb0, NULL, BSM_E0},                   // Tutankham_(eks)
  {0xab50bf11, NULL, BSM_E0},                   // Star Wars - The Arcade Game (proto)
  {0x549a1b6b, NULL, BSM_E0},                   // Star Wars - The Arcade Game (PAL)
  {0x36910e4d, NULL, BSM_E0},                   // Frogger II - Threedeep! (PAL)
  {0xb8bb2361, NULL, BSM_E0},                   // Gyruss (PAL)
  // Tigervision
  {0x584f6777, NULL, BSM_3F},                   // Espial [b]
  {0x8d70fa42, NULL, BSM_3F},                   // Espial
  {0x8beb03d4, NULL, BSM_3F},                   // Miner 2049er [b1]
  {0x33f2856f, NULL, BSM_3F},                   // Miner 2049er [b2]
  {0xf859122e, NULL, BSM_3F},                   // Miner 2049er Vol. 2 [b1]
  {0x281a1ca1, NULL, BSM_3F},                   // Miner 2049er Vol. 2 [b2]
  {0x350c63ba, NULL, BSM_3F},                   // Miner 2049er Vol. 2
  {0x728b941c, NULL, BSM_3F},                   // Miner 2049er
  {0x13bf2da3, NULL, BSM_3F},                   // Polaris [b]
  {0x7ce5312e, NULL, BSM_3F},                   // Polaris
  {0x40706361, NULL, BSM_3F},                   // River Patrol (Tigervision)
  {0x2c34898f, NULL, BSM_3F},                   // Springer
  // Activision 8K flat model
  {0x7d23e780, NULL, BSM_FE},                   // Decathlon
  {0xa51c0236, NULL, BSM_FE},                   // Robot Tank
  {0xd8ecf576, NULL, BSM_FE},                   // Decathlon (PAL)
  {0x0e8757b0, NULL, BSM_FE},                   // Robot Tank (PAL)
  {0x94e8df6b, NULL, BSM_FE},                   // Space Shuttle (PAL)
  // 16K Superchip that can't be recognized automatically
  {0xa972c32b, NULL, BSM_F6SC},                 // Dig Dug
  {0x66cdb94b, NULL, BSM_F6SC},                 // Off the Wall [o]
  {0xbd75d92b, NULL, BSM_F6SC},                 // Off the Wall
  // M Network 16K
  {0x8eed6b02, NULL, BSM_E7},                   // Bump n Jump [b]
  {0xd523e776, NULL, BSM_E7},                   // Bump n Jump
  {0x24c35820, NULL, BSM_E7},                   // Burgertime
  {0x5c161fe4, NULL, BSM_E7},                   // Masters of the Universe - The Power of He-Man
//  {0xbe1047cf, NULL, 0},                        // Fatal Run (PAL)
//  {0x6a31beac, NULL, 0},                        // Private Eye (CCE)
//  {0x3fa749c0, NULL, 0},                        // Private Eye [b]
//  {0x33242242, NULL, 0},                        // Private Eye
  // from Stella sources
  {0, "5336f86f6b982cc925532f2e80aa1e17", BSM_E0},    // Death Star
  {0, "b311ab95e85bc0162308390728a7361d", BSM_E0},    // Gyruss
  {0, "c29f8db680990cb45ef7fef6ab57a2c2", BSM_E0},    // Super Cobra
  {0, "085322bae40d904f53bdcc56df0593fc", BSM_E0},    // Tutankamn
  {0, "c7f13ef38f61ee2367ada94fdcc6d206", BSM_E0},    // Popeye
  {0, "6339d28c9a7f92054e70029eb0375837", BSM_E0},    // Star Wars, Arcade
  {0, "27c6a2ca16ad7d814626ceea62fa8fb4", BSM_E0},    // Frogger II
  {0, "3347a6dd59049b15a38394aa2dafa585", BSM_E0},    // Montezuma's Revenge
  {0, "6dda84fb8e442ecf34241ac0d1d91d69", BSM_F6SC},  // Dig Dug
  {0, "57fa2d09c9e361de7bd2aa3a9575a760", BSM_F8SC},  // Stargate
  {0, "3a771876e4b61d42e3a3892ad885d889", BSM_F8SC},  // Defender ][
  {0, "efefc02bbc5258815457f7a5b8d8750a", BSM_FASC},  // Tunnel runner
  {0, "7e51a58de2c0db7d33715f518893b0db", BSM_FASC},  // Mountain King
  {0, "9947f1ebabb56fd075a96c6d37351efa", BSM_FASC},  // Omega Race
  {0, "0443cfa9872cdb49069186413275fa21", BSM_E7},    // Burger Timer
  {0, "76f53abbbf39a0063f24036d6ee0968a", BSM_E7},    // Bump-N-Jump
  {0, "3b76242691730b2dd22ec0ceab351bc6", BSM_E7},    // He-Man
  {0, "ac7c2260378975614192ca2bc3d20e0b", BSM_FE},    // Decathlon
  {0, "4f618c2429138e0280969193ed6c107e", BSM_FE},    // Robot Tank
  {0, "6d842c96d5a01967be9680080dd5be54", BSM_DPC},   // Pitfall II
  {0, "d3bb42228a6cd452c111c1932503cc03", BSM_UA},    // Funky Fish
  {0, "8bbfd951c89cc09c148bfabdefa08bec", BSM_UA},    // Pleiades
  {0, NULL, 0}
};


static st_atari_bsmode_t *
get_bsmode_by_id (int bsm)
{
  int i = 0;

  while (bsmodes[i].bsm)
    {
      if (bsmodes[i].bsm == bsm)
        return &bsmodes[i];
      i++;
    }

  return NULL;
}


static int
get_game_bsmode_by_crc (uint32_t crc)
{
  int i = 0;

  while (atari_bsmodes[i].bsmode)
    {
      if (atari_bsmodes[i].crc)
        if (atari_bsmodes[i].crc == crc)
          return atari_bsmodes[i].bsmode;
      i++;
    }

  return -1;
}


static int
get_game_bsmode_by_md5 (const char *md5)
{
  int i = 0;

  while (atari_bsmodes[i].bsmode)
    {
      if (atari_bsmodes[i].md5)
        if (!stricmp (atari_bsmodes[i].md5, md5))
          return atari_bsmodes[i].bsmode;
      i++;
    }

  return -1;
}


static int
is_probably_3f (const unsigned char *image, unsigned int size)
{
  unsigned long count = 0, i;

  for (i = 0; i < size - 1; i++)
    if ((image[i] == 0x85) && (image[i + 1] == 0x3F))
      ++count;

  return count > 2 ? TRUE : FALSE;
}


int
atari_init (st_ucon64_nfo_t * rominfo)
{
  int i, j, bsmode, size = ucon64.file_size;
  unsigned int crc32;
  static char backup_usage[80];
  unsigned char first, image[ATARI_ROM_SIZE], buffer[0x200];
  char md5[32];

  if (size > ATARI_ROM_SIZE)
    return -1;

  if (size != 0x800  && size != 0x1000  && size != 0x2000 && size != 0x2100 &&
      size != 0x2800 && size != 0x28ff  && size != 0x3000 && size != 0x4000 &&
      size != 0x8000 && size != 0x10000 && size != 0x20000)
    return -1;

  ucon64_fread (image, 0, size, ucon64.fname);
  ucon64_chksum (NULL, md5, &crc32, ucon64.fname, ucon64.file_size, 0);

  bsmode = get_game_bsmode_by_crc (crc32);
  if (bsmode == -1)
    bsmode = get_game_bsmode_by_md5 (md5);

  if (bsmode == -1)
    {
      if (!(size % 8448))
        bsmode = BSM_AR;
      else if (size == 2048 || !memcmp (image, image + 2048, 2048))
        bsmode = BSM_2K;
      else if (size == 4096 || !memcmp (image, image + 4096, 4096))
        bsmode = BSM_4K;
      else if (size == 8192 || !memcmp (image, image + 8192, 8192))
        bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_F8;
      else if (size == 10495 || (size == 10240))
        bsmode = BSM_DPC;
      else if (size == 12288)
        bsmode = BSM_FASC;
      else if (size == 32768)
        {
          // Assume this is a 32K super-cart then check to see if it is
          bsmode = BSM_F4SC;

          first = image[0];
          for (i = 0; i < 0x100; i++)
            if (image[i] != first)
              {
                // It's not a super cart (probably)
                bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_F4;
                break;
              }
        }
      else if (size == 65536)
        bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_MB;
      else if (size == 131072)
        bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_MC;
      else
        {
          // Assume this is a 16K super-cart then check to see if it is
          bsmode = BSM_F6SC;

          first = image[0];
          for (i = 0; i < 0x100; i++)
            if (image[i] != first)
              {
                // It's not a super cart (probably)
                bsmode = is_probably_3f (image, size) ? BSM_3F : BSM_F6;
                break;
              }
        }
    }

  if (bsmode > 0)
    {
      memset (&atari_rominfo, 0, sizeof (st_atari_rominfo_t));

      atari_rominfo.bsm = bsmode;

      // set game_page_count and empty_page[]
      for (i = 0; i < size / 0x100; i++)
        {
          ucon64_fread (buffer, i * 0x100, 0x100, ucon64.fname);
          atari_rominfo.empty_page[i] = 1;

          for (j = 0; j < 0x100 - 1; j++)
            if (buffer[j] != buffer[j + 1])
              {
                atari_rominfo.empty_page[i] = 0;
                atari_rominfo.game_page_count++;
                break;
              }
        }

      atari_rominfo.speed_hi = (unsigned char) (atari_rominfo.game_page_count / 21 + 1);
      atari_rominfo.speed_low = (unsigned char) (atari_rominfo.game_page_count * 0x100 / 21 - (atari_rominfo.speed_hi - 1) * 0x100);

      atari_rominfo.ctrl_byte = get_bsmode_by_id (atari_rominfo.bsm)->ctrl_byte;

      // the first two bytes of data indicate the beginning address of the code
      if (atari_rominfo.bsm != BSM_3F)
        {
          ucon64_fread (buffer, get_bsmode_by_id (atari_rominfo.bsm)->start_page * 0x100, 0x100, ucon64.fname);
          atari_rominfo.start_low = buffer[0xfc];
          atari_rominfo.start_hi = buffer[0xfd];
        }

      rominfo->console_usage = atari_usage[0].help;
      // "Cuttle Card (2)/Starpath) Supercharger/YOKO backup unit"
      snprintf (backup_usage, 80, "%s/%s/%s", cc2_usage[0].help,
                spsc_usage[0].help, yoko_usage[0].help);
      backup_usage[80 - 1] = 0;
      rominfo->backup_usage = backup_usage;

      sprintf (rominfo->misc,
               "Bankswitch type: %s\n"
               "Start address: 0x%02x%02x\n"
               "Speed: 0x%02x%02x\n"
               "Control byte: 0x%02x\n"
               "Page count: %d\n"
               "Blank pages: %d\n"
               "Start page: %d",
               get_bsmode_by_id (atari_rominfo.bsm)->bsm_s,
               atari_rominfo.start_hi,
               atari_rominfo.start_low,
               atari_rominfo.speed_hi,
               atari_rominfo.speed_low,
               atari_rominfo.ctrl_byte,
               atari_rominfo.game_page_count,
               size / 0x100 - atari_rominfo.game_page_count,
               get_bsmode_by_id (atari_rominfo.bsm)->start_page);
      return 0;
    }

  return -1;
}
