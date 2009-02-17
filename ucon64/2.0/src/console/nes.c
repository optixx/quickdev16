/*
nes.c - Nintendo Entertainment System support for uCON64

Copyright (c) 1999 - 2003 NoisyB <noisyb@gmx.net>
Copyright (c) 2002 - 2004 dbjh


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
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>                             // va_list()
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/bswap.h"
#include "misc/misc.h"
#include "misc/string.h"
#include "misc/chksum.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "nes.h"
#include "backup/smc.h"


#define STD_COMMENT     "Written with uCON64 "  // first part of text to put in READ chunk


const st_getopt2_t nes_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Nintendo Entertainment System/NES/Famicom/Game Axe (Redant)"
      /*"1983 Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      "nes", 0, 0, UCON64_NES,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "n", 1, 0, UCON64_N,
      "NEW_NAME", "change internal ROM name to NEW_NAME (UNIF only)",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      "unif", 0, 0, UCON64_UNIF,
      NULL, "convert to UNIF format/UNF (uses default values)",
      &ucon64_wf[WF_OBJ_NES_DEFAULT]
    },
    {
      "ines", 0, 0, UCON64_INES,
      NULL, "convert to iNES format/NES (uses default values)",
      &ucon64_wf[WF_OBJ_NES_DEFAULT]
    },
    {
      "ineshd", 0, 0, UCON64_INESHD,
      NULL, "extract iNES header from ROM (16 Bytes)",
      &ucon64_wf[WF_OBJ_NES_DEFAULT]
    },
    {
      "j", 0, 0, UCON64_J,
      NULL, "join Pasofami/PRM/700/PRG/CHR/split ROM (Pasofami -> iNES)",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE]
    },
    {
      "pasofami", 0, 0, UCON64_PASOFAMI,
      NULL, "convert to Pasofami/PRM/700/PRG/CHR",
      &ucon64_wf[WF_OBJ_NES_DEFAULT]
    },
    {
      "s", 0, 0, UCON64_S,
      NULL, "convert/split to Pasofami/PRM/700/PRG/CHR (iNES -> Pasofami)",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT_NO_SPLIT]
    },
    {
      "ffe", 0, 0, UCON64_FFE,
      NULL, "convert to FFE format (Super Magic Card)",
      &ucon64_wf[WF_OBJ_NES_DEFAULT]
    },
    {
      "mapr", 1, 0, UCON64_MAPR,
      "MAPR", "specify board name or mapper number for conversion options\n"
      "MAPR must be a board name for UNIF or a number for Pasofami\n"
      "and iNES",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "dint", 0, 0, UCON64_DINT,
      NULL, "deinterleave ROM (regardless whether the ROM is interleaved)",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_NO_SPLIT]
    },
    {
      "ctrl", 1, 0, UCON64_CTRL,
      "TYPE", "specify controller type (UNIF only)\n"
      "TYPE=0 regular joypad\n"
      "TYPE=1 zapper\n"
      "TYPE=2 R.O.B.\n"
      "TYPE=3 Arkanoid controller\n"
      "TYPE=4 powerpad\n"
      "TYPE=5 four-score adapter",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "ntsc", 0, 0, UCON64_NTSC,
      NULL, "specify TV standard is NTSC (UNIF only)",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "pal", 0, 0, UCON64_PAL,
      NULL, "specify TV standard is PAL (UNIF only)",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "bat", 0, 0, UCON64_BAT,
      NULL, "specify battery is present",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "nbat", 0, 0, UCON64_NBAT,
      NULL, "specify battery is not present",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "vram", 0, 0, UCON64_VRAM,
      NULL, "specify VRAM override (UNIF only)",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "nvram", 0, 0, UCON64_NVRAM,
      NULL, "specify no VRAM override (UNIF only)",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "mirr", 1, 0, UCON64_MIRR,
      "MTYPE", "specify mirroring type\n"
      "MTYPE=0 horizontal mirroring\n"
      "MTYPE=1 vertical mirroring\n"
      "MTYPE=2 mirror all pages from $2000 (UNIF only)\n"
      "MTYPE=3 mirror all pages from $2400 (UNIF only)\n"
      "MTYPE=4 four screens of VRAM\n"
      "MTYPE=5 mirroring controlled by mapper hardware (UNIF only)",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
#if     UNIF_REVISION > 7
    {
      "cmnt", 1, 0, UCON64_CMNT,
      "TEXT", "specify that TEXT should be used as comment (UNIF only)",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
#endif
    {
      "dumpinfo", 1, 0, UCON64_DUMPINFO,
      "FILE", "use dumper info from FILE when converting to UNIF",
      &ucon64_wf[WF_OBJ_NES_SWITCH]
    },
    {
      "fds", 0, 0, UCON64_FDS,
      NULL, "convert Famicom Disk System file (diskimage) from FAM to FDS",
      &ucon64_wf[WF_OBJ_NES_DEFAULT]
    },
    {
      "fdsl", 0, 0, UCON64_FDSL,
      NULL, "list Famicom Disk System/FDS (diskimage) contents",
      &ucon64_wf[WF_OBJ_NES_DEFAULT]
    },
#if 0
    {
      "fam", 0, 0, UCON64_FAM,
      NULL, "convert Famicom Disk System file (diskimage) from FDS to FAM",
      NULL
    },
    {
      "tr", 0, 0, UCON64_TR,
      NULL, "truncate doubled PRG/CHR",
      NULL
    },
    {
      "nfs", 0, 0, UCON64_NFS,
      NULL, "convert NFS sound to WAV; " OPTION_LONG_S "rom=NFSFILE",
      NULL
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

#if 0
const char *nes_boardtypes =
  {
    "351258: UNROM\n"
    "351298: UNROM\n"
    "351908\n"
    "352026: TLROM (w/ LS32 for VROM enable control)\n"
    "51555: Acclaim, MMC3B mapper, PRG ROM, CHR ROM\n"
    "53361\n"
    "54425\n"
    "55741\n"
    "56504\n"
    "AMROM: LS161, VRAM, PRG-ROM\n"
    "ANROM: LS161+LS02 mapper, PRG-ROM, CHR-RAM\n"
    "AOROM: LS161 mapper, PRG-ROM, CHR-ROM\n"
    "BNROM: LS161, VRAM, PRG-ROM (Different LS161 bits?  Only used on Deadly Towers)\n"
    "CNROM: LS161 mapper, PRG-ROM, CHR-ROM?/CHR-RAM\n"
    "COB:   \"Glop Top\" style board\n"
    "CPROM: LS04, LS08, LS161, 32K ROM, 16K VRAM (bankswitched, Videomation only)\n"
    "DEIROM\n"
    "DEROM\n"
    "DRROM: MMC3, 4K of nametable RAM (for 4-screen), PRG-ROM, CHR-ROM (only in Gauntlet)\n"
    "EKROM\n"
    "ELROM: MMC5, PRG-ROM, CHR-ROM\n"
    "ETROM: MMC5, PRG-ROM, CHR-ROM, 2x 8k optionnal RAM (battery)\n"
    "EWROM: MMC5, PRG-ROM, CHR-ROM, 32k optionnal RAM (battery)\n"
    "GNROM: LS161 mapper, PRG ROM, CHR ROM\n"
    "HKROM: MMC6B, PRG-ROM, CHR-ROM, Battery\n"
    "MHROM: LS161 mapper, black blob chips. Mario Bros / Duck Hunt multi\n"
    "NES-B4: Same as TLROM\n"
    "NES-BTR: Sunsoft FME7 mapper, PRG ROM, CHR ROM, 8k optionnal RAM \n"
    "NES-QJ:\n"
    "NES-RROM: Same as NROM (Only used in Clu Clu land)\n"
    "NROM: No mapper, PRG-ROM, CHR-ROM\n"
    "PNROM: MMC2, PRG-ROM, CHR-ROM\n"
    "SAROM: MMC1B, PRG ROM, CHR ROM, optional 8k of RAM (battery)\n"
    "SBROM: MMC1A, PRG ROM, CHR ROM (only 32K of CHR ROM)\n"
    "SCEOROM\n"
    "SC1ROM: MMC1B, PRG ROM, CHR ROM\n"
    "SCROM: LS161, LS02, VRAM, PRG-ROM (Similar to UNROM)\n"
    "SEROM: MMC1B, PRG ROM, CHR ROM\n"
    "SFROM\n"
    "SGROM: MMC1B, PRG ROM, 8k CHR RAM\n"
    "SHROM\n"
    "SJROM\n"
    "SKROM: MMC1B, PRG ROM, CHR ROM, 8k optional RAM (battery)\n"
    "SL1ROM: MMC3, PRG ROM, CHR ROM, LS32 (for 128K 28 pin CHR ROMs)\n"
    "SL2ROM\n"
    "SL3ROM\n"
    "SLROM: MMC1A, PRG ROM, CHR ROM\n"
    "SLRROM\n"
    "SN1-ROM AW (Overlord only)\n"
    "SNROM: MMC1A, PRG ROM, CHR ROM/RAM ?, 8k optional RAM (battery)  \n"
    "SOROM: MMC1B2, PRG ROM, VRAM, 16K of WRAM (Battery) Only 8K battery-backed\n"
    "SVROM: MMC1B2, PRG ROM, VRAM, WRAM (Battery)\n"
    "SUROM: MMC1B2, PRG ROM, CHR RAM/(ROM?), 8k battery-backed RAM (DW4?)\n"
    "TEROM: MMC3A, PRG ROM, CHR ROM, (32k ROMs)\n"
    "TFROM: MMC3B, PRG ROM, CHR ROM (64K of CHR only)\n"
    "TGROM: MMC3C, PRG ROM, VRAM (512K of PRG)\n"
    "TKROM: MMC3A, PRG ROM, CHR ROM, 8k optional RAM (battery)\n"
    "TL1ROM: Same as TLROM\n"
    "TLROM: MMC3B, PRG ROM, CHR ROM\n"
    "TLSROM: Same as TLROM\n"
    "TQROM: MMC3B+74HC32, PRG ROM, CHR ROM + 8k of CHR-RAM\n"
    "TSROM: MMC3A, PRG ROM, CHR ROM, 8k optionnal RAM\n"
    "TVROM: MMC3B, PRG ROM, CHR ROM, 4K of Nametable RAM (4-screen)\n"
    "UNROM: 74LS32+74LS161 mapper, 128k PRG, 8k CHR-RAM\n"
    "UOROM\n"
  };
#endif

#define NES_MAKER_MAX 203
static const char *nes_maker[NES_MAKER_MAX] =
  {
    NULL, "A-Wave", "ASCII", "Absolute Entertainment", "Acclaim",
    "Active Enterprises", "Activision", "Activision/Bullet Proof", "Akujin", "Altron",
    "American Game Carts", "American Sammy Corp.", "American Sammy", "American Softworks",
      "American Technos",
    "American Video", "Angel", "Arcadia Systems", "Asder", "Ask Koudansha",
    "Asmik", "Athena", "Atlus", "BPS", "Bandai",
    "Banpresto", "Beam Software", "Beam Software/Mattel", "Bothtec", "Broderbund",
    "Bullet-Proof", "Bunch Games", "CBS Sony Group", "Camerica", "Capcom",
    "Cart-Saint", "Casady", "Character Soft", "Chris Covell", "Coconuts Japan",
    "Color Dreams", "Cony", "Crush Productions", "Culture Brain", "DB Soft",
    "Darknight13", "Data East", "Disconnected Translations", "Dragoon-X", "EAV",
    "Electro Brain", "Electronic Arts", "Elite", "Enix", "Epic-Sony Records",
    "Epoch Yashiro", "Eurocom", "FCI", "FCI/Pony Canyon", "Face",
    "Fuji Television", "G.O 1", "Gaijin Productions", "Gakushuu Kenkyuusha", "Galoob",
    "GameTek", "Grimlick", "Hal America Inc.", "Hal America", "Hal Kenkyuujo",
    "Hal", "Halken", "Hector", "Hero", "Hi Tech Expressions",
    "High Score Media Work", "Hot B", "Hudson Soft", "Human", "HummingBird Soft",
    "I'Max", "IGS", "INTV", "Ian Bell", "Imagineer",
    "Induction Produce", "Infocom", "Infogrames", "Interplay", "Irem",
    "J2E Translations", "JVC", "Jaleco", "K Amusement Lease", "Kakenhi",
    "Kasady", "Kawada", "Kemco", "King Records", "Koei",
    "Konami", "Kotobuki System", "Kyuugo Boueki", "LHN", "LJN",
    "Lucasfilm Games", "M&M", "Manipulate", "Masiya", "Matchbox",
    "Mattel", "Meldac", "Memblers", "Microprose", "Milton Bradley",
    "Mindscape", "NTVIC", "Namco", "Namcot", "Nameo",
    "Natsume", "Naxat", "Neo Demiforce", "Nexoft", "Nihon Bussan",
    "Nihon Computer System", "Nintendo", "Ocean", "Pack-In Video", "Palcom",
    "Panesian", "Parker Brothers", "Pixel", "Pony Canyon", "Pope Hentai",
    "Proto", "Quest", "RCM Group", "RKH Translation", "RPGe",
    "Rage Games", "Rare Ltd.", "Romstar", "SNK", "Sammy",
    "Sanritsu Denki", "Seta USA", "Seta", "Sigma Shouji", "Sigma",
    "Sofel", "Soft Pro", "Software Toolworks", "Sony Imagesoft", "Square Soft",
    "Square", "Sunsoft", "SuperVision", "T*HQ", "TSS",
    "Taito", "Takara", "Takeru", "Taxan", "Technos Japan",
    "Tecmo", "Tengen", "Tengen/Mindscape", "Tennessee Carmel-Veilleux", "The Mad Hacker",
    "The Sales Curve", "The Spoony Bard", "Titus Software", "Toho", "Tokuma Shoten",
    "Tomy", "Tonkin House", "Tony Young", "Touei Douga", "Toukyou Shoseki",
    "Toushiba EMI", "Towa Chiki", "Tradewest", "TransBRC", "Triffix",
    "Tsang Hai", "UBI Soft", "UPL", "Ultra Games", "Ultra Games/Konami",
    "Use", "Vap", "Varie", "Vic Tokai", "Victor Musical Industries",
    "Victor Ongaku Sangyou", "Virgin Games", "Visco", "Wakd Hacks", "Will B.",
    "Wisdom Tree", "Yonezawa", "Yutaka"
  };

#define NES_COUNTRY_MAX 4
static const char *nes_country[NES_COUNTRY_MAX] =
  {
    "Japan",
    "U.S.A.",
    "Europe, Oceania & Asia",                   // Australia is part of Oceania
    NULL
  };

typedef struct
{
  uint32_t crc32;
  uint8_t maker;
  uint8_t country;
  uint16_t date;
} st_nes_data_t;

static const st_nes_data_t nes_data[] =
{
  {0x00161afd, 195, 0, 0},
  {0x0021ed29, 133, 0, 0},
  {0x003a1bd1, 133, 0, 0},
  {0x004e5381, 117, 0, 0},
  {0x005682d5, 63, 0, 0},
  {0x005ff9a8, 0, 1, 0},
  {0x00837960, 100, 1, 692},
  {0x0091afbe, 92, 0, 0},
  {0x009af6be, 65, 1, 390},
  {0x009da4c0, 81, 0, 0},
  {0x00ad1189, 129, 2, 0},
  {0x00b82d5c, 166, 1, 0},
  {0x00bec4d7, 104, 1, 1092},
  {0x00e95d86, 186, 1, 1193},
  {0x00f0fbbc, 126, 0, 0},
  {0x011adaa1, 140, 0, 0},
  {0x0123bffe, 196, 1, 793},
  {0x0147766c, 143, 0, 86},
  {0x014a755a, 126, 2, 0},
  {0x0154c43f, 193, 0, 0},
  {0x016c93d8, 77, 0, 0},
  {0x018a8699, 104, 1, 1091},
  {0x01901d84, 95, 0, 0},
  {0x01934171, 0, 1, 0},
  {0x01a1509a, 131, 1, 690},
  {0x01b418c5, 77, 1, 988},
  {0x01b4ca89, 154, 1, 989},
  {0x01be8b6f, 0, 1, 0},
  {0x01e06ab1, 46, 0, 0},
  {0x01ee0720, 40, 1, 0},
  {0x021193b2, 53, 0, 0},
  {0x022589b9, 165, 0, 0},
  {0x02361407, 179, 0, 0},
  {0x023a5a32, 126, 1, 1085},
  {0x023c7774, 46, 1, 88},
  {0x02589598, 155, 0, 0},
  {0x026a41ee, 4, 1, 992},
  {0x026c5fca, 24, 0, 0},
  {0x026e41c5, 115, 1, 790},
  {0x02738c68, 117, 0, 0},
  {0x02863604, 178, 0, 0},
  {0x028912ba, 166, 1, 0},
  {0x02931525, 34, 1, 690},
  {0x0296e5f4, 0, 1, 0},
  {0x02a7ad0f, 195, 0, 0},
  {0x02b0b405, 92, 0, 0},
  {0x02b20ca7, 191, 0, 0},
  {0x02b26e69, 64, 0, 0},
  {0x02b9e7c2, 24, 1, 689},
  {0x02cc3973, 24, 1, 1086},
  {0x02cceca7, 44, 0, 0},
  {0x02d7976b, 2, 0, 0},
  {0x02e0ada4, 92, 2, 91}, // Totally Rad (E)
  {0x02e20d38, 0, 1, 0},
  {0x02e67223, 143, 1, 87},
  {0x02ed6298, 166, 1, 0},
  {0x02ee3706, 34, 1, 789},
  {0x03272e9b, 193, 1, 191},
  {0x03349045, 0, 1, 0},
  {0x0336f9f3, 146, 1, 89},
  {0x035cc54b, 92, 0, 0},
  {0x035dc2e9, 126, 1, 1085},
  {0x0364c3ec, 91, 1, 690},
  {0x0373a432, 66, 0, 0},
  {0x03840ec5, 142, 1, 1289},
  {0x039b4a9c, 156, 0, 0},
  {0x03d56cf7, 84, 0, 0},
  {0x03dcfddb, 72, 0, 0},
  {0x03e2898f, 126, 0, 0},
  {0x03ec46af, 156, 1, 1291},
  {0x03eeaeb7, 131, 1, 591},
  {0x03f899cd, 74, 1, 191},
  {0x03fb57b6, 111, 1, 991},
  {0x040c8685, 126, 1, 387},
  {0x04109355, 89, 0, 0},
  {0x04142764, 52, 2, 0},
  {0x042c6f99, 153, 1, 993},
  {0x042e9c4d, 12, 1, 990},
  {0x042f17c4, 2, 0, 0},
  {0x045e8cd8, 128, 0, 0},
  {0x04766130, 97, 1, 1092},
  {0x049325d9, 18, 0, 0},
  {0x04977e62, 156, 0, 0},
  {0x04be2206, 77, 0, 0},
  {0x04d0d47c, 126, 0, 0},
  {0x04d6b4f6, 97, 0, 0},
  {0x04f3354d, 104, 1, 289},
  {0x0504b007, 126, 1, 1085},
  {0x0516375e, 126, 1, 1085},
  {0x051cd5f2, 39, 0, 0},
  {0x0537322a, 4, 1, 890},
  {0x05378607, 92, 1, 391},
  {0x053c4699, 20, 0, 0},
  {0x0546bd12, 173, 0, 0},
  {0x054cb4eb, 92, 1, 390},
  {0x0556dc12, 161, 0, 0},
  {0x055fb6e5, 0, 1, 0},
  {0x0573f281, 155, 0, 0},
  {0x058f23a2, 89, 1, 790},
  {0x059e0cdf, 50, 1, 1191},
  {0x05a688c8, 150, 1, 1089},
  {0x05b05500, 0, 1, 0},
  {0x05b3f461, 173, 1, 790},
  {0x05b7b507, 0, 1, 0},
  {0x05c97f64, 182, 0, 0},
  {0x05ce560c, 24, 1, 192},
  {0x05cf9eb0, 34, 0, 0},
  {0x05d45a1a, 95, 0, 0},
  {0x05d70600, 166, 1, 0},
  {0x05de5afd, 193, 1, 690},
  {0x05f04eac, 160, 0, 0},
  {0x05f76a57, 0, 1, 0},
  {0x05fe773b, 182, 1, 489},
  {0x06093b5e, 34, 0, 0},
  {0x060f6e75, 77, 0, 0},
  {0x06144b4a, 89, 0, 0},
  {0x062ec2b9, 24, 0, 0},
  {0x0630c234, 200, 1, 0},
  {0x0639e88e, 0, 1, 0},
  {0x063e5653, 127, 1, 893},
  {0x066f5a83, 116, 1, 190},
  {0x067bd24c, 99, 0, 0},
  {0x06961be4, 51, 1, 990},
  {0x06a4345c, 115, 1, 790},
  {0x06b0556c, 77, 1, 992},
  {0x06bb007a, 156, 0, 0},
  {0x06c4fd92, 51, 1, 990},
  {0x06cae67f, 100, 1, 192},
  {0x06d72c83, 163, 1, 489},
  {0x06e97649, 1, 0, 0},
  {0x06f05358, 133, 0, 0},
  {0x06f9c714, 92, 0, 0},
  {0x0719260c, 72, 0, 0},
  {0x071f2e80, 57, 1, 490},
  {0x07259ba7, 97, 0, 0},
  {0x07299793, 126, 1, 1093},
  {0x072b8659, 132, 0, 0},
  {0x0736e382, 46, 1, 1190},
  {0x074ec424, 43, 0, 0},
  {0x075a69e6, 160, 2, 0},
  {0x076b00ab, 120, 0, 0},
  {0x07838ef9, 126, 0, 0},
  {0x07854b3f, 126, 1, 1088},
  {0x0786471d, 120, 0, 0},
  {0x078ced30, 43, 0, 0},
  {0x0794f2a5, 53, 0, 0},
  {0x07964c82, 90, 0, 0},
  {0x07977186, 43, 0, 0},
  {0x07af9301, 46, 0, 0},
  {0x07dbd082, 74, 1, 191},
  {0x07e20334, 115, 1, 993},
  {0x07ee6d8f, 160, 0, 0},
  {0x08077383, 165, 1, 787},
  {0x082d63c7, 100, 0, 0},
  {0x083c02c1, 0, 1, 0},
  {0x083e4fc1, 129, 2, 0},
  {0x0847c623, 74, 1, 189},
  {0x0857df48, 166, 1, 0},
  {0x085de7c9, 6, 1, 192},
  {0x08626711, 93, 0, 0},
  {0x0897021b, 156, 0, 0},
  {0x08b04364, 100, 0, 0},
  {0x08e2af80, 127, 1, 91},
  {0x08ebde64, 15, 1, 0},
  {0x0902c8f0, 97, 0, 0},
  {0x0905e0d5, 39, 0, 0},
  {0x091ed5a9, 32, 0, 0},
  {0x092b5371, 24, 1, 1086},
  {0x092ec15c, 179, 0, 0},
  {0x093311aa, 166, 1, 0},
  {0x0939852f, 115, 1, 990},
  {0x093e845f, 124, 0, 0},
  {0x09499f4d, 24, 0, 0},
  {0x094afab5, 156, 0, 0},
  {0x0971cc4f, 100, 0, 0},
  {0x0973f714, 195, 0, 0},
  {0x09874777, 114, 1, 389},
  {0x098c672a, 117, 0, 0},
  {0x09948780, 100, 0, 0},
  {0x099b8caa, 120, 0, 0},
  {0x09a6ff02, 34, 0, 0},
  {0x09ae2e75, 89, 0, 0},
  {0x09c083b7, 100, 1, 689},
  {0x09c1fc7d, 34, 0, 0},
  {0x09c31cd4, 31, 1, 0},
  {0x09e4c3e0, 34, 2, 0},
  {0x09eefde3, 165, 0, 0},
  {0x09efe54b, 174, 0, 0},
  {0x09ffdf45, 77, 0, 0},
  {0x0a0926bd, 109, 1, 192},
  {0x0a1bcd91, 133, 0, 0},
  {0x0a2490af, 126, 1, 387},
  {0x0a42d84f, 76, 0, 0},
  {0x0a640eea, 24, 0, 0},
  {0x0a68452e, 43, 0, 0},
  {0x0a686042, 46, 1, 491},
  {0x0a73a792, 181, 0, 0},
  {0x0a7767eb, 16, 0, 0},
  {0x0a7e62d4, 113, 1, 1292},
  {0x0a866c94, 100, 1, 891},
  {0x0ab06c51, 68, 1, 1088},
  {0x0ab26db6, 200, 1, 0},
  {0x0ab39900, 174, 0, 0},
  {0x0abdd5ca, 23, 0, 0},
  {0x0ac1aa8f, 100, 1, 587},
  {0x0ac631ba, 100, 1, 89},
  {0x0ac65c40, 100, 0, 0},
  {0x0ae3cc5e, 46, 0, 0},
  {0x0ae6c9e2, 184, 1, 691},
  {0x0af937d1, 160, 1, 1291},
  {0x0afb395e, 100, 0, 0},
  {0x0b0d4d1b, 2, 0, 0},
  {0x0b3d7b44, 165, 0, 0},
  {0x0b404915, 115, 1, 991},
  {0x0b4c197a, 92, 0, 0},
  {0x0b4c2ddc, 117, 0, 0},
  {0x0b561ad2, 72, 0, 0},
  {0x0b5667e9, 126, 2, 0},
  {0x0b57cfda, 0, 1, 0},
  {0x0b58880c, 126, 0, 0},
  {0x0b6443d4, 195, 0, 0},
  {0x0b65a917, 126, 0, 0},
  {0x0b7f1947, 77, 0, 0},
  {0x0b8e8649, 43, 0, 0},
  {0x0b97b2af, 126, 1, 1085},
  {0x0b9fa342, 126, 1, 1088},
  {0x0baf01d0, 160, 0, 0},
  {0x0bb5b3a0, 21, 0, 0},
  {0x0bbacf8f, 100, 0, 0},
  {0x0bbd85ff, 100, 0, 0},
  {0x0bbf80cb, 160, 0, 0},
  {0x0bc73114, 156, 0, 0},
  {0x0bcaa4d7, 113, 1, 292},
  {0x0bdd8dd9, 65, 1, 690},
  {0x0bdf73aa, 66, 0, 0},
  {0x0be0a328, 24, 0, 0},
  {0x0be230b7, 46, 1, 1291},
  {0x0bf306d1, 164, 0, 0},
  {0x0bf31a3d, 16, 0, 0},
  {0x0c1792da, 117, 0, 0},
  {0x0c187747, 89, 0, 0},
  {0x0c198d4f, 43, 0, 0},
  {0x0c2e7863, 115, 1, 1290},
  {0x0c401790, 77, 0, 0},
  {0x0c47946d, 117, 0, 0},
  {0x0c4b76ec, 181, 0, 0},
  {0x0c5a6297, 156, 0, 0},
  {0x0c5c353a, 77, 0, 0},
  {0x0c5e6280, 157, 0, 0},
  {0x0c5f40a6, 100, 1, 0},
  {0x0c6c71a3, 100, 0, 0},
  {0x0c90a3cd, 143, 0, 87},
  {0x0c935dfe, 63, 0, 0},
  {0x0cabbf0f, 120, 0, 0},
  {0x0caf2f15, 124, 0, 0},
  {0x0cb8d92d, 97, 0, 0},
  {0x0cc9ffec, 100, 0, 0},
  {0x0ccd28d5, 188, 1, 689},
  {0x0cd79b71, 2, 0, 0},
  {0x0d3482d7, 160, 0, 0},
  {0x0d365112, 164, 0, 0},
  {0x0d44bacc, 43, 0, 0},
  {0x0d52e2e6, 69, 0, 0},
  {0x0d65e7c7, 156, 0, 0},
  {0x0d6b2b30, 51, 1, 990},
  {0x0d6cf078, 164, 0, 0},
  {0x0d86628e, 96, 0, 0},
  {0x0d8a30a2, 100, 0, 0},
  {0x0d9f5bd1, 92, 1, 990},
  {0x0da00298, 69, 0, 0},
  {0x0daf8a80, 100, 1, 0},
  {0x0db4b382, 92, 0, 0},
  {0x0dc53188, 98, 0, 0},
  {0x0e0c4221, 156, 2, 0},
  {0x0e1683c5, 160, 0, 0},
  {0x0e2ae6e1, 127, 1, 893},
  {0x0e8c28f9, 38, 0, 0},
  {0x0e8cfd62, 59, 0, 0},
  {0x0e997cf6, 76, 0, 0},
  {0x0eaa7515, 92, 0, 0},
  {0x0eb63e83, 0, 1, 0},
  {0x0ebf76e2, 117, 0, 0},
  {0x0ec6c023, 99, 1, 392},
  {0x0ed96f42, 156, 1, 1090},
  {0x0ef730e7, 12, 1, 190},
  {0x0efde8c0, 97, 0, 0},
  {0x0f0d64b7, 9, 0, 0},
  {0x0f0f9c73, 15, 1, 0},
  {0x0f141525, 160, 0, 0},
  {0x0f1c3afc, 34, 1, 787},
  {0x0f1cc048, 182, 1, 688},
  {0x0f4ff4a0, 122, 0, 0},
  {0x0f5410e3, 160, 0, 0},
  {0x0f5f1f86, 193, 0, 0},
  {0x0f6c01e3, 160, 1, 887},
  {0x0f86feb4, 173, 1, 591},
  {0x0fae316f, 117, 0, 0},
  {0x0fb244c8, 20, 0, 0},
  {0x0fb6032a, 97, 0, 0},
  {0x0fc72a80, 158, 1, 191},
  {0x0fc8e9b7, 97, 0, 0},
  {0x0fcfc04d, 34, 1, 689},
  {0x0fd5aeeb, 40, 1, 0},
  {0x0fd6bfc8, 197, 0, 0},
  {0x0fe4fc74, 104, 1, 292},
  {0x0fec90d2, 34, 1, 787},
  {0x0ff6a3b5, 156, 0, 0},
  {0x0ffd2cb9, 34, 1, 1092},
  {0x0ffde258, 166, 1, 0},
  {0x10092781, 0, 1, 0},
  {0x10119e6b, 156, 0, 0},
  {0x10180072, 43, 1, 291},
  {0x1027c432, 77, 0, 0},
  {0x102823a7, 92, 1, 190},
  {0x1028fc27, 99, 0, 0},
  {0x10327e0e, 176, 0, 0},
  {0x103e7e7f, 34, 1, 1289},
  {0x106af52a, 180, 0, 0},
  {0x109f1acc, 133, 0, 0},
  {0x10b0f8b0, 160, 0, 0},
  {0x10baeef3, 34, 0, 0},
  {0x10bb8f9a, 24, 0, 0},
  {0x10bbd4ba, 126, 2, 0},
  {0x10c8f2fa, 117, 0, 0},
  {0x10c9a789, 117, 0, 0},
  {0x10d20915, 6, 1, 490},
  {0x112140a4, 100, 0, 0},
  {0x11469ce3, 54, 0, 0},
  {0x116ccd44, 32, 0, 0},
  {0x1170392a, 202, 0, 0},
  {0x119dacf3, 99, 0, 0},
  {0x11b038fe, 117, 0, 0},
  {0x11c2e664, 21, 0, 0},
  {0x11c48002, 0, 1, 0},
  {0x11d08cc6, 40, 1, 0},
  {0x11dc4071, 158, 1, 91},
  {0x11eaad26, 99, 1, 991},
  {0x11fc8686, 156, 0, 0},
  {0x1202414b, 160, 0, 0},
  {0x12078afd, 77, 1, 1090},
  {0x1208e754, 84, 0, 0},
  {0x1221f97c, 66, 0, 0},
  {0x1248326d, 97, 0, 0},
  {0x124af039, 31, 1, 0},
  {0x1253ea03, 34, 1, 390},
  {0x1255036f, 182, 1, 791},
  {0x126ea4a0, 121, 0, 0},
  {0x126ebf66, 97, 1, 990},
  {0x127436fc, 15, 1, 0},
  {0x12748678, 115, 1, 1090},
  {0x12906664, 123, 1, 989},
  {0x1294ab5a, 117, 0, 0},
  {0x12b2c361, 104, 1, 989},
  {0x12bba335, 49, 0, 0},
  {0x12c6d5c7, 34, 1, 1088},
  {0x12c90991, 143, 0, 0},
  {0x12ee543e, 160, 0, 0},
  {0x12f048df, 92, 0, 0},
  {0x12f405cf, 143, 1, 291},
  {0x1300a8b7, 117, 0, 0},
  {0x131a18cb, 44, 0, 0},
  {0x13332bfa, 126, 1, 1085},
  {0x1335cb05, 143, 1, 790},
  {0x134c1a50, 165, 0, 0},
  {0x1352f1b9, 196, 1, 792},
  {0x1353a134, 129, 2, 0},
  {0x13556927, 69, 0, 0},
  {0x135adf7c, 117, 0, 0},
  {0x137b27d8, 77, 0, 0},
  {0x1388aeb9, 160, 1, 589},
  {0x1394e1a2, 160, 0, 0},
  {0x139b07db, 34, 1, 990},
  {0x139b15ba, 61, 0, 0},
  {0x139eb5b5, 166, 1, 0},
  {0x13be9a74, 46, 1, 1091},
  {0x13c6617e, 156, 1, 290},
  {0x13cf3d5d, 127, 1, 191},
  {0x13d5b1a4, 114, 1, 990},
  {0x13da2122, 117, 0, 0},
  {0x13dd7461, 160, 0, 0},
  {0x13e9b20a, 160, 1, 0},
  {0x13ec1c90, 117, 0, 0},
  {0x14105c13, 200, 1, 0},
  {0x1411005b, 164, 0, 0},
  {0x14238d61, 163, 1, 489},
  {0x1425d7f4, 12, 1, 690},
  {0x142f7f3f, 92, 0, 0},
  {0x144a9380, 113, 1, 292},
  {0x144ca9e5, 182, 2, 0},
  {0x144d31cd, 92, 0, 0},
  {0x145a9a6c, 126, 0, 0},
  {0x1461d1f8, 41, 0, 0},
  {0x146fb9c3, 24, 0, 0},
  {0x14942c06, 117, 0, 0},
  {0x1495414c, 150, 0, 0},
  {0x149c0ec3, 80, 0, 0},
  {0x14a01c70, 144, 0, 0},
  {0x14a45522, 164, 0, 0},
  {0x14a81635, 31, 1, 0},
  {0x14cbb22a, 126, 1, 686},
  {0x14cd576e, 120, 0, 0},
  {0x1500e835, 160, 0, 0},
  {0x1512e0b1, 126, 0, 0},
  {0x15141401, 20, 0, 0},
  {0x15249a59, 117, 0, 0},
  {0x153cdc91, 104, 1, 990},
  {0x153dd212, 158, 1, 1292},
  {0x154a31b6, 118, 0, 86},
  {0x1554fd9d, 143, 1, 689},
  {0x15662081, 77, 0, 0},
  {0x156c75ff, 143, 1, 0}, // "Proto"
  {0x158fc675, 24, 0, 0},
  {0x15be6e58, 117, 0, 0},
  {0x15e9d230, 126, 1, 1085},
  {0x15f0d3f1, 158, 1, 191},
  {0x15fe6d0f, 99, 1, 1290},
  {0x16192bf2, 12, 1, 1191},
  {0x161d717b, 46, 1, 790},
  {0x16342ad0, 178, 0, 0},
  {0x16388a44, 126, 1, 0},
  {0x163a5eda, 0, 0, 93}, // Batman 3 [p1]
  {0x163e86c0, 126, 1, 1189},
  {0x163eccae, 100, 0, 0},
  {0x1644c162, 39, 0, 0},
  {0x16a0a3a3, 34, 0, 0},
  {0x16aa4e2d, 126, 0, 0},
  {0x16d10b24, 2, 0, 0},
  {0x16d55a59, 165, 1, 289},
  {0x16e93f39, 54, 0, 0},
  {0x16eba50a, 188, 1, 292},
  {0x170250de, 24, 0, 0},
  {0x170739cf, 128, 0, 0},
  {0x171251e3, 34, 1, 1186},
  {0x1718473c, 188, 1, 292},
  {0x172ad0f5, 160, 0, 0},
  {0x17421900, 117, 0, 0},
  {0x175c4a3c, 92, 0, 0},
  {0x175e7161, 46, 1, 1090},
  {0x175eda0b, 197, 0, 0},
  {0x17627d4b, 160, 0, 0},
  {0x1771ea8f, 24, 1, 787},
  {0x179a0d57, 165, 1, 1291},
  {0x17ab5897, 31, 1, 0},
  {0x17ac5303, 191, 0, 0},
  {0x17b49292, 72, 0, 0},
  {0x17c111e0, 76, 1, 1190},
  {0x17f72ec8, 126, 1, 990},
  {0x1808061a, 89, 1, 987},
  {0x1814ce64, 38, 0, 0},
  {0x181ec6b4, 195, 0, 0},
  {0x1826e6d2, 46, 1, 90},
  {0x1829616a, 174, 0, 0},
  {0x182f09d8, 4, 1, 93},
  {0x183859d2, 24, 0, 0},
  {0x184c2124, 99, 0, 0},
  {0x185633d5, 84, 0, 0},
  {0x18a885b0, 34, 0, 0},
  {0x18a9f0d9, 142, 1, 792},
  {0x18b249e5, 74, 1, 0},
  {0x18ba5ac0, 165, 1, 787},
  {0x18bd1afb, 158, 1, 1292},
  {0x18c0623b, 117, 0, 0},
  {0x18c64981, 160, 2, 0},
  {0x18d44bba, 84, 0, 0},
  {0x18d5af7a, 24, 1, 988},
  {0x190439b9, 46, 0, 0},
  {0x190a3e11, 46, 0, 0},
  {0x1924f963, 143, 1, 89},
  {0x192d546f, 46, 1, 1289},
  {0x19411bd7, 133, 0, 0},
  {0x194942db, 40, 1, 0},
  {0x194f9844, 77, 1, 194},
  {0x19555d39, 6, 1, 1088},
  {0x196f05dd, 174, 0, 0},
  {0x1973aea8, 65, 1, 1091},
  {0x198237c5, 117, 0, 0},
  {0x198b4e3b, 0, 1, 0},
  {0x198c2f41, 126, 1, 1090},
  {0x198e279a, 65, 1, 390},
  {0x1992d163, 68, 1, 390},
  {0x1995ac4e, 39, 0, 0},
  {0x19b1a2e7, 97, 0, 0},
  {0x19cc71d2, 89, 0, 0},
  {0x19df419a, 126, 1, 1085},
  {0x1a2d3b17, 40, 1, 0},
  {0x1a2ea6b9, 39, 0, 0},
  {0x1a39343c, 151, 0, 0},
  {0x1a3f0a61, 15, 1, 0},
  {0x1a3f1f1c, 191, 0, 0},
  {0x1a6693db, 166, 1, 0},
  {0x1aa0479c, 100, 0, 0},
  {0x1aa0dbaf, 166, 1, 0},
  {0x1abb9511, 126, 1, 1090},
  {0x1ac701b5, 34, 0, 0},
  {0x1ae527f1, 197, 0, 0},
  {0x1ae7b933, 110, 1, 989},
  {0x1af0ec24, 187, 0, 0},
  {0x1af80f18, 0, 1, 0},
  {0x1afb26d7, 126, 1, 1294},
  {0x1b2bad13, 0, 1, 0},
  {0x1b2f3545, 173, 1, 1089},
  {0x1b3a3db3, 91, 1, 392},
  {0x1b45a73e, 92, 0, 0},
  {0x1b71ccdb, 115, 1, 990},
  {0x1b7bd879, 92, 0, 0},
  {0x1b932bea, 34, 2, 0},
  {0x1bc686a8, 33, 1, 0},
  {0x1bd39032, 0, 1, 0},
  {0x1bebf407, 160, 1, 1092},
  {0x1c059dfb, 0, 1, 0},
  {0x1c212e9d, 0, 2, 0},
  {0x1c238ad9, 142, 1, 792},
  {0x1c2a58ff, 192, 0, 0},
  {0x1c309d0a, 61, 0, 0},
  {0x1c3a73f0, 25, 0, 0},
  {0x1c4f2651, 137, 1, 90},
  {0x1c66baf6, 133, 0, 0},
  {0x1c6ba3b2, 126, 0, 0},
  {0x1c6f3036, 114, 1, 88},
  {0x1c779cd7, 15, 1, 0},
  {0x1caae58d, 160, 0, 0},
  {0x1cac1791, 69, 0, 0},
  {0x1cba7eb4, 126, 1, 493},
  {0x1cc26e7e, 100, 0, 0},
  {0x1ccba824, 0, 1, 0},
  {0x1ced086f, 99, 0, 0},
  {0x1cee0c21, 114, 1, 1290},
  {0x1d0f4d6b, 76, 1, 989},
  {0x1d17ad3b, 45, 0, 0},
  {0x1d20a5c6, 6, 1, 291},
  {0x1d27f817, 60, 0, 0},
  {0x1d2d93ff, 163, 1, 191},
  {0x1d2e5018, 34, 0, 0},
  {0x1d41cc8c, 188, 1, 289},
  {0x1d4a46a4, 74, 1, 292},
  {0x1d5b03a5, 100, 1, 988},
  {0x1d5fe853, 69, 0, 0},
  {0x1d62f13f, 96, 0, 0},
  {0x1d6deccc, 24, 1, 591},
  {0x1d873633, 15, 1, 0},
  {0x1d8b2f59, 43, 0, 0},
  {0x1d8bf724, 192, 0, 0},
  {0x1dac6e97, 69, 0, 0},
  {0x1db07c0d, 24, 1, 988},
  {0x1dbd1d2b, 2, 0, 0},
  {0x1dc0f740, 117, 0, 0},
  {0x1dd6619b, 117, 0, 0},
  {0x1dea55eb, 39, 0, 0},
  {0x1df00462, 100, 0, 0},
  {0x1e0a01ea, 92, 1, 1292},
  {0x1e0c7ea3, 133, 0, 0},
  {0x1e1fbd63, 68, 1, 489},
  {0x1e2cb8cc, 24, 0, 0},
  {0x1e2f89c8, 190, 0, 0},
  {0x1e3685d0, 126, 1, 1085},
  {0x1e438d52, 100, 0, 0},
  {0x1e4afd90, 117, 0, 0},
  {0x1e4d3831, 126, 0, 0},
  {0x1e598a14, 126, 1, 290},
  {0x1e5fdfa3, 165, 1, 1187},
  {0x1e7ff743, 66, 0, 0},
  {0x1e9ebb00, 76, 0, 0},
  {0x1ea0052e, 77, 0, 0},
  {0x1ea703cb, 92, 0, 0},
  {0x1eaf333c, 0, 1, 0},
  {0x1eb4a920, 15, 1, 0},
  {0x1ebb5b42, 77, 1, 293},
  {0x1ec1bf08, 66, 0, 0},
  {0x1ed48c5c, 197, 0, 0},
  {0x1ed5c801, 126, 2, 0},
  {0x1edadc3d, 0, 1, 0},
  {0x1efcac48, 34, 1, 9},
  {0x1f0d03f8, 160, 2, 0},
  {0x1f2f4861, 81, 0, 0},
  {0x1f3d2cdf, 0, 1, 0},
  {0x1f59ef24, 156, 0, 0},
  {0x1f5f88dd, 40, 1, 0},
  {0x1f6ea423, 43, 1, 390},
  {0x1f818018, 100, 0, 0},
  {0x1f864492, 92, 0, 0},
  {0x1fa8c4a4, 97, 1, 1288},
  {0x1fc5740d, 24, 1, 689},
  {0x1fdc664c, 117, 0, 0},
  {0x1ff7fc0d, 100, 1, 992},
  {0x2001f0ed, 100, 0, 0},
  {0x20028703, 12, 1, 190},
  {0x2015226d, 151, 0, 0},
  {0x2019fe65, 156, 0, 0},
  {0x202d70f8, 92, 1, 391},
  {0x202df297, 160, 0, 0},
  {0x203d32ab, 92, 0, 0},
  {0x203d32b5, 166, 1, 0},
  {0x2046c4e9, 92, 1, 391},
  {0x2055971a, 152, 1, 793},
  {0x20574509, 74, 1, 293},
  {0x2061772a, 46, 0, 0},
  {0x2063f8be, 191, 0, 0},
  {0x2078dce4, 126, 0, 0},
  {0x207e618c, 156, 0, 0},
  {0x209b4bed, 100, 0, 0},
  {0x209f3587, 127, 1, 191},
  {0x20a5219b, 20, 1, 1190},
  {0x20a9e4a2, 3, 1, 89},
  {0x20af7e1a, 22, 0, 0},
  {0x20b04883, 123, 1, 691},
  {0x20c5d187, 161, 0, 0},
  {0x20cc079d, 126, 0, 0},
  {0x20d22251, 192, 0, 0},
  {0x20e781c2, 165, 1, 1192},
  {0x20fc87a0, 165, 1, 787},
  {0x212b4ba0, 74, 1, 0},
  {0x213cb3fb, 164, 0, 0},
  {0x21490e20, 120, 0, 0},
  {0x219ce59d, 160, 0, 0},
  {0x219dfabf, 187, 0, 0},
  {0x21a3f190, 188, 1, 192},
  {0x21a5a053, 179, 0, 0},
  {0x21b099f3, 126, 1, 1085},
  {0x21d7517a, 6, 1, 1089},
  {0x21dd2174, 2, 0, 0},
  {0x21ddbc1c, 66, 0, 0},
  {0x21ebe345, 133, 0, 0},
  {0x21f2a1a6, 104, 2, 0},
  {0x21f85681, 100, 0, 0},
  {0x21f8c4ab, 156, 0, 0},
  {0x2220e14a, 3, 1, 1191},
  {0x2224b048, 157, 0, 0},
  {0x2225c20f, 99, 1, 190},
  {0x22276a55, 0, 1, 0},
  {0x227cf577, 158, 1, 1292},
  {0x228fc66e, 199, 0, 0},
  {0x22d6d5bd, 117, 0, 0},
  {0x22f1ca9e, 165, 0, 0},
  {0x22f95ff1, 46, 0, 0},
  {0x23040fc4, 126, 1, 1289},
  {0x2305b528, 77, 0, 0},
  {0x2328046e, 0, 1, 0},
  {0x23317897, 100, 1, 992},
  {0x23386b90, 41, 0, 0},
  {0x236cf56c, 95, 0, 0},
  {0x2370c0a9, 74, 1, 293},
  {0x2389a6b3, 124, 0, 0},
  {0x238e1848, 160, 1, 887},
  {0x23a60a62, 126, 1, 1085},
  {0x23b719c1, 95, 0, 0},
  {0x23bf0507, 193, 0, 0},
  {0x23c3fb2d, 160, 1, 790},
  {0x23d17f5e, 100, 1, 891},
  {0x23d7d48f, 182, 2, 0},
  {0x23d809af, 92, 0, 0},
  {0x23e6d471, 160, 0, 0},
  {0x23e9c736, 178, 0, 0},
  {0x23f4b48f, 34, 0, 0},
  {0x23ff7365, 0, 1, 0},
  {0x240c6de8, 174, 0, 0},
  {0x240de736, 91, 1, 392},
  {0x242a270c, 104, 1, 1187},
  {0x243a8735, 89, 0, 0},
  {0x24428a5d, 154, 1, 989},
  {0x2447e03b, 117, 0, 0},
  {0x245870ed, 124, 0, 0},
  {0x24598791, 126, 1, 1085},
  {0x245ab38b, 0, 1, 0},
  {0x245fd217, 114, 1, 390},
  {0x2470402b, 80, 0, 0},
  {0x2472c3eb, 15, 1, 0},
  {0x2477a571, 117, 1, 0},
  {0x248566a7, 4, 1, 890},
  {0x248e7693, 4, 1, 1288},
  {0x24900c64, 46, 1, 1187},
  {0x24a0aa1a, 155, 0, 0},
  {0x24ba12dd, 33, 1, 0},
  {0x24ba90ca, 46, 0, 0},
  {0x24eecc15, 74, 1, 1191},
  {0x250a8eaa, 188, 0, 0},
  {0x250f7913, 133, 0, 0},
  {0x25225c70, 0, 1, 0},
  {0x2526c943, 77, 0, 0},
  {0x252ffd12, 34, 0, 0},
  {0x2538d860, 92, 1, 690},
  {0x2545214c, 126, 1, 0},
  {0x25482ceb, 117, 0, 0},
  {0x2554f9fd, 156, 0, 0},
  {0x255b129c, 100, 0, 0},
  {0x256441eb, 178, 0, 0},
  {0x2565786d, 117, 0, 0},
  {0x25687c07, 0, 1, 0},
  {0x256cfb05, 57, 1, 1088},
  {0x25952141, 0, 1, 0},
  {0x25a44d1b, 104, 1, 992},
  {0x25da821f, 45, 0, 0},
  {0x25df361f, 57, 1, 1088},
  {0x25e389da, 4, 1, 987},
  {0x25edaf5c, 92, 0, 0},
  {0x25f872d4, 100, 1, 487},
  {0x262b5a1d, 163, 1, 189},
  {0x263ac8a0, 46, 1, 1288},
  {0x264f26b1, 24, 0, 0},
  {0x265167e1, 160, 0, 0},
  {0x2651f227, 165, 1, 1192},
  {0x26533405, 99, 1, 1191},
  {0x265d7833, 102, 0, 0},
  {0x2669ef4e, 104, 1, 1091},
  {0x26717e19, 117, 0, 0},
  {0x26796758, 92, 1, 192},
  {0x267de4cc, 0, 1, 0},
  {0x267e592f, 117, 0, 0},
  {0x26898b6a, 174, 0, 0},
  {0x268d566d, 104, 1, 1187},
  {0x268e39d0, 46, 0, 0},
  {0x269b5f1e, 93, 0, 0},
  {0x26ad7cef, 92, 1, 192},
  {0x26bb1c8c, 37, 0, 0},
  {0x26bd6ec6, 117, 0, 0},
  {0x26bfed27, 43, 0, 0},
  {0x26ccc20c, 156, 0, 0},
  {0x26cec726, 195, 0, 0},
  {0x26d3082c, 46, 1, 1292},
  {0x26d42cee, 180, 0, 0},
  {0x26de121d, 156, 0, 0},
  {0x26e39935, 72, 0, 0},
  {0x26e82008, 100, 0, 0},
  {0x26f2b268, 65, 1, 591},
  {0x26ff3ea2, 100, 0, 0},
  {0x2705eaeb, 15, 1, 0},
  {0x270eaed5, 133, 0, 0},
  {0x2716bd4b, 126, 1, 1085},
  {0x272e96a6, 93, 0, 0},
  {0x2735af2e, 158, 1, 991},
  {0x2746b39e, 126, 0, 0},
  {0x274cb4c6, 117, 0, 0},
  {0x275dcc83, 92, 0, 0},
  {0x27738241, 126, 0, 0},
  {0x27777635, 126, 1, 387},
  {0x278bf2ea, 0, 1, 0},
  {0x278db9e3, 4, 2, 0},
  {0x2795e4f6, 156, 0, 0},
  {0x279710dc, 182, 1, 691},
  {0x279b69c0, 115, 1, 592},
  {0x27aa3933, 57, 1, 1088},
  {0x27aeaf2f, 126, 1, 387},
  {0x27b29870, 69, 0, 0},
  {0x27b9cbab, 183, 0, 0},
  {0x27c16011, 93, 0, 0},
  {0x27d14a54, 104, 1, 1187},
  {0x27d34a57, 193, 0, 0},
  {0x27d3d6eb, 117, 0, 0},
  {0x27d48cb2, 66, 0, 0},
  {0x27ddf227, 143, 1, 887},
  {0x27e4d335, 126, 0, 0},
  {0x27f8d0d2, 104, 1, 1190},
  {0x281160ae, 24, 0, 0},
  {0x28154f6c, 193, 1, 191},
  {0x282aa7e0, 195, 0, 0},
  {0x283913f2, 92, 1, 390},
  {0x283ad224, 89, 0, 0},
  {0x284e65e8, 126, 1, 1085},
  {0x2856111f, 196, 1, 193},
  {0x2858933b, 69, 0, 0},
  {0x286613d8, 99, 0, 0},
  {0x286fcd20, 100, 0, 0},
  {0x288662e9, 160, 0, 0},
  {0x28aa07ba, 0, 1, 0},
  {0x28c11d24, 178, 0, 0},
  {0x28c1d3d5, 178, 0, 0},
  {0x28c2dfce, 128, 0, 0},
  {0x28c90b01, 161, 0, 0},
  {0x28f9b41f, 92, 1, 493},
  {0x28fb71ae, 188, 1, 789},
  {0x29068417, 122, 0, 0},
  {0x291bcd7d, 39, 0, 0},
  {0x29401686, 37, 0, 0},
  {0x294299ef, 16, 0, 0},
  {0x29449ba9, 99, 0, 0},
  {0x29470b20, 77, 0, 0},
  {0x2959b3fa, 89, 0, 0},
  {0x296b915f, 0, 1, 0},
  {0x297198b9, 69, 0, 0},
  {0x2989ead6, 4, 1, 991},
  {0x29c36bf4, 161, 0, 0},
  {0x29c61b41, 117, 0, 0},
  {0x29cc4dee, 44, 0, 0},
  {0x29e173ff, 160, 0, 0},
  {0x29ec0fd1, 39, 0, 0},
  {0x2a01f9d1, 117, 0, 0},
  {0x2a0769e1, 34, 0, 0},
  {0x2a1919fe, 16, 0, 0},
  {0x2a3ca509, 164, 0, 0},
  {0x2a601192, 77, 0, 0},
  {0x2a629f7d, 180, 0, 0},
  {0x2a6559a1, 160, 0, 0},
  {0x2a763c9c, 92, 0, 0},
  {0x2a79d3ba, 182, 1, 791},
  {0x2a7d3adf, 117, 0, 0},
  {0x2a83ddc5, 0, 1, 0},
  {0x2a971204, 160, 0, 0},
  {0x2ac5233c, 100, 0, 0},
  {0x2ac87283, 160, 1, 887},
  {0x2ae97660, 6, 1, 490},
  {0x2b11e0b0, 99, 0, 0},
  {0x2b1497dc, 97, 0, 0},
  {0x2b160bf0, 34, 0, 0},
  {0x2b2d47f1, 46, 1, 587},
  {0x2b378d11, 65, 1, 490},
  {0x2b462010, 126, 1, 686},
  {0x2b548d75, 99, 1, 1290},
  {0x2b6d2447, 143, 0, 0},
  {0x2b72fe7e, 191, 0, 0},
  {0x2b7fe4d7, 100, 0, 0},
  {0x2b814116, 0, 1, 0},
  {0x2b825ce1, 117, 0, 0},
  {0x2b85420e, 126, 0, 0},
  {0x2ba30713, 100, 0, 0},
  {0x2bb33c69, 0, 1, 0},
  {0x2bb3dabe, 160, 0, 0},
  {0x2bb7f34a, 165, 0, 0},
  {0x2bc25d5a, 50, 1, 392},
  {0x2bc67aa8, 34, 1, 192},
  {0x2bcf2132, 46, 0, 0},
  {0x2bd44c6a, 68, 1, 991},
  {0x2be254e9, 117, 0, 0},
  {0x2bf61c53, 160, 1, 1292},
  {0x2c003fb2, 46, 0, 0},
  {0x2c088dc5, 126, 2, 0},
  {0x2c14d55e, 97, 0, 0},
  {0x2c2ddfb4, 46, 1, 790},
  {0x2c4421b2, 24, 0, 0},
  {0x2c5908a7, 0, 1, 0},
  {0x2c609b52, 115, 1, 990},
  {0x2c818014, 126, 1, 0},
  {0x2c945f19, 100, 1, 1091},
  {0x2caae01c, 77, 1, 1092},
  {0x2cb269d5, 151, 0, 0},
  {0x2cba26ac, 24, 0, 0},
  {0x2cc9e8aa, 165, 0, 0},
  {0x2cdef25f, 143, 1, 988},
  {0x2ce45ff2, 40, 1, 0},
  {0x2d0d4ff9, 0, 1, 0},
  {0x2d2f91b8, 202, 0, 0},
  {0x2d41ef92, 104, 1, 1289},
  {0x2d664d99, 34, 0, 0},
  {0x2d69dd94, 165, 1, 90},
  {0x2d75c7a9, 126, 1, 789},
  {0x2d76a271, 126, 1, 888},
  {0x2d782595, 133, 0, 0},
  {0x2d7bf9a3, 89, 0, 0},
  {0x2d84f5b3, 14, 1, 1092},
  {0x2d8c2829, 0, 1, 0},
  {0x2db7c31e, 54, 0, 0},
  {0x2dbddd11, 126, 2, 0},
  {0x2dbfa36a, 126, 0, 0},
  {0x2dc05a6f, 2, 0, 0},
  {0x2dc27941, 134, 0, 0},
  {0x2dc331a2, 22, 0, 0},
  {0x2dc3817d, 20, 0, 0},
  {0x2dc4f189, 89, 0, 0},
  {0x2dcd824f, 92, 0, 0},
  {0x2ddc2dc3, 6, 1, 990},
  {0x2ddd5459, 200, 1, 0},
  {0x2dff7fdc, 158, 1, 1292},
  {0x2e069e5f, 0, 1, 0},
  {0x2e0741b6, 158, 1, 1092},
  {0x2e1790a4, 0, 2, 0},
  {0x2e1b6b3c, 0, 1, 0},
  {0x2e1e7fd8, 34, 0, 0},
  {0x2e2acae9, 20, 0, 0},
  {0x2e2f9018, 124, 0, 0},
  {0x2e326a1d, 166, 1, 0},
  {0x2e35dd2e, 152, 1, 694},
  {0x2e4ccf46, 64, 0, 0},
  {0x2e52d091, 0, 1, 0},
  {0x2e563c66, 117, 0, 0},
  {0x2e6301ed, 126, 1, 290},
  {0x2e6c3ca9, 74, 1, 989},
  {0x2e6ee98d, 65, 1, 391},
  {0x2e7f0b29, 12, 1, 690},
  {0x2e91eb15, 53, 1, 1092},
  {0x2e991109, 24, 0, 0},
  {0x2ea8cc16, 0, 1, 0},
  {0x2eb08777, 165, 0, 0},
  {0x2eb27d6e, 188, 1, 990},
  {0x2ebf2e0d, 1, 0, 0},
  {0x2ed0e5b2, 157, 0, 0},
  {0x2ee3ef15, 34, 1, 793},
  {0x2ee9e0d8, 24, 1, 690},
  {0x2f1c2b30, 0, 1, 0},
  {0x2f27cdef, 24, 0, 0},
  {0x2f2d1fa9, 146, 1, 690},
  {0x2f2e30f7, 165, 0, 0},
  {0x2f50bd38, 99, 1, 1191},
  {0x2f66e302, 114, 1, 689},
  {0x2f698c4d, 82, 1, 991},
  {0x2f737eb7, 24, 0, 0},
  {0x2f928646, 43, 1, 291},
  {0x2f97d456, 34, 1, 194},
  {0x2fa1e52a, 66, 0, 0},
  {0x2fa2df5f, 193, 1, 190},
  {0x2fb0ff59, 100, 1, 1286},
  {0x2fc1abae, 54, 0, 0},
  {0x2fdedcb5, 22, 1, 1291},
  {0x2fe20d79, 160, 1, 1291},
  {0x2ffde228, 173, 0, 0},
  {0x300e41b7, 34, 1, 0},
  {0x30310df1, 99, 0, 0},
  {0x303d4371, 114, 1, 889},
  {0x304827ec, 104, 1, 1187},
  {0x3057b904, 46, 0, 0},
  {0x305b4e62, 100, 1, 490},
  {0x306ebbde, 117, 0, 0},
  {0x3078cda9, 169, 0, 0},
  {0x307d0fc4, 0, 1, 0},
  {0x309d222b, 92, 0, 0},
  {0x30a174ac, 87, 2, 0},
  {0x30bf2dba, 92, 0, 0},
  {0x30ca59c8, 133, 0, 0},
  {0x30e64d03, 100, 0, 0},
  {0x30f39b6b, 7, 1, 89},
  {0x30fd5776, 83, 0, 0},
  {0x311b5a58, 0, 1, 0},
  {0x313cfd8a, 164, 0, 0},
  {0x314ee295, 126, 0, 0},
  {0x316eb515, 165, 1, 0},
  {0x317b3237, 38, 0, 0},
  {0x318b4122, 43, 1, 1290},
  {0x31957ae4, 76, 0, 0},
  {0x319730e3, 165, 0, 0},
  {0x319ccfcc, 22, 1, 991},
  {0x31b068c6, 95, 0, 0},
  {0x31b44c65, 165, 0, 0},
  {0x31c3e548, 100, 1, 294},
  {0x31c7ad13, 161, 0, 0},
  {0x31cd9903, 24, 0, 0},
  {0x31d239d6, 141, 1, 88},
  {0x31d6b7bd, 122, 0, 0},
  {0x32086826, 115, 1, 1288},
  {0x322c9b09, 100, 0, 0},
  {0x322f723a, 178, 0, 0},
  {0x323b547d, 43, 0, 0},
  {0x32490787, 40, 1, 0},
  {0x3256114c, 175, 0, 0},
  {0x326ab3b6, 163, 1, 190},
  {0x3272bc3c, 77, 0, 0},
  {0x3275fd7e, 190, 0, 0},
  {0x327946e3, 12, 1, 690},
  {0x3293afea, 92, 0, 0},
  {0x3296ff7a, 117, 0, 0},
  {0x32979126, 98, 0, 0},
  {0x32a1290c, 42, 0, 0},
  {0x32a33bd1, 115, 1, 190},
  {0x32b45889, 0, 1, 0},
  {0x32c1ce97, 155, 0, 0},
  {0x32c4b857, 129, 2, 0},
  {0x32cf4307, 193, 1, 390},
  {0x32e59039, 200, 1, 0},
  {0x32e8baf3, 34, 0, 0},
  {0x32f85838, 52, 2, 0},
  {0x32fa246f, 117, 0, 0},
  {0x32fb0583, 160, 1, 887},
  {0x330de468, 24, 0, 0},
  {0x3311562e, 156, 0, 0},
  {0x33159eac, 93, 0, 0},
  {0x331cc7ef, 100, 0, 0},
  {0x33258813, 4, 1, 987},
  {0x332bacdf, 160, 2, 0},
  {0x332c47e0, 165, 0, 0},
  {0x33346cd2, 117, 0, 0},
  {0x333c48a0, 46, 1, 1190},
  {0x3346e19e, 57, 1, 1088},
  {0x3348e3bd, 28, 0, 0},
  {0x3349ba0e, 152, 1, 0},
  {0x335cfba6, 24, 0, 0},
  {0x336e2a6f, 20, 0, 0},
  {0x33716316, 77, 0, 0},
  {0x339437f6, 74, 1, 189},
  {0x339c034c, 53, 0, 0},
  {0x33b899c9, 24, 0, 0},
  {0x33bd809c, 117, 0, 0},
  {0x33cd1506, 15, 1, 0},
  {0x33ce3ff0, 100, 0, 0},
  {0x33d0282f, 40, 1, 0},
  {0x33d07e45, 2, 0, 0},
  {0x3403b1fc, 97, 0, 0},
  {0x3409413f, 126, 0, 0},
  {0x3417ec46, 4, 1, 890},
  {0x3420711c, 158, 1, 192},
  {0x34338616, 166, 1, 0},
  {0x343c7bb0, 166, 1, 0},
  {0x343e9146, 2, 0, 0},
  {0x345d3a1a, 31, 1, 0},
  {0x346cd5d3, 164, 0, 0},
  {0x347d7d34, 117, 1, 1193},
  {0x3482720c, 0, 1, 0},
  {0x3484ab0c, 74, 1, 0},
  {0x348ae7d4, 126, 1, 490},
  {0x349313df, 34, 0, 0},
  {0x349391b5, 160, 1, 391},
  {0x34a516fd, 77, 0, 0},
  {0x34a8bd90, 77, 1, 988},
  {0x34bb757b, 77, 2, 0},
  {0x34cfa970, 39, 0, 0},
  {0x34d5fc6e, 4, 1, 1289},
  {0x34dfb13c, 126, 0, 0},
  {0x34eab034, 46, 1, 390},
  {0x34fbe63e, 92, 0, 0},
  {0x350bf21b, 24, 0, 0},
  {0x350d835e, 100, 1, 1286},
  {0x3515fc5b, 0, 1, 0},
  {0x351f1f5b, 195, 0, 0},
  {0x35226b99, 126, 0, 0},
  {0x3523214b, 156, 1, 989},
  {0x35476e87, 104, 1, 1091},
  {0x355ecafd, 143, 0, 0},
  {0x35893b67, 160, 0, 0},
  {0x3598238f, 117, 0, 0},
  {0x359c3631, 46, 1, 188},
  {0x35a465e5, 165, 0, 0},
  {0x35b6febf, 104, 1, 989},
  {0x35c41cd4, 68, 1, 989},
  {0x35c6f574, 92, 1, 1292},
  {0x35c9b9e0, 165, 0, 0},
  {0x35d8b101, 2, 0, 0},
  {0x35e3eea2, 76, 0, 0},
  {0x35e65a8c, 89, 0, 0},
  {0x35effd0e, 128, 0, 0},
  {0x360aa8b4, 93, 0, 0},
  {0x36305fa3, 156, 0, 88},
  {0x36520684, 24, 1, 1289},
  {0x36584c96, 81, 0, 0},
  {0x3674ffdb, 85, 0, 0},
  {0x3691c120, 92, 0, 0},
  {0x3691e09e, 16, 0, 0},
  {0x369da42d, 117, 0, 0},
  {0x36af368f, 164, 0, 0},
  {0x36b93d27, 119, 0, 0},
  {0x36c5ee93, 165, 0, 0},
  {0x36d5482b, 126, 0, 0},
  {0x36df877a, 74, 1, 91},
  {0x37088eff, 126, 0, 0},
  {0x37138039, 4, 1, 189},
  {0x3719a26d, 117, 0, 0},
  {0x371e771c, 156, 0, 0},
  {0x3746f951, 92, 0, 0},
  {0x374cb8a6, 77, 0, 0},
  {0x374d6871, 4, 1, 987},
  {0x376138d8, 160, 0, 0},
  {0x37a4552f, 25, 0, 0},
  {0x37a5eb52, 100, 2, 0},
  {0x37b3ad54, 0, 1, 0},
  {0x37b62d04, 195, 0, 0},
  {0x37ba3261, 104, 1, 990},
  {0x37bf04d7, 34, 0, 0},
  {0x37d13f96, 54, 0, 0},
  {0x37ef8319, 4, 1, 89},
  {0x37f49572, 128, 0, 0},
  {0x37f59450, 193, 0, 0},
  {0x37fc3443, 0, 1, 0},
  {0x3802276d, 0, 1, 0},
  {0x3824f7a5, 126, 2, 0},
  {0x3836eeac, 133, 0, 0},
  {0x3849d0ee, 58, 1, 0},
  {0x38810a91, 126, 1, 1085},
  {0x38946c43, 0, 1, 0},
  {0x389960db, 126, 1, 1085},
  {0x38a02977, 43, 0, 0},
  {0x38b590e4, 29, 1, 790},
  {0x38d314db, 25, 0, 0},
  {0x38ef66b5, 92, 0, 0},
  {0x38fbcc85, 33, 1, 0},
  {0x38fdb7f4, 0, 1, 0},
  {0x390e0320, 175, 0, 0},
  {0x3918e04d, 156, 0, 0},
  {0x391aa1b8, 178, 0, 0},
  {0x3940b0f9, 117, 0, 0},
  {0x394fd25a, 0, 1, 0},
  {0x3951afec, 46, 1, 491},
  {0x395f36c9, 24, 1, 1086},
  {0x396f0d59, 156, 0, 0},
  {0x398b8182, 127, 1, 1091},
  {0x39aa20f0, 28, 0, 0},
  {0x39b2ecda, 179, 0, 0},
  {0x39b5e5d2, 0, 1, 0},
  {0x39b7e9c4, 193, 1, 191},
  {0x39c879bb, 0, 1, 0},
  {0x39d1fa03, 126, 1, 1085},
  {0x39d43261, 97, 0, 0},
  {0x39d622f7, 66, 0, 0},
  {0x39e1a201, 92, 0, 0},
  {0x39f2ce4b, 99, 0, 0},
  {0x3a0965b1, 115, 1, 492},
  {0x3a15a108, 114, 1, 390},
  {0x3a15b344, 97, 0, 0},
  {0x3a1694f9, 164, 0, 0},
  {0x3a390b54, 44, 0, 0},
  {0x3a51eb04, 2, 0, 0},
  {0x3a5cc3fa, 15, 1, 0},
  {0x3a622c78, 34, 0, 0},
  {0x3a7203ae, 0, 1, 0},
  {0x3a8f81b0, 156, 0, 0},
  {0x3a94fa0b, 126, 1, 1085},
  {0x3a990ee0, 33, 1, 0},
  {0x3ab1e983, 115, 1, 190},
  {0x3ac0830a, 0, 2, 0},
  {0x3acd4bf1, 126, 1, 1085},
  {0x3ae06d19, 117, 0, 0},
  {0x3ae528ff, 37, 0, 0},
  {0x3b05ac54, 80, 0, 0},
  {0x3b0fb600, 195, 0, 0},
  {0x3b1a7eef, 147, 0, 0},
  {0x3b28386e, 156, 1, 990},
  {0x3b3f88f0, 126, 1, 889},
  {0x3b5586c3, 66, 0, 0},
  {0x3b73c254, 200, 1, 0},
  {0x3b7b3be1, 138, 0, 0},
  {0x3b7f5b3b, 127, 1, 693},
  {0x3b86ccb7, 91, 1, 690},
  {0x3b997543, 115, 2, 0},
  {0x3bb31e38, 43, 2, 0},
  {0x3bbff3a6, 2, 0, 0},
  {0x3bc3a97b, 34, 0, 0},
  {0x3bcd370e, 0, 1, 0},
  {0x3be244ef, 34, 1, 791},
  {0x3be91a23, 34, 0, 0},
  {0x3bf15767, 24, 0, 0},
  {0x3bf38115, 156, 0, 0},
  {0x3bf55966, 191, 0, 0},
  {0x3c13eaa0, 117, 0, 0},
  {0x3c361b36, 92, 0, 0},
  {0x3c54a0bf, 54, 0, 0},
  {0x3c5a512b, 2, 0, 0},
  {0x3c5ae54b, 39, 0, 0},
  {0x3c5c81d4, 166, 1, 0},
  {0x3c7e38f5, 40, 1, 0},
  {0x3c8f2d80, 0, 1, 0},
  {0x3c9e8124, 127, 1, 192},
  {0x3c9fe649, 104, 1, 1190},
  {0x3ccb5d57, 77, 0, 0},
  {0x3cd6bb0e, 161, 0, 0},
  {0x3cdea85b, 173, 0, 0},
  {0x3ced963a, 200, 1, 0},
  {0x3cf67aec, 197, 0, 0},
  {0x3cfe69c7, 117, 0, 0},
  {0x3d0996b2, 188, 1, 1091},
  {0x3d1c4894, 12, 1, 1290},
  {0x3d3a0367, 160, 0, 0},
  {0x3d3aa7b4, 196, 1, 0}, // "Proto"
  {0x3d3d8f58, 142, 1, 792},
  {0x3d4b64f1, 49, 0, 0},
  {0x3d4d2ba5, 147, 0, 0},
  {0x3d564757, 126, 1, 1085},
  {0x3d731836, 54, 0, 0},
  {0x3d8c4a06, 40, 1, 0},
  {0x3d8de75f, 95, 0, 0},
  {0x3d95d866, 37, 0, 0},
  {0x3d96a1d8, 166, 1, 0},
  {0x3d9fddd5, 191, 0, 0},
  {0x3da2085e, 92, 0, 0},
  {0x3da82381, 76, 0, 0},
  {0x3dae3335, 99, 0, 0},
  {0x3db3202e, 54, 0, 0},
  {0x3db9509a, 38, 0, 0},
  {0x3dc4d6c5, 46, 0, 0},
  {0x3dd3ba1e, 34, 0, 0},
  {0x3deac303, 117, 0, 0},
  {0x3df6d493, 38, 0, 0},
  {0x3df8e170, 126, 1, 8},
  {0x3e054f19, 25, 0, 0},
  {0x3e078f29, 164, 0, 0},
  {0x3e1271d5, 15, 1, 0},
  {0x3e58a87e, 156, 1, 488},
  {0x3e785dc3, 69, 0, 0},
  {0x3e840b56, 34, 1, 990},
  {0x3e8dec2f, 0, 1, 0},
  {0x3e8ded4e, 0, 2, 0},
  {0x3e95ba25, 117, 0, 0},
  {0x3eca3dda, 92, 1, 991},
  {0x3ecaa6a3, 143, 0, 0},
  {0x3ecdb1f7, 100, 0, 0},
  {0x3ed8c6b5, 126, 0, 0},
  {0x3edcf7e8, 34, 1, 1293},
  {0x3eff62e4, 46, 0, 0},
  {0x3f0c8136, 72, 0, 0},
  {0x3f0fd764, 156, 1, 1188},
  {0x3f150354, 0, 1, 0},
  {0x3f15d20d, 24, 0, 0},
  {0x3f2bda65, 92, 0, 0},
  {0x3f39f35c, 0, 1, 0},
  {0x3f49d239, 90, 0, 0},
  {0x3f56a392, 32, 0, 0},
  {0x3f5a4dd0, 160, 0, 0},
  {0x3f78037c, 34, 1, 793},
  {0x3f88d76f, 91, 1, 690},
  {0x3f8d6889, 92, 0, 0},
  {0x3f9970f9, 143, 0, 0},
  {0x3fa96277, 165, 0, 0},
  {0x3faaa154, 77, 0, 0},
  {0x3fbefd71, 100, 0, 0},
  {0x3fbf0480, 38, 0, 0},
  {0x3fc1dc19, 21, 0, 0},
  {0x3fccdc7b, 143, 0, 0},
  {0x3fdec942, 126, 1, 1085},
  {0x3fe272fb, 126, 1, 0},
  {0x3fea656a, 133, 0, 0},
  {0x3ffa5762, 39, 0, 0},
  {0x401521f7, 22, 1, 592},
  {0x40280a0f, 126, 0, 0},
  {0x404b2e8b, 155, 1, 690},
  {0x4057c51b, 174, 0, 0},
  {0x406fe900, 126, 1, 790},
  {0x4076e7a6, 156, 2, 0},
  {0x407793c0, 111, 1, 991},
  {0x408b54ee, 100, 0, 0},
  {0x408d72db, 32, 0, 0},
  {0x40a26fdb, 25, 0, 0},
  {0x40c6e687, 0, 1, 0},
  {0x40ce2fc0, 0, 1, 0},
  {0x40d159b6, 143, 1, 789},
  {0x40dafcba, 165, 1, 690},
  {0x40ed2a9d, 182, 1, 489},
  {0x40f96469, 38, 0, 0},
  {0x411edf39, 143, 0, 0},
  {0x41243492, 163, 1, 990},
  {0x41413b06, 53, 1, 0},
  {0x41482da3, 2, 0, 0},
  {0x4156a3cd, 43, 0, 0},
  {0x415e5109, 55, 0, 0},
  {0x4187ec10, 165, 1, 490},
  {0x418d26d7, 100, 0, 0},
  {0x4192d82c, 117, 0, 0},
  {0x419461d0, 50, 1, 291},
  {0x41cc30a7, 20, 0, 0},
  {0x41cf3616, 126, 1, 0},
  {0x41f5d38d, 43, 0, 0},
  {0x41f87a9b, 39, 0, 0},
  {0x41f9e0aa, 92, 1, 0},
  {0x4220c170, 65, 1, 192},
  {0x4232c609, 135, 1, 0},
  {0x423ada8e, 188, 1, 1288},
  {0x424ec0a6, 92, 0, 0},
  {0x42520393, 158, 1, 1292},
  {0x425357bf, 166, 1, 0},
  {0x42607a97, 166, 1, 0},
  {0x42749a95, 40, 1, 0},
  {0x42791a4d, 6, 1, 1089},
  {0x428dfac6, 24, 1, 490},
  {0x429103c9, 117, 0, 0},
  {0x429124f5, 117, 0, 0},
  {0x429fd177, 117, 0, 0},
  {0x42b36609, 180, 0, 0},
  {0x42d893e4, 160, 0, 0},
  {0x42ec339a, 0, 1, 0},
  {0x42f75eae, 24, 0, 0},
  {0x4301b96c, 171, 0, 0},
  {0x4318a2f8, 126, 1, 890},
  {0x4328b273, 126, 1, 1085},
  {0x4334890e, 124, 0, 0},
  {0x4344d265, 77, 0, 0},
  {0x43504731, 134, 0, 0},
  {0x435313c3, 72, 0, 0},
  {0x43539a3c, 161, 0, 0},
  {0x4356194e, 165, 1, 787},
  {0x435aeec6, 160, 1, 1092},
  {0x437e7b69, 100, 1, 987},
  {0x43a357ef, 126, 0, 0},
  {0x43d01c10, 97, 1, 1290},
  {0x43d30c2f, 166, 1, 0},
  {0x43e95611, 178, 0, 0},
  {0x43ecfc00, 34, 1, 791},
  {0x441aeae6, 34, 0, 0},
  {0x4459ca3c, 100, 0, 0},
  {0x445dd134, 0, 1, 0},
  {0x4468ca43, 81, 0, 0},
  {0x4471d531, 76, 0, 0},
  {0x449abfed, 15, 1, 0},
  {0x449e6557, 104, 2, 0},
  {0x44b060da, 72, 0, 0},
  {0x44bb1665, 57, 1, 490},
  {0x44ca9a8d, 53, 0, 0},
  {0x44f34172, 89, 1, 987},
  {0x44f92026, 39, 0, 0},
  {0x45111a7f, 133, 0, 0},
  {0x453be513, 111, 0, 0},
  {0x454f8e59, 29, 1, 790},
  {0x4582f22e, 106, 0, 0},
  {0x458936ef, 6, 1, 192},
  {0x45917511, 34, 0, 0},
  {0x4591be98, 126, 1, 0},
  {0x459adfa8, 165, 0, 0},
  {0x459d0c2a, 120, 0, 0},
  {0x45a2acc4, 115, 1, 190},
  {0x45a2fd26, 58, 1, 91},
  {0x45a3f009, 190, 0, 0},
  {0x45a41784, 77, 1, 1290},
  {0x45ab39cd, 200, 1, 91},
  {0x45bcd662, 115, 1, 1288},
  {0x45f03d2e, 123, 1, 691},
  {0x45f4c931, 117, 0, 0},
  {0x45f4fa8c, 122, 0, 0},
  {0x45fb6d34, 100, 0, 0},
  {0x460710fa, 140, 0, 0},
  {0x460a9cb0, 15, 1, 0},
  {0x4640ebe0, 76, 0, 0},
  {0x4642dda6, 99, 1, 689},
  {0x46480432, 173, 2, 0},
  {0x465e5483, 6, 1, 192},
  {0x46652449, 126, 0, 0},
  {0x466efdc2, 155, 0, 0},
  {0x467515fe, 90, 0, 0},
  {0x46752bc8, 94, 0, 0},
  {0x4677a2be, 92, 0, 0},
  {0x467fe41e, 90, 0, 0},
  {0x468db9a0, 46, 1, 1289},
  {0x46914e3e, 126, 0, 0},
  {0x46931ea0, 126, 2, 0},
  {0x46dc6e57, 24, 0, 0},
  {0x46e83313, 124, 0, 0},
  {0x46fd7843, 117, 0, 0},
  {0x470b031b, 189, 1, 90},
  {0x47232739, 192, 0, 0},
  {0x472d4d56, 165, 1, 787},
  {0x4751a751, 65, 1, 1093},
  {0x475a574e, 0, 1, 0},
  {0x475cdbfe, 92, 0, 0},
  {0x476e022b, 116, 1, 190},
  {0x4776bb2c, 92, 0, 0},
  {0x477db261, 77, 0, 0},
  {0x4788c2a2, 6, 1, 192},
  {0x478a04d8, 0, 1, 0},
  {0x478a11ae, 117, 0, 0},
  {0x47a57fc3, 43, 0, 0},
  {0x47a60cf1, 39, 0, 0},
  {0x47b6a39f, 126, 2, 0},
  {0x47c2020b, 117, 0, 0},
  {0x47ed7979, 6, 1, 489},
  {0x47f4e00f, 182, 1, 691},
  {0x47f552cd, 76, 0, 0},
  {0x47f77a4c, 57, 1, 689},
  {0x47fd88cf, 126, 2, 0},
  {0x480b35d1, 0, 2, 0},
  {0x481519b1, 165, 0, 0},
  {0x481608a9, 117, 0, 0},
  {0x4819a595, 160, 0, 0},
  {0x4823eefe, 57, 1, 193},
  {0x48349b0b, 53, 0, 87}, // Dragon Quest 2 (J)
  {0x485ac098, 37, 0, 0},
  {0x4864c304, 126, 1, 686},
  {0x489d19ab, 77, 0, 0},
  {0x489ef6a2, 4, 1, 689},
  {0x48a941f0, 50, 1, 1292},
  {0x48c8de53, 46, 0, 0},
  {0x48dc9540, 97, 0, 0},
  {0x48dc9e6a, 178, 0, 0},
  {0x48e904d0, 188, 1, 490}, // Snake's Revenge (U)
  {0x48ecc48a, 188, 1, 289},
  {0x48f68d40, 126, 1, 1085},
  {0x48f6a8e2, 62, 0, 0},
  {0x48f8d55e, 24, 0, 0},
  {0x490e8a4c, 100, 0, 0},
  {0x49123146, 100, 0, 0},
  {0x491d8cdb, 117, 0, 0},
  {0x491f0de8, 100, 1, 493},
  {0x492a8782, 46, 0, 0},
  {0x49362c1e, 155, 1, 0},
  {0x493680c8, 100, 0, 0},
  {0x4937f7a1, 92, 0, 0},
  {0x493bd2ff, 69, 0, 0},
  {0x4942bda8, 144, 0, 0},
  {0x497b9d15, 38, 0, 0},
  {0x497e9336, 180, 0, 0},
  {0x498187b6, 2, 0, 0},
  {0x499bd8e2, 0, 1, 0},
  {0x49a42938, 156, 0, 0},
  {0x49aa5257, 160, 0, 0},
  {0x49aeb3a6, 126, 1, 1085},
  {0x49c84b4e, 160, 0, 0},
  {0x49c9ff12, 0, 1, 0},
  {0x49da2f76, 92, 0, 0},
  {0x49e834c3, 132, 0, 0},
  {0x49f745e0, 115, 1, 1189},
  {0x4a172848, 160, 0, 0},
  {0x4a339590, 69, 0, 0},
  {0x4a5a4e1e, 5, 1, 0},
  {0x4a5d8469, 160, 1, 1192},
  {0x4a601a2c, 100, 0, 0},
  {0x4a99b47e, 126, 0, 0},
  {0x4a9f735c, 104, 1, 1289},
  {0x4ae58f5d, 92, 0, 0},
  {0x4aea40f7, 9, 0, 0},
  {0x4afd02b9, 20, 1, 1191},
  {0x4affeee7, 34, 1, 1190},
  {0x4b041b6b, 182, 1, 490},
  {0x4b0dacce, 181, 0, 0},
  {0x4b14f688, 155, 0, 0},
  {0x4b2cc73e, 43, 2, 0},
  {0x4b2dce64, 166, 1, 0},
  {0x4b5177e9, 164, 0, 0},
  {0x4b647c55, 104, 1, 292},
  {0x4ba78222, 160, 0, 0},
  {0x4ba8fcb6, 16, 0, 0},
  {0x4bb9b840, 191, 2, 0},
  {0x4bd82f51, 0, 1, 0},
  {0x4bf3972d, 126, 0, 0},
  {0x4c03dfdd, 95, 0, 0},
  {0x4c049cfe, 156, 0, 0},
  {0x4c25d4f5, 100, 1, 689},
  {0x4c32e3ae, 46, 1, 1187},
  {0x4c5836bd, 117, 0, 0},
  {0x4c5fa6ef, 126, 0, 0},
  {0x4c6faf64, 24, 1, 1289},
  {0x4c7c1af3, 196, 1, 1292},
  {0x4c923dae, 100, 0, 0},
  {0x4c9b6d43, 126, 0, 0},
  {0x4cccd878, 165, 0, 0},
  {0x4cdcaf6b, 29, 1, 489},
  {0x4cede4e0, 160, 0, 0},
  {0x4d1ac58c, 3, 1, 190},
  {0x4d1df589, 46, 2, 0},
  {0x4d21172c, 117, 0, 0},
  {0x4d2edf70, 128, 0, 0},
  {0x4d345422, 4, 2, 0},
  {0x4d3aa180, 97, 1, 0}, // "Proto"
  {0x4d3fba78, 24, 1, 489},
  {0x4d527d4a, 31, 1, 0},
  {0x4d5f2eb0, 0, 1, 0},
  {0x4d7859a9, 156, 0, 0},
  {0x4da464e9, 156, 0, 0},
  {0x4dc43d8c, 178, 0, 0},
  {0x4dc73149, 24, 1, 790},
  {0x4dc910fd, 166, 1, 0},
  {0x4dcd15ee, 159, 0, 0},
  {0x4ddb4dce, 45, 0, 0},
  {0x4de477be, 160, 0, 0},
  {0x4de66107, 104, 1, 1187},
  {0x4de7236f, 0, 1, 0},
  {0x4df3f32e, 126, 0, 0},
  {0x4dfd949e, 89, 0, 0},
  {0x4e18ca2d, 97, 0, 0},
  {0x4e1c1e3c, 46, 1, 188},
  {0x4e220484, 126, 1, 1085},
  {0x4e22368d, 100, 1, 1187},
  {0x4e2a57dd, 24, 1, 987},
  {0x4e42f13a, 34, 0, 0},
  {0x4e44ff44, 77, 1, 194},
  {0x4e5257d7, 24, 0, 0},
  {0x4e550aa9, 100, 1, 91},
  {0x4e5c1c1d, 133, 0, 0},
  {0x4e7db5af, 151, 0, 0},
  {0x4e800048, 35, 0, 0},
  {0x4e8e2951, 90, 0, 0},
  {0x4e959173, 104, 1, 1187},
  {0x4ebdb122, 160, 1, 790},
  {0x4ebe8a31, 46, 1, 987},
  {0x4ec0fecc, 156, 2, 0},
  {0x4ed0d752, 50, 1, 392},
  {0x4edc5e95, 48, 0, 0},
  {0x4ee69e3d, 16, 0, 0},
  {0x4ee735c1, 126, 1, 1085},
  {0x4f089e8a, 6, 2, 0},
  {0x4f16c504, 160, 0, 0},
  {0x4f2f1846, 117, 0, 0},
  {0x4f3ada21, 177, 0, 0},
  {0x4f467410, 143, 1, 488},
  {0x4f48b240, 34, 2, 0},
  {0x4f50457e, 34, 1, 1092},
  {0x4f593fa0, 24, 0, 0},
  {0x4f5f0a80, 0, 1, 0},
  {0x4f9bdad2, 92, 1, 1291},
  {0x4fb460cd, 164, 0, 0},
  {0x4fbbfa74, 104, 1, 1187},
  {0x4fc2f673, 100, 0, 0},
  {0x4ff64765, 81, 0, 0},
  {0x50059012, 57, 1, 193},
  {0x5012d5d0, 156, 0, 0},
  {0x501376e8, 143, 1, 789},
  {0x5030bca8, 89, 0, 0},
  {0x50322dfd, 15, 1, 0},
  {0x504c6455, 117, 0, 0},
  {0x505f9715, 4, 1, 1287},
  {0x506e259d, 53, 1, 1092},
  {0x50734772, 40, 1, 0},
  {0x5077a2d7, 46, 1, 1186},
  {0x50893b58, 165, 0, 0},
  {0x50920b03, 160, 0, 0},
  {0x50975983, 148, 0, 0},
  {0x509cf04a, 77, 0, 0},
  {0x50a1b3fe, 126, 0, 0},
  {0x50a6f31b, 155, 0, 87},
  {0x50c69602, 193, 1, 1289},
  {0x50ccda33, 179, 0, 0},
  {0x50d141fc, 34, 1, 1190},
  {0x50d296b3, 86, 1, 691},
  {0x50da4867, 133, 0, 0},
  {0x50ec5e8b, 2, 0, 0},
  {0x50fad784, 126, 0, 0},
  {0x50fd0cc6, 4, 1, 291},
  {0x5104833e, 160, 1, 192},
  {0x5112dc21, 126, 1, 1085},
  {0x514b8c43, 92, 1, 689},
  {0x515f738a, 202, 0, 0},
  {0x516b2412, 16, 0, 0},
  {0x5174dd11, 90, 0, 0},
  {0x5190c450, 46, 1, 1289},
  {0x5193fb54, 160, 0, 0},
  {0x51bee3ea, 65, 1, 591},
  {0x51bf28af, 114, 2, 0},
  {0x51c0b27e, 5, 1, 592},
  {0x51c142ff, 166, 1, 0},
  {0x51c51c35, 100, 2, 0},
  {0x51c70247, 200, 1, 0},
  {0x51c79fd7, 77, 0, 0},
  {0x51c7c66a, 117, 0, 0},
  {0x51d2112f, 100, 1, 691},
  {0x5229fcdd, 73, 0, 0},
  {0x523ca830, 66, 0, 0},
  {0x52449508, 46, 0, 0},
  {0x524a5a32, 182, 2, 0},
  {0x526ad690, 0, 1, 0},
  {0x52b58732, 126, 1, 493},
  {0x52bcf64a, 156, 1, 88},
  {0x52c851fb, 166, 1, 0},
  {0x52e2b5e0, 126, 0, 0},
  {0x530bccb4, 34, 0, 0},
  {0x530f8b13, 143, 0, 0},
  {0x5318cdb9, 16, 0, 0},
  {0x532a27e6, 12, 1, 892},
  {0x5337f73c, 195, 0, 0},
  {0x533f5707, 126, 2, 0},
  {0x534865c6, 126, 0, 0},
  {0x535a7dee, 4, 1, 987},
  {0x538218b2, 143, 2, 0},
  {0x538cd2ea, 6, 1, 1089},
  {0x53991500, 92, 0, 0},
  {0x53a94738, 28, 0, 0},
  {0x53a9e2ba, 126, 1, 0}, // "Proto"
  {0x53b02f3d, 0, 1, 0},
  {0x53ca161a, 92, 0, 0},
  {0x53cb7ced, 70, 0, 87},
  {0x53d67738, 100, 0, 0},
  {0x53f86791, 114, 1, 390},
  {0x540ec028, 100, 0, 0},
  {0x54163624, 160, 0, 0},
  {0x541989e1, 200, 1, 0},
  {0x542f7b53, 29, 1, 1289},
  {0x54386491, 0, 1, 0},
  {0x5438a0ac, 0, 1, 0},
  {0x5440811c, 93, 0, 0},
  {0x54430b24, 97, 1, 689},
  {0x54531910, 21, 0, 0},
  {0x5469dba3, 46, 0, 0},
  {0x547ad451, 165, 0, 0},
  {0x547e6cc1, 160, 0, 0},
  {0x54b0e5dd, 165, 0, 0},
  {0x54b80a8a, 117, 0, 0},
  {0x54faee6e, 36, 0, 0},
  {0x5506b1ca, 0, 1, 0},
  {0x552f1143, 15, 1, 0},
  {0x55397db3, 2, 0, 0},
  {0x554704f0, 100, 1, 92},
  {0x554b51d8, 3, 1, 990},
  {0x555042b3, 37, 0, 0},
  {0x55761931, 77, 0, 0},
  {0x55773880, 24, 1, 790},
  {0x5581e835, 126, 1, 686},
  {0x558fcb6d, 97, 1, 1290},
  {0x559fc685, 178, 0, 0},
  {0x55a7321e, 92, 1, 991},
  {0x55c3589c, 92, 0, 0},
  {0x55ca12c9, 126, 1, 1085},
  {0x55d06deb, 22, 1, 592},
  {0x55db7e2a, 152, 1, 694},
  {0x55e73ce5, 12, 1, 892},
  {0x5603f579, 126, 0, 0},
  {0x560e44b9, 166, 1, 0},
  {0x563aa29d, 117, 0, 0},
  {0x563c2cc0, 160, 1, 391},
  {0x565a4681, 148, 0, 0},
  {0x565a57e5, 100, 0, 0},
  {0x565b1bdb, 126, 2, 0},
  {0x56756615, 77, 1, 291},
  {0x5679e4dc, 124, 0, 0},
  {0x567dccbd, 197, 0, 0},
  {0x567e1620, 143, 1, 291},
  {0x56b87cb5, 117, 0, 0},
  {0x56dbfd1f, 0, 1, 0},
  {0x56f01370, 97, 0, 0},
  {0x56f1760c, 126, 0, 0},
  {0x572ca03a, 117, 0, 0},
  {0x5746a461, 117, 0, 0},
  {0x5755a36c, 129, 2, 0},
  {0x575ed2fe, 126, 1, 686},
  {0x576a0de8, 74, 1, 191},
  {0x576b493c, 0, 1, 0},
  {0x576cc1a0, 0, 1, 0},
  {0x577482bb, 160, 0, 0},
  {0x57783e71, 24, 0, 0},
  {0x5795f169, 117, 0, 0},
  {0x57970078, 126, 0, 0},
  {0x579e5d0b, 34, 0, 0},
  {0x57abbf15, 51, 1, 90},
  {0x57ac67af, 126, 1, 1088},
  {0x57c12280, 160, 1, 190},
  {0x57c12c17, 72, 0, 0},
  {0x57c2ae4e, 153, 1, 1193},
  {0x57ca4dbb, 47, 0, 0},
  {0x57d162f1, 97, 0, 0},
  {0x57db9140, 117, 0, 0},
  {0x57dd23d1, 126, 1, 889},
  {0x57de2c14, 46, 1, 1289},
  {0x57dea1fd, 120, 0, 0},
  {0x57e220d0, 155, 0, 0},
  {0x57e3218b, 99, 0, 0},
  {0x57e9b21c, 148, 0, 0},
  {0x57f33f70, 99, 0, 0},
  {0x57f72adf, 77, 0, 0},
  {0x5800be2d, 166, 1, 0},
  {0x580aa5e7, 126, 0, 0},
  {0x5816ccc5, 178, 0, 0},
  {0x5820ea66, 29, 1, 1289},
  {0x583d21ca, 77, 0, 0},
  {0x58507bc9, 99, 0, 0},
  {0x5857681c, 147, 0, 0},
  {0x58581770, 117, 0, 0},
  {0x585ba83d, 4, 2, 0},
  {0x5866aebe, 115, 1, 1292},
  {0x586915a7, 190, 0, 0},
  {0x586a3277, 77, 1, 988},
  {0x588e7492, 165, 0, 0},
  {0x589b2ec8, 89, 0, 0},
  {0x58a407bb, 100, 1, 393},
  {0x58c7ddaf, 46, 1, 1291},
  {0x58e34e66, 114, 1, 390},
  {0x58e5d1ea, 38, 0, 0},
  {0x58ebd5d7, 77, 0, 0},
  {0x58fe5467, 133, 0, 0},
  {0x591364c9, 97, 1, 1289},
  {0x59138e01, 117, 0, 0},
  {0x59276597, 160, 0, 0},
  {0x59280bec, 77, 0, 0},
  {0x592e3567, 13, 1, 1291},
  {0x5931be01, 69, 0, 0},
  {0x595b2b26, 34, 0, 0},
  {0x59643ed6, 115, 1, 1288},
  {0x59794f2d, 24, 0, 0},
  {0x598a7398, 92, 0, 0},
  {0x5991b9d0, 0, 1, 0},
  {0x59977a46, 126, 1, 1085},
  {0x59a6c2ac, 120, 0, 0},
  {0x59a706e4, 123, 1, 991},
  {0x59cd0c31, 160, 0, 0},
  {0x59e3343f, 156, 1, 488},
  {0x59f0f3ca, 160, 0, 0},
  {0x5a032058, 66, 0, 0},
  {0x5a0454f3, 195, 0, 0},
  {0x5a046cdc, 95, 0, 0},
  {0x5a09134f, 104, 1, 90},
  {0x5a18f611, 195, 0, 0},
  {0x5a2d1abf, 34, 2, 0},
  {0x5a3e41d8, 156, 1, 290},
  {0x5a3ec21c, 100, 0, 0},
  {0x5a4026c8, 92, 0, 0},
  {0x5a40f532, 89, 0, 0},
  {0x5a4f156d, 153, 1, 292},
  {0x5a5a0cd9, 28, 0, 0},
  {0x5a6860f1, 72, 0, 0},
  {0x5a7b15c8, 89, 0, 0},
  {0x5a7fd507, 46, 0, 0},
  {0x5a8b4da8, 146, 1, 1190},
  {0x5a9824e0, 156, 1, 1288},
  {0x5ab54795, 120, 0, 0},
  {0x5ace8ca0, 77, 0, 0},
  {0x5adbf660, 100, 0, 0},
  {0x5b064824, 34, 1, 789},
  {0x5b0b26be, 156, 0, 0},
  {0x5b11eb85, 192, 0, 0},
  {0x5b16a3c8, 200, 1, 0},
  {0x5b2b72cb, 33, 1, 0},
  {0x5b4b6056, 34, 1, 990},
  {0x5b4c6146, 117, 0, 0},
  {0x5b58b662, 160, 0, 0},
  {0x5b5f1d5c, 0, 1, 0},
  {0x5b68abd1, 46, 0, 0},
  {0x5b6ca654, 74, 1, 1291},
  {0x5b7ac91f, 92, 0, 0},
  {0x5b80ff0c, 34, 1, 1092},
  {0x5b837e8d, 156, 0, 0},
  {0x5b838ce2, 126, 1, 889},
  {0x5bb62688, 46, 1, 987},
  {0x5bc9d7a1, 46, 1, 390},
  {0x5bf4da62, 72, 0, 0},
  {0x5bf675ba, 100, 2, 0},
  {0x5c123ef7, 2, 0, 0},
  {0x5c1d053a, 25, 0, 0},
  {0x5c2cfe0e, 69, 0, 0},
  {0x5c629297, 193, 0, 0},
  {0x5c707ac4, 126, 0, 0},
  {0x5c9063e0, 126, 1, 1085},
  {0x5caa3e61, 10, 1, 0},
  {0x5cb41fed, 102, 0, 0},
  {0x5cc1e2c6, 2, 0, 0},
  {0x5ceb1256, 115, 1, 1288},
  {0x5cf536f4, 160, 1, 391},
  {0x5cf548d3, 126, 1, 0},
  {0x5d044347, 126, 1, 1085},
  {0x5d05ccd0, 89, 1, 291},
  {0x5d105c10, 25, 0, 0},
  {0x5d1301c5, 117, 0, 0},
  {0x5d1c5f7e, 160, 1, 1092},
  {0x5d2b1962, 126, 2, 0},
  {0x5d63ed6f, 200, 1, 0},
  {0x5d656d35, 66, 0, 0},
  {0x5d6952b9, 159, 0, 0},
  {0x5d8992c6, 78, 0, 0},
  {0x5d965b46, 62, 0, 0},
  {0x5d9d9891, 53, 0, 0},
  {0x5da9cec8, 31, 1, 0},
  {0x5dbd6099, 34, 1, 690},
  {0x5dca93ef, 68, 1, 489},
  {0x5dce2eea, 34, 1, 692},
  {0x5de61639, 0, 1, 0},
  {0x5de7f27c, 165, 0, 0},
  {0x5dec84f8, 191, 0, 0},
  {0x5ded683e, 34, 1, 0},
  {0x5e023291, 34, 1, 92},
  {0x5e137c5b, 126, 2, 0},
  {0x5e24eeda, 148, 0, 0},
  {0x5e268761, 34, 1, 0},
  {0x5e33b189, 24, 1, 489},
  {0x5e345b6d, 124, 0, 0},
  {0x5e35ad1d, 100, 0, 0},
  {0x5e3f7004, 176, 0, 0},
  {0x5e5723f4, 77, 0, 0},
  {0x5e5d5963, 24, 0, 0},
  {0x5e66eaea, 158, 1, 691},
  {0x5e6d9975, 4, 2, 0},
  {0x5e863db6, 140, 0, 0},
  {0x5e900522, 160, 1, 1188},
  {0x5e98ba4a, 89, 0, 0},
  {0x5e9bc161, 160, 0, 0},
  {0x5ea7d410, 57, 1, 490},
  {0x5eb21035, 34, 0, 0},
  {0x5eb8e707, 142, 1, 991},
  {0x5eb94b11, 15, 1, 0},
  {0x5ee5a942, 104, 1, 1187},
  {0x5ee6008e, 143, 1, 690},
  {0x5eea106c, 36, 0, 0},
  {0x5ef4f30c, 126, 1, 686},
  {0x5ef7f6e2, 34, 0, 0},
  {0x5efbded8, 117, 0, 0},
  {0x5efc27d6, 126, 0, 0},
  {0x5f0bce2a, 57, 1, 193},
  {0x5f14dc48, 77, 0, 0},
  {0x5f162195, 117, 0, 0},
  {0x5f1e7b19, 165, 1, 0},
  {0x5f30fcd8, 166, 1, 0},
  {0x5f6798f4, 2, 0, 0},
  {0x5f6e8a07, 32, 0, 0},
  {0x5f7c1d17, 160, 1, 893},
  {0x5f807010, 150, 1, 192},
  {0x5f82cb7d, 100, 0, 0},
  {0x5fab6bce, 126, 0, 0},
  {0x5fbd9178, 171, 0, 0},
  {0x5fd2aab1, 46, 1, 1091},
  {0x603aaa57, 34, 1, 1190},
  {0x603c8abe, 77, 0, 0},
  {0x6055fe9b, 165, 0, 0},
  {0x6058c65d, 126, 0, 0},
  {0x605f5d51, 181, 0, 0},
  {0x607db27a, 97, 0, 0},
  {0x607f9765, 165, 1, 690},
  {0x60a01cab, 152, 1, 793},
  {0x60a3b803, 178, 0, 0},
  {0x60d88631, 19, 0, 0},
  {0x60e63537, 126, 1, 1290},
  {0x60f1fd6f, 92, 0, 0},
  {0x60ff6444, 16, 0, 0},
  {0x61179bfa, 196, 1, 894},
  {0x611d2586, 20, 0, 0},
  {0x61253d1c, 40, 1, 0},
  {0x614afbea, 0, 1, 0},
  {0x6150517c, 34, 0, 0},
  {0x615d0a50, 97, 0, 0},
  {0x619bea12, 144, 0, 0},
  {0x619bfd3e, 65, 1, 390},
  {0x61a852ea, 81, 0, 0},
  {0x61cb190c, 14, 1, 190},
  {0x61d86167, 24, 1, 689},
  {0x61f96440, 117, 0, 0},
  {0x61fbd9a6, 200, 1, 0},
  {0x62119cb5, 126, 1, 1085},
  {0x622f059d, 160, 0, 0},
  {0x6231e6df, 117, 0, 0},
  {0x6267fbd1, 160, 0, 0},
  {0x626abd49, 77, 0, 0},
  {0x6272c549, 115, 1, 1292},
  {0x627ad380, 15, 1, 0},
  {0x628cc178, 165, 0, 0},
  {0x62a94f97, 37, 0, 0},
  {0x62afe166, 0, 1, 0},
  {0x62b71912, 0, 1, 0},
  {0x62c67984, 164, 0, 0},
  {0x62dfc064, 91, 1, 690},
  {0x62e2e7fc, 50, 1, 1092},
  {0x62ef6c79, 33, 1, 0},
  {0x630be870, 143, 1, 790},
  {0x632edf4e, 6, 2, 0},
  {0x632f6053, 0, 1, 0},
  {0x6332e4ca, 10, 1, 0},
  {0x6377cb75, 133, 0, 0},
  {0x637e366a, 100, 2, 0},
  {0x637fe65c, 15, 1, 0},
  {0x63968624, 24, 1, 987},
  {0x6396b988, 99, 0, 0},
  {0x639d74e4, 173, 0, 0},
  {0x63abf889, 126, 0, 0},
  {0x63aea200, 126, 0, 0},
  {0x63af202f, 155, 0, 0},
  {0x63af464b, 117, 0, 0},
  {0x63b00ea7, 92, 0, 0},
  {0x63b7637c, 38, 0, 0},
  {0x63bb86b5, 160, 0, 0},
  {0x63c4e122, 143, 0, 0},
  {0x63ddd219, 0, 1, 0},
  {0x63e929c4, 166, 1, 0},
  {0x63e992ac, 115, 1, 294},
  {0x6414e03d, 89, 0, 0},
  {0x6438d8b0, 117, 0, 0},
  {0x644dfe2b, 178, 0, 0},
  {0x644e312b, 179, 0, 0},
  {0x6467a5c4, 15, 1, 0},
  {0x646ed7f4, 180, 0, 0},
  {0x6479e76a, 92, 0, 0},
  {0x647eafa9, 103, 1, 894},
  {0x64818fc5, 100, 0, 0},
  {0x64a02715, 100, 0, 0},
  {0x64a75e8d, 181, 0, 0},
  {0x64bd6cdb, 161, 0, 0},
  {0x64c0fa3b, 34, 0, 0},
  {0x64c96f53, 77, 0, 0},
  {0x64eb4449, 38, 0, 0},
  {0x64edbae3, 77, 0, 0},
  {0x64fc3667, 0, 1, 0},
  {0x64fd3ba6, 164, 0, 0},
  {0x650353f7, 117, 0, 0},
  {0x651700a5, 166, 1, 0},
  {0x652f3324, 92, 0, 0},
  {0x654d7a3e, 23, 0, 0},
  {0x654f4e90, 155, 2, 0},
  {0x65518eae, 127, 1, 192},
  {0x655efeed, 143, 1, 587},
  {0x656196a6, 117, 0, 0},
  {0x65636a4e, 117, 0, 0},
  {0x656d4265, 126, 1, 686},
  {0x656fa3b5, 92, 0, 0},
  {0x65b6c0a1, 93, 0, 0},
  {0x65b96d61, 180, 0, 0},
  {0x65fb428a, 91, 1, 392},
  {0x660e8b34, 164, 0, 0},
  {0x6611497f, 144, 0, 0},
  {0x6611d8f3, 160, 0, 0},
  {0x664281ac, 76, 1, 792},
  {0x664e9906, 21, 0, 0},
  {0x666be5ec, 160, 2, 0},
  {0x666e905b, 55, 0, 0},
  {0x6679648a, 15, 1, 0},
  {0x667982c1, 57, 1, 792},
  {0x667eebb4, 0, 1, 0},
  {0x66a0c7f2, 92, 0, 86},
  {0x66b2dec7, 24, 0, 0},
  {0x66d65a58, 126, 0, 0},
  {0x66dd04e1, 190, 0, 0},
  {0x66e64e41, 92, 0, 0},
  {0x66ed9c00, 161, 0, 0},
  {0x66f11648, 126, 1, 1085},
  {0x66f4d9f5, 128, 0, 0},
  {0x66f6a39e, 34, 2, 0},
  {0x6701a19f, 92, 0, 0},
  {0x67193fc8, 0, 1, 0},
  {0x671f23a8, 100, 2, 0},
  {0x6720abac, 129, 2, 0},
  {0x6756763b, 24, 0, 0},
  {0x6772ca86, 20, 0, 0},
  {0x67751094, 100, 1, 689},
  {0x67755041, 160, 0, 0},
  {0x6776a977, 54, 0, 0},
  {0x67811da6, 46, 1, 290},
  {0x67a0d115, 164, 0, 0},
  {0x67a3c362, 93, 0, 0},
  {0x67c89f06, 126, 1, 1088},
  {0x67d5c3f9, 37, 0, 0},
  {0x67e1c05c, 126, 1, 0},
  {0x67f77118, 97, 1, 690},
  {0x6800c5b3, 147, 0, 0},
  {0x6817857d, 74, 1, 0},
  {0x6823cbc8, 77, 0, 0},
  {0x682d2df1, 0, 1, 0},
  {0x68309d06, 0, 1, 0},
  {0x68383607, 65, 1, 1089},
  {0x684afccd, 97, 0, 0},
  {0x684b292f, 117, 0, 0},
  {0x685257a8, 92, 1, 690},
  {0x687ebcd7, 156, 0, 0},
  {0x6893f7b3, 150, 1, 791},
  {0x689971f9, 153, 1, 689},
  {0x68a8cf73, 92, 1, 391},
  {0x68afef5f, 130, 1, 0},
  {0x68c83aa3, 115, 1, 790},
  {0x68e8a520, 97, 0, 0},
  {0x68ec97cb, 97, 1, 689},
  {0x68ecb923, 197, 0, 0},
  {0x68f9b5f5, 129, 2, 0},
  {0x6901346e, 117, 0, 0},
  {0x6906e0da, 100, 1, 292},
  {0x69240dcf, 63, 0, 0},
  {0x692c1c3c, 100, 0, 0},
  {0x6944a01a, 116, 1, 291},
  {0x69488c31, 0, 2, 0},
  {0x69565f13, 54, 0, 0},
  {0x695cb142, 92, 0, 0},
  {0x69635a6e, 68, 1, 290},
  {0x69684e1e, 24, 1, 192},
  {0x696d7839, 165, 1, 992},
  {0x697e10f6, 74, 1, 1191},
  {0x69885e71, 126, 0, 0},
  {0x6997f5e1, 115, 1, 690},
  {0x699fa085, 4, 1, 1288},
  {0x69bcdb8b, 80, 0, 0},
  {0x69d07ddb, 92, 0, 0},
  {0x69da7649, 126, 1, 0},
  {0x69e534eb, 69, 0, 0},
  {0x6a10add2, 193, 0, 0},
  {0x6a154b68, 6, 1, 1088},
  {0x6a1f628a, 97, 1, 1289},
  {0x6a221cbf, 59, 0, 0},
  {0x6a27567e, 156, 1, 1192},
  {0x6a483073, 40, 1, 0},
  {0x6a50b553, 100, 0, 0},
  {0x6a5abfac, 34, 1, 892},
  {0x6a6b7239, 195, 0, 0},
  {0x6a80de01, 126, 2, 0},
  {0x6a82ac89, 160, 1, 188},
  {0x6a88579f, 65, 1, 1089},
  {0x6aad2985, 38, 0, 0},
  {0x6ae69227, 164, 0, 0},
  {0x6af5f21b, 100, 1, 193},
  {0x6b403c04, 97, 0, 0},
  {0x6b523bd7, 33, 1, 0},
  {0x6b53006a, 29, 1, 1289},
  {0x6b53222c, 117, 0, 0},
  {0x6b53d59a, 0, 1, 0},
  {0x6b5ca5ae, 46, 0, 0},
  {0x6b8d777d, 0, 1, 0},
  {0x6b8f23e0, 0, 1, 0},
  {0x6b999aaf, 34, 1, 1190},
  {0x6bb6a0ce, 76, 1, 1190},
  {0x6bbd9f11, 161, 0, 0},
  {0x6bbe2be5, 92, 0, 0},
  {0x6bc33d2f, 2, 1, 492},
  {0x6bc65d7e, 92, 0, 0},
  {0x6bd7dafe, 100, 0, 0},
  {0x6be54033, 100, 0, 0},
  {0x6bfe4ff1, 117, 0, 0},
  {0x6c040686, 33, 1, 0},
  {0x6c1390b4, 77, 0, 0},
  {0x6c15f90e, 196, 1, 292},
  {0x6c168561, 92, 1, 190},
  {0x6c3ca47b, 72, 0, 0},
  {0x6c42b2c6, 77, 1, 1290},
  {0x6c438bb2, 160, 0, 0},
  {0x6c5dbedf, 175, 0, 0},
  {0x6c61b622, 92, 0, 0},
  {0x6c6c2feb, 24, 0, 0},
  {0x6c70a17b, 193, 0, 0},
  {0x6c93377c, 33, 1, 0},
  {0x6cc62c06, 126, 0, 0},
  {0x6cd204b5, 77, 0, 0},
  {0x6cd46979, 24, 1, 289},
  {0x6cd9cc23, 128, 0, 0},
  {0x6cdc0cd9, 160, 0, 0},
  {0x6cf4d165, 10, 1, 0},
  {0x6d2b7846, 34, 0, 0},
  {0x6d3396e3, 0, 1, 0},
  {0x6d612468, 65, 1, 392},
  {0x6d65cac6, 124, 0, 0},
  {0x6d867d34, 126, 1, 490},
  {0x6d9ba342, 163, 1, 489},
  {0x6daaa704, 126, 0, 0},
  {0x6db30701, 46, 0, 86},
  {0x6dc28b5a, 100, 0, 0},
  {0x6dce4b23, 164, 0, 0},
  {0x6de472f4, 89, 0, 0},
  {0x6decd886, 143, 1, 689},
  {0x6e0eb43e, 50, 1, 690},
  {0x6e16adac, 126, 0, 0},
  {0x6e2e53c3, 126, 0, 0},
  {0x6e32f3c2, 97, 0, 0},
  {0x6e4697bf, 9, 0, 0},
  {0x6e4c0641, 174, 0, 0},
  {0x6e4dcfd2, 115, 1, 592},
  {0x6e53128e, 160, 0, 0},
  {0x6e53eaa9, 117, 0, 0},
  {0x6e593fc7, 0, 2, 0},
  {0x6e6490cd, 143, 1, 291},
  {0x6e68e31a, 24, 0, 0},
  {0x6e72b8ff, 166, 1, 0},
  {0x6e85d8dd, 146, 1, 889},
  {0x6e899ccf, 43, 0, 0},
  {0x6e9a5b6f, 46, 1, 188},
  {0x6ec51de5, 117, 0, 0},
  {0x6ec74e4c, 92, 0, 0},
  {0x6ed3ba25, 40, 1, 0},
  {0x6ee4bb0a, 34, 1, 1287},
  {0x6ee68312, 77, 1, 690},
  {0x6ee94d32, 4, 1, 991},
  {0x6f10097d, 4, 1, 91}, // Simpsons - Bart Vs the Space Mutants, The (U)
  {0x6f27300b, 129, 2, 0},
  {0x6f365e7b, 156, 0, 0},
  {0x6f3d2737, 4, 1, 991},
  {0x6f4e4312, 99, 0, 0},
  {0x6f5d9b2a, 89, 0, 0},
  {0x6f5f60b9, 77, 0, 0},
  {0x6f605522, 77, 1, 989},
  {0x6f6686b0, 89, 0, 0},
  {0x6f742ef2, 115, 1, 993},
  {0x6f860e89, 71, 2, 0},
  {0x6f8af3e8, 100, 1, 190},
  {0x6f96ed15, 191, 0, 0},
  {0x6f97c721, 126, 1, 686},
  {0x6fa0e650, 77, 1, 291},
  {0x6fb349e2, 74, 1, 0},
  {0x6fb51ffb, 34, 0, 0},
  {0x6fd46392, 97, 1, 190},
  {0x6fd5a271, 92, 1, 391},
  {0x70080810, 126, 1, 0},
  {0x701b1adf, 77, 0, 0},
  {0x702d9b33, 195, 0, 0},
  {0x703e1948, 126, 1, 1088},
  {0x704ad587, 34, 0, 0},
  {0x705bd7c3, 133, 0, 0},
  {0x705e13ca, 77, 0, 0},
  {0x705ef660, 156, 0, 0},
  {0x7062f3fa, 100, 1, 491},
  {0x7071eef2, 75, 0, 0},
  {0x7077a85d, 0, 1, 0},
  {0x7077b075, 127, 1, 493},
  {0x7080d1f8, 174, 0, 0},
  {0x708ea2be, 126, 0, 0},
  {0x7090b851, 53, 0, 0},
  {0x7095ac65, 100, 0, 0},
  {0x709c9399, 165, 2, 0},
  {0x70a1f862, 143, 1, 887},
  {0x70a6bfc0, 75, 0, 0},
  {0x70bb52d7, 34, 0, 0},
  {0x70ce3771, 196, 1, 1192},
  {0x70d49617, 66, 0, 0},
  {0x70e0b7d8, 156, 0, 0},
  {0x70f4dadb, 91, 2, 0},
  {0x70f67ab7, 148, 0, 0},
  {0x711896b8, 156, 1, 1288},
  {0x711c2b0e, 117, 0, 0},
  {0x713309ba, 0, 1, 0},
  {0x713cfeb9, 92, 0, 0},
  {0x71547d94, 130, 1, 0},
  {0x7154acb5, 166, 1, 0},
  {0x7156cb4d, 74, 1, 1190},
  {0x716c3fce, 174, 0, 0},
  {0x716daea5, 119, 0, 0},
  {0x7172f3d4, 72, 0, 0},
  {0x717e1169, 100, 0, 0},
  {0x717e1c46, 188, 1, 1290},
  {0x71b107e7, 156, 0, 0},
  {0x71baecec, 129, 2, 0},
  {0x71c9ed1e, 133, 0, 0},
  {0x71d868c4, 0, 1, 0},
  {0x71d8c6e9, 133, 0, 0},
  {0x720fff06, 124, 0, 0},
  {0x72125764, 156, 0, 0},
  {0x721b5217, 180, 0, 0},
  {0x721be58a, 97, 1, 88},
  {0x72342953, 77, 0, 0},
  {0x725b0234, 126, 2, 0},
  {0x7262843c, 92, 1, 1192},
  {0x7265a393, 178, 0, 0},
  {0x7266b1b3, 126, 0, 0},
  {0x726edb66, 89, 0, 0},
  {0x72725a0b, 193, 1, 988},
  {0x72823de0, 100, 0, 0},
  {0x7287d1c0, 153, 1, 292},
  {0x728bfa8d, 156, 0, 0},
  {0x728c3d98, 164, 0, 0},
  {0x72924076, 156, 0, 0},
  {0x72928698, 156, 0, 0},
  {0x72a0d9a7, 6, 1, 1290},
  {0x72c39b9e, 143, 0, 0},
  {0x72d81daa, 100, 0, 0},
  {0x72dd850d, 43, 1, 89},
  {0x72ddbd39, 100, 2, 0},
  {0x72e2395b, 0, 1, 0},
  {0x72e32402, 179, 0, 0},
  {0x72e65d5c, 117, 0, 0},
  {0x72e66392, 40, 1, 0},
  {0x73140eef, 200, 1, 0},
  {0x73272e0a, 77, 0, 0},
  {0x7329118d, 150, 1, 0},
  {0x732b0675, 126, 0, 0},
  {0x73620901, 57, 1, 1188},
  {0x736825f6, 53, 0, 0},
  {0x7376568d, 100, 0, 0},
  {0x737dd1bf, 126, 0, 0},
  {0x737ec182, 38, 0, 0},
  {0x73921674, 96, 0, 0},
  {0x739a1027, 77, 0, 0},
  {0x73ac76db, 24, 0, 0},
  {0x73ad02b7, 165, 0, 0},
  {0x73c7fcf4, 34, 1, 693},
  {0x73cefba7, 0, 1, 0},
  {0x73e41ac7, 150, 0, 0},
  {0x73f7e5d8, 79, 0, 0},
  {0x73ff22a9, 46, 0, 0},
  {0x740c7582, 34, 1, 1191},
  {0x74189e12, 120, 1, 691},
  {0x743387ff, 100, 0, 0},
  {0x7434dc82, 0, 1, 0},
  {0x74386f15, 173, 1, 1089},
  {0x74452e18, 77, 0, 0},
  {0x74663267, 43, 0, 0},
  {0x7474ac92, 68, 1, 191},
  {0x749dc135, 123, 1, 989},
  {0x74b8498e, 0, 1, 0},
  {0x74c08f86, 65, 1, 91},
  {0x7510180a, 126, 1, 1189},
  {0x751fedb7, 77, 0, 0},
  {0x75255f88, 143, 1, 989},
  {0x752743ec, 34, 0, 0},
  {0x753768a6, 12, 1, 1191},
  {0x75547d3e, 165, 0, 0},
  {0x758afbbd, 128, 0, 0},
  {0x75901b18, 150, 0, 0},
  {0x7595c7fc, 46, 1, 1090},
  {0x759c40a1, 89, 0, 0},
  {0x75a7e399, 174, 0, 0},
  {0x75a8943b, 123, 1, 790},
  {0x75b3eb37, 92, 0, 0},
  {0x75b9c0db, 46, 2, 0},
  {0x75ce31c0, 127, 1, 693},
  {0x75f6a9f3, 126, 1, 686},
  {0x75f96142, 92, 0, 0},
  {0x761ccfb5, 117, 0, 0},
  {0x762c72ae, 31, 1, 0},
  {0x764bea6b, 89, 0, 0},
  {0x766727b2, 163, 1, 189},
  {0x766c2cac, 126, 0, 0},
  {0x7671bc51, 34, 0, 0},
  {0x7677a6a6, 38, 0, 0},
  {0x7678f1d5, 160, 0, 0},
  {0x767a6a66, 92, 0, 0},
  {0x768a1b6a, 192, 0, 0},
  {0x76a654b9, 21, 0, 0},
  {0x76b56d0a, 89, 0, 0},
  {0x76c161e3, 77, 2, 0},
  {0x76d5175e, 117, 0, 0},
  {0x76e85ada, 69, 0, 0},
  {0x76fff9d5, 156, 0, 0},
  {0x7709a696, 35, 0, 0},
  {0x771c8855, 0, 1, 0},
  {0x772513f4, 161, 0, 0},
  {0x77415ad3, 178, 0, 0},
  {0x77512388, 69, 0, 0},
  {0x7751588d, 126, 2, 0}, // Metroid (E) [!]
  {0x77540bb5, 143, 1, 788},
  {0x775b8ed9, 89, 0, 0},
  {0x77833016, 24, 1, 1086},
  {0x778aaf25, 92, 1, 788},
  {0x77bf8b23, 57, 1, 689},
  {0x77c51d28, 120, 0, 0},
  {0x77dafd89, 46, 1, 1086},
  {0x77dcbba3, 69, 0, 0},
  {0x7826fd3e, 163, 1, 390},
  {0x7831b2ff, 72, 0, 0},
  {0x7840b18d, 150, 0, 0},
  {0x7848dad8, 173, 0, 0},
  {0x786148b6, 100, 0, 0},
  {0x7866fb2c, 160, 0, 0},
  {0x7876e439, 72, 0, 0},
  {0x78784cbf, 14, 1, 190},
  {0x787b41cc, 46, 1, 987},
  {0x7884b56e, 109, 1, 192},
  {0x7889f720, 166, 1, 0},
  {0x788bed9a, 89, 0, 0},
  {0x78a48b23, 126, 0, 0},
  {0x78b09986, 100, 0, 0},
  {0x78b657ac, 81, 0, 0},
  {0x78bb266a, 126, 1, 1088},
  {0x78bd3c9a, 34, 0, 0},
  {0x78bdf588, 100, 0, 0},
  {0x78bff6b0, 133, 0, 0},
  {0x78c4460d, 162, 0, 0},
  {0x78dfb5d3, 143, 0, 0},
  {0x78fcf4fa, 157, 0, 0},
  {0x790d2916, 165, 2, 0},
  {0x791138d9, 126, 1, 686},
  {0x79123682, 0, 1, 0},
  {0x79149ebf, 182, 1, 688},
  {0x7918f29c, 24, 0, 0},
  {0x792070a9, 33, 1, 0},
  {0x7936bc91, 66, 0, 0},
  {0x7950b715, 89, 1, 987},
  {0x795d23ec, 5, 1, 592},
  {0x79698b98, 143, 0, 0},
  {0x7980c4f7, 24, 0, 0},
  {0x798ac9dc, 46, 0, 0},
  {0x798b491d, 66, 0, 0},
  {0x798eeb98, 34, 1, 1291},
  {0x798fbc68, 165, 0, 0},
  {0x799ad3c6, 115, 1, 993},
  {0x79bfe095, 0, 1, 0},
  {0x79c91906, 124, 0, 0},
  {0x79cac545, 24, 0, 0},
  {0x79d8c39d, 126, 1, 1188},
  {0x79f688bc, 115, 2, 0},
  {0x79fba5a2, 21, 0, 0},
  {0x79ff3562, 66, 0, 0},
  {0x7a11d2c9, 72, 0, 0},
  {0x7a365bf8, 0, 1, 0},
  {0x7a373833, 163, 1, 191},
  {0x7a424c07, 104, 1, 989},
  {0x7a497ae3, 160, 0, 0},
  {0x7a4d4eaf, 163, 1, 291},
  {0x7a508dbb, 100, 0, 0},
  {0x7a6e0454, 0, 1, 0},
  {0x7a748058, 9, 0, 0},
  {0x7a76e057, 39, 0, 0},
  {0x7a8ae523, 117, 0, 0},
  {0x7ae0bf3c, 126, 0, 0},
  {0x7ae5c002, 77, 2, 91}, // Jackie Chan's Action Kung Fu (E)
  {0x7aee2161, 179, 0, 0},
  {0x7af3afb7, 156, 0, 0},
  {0x7af8204a, 77, 0, 0},
  {0x7b0a41b9, 92, 0, 0},
  {0x7b1b4dd0, 89, 1, 790},
  {0x7b35964e, 100, 1, 192},
  {0x7b44fb2a, 34, 0, 0},
  {0x7b4ed0bb, 104, 1, 1193},
  {0x7b5206af, 24, 0, 0},
  {0x7b5bd2de, 126, 1, 686},
  {0x7b6dc772, 117, 0, 0},
  {0x7b72fba4, 128, 0, 0},
  {0x7b82400f, 174, 0, 0},
  {0x7bb5664f, 117, 0, 0},
  {0x7bc98e11, 68, 1, 788},
  {0x7bd7b849, 164, 0, 0},
  {0x7bd8f902, 77, 0, 0},
  {0x7be93173, 4, 1, 190},
  {0x7bec1745, 160, 0, 0},
  {0x7bf8a890, 165, 0, 0},
  {0x7c27ab86, 128, 0, 0},
  {0x7c393da4, 100, 0, 0},
  {0x7c3d2ea3, 178, 0, 0},
  {0x7c40d6c6, 0, 1, 0},
  {0x7c46998b, 2, 0, 0},
  {0x7c4a72d8, 165, 1, 0},
  {0x7c596e45, 53, 0, 0},
  {0x7c6a3d51, 34, 1, 1088},
  {0x7c6f615f, 158, 1, 192},
  {0x7c7ab58e, 117, 0, 0},
  {0x7c854039, 117, 0, 0},
  {0x7c864796, 160, 0, 0},
  {0x7ca47b40, 0, 1, 0},
  {0x7ca52798, 22, 0, 0},
  {0x7cba563f, 92, 0, 0},
  {0x7cc4778c, 100, 0, 0},
  {0x7cc9c669, 0, 1, 0},
  {0x7ce3dab2, 166, 1, 0},
  {0x7cff0f84, 126, 0, 0},
  {0x7d139211, 195, 0, 0},
  {0x7d223c3d, 117, 0, 0},
  {0x7d280346, 126, 1, 1085},
  {0x7d32acb9, 133, 0, 0},
  {0x7d38027a, 50, 1, 1092},
  {0x7d44b948, 156, 0, 0},
  {0x7d4caf6c, 0, 2, 0},
  {0x7d55cf29, 133, 0, 0},
  {0x7d56840a, 126, 0, 0},
  {0x7d5ca373, 155, 0, 0},
  {0x7d9ad28f, 77, 0, 0},
  {0x7d9d214b, 24, 1, 1086},
  {0x7da80b1b, 117, 0, 0},
  {0x7da8aead, 134, 0, 0},
  {0x7dab215c, 0, 1, 0},
  {0x7dc49898, 0, 1, 0},
  {0x7dcb4c18, 13, 1, 492},
  {0x7dcbea73, 166, 1, 0},
  {0x7dd82754, 143, 0, 0},
  {0x7e036525, 56, 2, 0},
  {0x7e053e64, 117, 0, 0},
  {0x7e26c7d0, 100, 1, 0},
  {0x7e4966e4, 117, 0, 0},
  {0x7e4ba78f, 0, 2, 0},
  {0x7e57fbec, 104, 1, 392},
  {0x7e5d2f1a, 2, 0, 0},
  {0x7e704a14, 24, 0, 0},
  {0x7e990ac3, 166, 1, 0},
  {0x7ec6f75b, 193, 0, 0},
  {0x7ed91f80, 10, 1, 0},
  {0x7ee02ca2, 133, 0, 0},
  {0x7ee625eb, 100, 0, 0},
  {0x7f0ad375, 117, 0, 0},
  {0x7f17c89f, 160, 0, 0},
  {0x7f1d087f, 69, 0, 0},
  {0x7f24efc0, 200, 1, 0},
  {0x7f2a04bf, 156, 0, 0},
  {0x7f397886, 180, 0, 0},
  {0x7f45cff5, 92, 0, 0},
  {0x7f531249, 6, 1, 291},
  {0x7f5f4e4a, 117, 0, 0},
  {0x7f68c629, 66, 0, 0},
  {0x7f7156a7, 68, 1, 489},
  {0x7f7f2821, 157, 0, 0},
  {0x7f7fef6e, 13, 1, 692},
  {0x7f93fb41, 99, 1, 1089},
  {0x7fb72a2c, 57, 1, 191},
  {0x7fb74a43, 160, 1, 1291},
  {0x7fb799fd, 24, 0, 0},
  {0x7fcc340a, 100, 0, 0},
  {0x7fedc0d7, 69, 0, 0},
  {0x7ff76219, 165, 1, 490},
  {0x7ffabb4c, 192, 0, 0},
  {0x801931af, 0, 1, 0},
  {0x801a6d23, 100, 0, 0},
  {0x80250d64, 129, 2, 0},
  {0x803b9979, 81, 0, 0},
  {0x804a0570, 104, 1, 1091},
  {0x804ef7f9, 0, 1, 0},
  {0x804f898a, 21, 0, 0},
  {0x805f81bc, 100, 0, 0},
  {0x80638505, 127, 2, 0},
  {0x808606f0, 117, 0, 0},
  {0x808b89c0, 158, 1, 1193},
  {0x809c3878, 90, 0, 0},
  {0x80c15e96, 95, 0, 0},
  {0x80c227ad, 182, 0, 0},
  {0x80c41616, 0, 1, 0},
  {0x80cd1919, 126, 2, 0},
  {0x80f0d0c0, 0, 1, 0},
  {0x80fb117e, 126, 1, 394},
  {0x80fb7e6b, 126, 0, 0},
  {0x81069812, 126, 1, 1188},
  {0x8106e694, 156, 2, 0},
  {0x810b7ab9, 92, 1, 1288},
  {0x810f3cf0, 34, 0, 0},
  {0x8111ba08, 182, 1, 990},
  {0x811f06d9, 24, 1, 388},
  {0x81293104, 8, 0, 0},
  {0x8161d619, 97, 1, 1288},
  {0x8168080f, 95, 0, 0},
  {0x8172fc55, 25, 0, 0},
  {0x817cfa97, 16, 0, 0},
  {0x818ce8aa, 72, 0, 0},
  {0x8192c804, 126, 0, 0},
  {0x8192d2e7, 46, 1, 1187},
  {0x81a15eb8, 24, 0, 0},
  {0x81a5eb65, 100, 1, 493},
  {0x81a743f5, 96, 0, 0},
  {0x81af4af9, 129, 2, 0},
  {0x81b2a3cd, 100, 2, 0},
  {0x81b7f1a8, 117, 0, 0},
  {0x81bb48c2, 100, 0, 0},
  {0x81d9f17c, 165, 1, 690},
  {0x81df3d70, 24, 0, 0},
  {0x81ecda0d, 15, 1, 0},
  {0x81f31409, 34, 0, 0},
  {0x8207a96c, 156, 1, 987},
  {0x82132616, 117, 0, 0},
  {0x8218c637, 97, 0, 0},
  {0x821f2f9f, 143, 0, 0},
  {0x821feb7a, 156, 0, 0},
  {0x822f17eb, 176, 0, 0},
  {0x8236c523, 34, 0, 0},
  {0x827e5df5, 126, 1, 0},
  {0x8288323f, 74, 1, 1291},
  {0x828d779b, 0, 1, 0},
  {0x828f8f1f, 97, 0, 0},
  {0x82ab60b9, 34, 0, 0},
  {0x82ad545e, 66, 0, 0},
  {0x82afa828, 193, 1, 190},
  {0x82be4724, 34, 1, 1186},
  {0x82cfde25, 126, 1, 288},
  {0x82dff13d, 126, 1, 388},
  {0x82f48389, 193, 0, 0},
  {0x82fcadde, 104, 1, 790},
  {0x8308fed7, 143, 0, 0},
  {0x831f8294, 80, 0, 0},
  {0x831f9c1a, 15, 1, 0},
  {0x83213ca0, 0, 1, 0},
  {0x832c3b9f, 126, 0, 0},
  {0x834273da, 195, 0, 0},
  {0x83431081, 54, 0, 0},
  {0x8348c788, 6, 1, 492},
  {0x834d1924, 166, 1, 0},
  {0x8351283e, 0, 1, 0},
  {0x8360fa88, 100, 0, 0},
  {0x836c4fa7, 89, 0, 0},
  {0x836cc1ab, 100, 0, 0},
  {0x836fe2c2, 100, 2, 0},
  {0x837c1342, 160, 0, 0},
  {0x838bf76f, 195, 0, 0},
  {0x839f2f71, 188, 0, 0},
  {0x83be000c, 34, 1, 88},
  {0x83c28d94, 128, 0, 0},
  {0x83cb743f, 93, 0, 0},
  {0x83ea7b04, 25, 0, 0},
  {0x83fc38f8, 163, 1, 489},
  {0x84148f73, 92, 1, 0},
  {0x841b69b6, 30, 1, 492},
  {0x8437e478, 156, 0, 0},
  {0x84382231, 126, 0, 0},
  {0x8441a9db, 126, 1, 1085},
  {0x8442b86c, 92, 0, 0},
  {0x847d672d, 100, 1, 491},
  {0x84b930a9, 74, 1, 1090},
  {0x84be00e9, 126, 0, 0},
  {0x84d51076, 158, 1, 191},
  {0x84efd927, 53, 0, 0},
  {0x84f7fc31, 34, 2, 0},
  {0x850090bc, 97, 0, 0},
  {0x8531c166, 174, 0, 0},
  {0x85323fd6, 15, 1, 0},
  {0x85498d45, 54, 0, 0},
  {0x854f4661, 92, 0, 0},
  {0x85534474, 77, 0, 0},
  {0x856114c8, 100, 1, 87},
  {0x856e7600, 97, 0, 0},
  {0x8575a0cb, 180, 0, 0},
  {0x857dbc36, 129, 2, 0},
  {0x858863af, 42, 0, 0},
  {0x8593e5ad, 142, 1, 491},
  {0x859c65e1, 143, 1, 790},
  {0x85a922fd, 66, 0, 0},
  {0x85bc0777, 72, 0, 0},
  {0x85c5953f, 97, 0, 0},
  {0x85c5b6b4, 164, 0, 0},
  {0x85cf16e0, 0, 1, 0},
  {0x85d02cd4, 97, 1, 0}, // "Proto"
  {0x85d75d58, 24, 1, 1289},
  {0x85e0090b, 161, 0, 0},
  {0x85e1a22a, 92, 0, 0},
  {0x85f12d37, 20, 0, 0},
  {0x85f58367, 15, 1, 0},
  {0x86167220, 126, 0, 0},
  {0x86277361, 104, 1, 894},
  {0x86291f44, 160, 0, 0},
  {0x862ab1e5, 67, 1, 89},
  {0x862c4e8d, 160, 0, 0},
  {0x8635fed1, 97, 0, 0},
  {0x863cea81, 23, 0, 0},
  {0x865ac145, 117, 0, 0},
  {0x86670c93, 126, 1, 887},
  {0x86759c0f, 175, 0, 0},
  {0x8685f366, 136, 0, 0},
  {0x86964edd, 100, 1, 294},
  {0x869652fe, 97, 1, 1088},
  {0x86974ccc, 200, 1, 0},
  {0x86a39645, 117, 0, 0},
  {0x86a637b0, 0, 1, 0},
  {0x86b0d1cf, 196, 1, 1191},
  {0x86b2355f, 156, 0, 0},
  {0x86c495c6, 34, 2, 0},
  {0x86ca126d, 117, 0, 0},
  {0x86cbc595, 4, 1, 290},
  {0x86d67a97, 68, 1, 390},
  {0x86f07508, 177, 0, 0},
  {0x87052ac0, 66, 0, 0},
  {0x87133e90, 53, 0, 0},
  {0x8751abe5, 95, 0, 0},
  {0x8752dccb, 81, 0, 0},
  {0x87645f89, 142, 1, 1289},
  {0x876b80bd, 66, 0, 0},
  {0x8776c0c2, 24, 1, 791},
  {0x877dba77, 25, 0, 0},
  {0x87a3f91e, 179, 0, 0},
  {0x87b2a02c, 77, 0, 0},
  {0x87bad69b, 164, 0, 0},
  {0x87c6a8da, 0, 1, 0},
  {0x87ce3f34, 97, 0, 0},
  {0x87ed54aa, 0, 1, 0},
  {0x87f699dc, 97, 1, 990},
  {0x88053d25, 53, 0, 0},
  {0x88062d9a, 164, 0, 0},
  {0x882e1901, 15, 1, 0},
  {0x88338ed5, 92, 1, 392},
  {0x883875b1, 53, 0, 0},
  {0x885acc2b, 126, 1, 1085},
  {0x8871b5c4, 100, 1, 692},
  {0x88739adf, 201, 0, 0},
  {0x8889c564, 51, 1, 1190},
  {0x889129cb, 126, 1, 1290},
  {0x8897a8f1, 100, 2, 0},
  {0x88a6b192, 15, 1, 0},
  {0x88aa8cd8, 46, 1, 1091},
  {0x88be42e7, 173, 0, 0},
  {0x88c15cf3, 0, 1, 0},
  {0x88c30fda, 84, 2, 0},
  {0x88c83a1d, 100, 0, 0},
  {0x88d3b565, 0, 1, 0},
  {0x88e1a5f4, 143, 1, 988},
  {0x8904149e, 182, 2, 0},
  {0x890d226e, 0, 1, 0},
  {0x892434dd, 33, 1, 0},
  {0x8927fd4c, 22, 1, 991},
  {0x89550500, 72, 0, 0},
  {0x89567668, 46, 0, 0},
  {0x895cbaf8, 34, 2, 0},
  {0x895faef2, 117, 0, 0},
  {0x8963ae6e, 126, 1, 1085},
  {0x8965c590, 34, 1, 1191},
  {0x89688cd5, 143, 0, 0},
  {0x898e4232, 126, 2, 0},
  {0x89984244, 196, 2, 0},
  {0x899a0067, 117, 0, 0},
  {0x89a45446, 153, 2, 0},
  {0x89c80b65, 99, 0, 0},
  {0x89e085fe, 180, 0, 0},
  {0x89f8ebda, 90, 0, 0},
  {0x8a043cd6, 193, 1, 690},
  {0x8a093f62, 169, 0, 0},
  {0x8a0c7337, 126, 2, 0},
  {0x8a20be5f, 143, 1, 788},
  {0x8a2824bb, 117, 0, 0},
  {0x8a368744, 126, 0, 0},
  {0x8a5b72c0, 24, 0, 0},
  {0x8a5bc0d3, 165, 0, 0},
  {0x8a640aef, 4, 1, 190},
  {0x8a65baff, 126, 2, 0},
  {0x8a7d0abe, 160, 0, 0},
  {0x8a82f9c2, 122, 0, 0},
  {0x8a9586f5, 143, 1, 1087},
  {0x8a96e00d, 100, 0, 0},
  {0x8aa4ace0, 0, 1, 0},
  {0x8aa7cce1, 179, 0, 0},
  {0x8ab52a24, 100, 1, 1288},
  {0x8acafe51, 100, 2, 0},
  {0x8ada3497, 115, 1, 190},
  {0x8af25130, 117, 0, 0},
  {0x8afec38f, 77, 0, 0},
  {0x8b03f74d, 100, 0, 0},
  {0x8b0e1235, 34, 0, 0},
  {0x8b1abbe2, 0, 1, 0},
  {0x8b2e3e81, 126, 1, 1085},
  {0x8b4a2866, 179, 0, 0},
  {0x8b56faeb, 193, 1, 1288},
  {0x8b59bac3, 165, 0, 0},
  {0x8b5a9d69, 126, 1, 0},
  {0x8b5c74db, 40, 1, 0},
  {0x8b60cc58, 126, 0, 0},
  {0x8b764fe5, 100, 0, 0},
  {0x8b781d39, 15, 1, 0},
  {0x8b957b50, 43, 1, 389},
  {0x8b9d3e9c, 126, 1, 1087},
  {0x8ba75848, 151, 0, 0},
  {0x8baa5ff2, 100, 0, 0},
  {0x8baedc0e, 112, 0, 0},
  {0x8bbc37a2, 69, 0, 0},
  {0x8bc2ec0c, 117, 0, 0},
  {0x8bca5146, 160, 1, 391},
  {0x8bcb0993, 39, 0, 0},
  {0x8be59ea9, 74, 1, 0},
  {0x8bf29cb6, 34, 1, 690},
  {0x8c0c2df5, 100, 0, 0},
  {0x8c1e1336, 24, 1, 1086},
  {0x8c37a7d5, 126, 0, 0},
  {0x8c399acd, 117, 0, 0},
  {0x8c3d54e8, 93, 0, 0},
  {0x8c46a487, 171, 0, 0},
  {0x8c4d59d6, 2, 0, 0},
  {0x8c5a784e, 53, 1, 990},
  {0x8c6237fd, 89, 0, 0},
  {0x8c6631c8, 156, 0, 0},
  {0x8c71f706, 40, 1, 0},
  {0x8c75a0b6, 66, 0, 0},
  {0x8c8dedb6, 34, 1, 392},
  {0x8c8fa83b, 24, 0, 0},
  {0x8c940984, 17, 1, 1190},
  {0x8cb9eff8, 24, 1, 1086},
  {0x8ce478db, 99, 1, 491},
  {0x8ce4875e, 92, 1, 0},
  {0x8ce8e264, 200, 1, 0},
  {0x8ce99fe4, 24, 0, 0},
  {0x8ce9c87b, 150, 0, 0},
  {0x8cea87c5, 120, 0, 0},
  {0x8cf1aae2, 156, 0, 0},
  {0x8d2454b7, 92, 0, 0},
  {0x8d2488d6, 146, 1, 1190},
  {0x8d26fdea, 25, 0, 0},
  {0x8d3c33b3, 46, 0, 0},
  {0x8d5b77c0, 97, 0, 0},
  {0x8d605a06, 38, 0, 0},
  {0x8d77e5e6, 72, 0, 0},
  {0x8d94797f, 97, 1, 1092},
  {0x8d9af7af, 188, 1, 990},
  {0x8da4e539, 181, 0, 0},
  {0x8da651d4, 34, 1, 990},
  {0x8da6667d, 126, 2, 0},
  {0x8db6d11f, 126, 1, 0},
  {0x8dcc9949, 15, 1, 0},
  {0x8dcd9486, 69, 0, 0},
  {0x8dd0a0c4, 95, 0, 0},
  {0x8dd92725, 68, 1, 991},
  {0x8de76f1d, 114, 1, 389},
  {0x8dedea07, 0, 1, 0},
  {0x8e0d9179, 160, 0, 0},
  {0x8e0ea20f, 24, 0, 0},
  {0x8e1e1181, 117, 0, 0},
  {0x8e2bd25c, 126, 1, 1085},
  {0x8e373118, 201, 0, 0},
  {0x8e504a98, 77, 1, 490},
  {0x8e5c2818, 165, 1, 1291},
  {0x8e7abdfc, 180, 0, 0},
  {0x8e86d6c0, 6, 1, 1289},
  {0x8e9a5e2f, 99, 1, 1191},
  {0x8eab381c, 15, 1, 0},
  {0x8eb11e94, 171, 0, 0},
  {0x8ecbc577, 3, 2, 0},
  {0x8ed0547e, 128, 0, 0},
  {0x8edeb257, 24, 0, 0},
  {0x8ee25f78, 160, 0, 0},
  {0x8ee6463a, 202, 0, 0},
  {0x8ee7c43e, 97, 1, 493},
  {0x8ee98523, 126, 0, 0},
  {0x8eef8b76, 24, 0, 0},
  {0x8f011713, 124, 0, 0},
  {0x8f3f8b1f, 89, 0, 0},
  {0x8f4497ee, 145, 0, 0},
  {0x8f506d75, 178, 0, 0},
  {0x8f628d51, 43, 0, 0},
  {0x8f726dbc, 156, 0, 0},
  {0x8f7719f3, 100, 0, 0},
  {0x8f81d008, 38, 0, 0},
  {0x8fa95456, 99, 0, 0},
  {0x8fb0cef1, 103, 1, 894},
  {0x8fdb8f62, 179, 0, 0},
  {0x900a55db, 117, 0, 0},
  {0x900c7442, 100, 0, 0},
  {0x900cf570, 126, 0, 0},
  {0x900e3a23, 147, 0, 0},
  {0x90226e40, 13, 1, 692},
  {0x902e3168, 165, 1, 891},
  {0x90388d1b, 99, 1, 1089},
  {0x90491713, 165, 1, 787},
  {0x90597545, 133, 0, 0},
  {0x9077a623, 126, 1, 593},
  {0x908505ee, 156, 0, 0},
  {0x90a33263, 34, 1, 1289},
  {0x90c3f886, 24, 0, 0},
  {0x90c773c1, 92, 1, 1192},
  {0x90ca616d, 126, 1, 1085},
  {0x90d3210a, 33, 1, 0},
  {0x90d68a43, 3, 1, 990},
  {0x90ecdade, 165, 0, 0},
  {0x90f12ac8, 0, 1, 0},
  {0x90f6fa33, 100, 0, 0},
  {0x90faa618, 117, 0, 0},
  {0x910cc30f, 117, 0, 0},
  {0x91209041, 92, 1, 291},
  {0x912b790e, 175, 0, 0},
  {0x912d82c5, 117, 0, 0},
  {0x91328c1d, 100, 0, 0},
  {0x9152ce50, 100, 0, 0},
  {0x91568f01, 92, 0, 0},
  {0x91585c4c, 117, 0, 0},
  {0x915a53a7, 100, 0, 0},
  {0x9160a87a, 174, 0, 0},
  {0x916913d2, 153, 1, 292},
  {0x917770d8, 100, 1, 1091},
  {0x917d9262, 133, 0, 0},
  {0x9183054e, 193, 0, 0},
  {0x918c1b71, 20, 0, 0},
  {0x9198279e, 114, 2, 0},
  {0x919ac0fe, 100, 0, 0},
  {0x91a140c9, 120, 1, 691},
  {0x91aa57f1, 74, 1, 393},
  {0x91ac514e, 117, 0, 0},
  {0x91b4b1d7, 126, 2, 0},
  {0x91c6ee43, 156, 1, 290},
  {0x91d1e85b, 97, 1, 1290},
  {0x91d24d7b, 50, 1, 690},
  {0x91d33e3c, 126, 1, 1085},
  {0x91d52e9a, 92, 0, 0},
  {0x91e2e863, 163, 1, 291},
  {0x91e4a289, 0, 1, 0},
  {0x9204a65d, 100, 0, 0},
  {0x92197173, 43, 1, 1289},
  {0x9227180c, 117, 0, 0},
  {0x923f915b, 202, 0, 0},
  {0x9247c38d, 126, 2, 0},
  {0x92547f1c, 195, 0, 0},
  {0x926f07af, 157, 0, 0},
  {0x9273f18e, 72, 0, 0},
  {0x927aba9d, 126, 0, 0},
  {0x927c7a3a, 69, 0, 0},
  {0x928361d4, 92, 1, 1088},
  {0x92861600, 104, 1, 289},
  {0x92873f0e, 165, 0, 0},
  {0x929c7b2f, 133, 0, 0},
  {0x92a2185c, 126, 1, 0},
  {0x92a2a702, 34, 0, 0},
  {0x92a3d007, 15, 1, 0},
  {0x92b07fd9, 202, 0, 0},
  {0x92bc8c2e, 24, 0, 0},
  {0x92c138e4, 152, 1, 0},
  {0x92d0a3e8, 124, 0, 0},
  {0x92d5e3dc, 193, 0, 0},
  {0x92edac5e, 92, 0, 0},
  {0x92f04530, 100, 0, 0},
  {0x93146bb0, 117, 0, 0},
  {0x931472d0, 92, 0, 0},
  {0x9316bf4b, 0, 1, 0},
  {0x932a077a, 100, 0, 0},
  {0x932c11c1, 24, 0, 0},
  {0x932ff06e, 65, 1, 990},
  {0x93368b81, 140, 0, 0},
  {0x934db14a, 193, 1, 1289},
  {0x9357a157, 40, 1, 89},
  {0x9369a2f8, 34, 2, 0},
  {0x936e229d, 46, 0, 0},
  {0x93794634, 100, 0, 0},
  {0x937aba88, 68, 1, 390},
  {0x9381a81d, 156, 1, 1288},
  {0x93991433, 163, 1, 990},
  {0x93a7d26c, 124, 0, 0},
  {0x93b49582, 160, 1, 390},
  {0x93b9b15c, 92, 0, 0},
  {0x93dc3c82, 97, 0, 0},
  {0x93f3a490, 166, 1, 0},
  {0x941ba912, 156, 0, 0},
  {0x942b1210, 12, 1, 690},
  {0x942f6bb0, 44, 0, 0},
  {0x943dfbbe, 126, 1, 1189},
  {0x9445a4ea, 0, 1, 0},
  {0x9474c09c, 126, 1, 692},
  {0x947f0728, 104, 1, 288},
  {0x948ad7f6, 52, 2, 0},
  {0x948e0bd6, 89, 0, 0},
  {0x94ccbbbc, 126, 1, 789},
  {0x9509f703, 46, 0, 0},
  {0x950aa028, 34, 1, 692},
  {0x95192148, 89, 0, 0},
  {0x951c12ae, 59, 0, 0},
  {0x9525fa38, 39, 0, 0},
  {0x952a9e77, 16, 0, 0},
  {0x953a3eaf, 156, 1, 989},
  {0x953ca1b6, 100, 0, 0},
  {0x953ffe4c, 48, 0, 0},
  {0x9552e8df, 24, 0, 0},
  {0x9561798d, 24, 0, 0},
  {0x95649977, 97, 1, 493},
  {0x957f3d28, 100, 0, 0},
  {0x958e4bae, 74, 1, 1090},
  {0x95aaed34, 160, 0, 0},
  {0x95af291f, 92, 1, 392},
  {0x95ba5733, 99, 0, 0},
  {0x95c04df9, 38, 0, 0},
  {0x95ca9ec7, 100, 1, 990},
  {0x95d6d531, 57, 1, 1088},
  {0x95dbb274, 160, 0, 0},
  {0x95e4e594, 160, 1, 191},
  {0x95e71ea9, 140, 0, 0},
  {0x96087988, 127, 1, 892},
  {0x9615209f, 34, 0, 0},
  {0x9622fbd9, 133, 0, 0},
  {0x96277a43, 93, 0, 0},
  {0x96533999, 117, 0, 0},
  {0x96546242, 34, 0, 0},
  {0x965b9a2f, 0, 1, 0},
  {0x9668488e, 158, 1, 1092},
  {0x96773f32, 117, 0, 0},
  {0x96788826, 158, 1, 1091},
  {0x967a605f, 126, 2, 0},
  {0x9684657f, 77, 0, 0},
  {0x968dcf09, 117, 0, 0},
  {0x9694bfb8, 115, 1, 993},
  {0x969ef9e4, 4, 1, 987},
  {0x96ba90b0, 173, 0, 0},
  {0x96c3e953, 156, 0, 0},
  {0x96c4ce38, 126, 1, 1289},
  {0x96cfb4d8, 114, 2, 0},
  {0x96d3f955, 188, 1, 990},
  {0x96dfc776, 166, 1, 0},
  {0x96f6051a, 160, 1, 1092},
  {0x96fcdc5f, 182, 1, 390},
  {0x97217057, 115, 1, 1290},
  {0x972d08c5, 144, 0, 0},
  {0x9731a9a3, 160, 1, 390},
  {0x9735d267, 126, 2, 0},
  {0x9747ac09, 131, 1, 591},
  {0x974d0745, 178, 0, 0},
  {0x974e8840, 2, 0, 0},
  {0x975b25ee, 43, 0, 0},
  {0x975ccfeb, 34, 1, 1186},
  {0x9768e920, 93, 0, 0},
  {0x977ee848, 166, 1, 0},
  {0x9786d132, 127, 1, 192},
  {0x979c5314, 6, 1, 1187},
  {0x97bc4585, 120, 0, 0},
  {0x97cad370, 126, 0, 0},
  {0x97d9f9b4, 62, 0, 0},
  {0x98011231, 0, 1, 0},
  {0x9806cb84, 125, 0, 0},
  {0x98087e4d, 117, 0, 0},
  {0x9808abb8, 166, 1, 0},
  {0x980be936, 100, 0, 0},
  {0x982189a2, 24, 1, 988},
  {0x982dfb38, 74, 1, 393},
  {0x9832d15a, 160, 0, 0},
  {0x983948a5, 104, 1, 1187},
  {0x985b1d05, 100, 0, 0},
  {0x986bb2d4, 46, 1, 188},
  {0x987dcda3, 24, 1, 689},
  {0x9881d1b4, 0, 1, 0},
  {0x988798a8, 34, 1, 394},
  {0x988b446d, 57, 1, 1087},
  {0x9894f766, 46, 0, 0},
  {0x98977591, 120, 0, 0},
  {0x98a8f5be, 104, 1, 591},
  {0x98a97a59, 39, 0, 0},
  {0x98aa7cab, 54, 0, 0},
  {0x98bb90d9, 4, 1, 790},
  {0x98c8e090, 99, 0, 0},
  {0x98ccc9ab, 156, 0, 0},
  {0x98ccd385, 163, 2, 0},
  {0x98dc1099, 179, 0, 0},
  {0x98e3c75a, 126, 0, 0},
  {0x98fca2b9, 24, 0, 0},
  {0x99004b45, 175, 0, 0},
  {0x99083b3a, 200, 1, 0},
  {0x9928ee82, 104, 1, 1190},
  {0x99344acd, 46, 1, 1288},
  {0x99580334, 100, 0, 0},
  {0x99686dad, 34, 0, 0},
  {0x9992f445, 21, 0, 0},
  {0x999577b6, 100, 1, 1187},
  {0x99a62e47, 76, 0, 0},
  {0x99a9f57e, 126, 1, 1188},
  {0x99b73746, 77, 0, 0},
  {0x99c395f9, 160, 0, 0},
  {0x99c88648, 26, 2, 0}, // "Proto"
  {0x99cc199b, 174, 0, 0},
  {0x99d27118, 0, 1, 0},
  {0x99d38676, 160, 0, 0},
  {0x99dddb04, 100, 1, 1291},
  {0x9a172152, 77, 0, 0},
  {0x9a273fc0, 0, 1, 0},
  {0x9a2b0641, 117, 0, 0},
  {0x9a2db086, 126, 2, 0},
  {0x9a35edfe, 37, 0, 0},
  {0x9a60bf47, 48, 0, 0},
  {0x9a808c3b, 99, 0, 0},
  {0x9a851990, 117, 0, 0},
  {0x9aac6754, 185, 0, 0},
  {0x9aacd75d, 179, 0, 0},
  {0x9ab274ae, 5, 1, 0},
  {0x9acded0e, 57, 1, 492},
  {0x9ad945eb, 126, 0, 0},
  {0x9adb2af7, 34, 0, 0},
  {0x9add521e, 164, 0, 0},
  {0x9adfc8f0, 16, 0, 0},
  {0x9ae5ae08, 117, 0, 0},
  {0x9b1bdbbe, 99, 1, 1290},
  {0x9b208ab1, 23, 0, 0},
  {0x9b323e20, 126, 0, 0},
  {0x9b37d149, 167, 1, 89},
  {0x9b38d9e8, 122, 0, 0},
  {0x9b3c5124, 39, 0, 0},
  {0x9b49657d, 0, 1, 0},
  {0x9b4f0405, 156, 0, 0},
  {0x9b506a48, 126, 1, 1085},
  {0x9b565541, 93, 0, 0},
  {0x9b6d2cb5, 77, 0, 0},
  {0x9b74e080, 34, 0, 0},
  {0x9b821a83, 57, 1, 1191},
  {0x9b88a185, 195, 0, 0},
  {0x9b8e02c0, 200, 1, 0},
  {0x9ba20dad, 57, 1, 1087},
  {0x9ba777e1, 51, 1, 1190},
  {0x9bac73ef, 97, 1, 691},
  {0x9bdcd892, 63, 0, 0},
  {0x9bde3267, 77, 1, 989},
  {0x9beed7a8, 66, 0, 0},
  {0x9bf99c3a, 50, 1, 1191},
  {0x9c015583, 155, 0, 0},
  {0x9c053f24, 72, 0, 0},
  {0x9c18762b, 99, 1, 1191},
  {0x9c521240, 165, 0, 0},
  {0x9c537919, 126, 1, 1093},
  {0x9c58f4a6, 77, 0, 0},
  {0x9c5e2b65, 171, 0, 0},
  {0x9c7e6421, 126, 1, 1085},
  {0x9c9f3571, 100, 1, 487},
  {0x9ca42697, 179, 0, 0},
  {0x9caaba9e, 199, 0, 0},
  {0x9cb55b96, 100, 1, 1291},
  {0x9cbadc25, 53, 0, 0},
  {0x9cbb0291, 9, 0, 0},
  {0x9cbc8253, 117, 0, 0},
  {0x9cf17fab, 195, 0, 0},
  {0x9d1deab8, 0, 1, 0},
  {0x9d21fe96, 117, 0, 0},
  {0x9d38f8f9, 166, 1, 0},
  {0x9d45d8ec, 72, 0, 0},
  {0x9da252b4, 100, 0, 85},
  {0x9db51b1f, 95, 0, 0},
  {0x9db6a3ed, 26, 2, 0}, // "Proto"
  {0x9dc96ec7, 92, 0, 0},
  {0x9def17bb, 160, 0, 0},
  {0x9df89be5, 160, 1, 190},
  {0x9e0bf355, 100, 0, 0},
  {0x9e0e1dc8, 160, 0, 0},
  {0x9e1ce13d, 21, 0, 0},
  {0x9e356267, 147, 0, 0},
  {0x9e379698, 33, 1, 0},
  {0x9e382ebf, 126, 1, 389},
  {0x9e4e9cc2, 117, 1, 1193},
  {0x9e6092a4, 100, 1, 1292},
  {0x9e777ea5, 102, 0, 0},
  {0x9ea1dc76, 160, 1, 691},
  {0x9eab6b1f, 100, 0, 0},
  {0x9ebdc94e, 89, 0, 0},
  {0x9eca0941, 156, 1, 1288},
  {0x9ecb9dcd, 176, 0, 0},
  {0x9ed99198, 126, 1, 1088},
  {0x9edd2159, 182, 1, 1292},
  {0x9ee83916, 92, 0, 0},
  {0x9eef47aa, 143, 1, 690},
  {0x9eff96d2, 156, 0, 0},
  {0x9f2eef20, 172, 1, 992},
  {0x9f3da143, 117, 0, 0},
  {0x9f432594, 77, 1, 690},
  {0x9f50a100, 92, 0, 0},
  {0x9f5138cb, 159, 0, 0},
  {0x9f6c119c, 74, 1, 590},
  {0x9f6ce171, 12, 1, 990},
  {0x9f75b83b, 126, 1, 490},
  {0x9f8336db, 98, 0, 0},
  {0x9f85fa70, 161, 0, 0},
  {0x9f8bd88d, 38, 0, 0},
  {0x9fa1c11f, 2, 0, 0},
  {0x9fae4d46, 34, 0, 0},
  {0x9fb32923, 173, 1, 292},
  {0x9fd0f213, 24, 1, 289},
  {0x9fd718fd, 201, 0, 0},
  {0x9fe0aef5, 126, 1, 0},
  {0x9ffe2f55, 160, 1, 989},
  {0xa01ef87e, 115, 1, 93},
  {0xa025344d, 92, 1, 493},
  {0xa029dfe9, 38, 0, 0},
  {0xa02b3a39, 102, 0, 0},
  {0xa03a422b, 104, 1, 488},
  {0xa03e89d9, 0, 1, 0},
  {0xa045fe1d, 33, 2, 0},
  {0xa0568e1d, 160, 1, 188},
  {0xa06c820c, 65, 1, 1091},
  {0xa085a697, 0, 1, 0},
  {0xa098ebd2, 177, 0, 0},
  {0xa0a095c4, 193, 1, 1288},
  {0xa0a5a0b9, 56, 2, 0},
  {0xa0a6e860, 100, 0, 0},
  {0xa0b0b742, 126, 1, 290},
  {0xa0b8d310, 176, 0, 0},
  {0xa0c31a57, 115, 1, 1288},
  {0xa0c9bf7e, 160, 1, 1292},
  {0xa0cad20f, 160, 0, 0},
  {0xa0dabe0a, 126, 1, 84},
  {0xa0dad6e4, 130, 1, 0},
  {0xa0ddf884, 77, 0, 0},
  {0xa0df4b8f, 4, 1, 992},
  {0xa0eca0f9, 0, 1, 0},
  {0xa0ed7d20, 126, 1, 290},
  {0xa0f56a29, 117, 0, 0},
  {0xa0f99bb8, 100, 0, 0},
  {0xa11fac93, 92, 0, 0},
  {0xa139009c, 13, 1, 492},
  {0xa1431bbd, 93, 0, 0},
  {0xa166548f, 150, 1, 192},
  {0xa16ab939, 195, 0, 0},
  {0xa186f064, 116, 1, 190},
  {0xa1a0c13f, 182, 2, 0},
  {0xa1a33b85, 2, 0, 0},
  {0xa1b67ac5, 34, 0, 0},
  {0xa1ff4e1d, 158, 1, 1292},
  {0xa2194cad, 34, 1, 788},
  {0xa21e675c, 89, 0, 0},
  {0xa22657fa, 126, 1, 1290},
  {0xa232e8be, 143, 1, 0}, // "Proto"
  {0xa25a750f, 57, 1, 1290},
  {0xa25ed91d, 66, 0, 0},
  {0xa262a81f, 24, 0, 0},
  {0xa2a6ee58, 165, 1, 0},
  {0xa2af25d0, 68, 1, 788},
  {0xa2bac574, 153, 1, 492},
  {0xa2c0cab7, 175, 0, 0},
  {0xa2c79ec5, 124, 0, 0},
  {0xa2c89cb9, 0, 1, 0},
  {0xa2c94c04, 178, 0, 0},
  {0xa2d6ee34, 24, 0, 0},
  {0xa2d79603, 117, 0, 0},
  {0xa2da133b, 6, 1, 990},
  {0xa2e4534f, 115, 1, 190},
  {0xa2e68da8, 100, 0, 0},
  {0xa2ee1350, 95, 0, 0},
  {0xa2f826f1, 182, 1, 390},
  {0xa3047263, 24, 0, 0},
  {0xa308a153, 100, 0, 0},
  {0xa30d8baf, 54, 0, 0},
  {0xa31142ff, 6, 2, 0},
  {0xa3414faa, 117, 0, 0},
  {0xa342a5fd, 4, 1, 588},
  {0xa37b0ee3, 46, 1, 188},
  {0xa38857eb, 156, 0, 0},
  {0xa38fe9ce, 168, 0, 0},
  {0xa397028a, 15, 1, 0},
  {0xa3a6184c, 21, 0, 0},
  {0xa3ad445d, 34, 1, 1288},
  {0xa3d59f62, 43, 0, 0},
  {0xa3da8777, 95, 0, 0},
  {0xa3e082a6, 117, 0, 0},
  {0xa3e37134, 126, 0, 0},
  {0xa3ea8dba, 126, 1, 1088},
  {0xa4062017, 57, 1, 289},
  {0xa45bcda0, 121, 0, 0},
  {0xa461436a, 126, 0, 0},
  {0xa4625dbc, 57, 1, 1088},
  {0xa46aa597, 150, 0, 0},
  {0xa479b08c, 97, 1, 1290},
  {0xa485abed, 39, 0, 0},
  {0xa49253c6, 117, 0, 0},
  {0xa49b48b8, 53, 0, 0},
  {0xa4a4f4bf, 46, 0, 0},
  {0xa4b947ca, 126, 1, 0},
  {0xa4bdcc1d, 0, 2, 0},
  {0xa4cb97df, 126, 1, 0},
  {0xa4dcdf28, 133, 0, 0},
  {0xa4e935df, 156, 0, 0},
  {0xa4ee65ac, 24, 0, 0},
  {0xa505d342, 117, 0, 0},
  {0xa50c51fb, 117, 0, 0},
  {0xa520f44c, 166, 1, 0},
  {0xa524ae9b, 106, 0, 0},
  {0xa52a8119, 24, 0, 0},
  {0xa52b28ed, 105, 0, 87},
  {0xa5313248, 24, 1, 689},
  {0xa547a6ec, 77, 0, 0},
  {0xa54d9086, 92, 0, 0},
  {0xa55701dd, 200, 1, 0},
  {0xa558fb52, 34, 1, 990},
  {0xa55fa397, 104, 1, 989},
  {0xa5636767, 77, 1, 194},
  {0xa56a1bd0, 72, 0, 0},
  {0xa5781280, 133, 0, 0},
  {0xa58a8da1, 202, 0, 0},
  {0xa59ca2ef, 24, 0, 0},
  {0xa5a19af5, 122, 0, 0},
  {0xa5b62a4c, 164, 0, 0},
  {0xa5d05d45, 90, 0, 0},
  {0xa5dfa8ce, 68, 1, 1088},
  {0xa5e6baf9, 117, 0, 0},
  {0xa5e89675, 40, 1, 0},
  {0xa5e8d2cd, 46, 1, 1187},
  {0xa60ca3d6, 188, 1, 192},
  {0xa60fba51, 108, 0, 0},
  {0xa65d3207, 79, 0, 0},
  {0xa6648353, 193, 0, 0},
  {0xa66596d9, 164, 0, 0},
  {0xa67ea466, 104, 1, 393},
  {0xa6819195, 133, 0, 0},
  {0xa695b076, 40, 1, 0},
  {0xa69f29fa, 156, 1, 1192},
  {0xa6a2ec56, 42, 0, 0},
  {0xa6a725b8, 50, 1, 1292},
  {0xa6b3f7b3, 156, 0, 0},
  {0xa6de7024, 160, 0, 0},
  {0xa6e16202, 124, 0, 0},
  {0xa6f5cb3c, 68, 1, 290},
  {0xa70b2813, 92, 0, 0},
  {0xa71c3452, 160, 0, 0},
  {0xa725b2d3, 91, 1, 792},
  {0xa72fde03, 92, 0, 0},
  {0xa7481c4b, 22, 0, 0},
  {0xa7b0536c, 160, 0, 0},
  {0xa7b0bc87, 143, 1, 989},
  {0xa7d3635e, 164, 0, 0},
  {0xa7de65e4, 6, 1, 489},
  {0xa7e784ed, 72, 0, 0},
  {0xa80a0f01, 104, 1, 894},
  {0xa8104fb2, 24, 2, 0},
  {0xa8138860, 65, 1, 989},
  {0xa817d175, 93, 0, 0},
  {0xa81c51ea, 92, 0, 0},
  {0xa848a2b1, 43, 1, 1289},
  {0xa86a5318, 53, 1, 392},
  {0xa86af976, 4, 1, 1292},
  {0xa86e3704, 100, 0, 0},
  {0xa875c5e9, 93, 0, 0},
  {0xa8784932, 34, 1, 288},
  {0xa8923256, 97, 1, 490},
  {0xa895d40a, 77, 0, 0},
  {0xa8a4a48b, 200, 1, 0},
  {0xa8a9b982, 180, 0, 0},
  {0xa8aa7994, 12, 1, 690},
  {0xa8b0cfce, 89, 1, 990},
  {0xa8b0da56, 3, 1, 592},
  {0xa8efac13, 0, 1, 0},
  {0xa8f5c2ab, 166, 1, 0},
  {0xa9004a88, 142, 1, 1290},
  {0xa905cc12, 0, 1, 0},
  {0xa9065101, 124, 0, 0},
  {0xa913a222, 77, 1, 189},
  {0xa91460b8, 54, 0, 0},
  {0xa9217ea2, 188, 1, 1290},
  {0xa922075e, 66, 0, 0},
  {0xa93527e2, 100, 2, 0},
  {0xa941eefa, 117, 0, 0},
  {0xa94591b0, 100, 1, 992},
  {0xa94686cb, 200, 1, 0},
  {0xa9541452, 24, 0, 0},
  {0xa97567a4, 84, 2, 0},
  {0xa98046b8, 126, 0, 0},
  {0xa9a4ea4c, 143, 0, 0},
  {0xa9bbf44f, 166, 1, 0},
  {0xa9c2c503, 100, 1, 88},
  {0xa9e2bf31, 0, 1, 0},
  {0xa9e30826, 100, 0, 0},
  {0xa9e70766, 41, 0, 0},
  {0xa9f0b6f3, 46, 0, 0},
  {0xaa14431a, 0, 1, 0},
  {0xaa174bc6, 4, 1, 689},
  {0xaa20f73d, 92, 1, 1291},
  {0xaa4318ae, 39, 0, 0},
  {0xaa6bb985, 193, 1, 390},
  {0xaa6e1a35, 92, 0, 0},
  {0xaa755715, 165, 0, 0},
  {0xaa96889c, 100, 0, 0},
  {0xaa97d0a0, 117, 0, 0},
  {0xaa9ca482, 0, 1, 0},
  {0xaaa67035, 92, 0, 0},
  {0xaaaa17bd, 68, 1, 290},
  {0xaad7bdc1, 156, 0, 0},
  {0xaaed295c, 126, 1, 288},
  {0xaaf2ce64, 24, 0, 0},
  {0xaafe699c, 165, 0, 0},
  {0xaafed9b4, 164, 0, 0},
  {0xab07c7f3, 155, 0, 0},
  {0xab12ece6, 0, 1, 0},
  {0xab1f0f44, 0, 1, 0},
  {0xab3062cf, 24, 0, 0},
  {0xab41445e, 156, 1, 292},
  {0xab459d2f, 97, 1, 1290},
  {0xab4ac985, 20, 0, 0},
  {0xab547071, 92, 2, 0},
  {0xab6b4794, 142, 1, 491},
  {0xab8371f6, 188, 1, 190},
  {0xab90e397, 155, 1, 690},
  {0xababffdb, 74, 1, 1090},
  {0xabe1a0c2, 126, 0, 0},
  {0xabef6b83, 100, 0, 0},
  {0xac05ebb7, 34, 0, 0},
  {0xac0d2d09, 46, 1, 290},
  {0xac136f2d, 155, 0, 0},
  {0xac273c14, 0, 1, 0},
  {0xac4bf9dc, 100, 0, 0},
  {0xac4f4e9f, 24, 0, 0},
  {0xac559fbd, 40, 1, 0},
  {0xac609320, 160, 2, 0},
  {0xac652b47, 100, 0, 0},
  {0xac75f8cd, 117, 0, 0},
  {0xac7a54cc, 34, 2, 0},
  {0xac8dcdea, 4, 1, 1289},
  {0xac9817e3, 4, 1, 291},
  {0xac9895cc, 100, 0, 0},
  {0xac98cd70, 100, 0, 0},
  {0xaca15643, 99, 1, 1191},
  {0xaca687c2, 165, 0, 0},
  {0xacd3e768, 20, 0, 0},
  {0xacde60c0, 127, 1, 192},
  {0xace56f39, 117, 0, 0},
  {0xacf912df, 132, 0, 0},
  {0xad12a34f, 165, 1, 189},
  {0xad16f6c7, 0, 1, 0},
  {0xad28aef6, 156, 0, 0},
  {0xad3df455, 24, 0, 0},
  {0xad50e497, 3, 1, 190},
  {0xad5f0653, 175, 0, 0},
  {0xad6cdf29, 126, 1, 1085},
  {0xad6e96f1, 185, 0, 0},
  {0xad7b97bc, 143, 0, 0},
  {0xad7f9480, 156, 1, 987},
  {0xada1b12f, 130, 1, 0},
  {0xada80d95, 140, 0, 0},
  {0xadb47286, 89, 0, 0},
  {0xadb5d0b3, 77, 0, 0},
  {0xadba7064, 156, 0, 0},
  {0xadbd4e48, 34, 1, 9},
  {0xadd6374f, 156, 0, 0},
  {0xade11141, 34, 0, 0},
  {0xadf606f6, 160, 0, 0},
  {0xadffd64f, 117, 0, 0},
  {0xae128fac, 133, 0, 0},
  {0xae19d06e, 165, 1, 787},
  {0xae280e20, 72, 0, 0},
  {0xae286904, 100, 1, 192},
  {0xae321339, 117, 0, 0},
  {0xae52dece, 68, 1, 1088},
  {0xae56518e, 160, 0, 0},
  {0xae64ca77, 126, 1, 388},
  {0xae6d99c7, 126, 1, 387},
  {0xae8666b4, 92, 1, 588},
  {0xae97627c, 97, 1, 0}, // "Proto"
  {0xaeb7fce9, 160, 0, 0},
  {0xaebd6549, 160, 0, 0},
  {0xaed1e6a4, 126, 0, 0},
  {0xaef78148, 194, 0, 86},
  {0xaf05f37e, 4, 1, 1292},
  {0xaf15338f, 117, 0, 0},
  {0xaf16ee39, 202, 0, 0},
  {0xaf2bb895, 77, 2, 0},
  {0xaf2bbcbc, 126, 1, 1085},
  {0xaf31a310, 171, 0, 0},
  {0xaf3ec4b1, 160, 0, 0},
  {0xaf4010ea, 126, 1, 888},
  {0xaf432446, 0, 1, 0},
  {0xaf4b5c8a, 127, 1, 892},
  {0xaf5676de, 29, 1, 987},
  {0xaf5ad5af, 147, 0, 0},
  {0xaf65aa84, 163, 2, 0},
  {0xaf6e8571, 117, 0, 0},
  {0xaf754426, 202, 0, 0},
  {0xaf85b53e, 169, 0, 0},
  {0xafb40372, 100, 0, 0},
  {0xafb46dd6, 12, 1, 789},
  {0xafc65de3, 190, 0, 0},
  {0xafdcbd24, 126, 1, 1085},
  {0xafeafeaa, 0, 1, 0},
  {0xb00abe1c, 77, 0, 0},
  {0xb00b4eb8, 91, 1, 1191},
  {0xb00b84d3, 143, 0, 0},
  {0xb0480ae9, 100, 1, 691},
  {0xb049a8c4, 24, 0, 0},
  {0xb04ba659, 76, 0, 0},
  {0xb051c0e1, 4, 1, 692},
  {0xb06c0674, 100, 0, 0},
  {0xb092dd8e, 126, 1, 1085},
  {0xb092fa3e, 92, 0, 0},
  {0xb097f651, 117, 0, 0},
  {0xb09e38f9, 126, 1, 790},
  {0xb0cd000f, 158, 1, 1193},
  {0xb0ebf3db, 196, 1, 292},
  {0xb1031788, 0, 1, 0},
  {0xb10429aa, 97, 1, 889},
  {0xb1250d0c, 100, 0, 0},
  {0xb134d713, 114, 1, 390},
  {0xb1378c99, 160, 1, 1188},
  {0xb13f00d4, 100, 2, 0},
  {0xb1494b9e, 191, 0, 0},
  {0xb14e668e, 37, 0, 0},
  {0xb14ea4d2, 143, 1, 788},
  {0xb1612fe6, 77, 1, 490},
  {0xb174b680, 117, 0, 0}, // Dig Dug (J)
  {0xb17574f3, 0, 1, 0},
  {0xb184060a, 47, 0, 0},
  {0xb1849d4e, 69, 0, 0},
  {0xb19a55dd, 166, 1, 0},
  {0xb19c48a5, 166, 1, 0},
  {0xb1a94b82, 24, 0, 0},
  {0xb1b16b8a, 21, 0, 0},
  {0xb1b9e187, 117, 0, 0},
  {0xb1c4c508, 126, 0, 0},
  {0xb1e84e5b, 38, 0, 0},
  {0xb1f7e3e9, 0, 1, 0},
  {0xb20001f4, 100, 0, 0},
  {0xb201b522, 100, 0, 0},
  {0xb20f87de, 160, 1, 1092},
  {0xb22c41cd, 100, 0, 0},
  {0xb24abc55, 100, 0, 0},
  {0xb252a5b9, 117, 0, 0},
  {0xb258d6ca, 97, 1, 691},
  {0xb27b8cf4, 100, 0, 0},
  {0xb297b5e7, 92, 0, 0},
  {0xb2ab361e, 192, 0, 0},
  {0xb2c0d62c, 115, 1, 1288},
  {0xb2fa62fd, 156, 1, 989},
  {0xb301b0dc, 34, 0, 0},
  {0xb30f6865, 117, 0, 0},
  {0xb330ed11, 188, 1, 291},
  {0xb33bc971, 65, 1, 391},
  {0xb3440357, 143, 1, 790},
  {0xb34ed396, 0, 1, 0},
  {0xb3598cd0, 179, 0, 0},
  {0xb36457c7, 97, 0, 0},
  {0xb3769a51, 156, 1, 1288},
  {0xb3783f2a, 165, 1, 787},
  {0xb37f48cd, 23, 0, 0},
  {0xb382aea4, 77, 0, 0},
  {0xb399bee4, 128, 0, 0},
  {0xb39bab34, 133, 0, 0},
  {0xb3c30bea, 117, 0, 0},
  {0xb3ca8bc5, 117, 0, 0},
  {0xb3cb3da9, 174, 0, 0},
  {0xb3d1178e, 166, 1, 0},
  {0xb3d74c0d, 126, 1, 686},
  {0xb3dbc08f, 117, 0, 0},
  {0xb3e2f2ef, 15, 1, 0},
  {0xb3fe1017, 32, 0, 0},
  {0xb40457d5, 34, 0, 0},
  {0xb40870a2, 156, 2, 0},
  {0xb41bbadf, 122, 0, 0},
  {0xb422a67a, 166, 1, 0},
  {0xb42a57c7, 124, 0, 0},
  {0xb4481476, 160, 0, 0},
  {0xb462718e, 33, 1, 0},
  {0xb4735fac, 69, 0, 0},
  {0xb47569e2, 97, 0, 0},
  {0xb4844ca5, 173, 1, 292},
  {0xb4adede3, 117, 0, 0},
  {0xb4d554d6, 46, 1, 390},
  {0xb4e4879e, 34, 1, 1186},
  {0xb4ff91e7, 181, 0, 0},
  {0xb515e7d4, 24, 0, 0},
  {0xb548fd2e, 161, 0, 0},
  {0xb54db5ef, 191, 0, 0},
  {0xb5576820, 77, 0, 0},
  {0xb55d5747, 88, 1, 90},
  {0xb55da544, 164, 0, 0},
  {0xb56958d1, 99, 1, 491},
  {0xb5897e94, 179, 0, 0},
  {0xb58d962b, 195, 0, 0},
  {0xb5902c20, 0, 1, 0},
  {0xb5976b7c, 179, 0, 0},
  {0xb59a7a29, 89, 0, 0},
  {0xb5af14ca, 176, 0, 0},
  {0xb5be4935, 104, 1, 0}, // "Proto"
  {0xb5d28ea2, 163, 1, 489},
  {0xb5e24324, 144, 0, 0},
  {0xb5e38091, 100, 1, 992},
  {0xb5e392e2, 160, 1, 1192},
  {0xb5f7e661, 46, 0, 0},
  {0xb5ff71ab, 117, 0, 0},
  {0xb627254b, 160, 1, 490},
  {0xb629d555, 92, 1, 391}, // Totally Rad (U)
  {0xb62a7b71, 117, 0, 0},
  {0xb64ab5e2, 92, 0, 0},
  {0xb66471cb, 0, 1, 0},
  {0xb6661bda, 150, 1, 690},
  {0xb668c7fc, 100, 1, 587},
  {0xb6758ee6, 74, 1, 292},
  {0xb69831ca, 45, 0, 0},
  {0xb6bf5137, 156, 1, 989},
  {0xb6ca8a0e, 95, 0, 0},
  {0xb6d2d300, 117, 1, 0},
  {0xb6dc9aa7, 55, 0, 0},
  {0xb6f94005, 160, 0, 0},
  {0xb70129f4, 16, 0, 0},
  {0xb775c9c1, 168, 0, 0},
  {0xb7773a07, 144, 0, 0},
  {0xb780521c, 165, 1, 590},
  {0xb786ab95, 180, 0, 0},
  {0xb788ebf8, 40, 1, 0},
  {0xb78970d2, 117, 0, 0},
  {0xb7925da2, 34, 1, 1186},
  {0xb79c320d, 34, 2, 0},
  {0xb7a338ca, 100, 0, 0},
  {0xb7b6800c, 117, 0, 0},
  {0xb7c0dae7, 100, 0, 0},
  {0xb7f28915, 24, 0, 0},
  {0xb7f485d8, 126, 1, 887},
  {0xb80192b7, 186, 2, 0},
  {0xb807446e, 30, 0, 0},
  {0xb811c054, 2, 0, 0},
  {0xb843eb84, 188, 1, 990},
  {0xb859ab5f, 24, 1, 492},
  {0xb8747abf, 2, 0, 0},
  {0xb89764a2, 92, 0, 0},
  {0xb89888c9, 33, 1, 0},
  {0xb8af9340, 37, 0, 0},
  {0xb8b2e479, 97, 0, 0},
  {0xb8bb48d3, 160, 1, 887},
  {0xb8d93cf2, 100, 0, 0},
  {0xb8f3781d, 161, 0, 0},
  {0xb90497aa, 126, 0, 0},
  {0xb90865cf, 165, 1, 690},
  {0xb918580c, 13, 1, 1291},
  {0xb938b7e9, 166, 1, 0},
  {0xb9661f04, 92, 0, 0},
  {0xb96ac600, 95, 0, 0},
  {0xb9762da8, 0, 1, 0},
  {0xb979cad5, 89, 0, 0},
  {0xb97afb85, 126, 0, 0},
  {0xb99085ce, 100, 2, 0},
  {0xb99394c3, 0, 1, 0},
  {0xb9ab06aa, 150, 0, 0},
  {0xb9b4d9e0, 126, 1, 990},
  {0xb9c20634, 117, 0, 0},
  {0xb9cf171f, 92, 1, 190},
  {0xba23e079, 174, 0, 0},
  {0xba322865, 126, 1, 0},
  {0xba428f36, 134, 0, 0},
  {0xba51ac6f, 89, 0, 0},
  {0xba6bdd6a, 100, 0, 0},
  {0xba766ec6, 46, 0, 0},
  {0xba898162, 0, 1, 0},
  {0xba9164e7, 0, 1, 0},
  {0xba93cb95, 40, 1, 0},
  {0xbab3ddb9, 100, 0, 0},
  {0xbaca10a9, 160, 0, 0},
  {0xbad36c17, 195, 0, 0},
  {0xbadafc2b, 90, 0, 0},
  {0xbb1b591b, 117, 0, 0},
  {0xbb375ed2, 24, 0, 0},
  {0xbb435255, 89, 0, 0},
  {0xbb4b6df7, 165, 0, 0},
  {0xbb4ee993, 160, 0, 0},
  {0xbb673749, 29, 1, 987},
  {0xbb6d7949, 100, 1, 292},
  {0xbb7bca36, 126, 0, 0},
  {0xbb7c5f7a, 156, 0, 0},
  {0xbb7f829a, 69, 0, 0},
  {0xbbc44e39, 126, 0, 0},
  {0xbbd8c622, 98, 0, 0},
  {0xbbe2d09d, 117, 0, 0},
  {0xbbe40dc4, 40, 1, 0},
  {0xbc065fc3, 30, 1, 990},
  {0xbc1197a4, 89, 0, 0},
  {0xbc11e61a, 117, 0, 0},
  {0xbc19f17e, 18, 0, 0},
  {0xbc1dce96, 10, 1, 0},
  {0xbc1f50b5, 0, 1, 0},
  {0xbc1fbd7b, 178, 0, 0},
  {0xbc5f6c94, 24, 1, 787},
  {0xbc7485b5, 83, 0, 0},
  {0xbc7b1d0f, 160, 0, 0},
  {0xbc7fedb9, 43, 1, 1290},
  {0xbc80fb52, 99, 0, 0},
  {0xbc8e8bc7, 160, 0, 0},
  {0xbc9bffcb, 100, 2, 0},
  {0xbcacbbf4, 89, 1, 291},
  {0xbccfef1c, 71, 2, 0},
  {0xbcd7e5ea, 66, 0, 0},
  {0xbce1da2c, 0, 1, 0},
  {0xbcf68611, 72, 0, 0},
  {0xbd0e29b3, 0, 1, 0},
  {0xbd154c3e, 33, 1, 0},
  {0xbd2269ad, 43, 0, 0},
  {0xbd29178a, 15, 1, 0},
  {0xbd2fe6a6, 143, 1, 788},
  {0xbd40d4be, 121, 0, 0},
  {0xbd523011, 117, 0, 0},
  {0xbd8a0d7d, 89, 0, 0},
  {0xbd9d0e85, 77, 0, 0},
  {0xbda8f8e4, 24, 0, 0},
  {0xbdabac0b, 42, 0, 0},
  {0xbdabc4a6, 117, 0, 0},
  {0xbdad07e0, 24, 0, 0},
  {0xbdad5548, 117, 0, 0},
  {0xbdbae3e0, 117, 0, 0},
  {0xbdc124e5, 25, 0, 0},
  {0xbdc65c91, 16, 0, 0},
  {0xbdd176af, 0, 1, 0},
  {0xbdd55ea1, 127, 1, 693},
  {0xbde10416, 180, 0, 0},
  {0xbde3ae9b, 77, 0, 0},
  {0xbde7a7b5, 15, 1, 0},
  {0xbde93999, 24, 1, 490},
  {0xbdf046ef, 114, 1, 690},
  {0xbdf1307c, 122, 0, 0},
  {0xbe00966b, 72, 0, 0},
  {0xbe06853f, 24, 0, 0},
  {0xbe1bf10c, 10, 1, 0},
  {0xbe250388, 34, 1, 892},
  {0xbe2e7055, 0, 1, 0},
  {0xbe3bf3b3, 76, 1, 690},
  {0xbe6dbb5d, 34, 1, 0},
  {0xbe7c4202, 43, 1, 889},
  {0xbe86ee6d, 15, 1, 0},
  {0xbe898565, 165, 0, 0},
  {0xbe8a744d, 92, 0, 0},
  {0xbe939fce, 126, 1, 1092},
  {0xbe95b219, 34, 0, 0},
  {0xbe9c42c9, 126, 0, 0},
  {0xbe9e3799, 117, 0, 0},
  {0xbea682e5, 191, 0, 0},
  {0xbeb15855, 104, 1, 289},
  {0xbeb30478, 92, 0, 0},
  {0xbeb88304, 126, 1, 1290},
  {0xbeb8ab01, 126, 1, 686},
  {0xbee1c0d9, 17, 1, 1190},
  {0xbee54426, 15, 1, 0},
  {0xbf250af2, 46, 1, 1086},
  {0xbf3635cf, 34, 0, 0},
  {0xbf3bb6d5, 69, 0, 0},
  {0xbf471693, 43, 0, 0},
  {0xbf4f4ba6, 77, 0, 0},
  {0xbf5e2513, 75, 0, 0},
  {0xbf7f54b4, 191, 0, 0},
  {0xbf865533, 100, 0, 0},
  {0xbf888b75, 127, 2, 0},
  {0xbf8c1116, 97, 0, 0},
  {0xbf8d7db6, 24, 0, 0},
  {0xbf93112a, 92, 0, 0},
  {0xbf968b1e, 165, 0, 0},
  {0xbf9adf43, 69, 0, 0},
  {0xbfbfd25d, 0, 1, 0},
  {0xbfc7a2e9, 24, 0, 0},
  {0xbfe0e37f, 68, 1, 191},
  {0xbffcf247, 117, 0, 0},
  {0xc0103592, 92, 2, 0},
  {0xc032e5b0, 126, 1, 1092},
  {0xc042515a, 95, 0, 0},
  {0xc05a365b, 174, 0, 0},
  {0xc05bbdfc, 148, 0, 0},
  {0xc05d2034, 188, 1, 490},
  {0xc0605945, 92, 0, 0},
  {0xc060ed0a, 192, 0, 0},
  {0xc09bf032, 160, 0, 0},
  {0xc0b23520, 188, 1, 289},
  {0xc0b2aa1f, 0, 1, 0},
  {0xc0bd6275, 92, 1, 788},
  {0xc0dd00e0, 97, 1, 1088},
  {0xc0e35dae, 156, 0, 0},
  {0xc0ededd0, 100, 2, 0},
  {0xc0ef49ac, 95, 0, 0},
  {0xc0f173c4, 161, 0, 0},
  {0xc0f73d85, 0, 1, 0},
  {0xc0fb91ac, 156, 0, 0},
  {0xc0fed437, 89, 0, 0},
  {0xc11c7da2, 115, 1, 1290},
  {0xc138cfea, 46, 0, 0},
  {0xc15a50fc, 126, 2, 0},
  {0xc16897d2, 166, 1, 0},
  {0xc17ae2dc, 143, 0, 0},
  {0xc1a9e6c0, 0, 1, 0},
  {0xc1b43207, 182, 1, 892},
  {0xc1b6b2a6, 117, 0, 0},
  {0xc1ba8bb9, 72, 0, 0},
  {0xc1c3636b, 91, 1, 1191},
  {0xc1cd15dd, 129, 2, 0},
  {0xc1cf486a, 180, 0, 0},
  {0xc1cf4948, 145, 0, 0},
  {0xc1d7ab1d, 24, 0, 0},
  {0xc1d9abbf, 24, 0, 0},
  {0xc1dc5b12, 124, 0, 0},
  {0xc1e91d3f, 97, 2, 90}, // Spy Vs Spy (E) [!]
  {0xc1fbf659, 100, 0, 0},
  {0xc2222bb1, 92, 0, 0},
  {0xc22c23ab, 39, 0, 0},
  {0xc22ff1d8, 46, 0, 0},
  {0xc237ccb0, 77, 0, 0},
  {0xc23a56da, 115, 1, 991},
  {0xc23cc4ee, 95, 0, 0},
  {0xc247a23d, 100, 1, 193},
  {0xc247cc80, 117, 0, 0},
  {0xc2730c30, 29, 1, 987},
  {0xc273751e, 58, 1, 92},
  {0xc2757acd, 126, 0, 0},
  {0xc283e72d, 126, 0, 0},
  {0xc2840372, 24, 0, 0},
  {0xc2c6827e, 57, 1, 1088},
  {0xc2cfd9d9, 77, 0, 0},
  {0xc2db7551, 24, 1, 689},
  {0xc2df0a00, 92, 0, 0},
  {0xc2ef3422, 2, 0, 0},
  {0xc30848d3, 126, 2, 0},
  {0xc30c9ec9, 60, 0, 0},
  {0xc313ef54, 100, 1, 390},
  {0xc31abe30, 146, 1, 889},
  {0xc31f5585, 126, 0, 0},
  {0xc3244352, 0, 1, 0},
  {0xc3463a3d, 158, 1, 991},
  {0xc346b2cc, 39, 0, 0},
  {0xc36abbec, 173, 0, 0},
  {0xc372399b, 117, 0, 0},
  {0xc382120c, 46, 0, 0},
  {0xc38b1aae, 126, 1, 0},
  {0xc38b62cb, 126, 1, 85},
  {0xc3a0a3e0, 57, 2, 0},
  {0xc3a1b9a8, 42, 0, 0},
  {0xc3aec9fa, 191, 2, 0},
  {0xc3ba98ee, 100, 0, 0},
  {0xc3c7a568, 4, 1, 987},
  {0xc3d9b673, 100, 1, 1288},
  {0xc3df85c3, 100, 0, 0},
  {0xc3f85d1e, 0, 1, 0},
  {0xc420552f, 20, 0, 0},
  {0xc43da8e2, 165, 1, 590},
  {0xc4482b58, 89, 0, 0},
  {0xc471e42d, 100, 1, 1288},
  {0xc47efc0e, 15, 1, 0},
  {0xc4835a34, 77, 0, 0},
  {0xc48363b4, 25, 0, 0},
  {0xc48ddb52, 54, 0, 0},
  {0xc4966242, 77, 0, 0},
  {0xc4b6ed3c, 104, 1, 891},
  {0xc4bc85a2, 100, 1, 888},
  {0xc4c3949a, 126, 1, 686},
  {0xc4cd7431, 104, 0, 0},
  {0xc4d97c46, 158, 1, 1193},
  {0xc4dcbb18, 169, 0, 0},
  {0xc4e1886f, 133, 0, 0},
  {0xc4e5abb8, 39, 0, 0},
  {0xc4e8c04a, 34, 1, 1186},
  {0xc50149db, 160, 0, 0},
  {0xc50a8304, 0, 1, 0},
  {0xc527c297, 104, 1, 393},
  {0xc529c604, 24, 0, 0},
  {0xc53cf1d0, 100, 2, 0},
  {0xc55a3393, 188, 1, 688},
  {0xc577fe32, 183, 0, 0},
  {0xc57fce87, 40, 1, 0},
  {0xc59641b6, 31, 1, 0},
  {0xc5b0b1ab, 100, 1, 987},
  {0xc5b80dce, 183, 0, 0},
  {0xc5b8efea, 77, 0, 0},
  {0xc5cfe54e, 97, 0, 0},
  {0xc5fea9f2, 25, 0, 0},
  {0xc6000085, 115, 1, 291},
  {0xc6182024, 99, 1, 1089},
  {0xc6224026, 77, 0, 0},
  {0xc62793cf, 100, 1, 1288},
  {0xc62d9630, 66, 0, 0},
  {0xc6475c2a, 46, 0, 0},
  {0xc651e21a, 0, 1, 0},
  {0xc6557e02, 117, 0, 0},
  {0xc655c077, 164, 0, 0},
  {0xc66fb098, 24, 0, 0},
  {0xc6790794, 0, 1, 0},
  {0xc68363f6, 124, 0, 0},
  {0xc6a4bf7b, 164, 0, 0},
  {0xc6a66b68, 24, 0, 0},
  {0xc6acceea, 2, 0, 0},
  {0xc6add8c5, 117, 0, 0},
  {0xc6b5d7e0, 76, 0, 0},
  {0xc6c226bf, 74, 1, 590},
  {0xc6cc7417, 54, 0, 0},
  {0xc6d62814, 100, 0, 0},
  {0xc6dd7e69, 6, 1, 1089},
  {0xc701a262, 1, 0, 0},
  {0xc708fbed, 166, 1, 0},
  {0xc70bd0f8, 165, 1, 690},
  {0xc7198f2d, 4, 2, 0},
  {0xc71c7619, 156, 1, 1188},
  {0xc71d4ce7, 100, 0, 0},
  {0xc71e77ca, 165, 0, 0},
  {0xc7347bc0, 46, 1, 188},
  {0xc73b82fc, 10, 1, 0},
  {0xc740eb46, 4, 1, 1190},
  {0xc747ac41, 156, 0, 0},
  {0xc74dd019, 156, 0, 0},
  {0xc7642467, 100, 0, 0},
  {0xc769bb34, 55, 0, 0},
  {0xc76aadf4, 174, 0, 0},
  {0xc76fc363, 92, 2, 0},
  {0xc78d17d3, 126, 1, 290},
  {0xc7908669, 0, 1, 0},
  {0xc79d050e, 100, 0, 0},
  {0xc7a79be2, 117, 0, 0},
  {0xc7bce866, 0, 1, 0},
  {0xc7c09049, 126, 0, 0},
  {0xc7c64533, 66, 0, 0},
  {0xc7ce1429, 30, 0, 0},
  {0xc7f0c457, 14, 1, 1092},
  {0xc7f5b3d8, 0, 1, 0},
  {0xc7fbecc3, 100, 1, 490},
  {0xc7fd5388, 55, 0, 0},
  {0xc811dc7a, 117, 0, 0},
  {0xc8133b04, 117, 0, 0},
  {0xc8160857, 92, 0, 0},
  {0xc8272d94, 27, 1, 89},
  {0xc85bf3e3, 0, 1, 0},
  {0xc86a1f7d, 92, 0, 0},
  {0xc8859038, 100, 0, 0},
  {0xc89385df, 153, 1, 1193},
  {0xc898e6c1, 0, 1, 0},
  {0xc8a10b71, 142, 1, 991},
  {0xc8ad4f32, 126, 1, 789},
  {0xc8edc97e, 175, 0, 0},
  {0xc9187b43, 132, 0, 0},
  {0xc92b814b, 160, 2, 0},
  {0xc94ac75f, 0, 1, 0},
  {0xc95321a8, 126, 0, 0},
  {0xc95480d0, 100, 1, 987},
  {0xc9556b36, 155, 0, 0},
  {0xc96971e2, 126, 1, 1085},
  {0xc96c6f04, 192, 0, 0},
  {0xc970ad09, 69, 0, 0},
  {0xc973699d, 68, 1, 390},
  {0xc9762ab4, 24, 0, 0},
  {0xc993258f, 195, 0, 0},
  {0xc99b690a, 160, 2, 0},
  {0xc9aa7110, 100, 0, 0},
  {0xc9acec1d, 77, 0, 0},
  {0xc9c2a1ec, 92, 0, 0},
  {0xc9cce8f2, 126, 0, 0},
  {0xc9edf585, 46, 0, 0},
  {0xc9ffc5fc, 126, 0, 0},
  {0xca026f1f, 104, 1, 392},
  {0xca033b3a, 153, 1, 1290},
  {0xca06cb60, 0, 1, 0},
  {0xca0a869e, 40, 1, 0},
  {0xca114aac, 40, 1, 0},
  {0xca24a1a2, 156, 0, 0},
  {0xca279a8f, 0, 1, 0},
  {0xca37aa92, 200, 1, 0},
  {0xca3e9b1a, 126, 0, 0},
  {0xca594ace, 126, 1, 1088},
  {0xca680e61, 74, 1, 0},
  {0xca69751b, 117, 0, 0},
  {0xca6a7bf1, 117, 0, 0},
  {0xca7afb3f, 66, 0, 0},
  {0xca8efc8a, 117, 0, 0},
  {0xca96ad0e, 100, 0, 0},
  {0xcaaf5c6b, 0, 1, 0},
  {0xcac5c86b, 135, 1, 0},
  {0xcaf9cc4c, 115, 1, 690},
  {0xcb02a930, 126, 1, 1090},
  {0xcb0a3af4, 174, 0, 0},
  {0xcb0a76b1, 155, 0, 0},
  {0xcb15ba34, 46, 1, 0},
  {0xcb1967e9, 160, 1, 188},
  {0xcb25a2ca, 104, 1, 1193},
  {0xcb275051, 100, 1, 689},
  {0xcb32e243, 34, 0, 0},
  {0xcb475567, 163, 1, 190},
  {0xcb5acb49, 34, 0, 0},
  {0xcb5c3b38, 178, 0, 0},
  {0xcb66694a, 66, 0, 0},
  {0xcb7e529d, 24, 0, 0},
  {0xcb9e67a6, 160, 0, 0},
  {0xcbb545c1, 160, 0, 0},
  {0xcbd413a9, 55, 0, 0},
  {0xcbd6cf2c, 158, 1, 1292},
  {0xcbe85490, 126, 0, 0},
  {0xcbe9482f, 24, 0, 0},
  {0xcbf4366f, 166, 1, 0},
  {0xcbf4cdf6, 77, 0, 0},
  {0xcc0034e3, 143, 0, 0},
  {0xcc2c4b5d, 126, 0, 0},
  {0xcc3544b0, 93, 0, 0},
  {0xcc37094c, 57, 1, 490},
  {0xcc65ab83, 160, 0, 0},
  {0xcc6ad67b, 198, 0, 0},
  {0xcc710d52, 126, 0, 0},
  {0xcc7a4dca, 111, 0, 0},
  {0xcc8a8652, 34, 0, 0},
  {0xcc93a320, 92, 0, 0},
  {0xcca983f8, 24, 1, 1086},
  {0xccaf32dc, 68, 1, 788},
  {0xccaf543a, 160, 0, 0},
  {0xcccaf368, 33, 1, 0},
  {0xccd575a1, 0, 1, 0},
  {0xccdcbfc6, 33, 1, 0},
  {0xcce0cc10, 34, 0, 0},
  {0xccf6bdb2, 165, 0, 0},
  {0xcd00249b, 126, 0, 0},
  {0xcd01471d, 116, 1, 190},
  {0xcd05d34e, 97, 1, 0}, // "Proto"
  {0xcd10dce2, 89, 1, 990},
  {0xcd2a6427, 158, 1, 1091},
  {0xcd2a73f0, 188, 1, 1091},
  {0xcd2ac92c, 126, 1, 1090},
  {0xcd2b058c, 0, 1, 0},
  {0xcd35e2e9, 99, 1, 1191},
  {0xcd50a092, 166, 1, 0},
  {0xcd6549ea, 100, 0, 0},
  {0xcd7a2fd7, 155, 0, 0},
  {0xcd8233ef, 133, 0, 0},
  {0xcd8b279f, 126, 2, 0},
  {0xcd9acf43, 69, 0, 0},
  {0xcdac270d, 68, 1, 1088},
  {0xcdb8c6a5, 41, 0, 0},
  {0xcdc3350b, 127, 1, 1091},
  {0xcdc641fc, 115, 1, 1093},
  {0xcdca4513, 34, 0, 0},
  {0xcdd62ab2, 45, 0, 0},
  {0xcdef1c0d, 150, 0, 0},
  {0xce00022d, 165, 1, 289},
  {0xce06f2d4, 76, 0, 0},
  {0xce07194f, 24, 0, 0},
  {0xce090b49, 33, 1, 0},
  {0xce228874, 163, 1, 390},
  {0xce27fa04, 126, 1, 1085},
  {0xce3f8820, 0, 1, 0},
  {0xce77b4be, 4, 1, 692},
  {0xce90983a, 160, 0, 0},
  {0xcea35d5a, 160, 0, 0},
  {0xceb65b06, 182, 1, 693},
  {0xcebd2a31, 126, 1, 590},
  {0xcebe3e09, 156, 0, 0},
  {0xcece4cfc, 92, 0, 0},
  {0xceceaa31, 0, 1, 0},
  {0xcee5857b, 2, 0, 0},
  {0xcf0c9d97, 97, 0, 0},
  {0xcf23290f, 117, 0, 0},
  {0xcf26a149, 142, 1, 1089},
  {0xcf26c7a1, 104, 1, 989},
  {0xcf322bb3, 182, 1, 389},
  {0xcf3b904c, 81, 0, 0},
  {0xcf40b1c5, 97, 0, 0},
  {0xcf4487a2, 65, 1, 991},
  {0xcf4dbdbe, 160, 0, 0},
  {0xcf5df8ba, 126, 1, 1093},
  {0xcf5f8af0, 22, 1, 1291},
  {0xcf6c88b6, 126, 1, 686},
  {0xcf7ca9bd, 170, 2, 0},
  {0xcf849f72, 165, 2, 0},
  {0xcf9cf7a2, 179, 0, 0},
  {0xcfae9dfa, 17, 1, 990},
  {0xcfd0cc7b, 193, 0, 0},
  {0xcfd4a281, 150, 0, 0},
  {0xcfd5ac62, 200, 1, 0},
  {0xd029f841, 34, 2, 0},
  {0xd0350e25, 41, 0, 0},
  {0xd054ffb0, 126, 1, 394},
  {0xd05cea69, 66, 0, 0},
  {0xd071a15d, 0, 0, 87},
  {0xd09b74dc, 143, 0, 0},
  {0xd0a9f4e1, 55, 0, 0},
  {0xd0c6e757, 195, 0, 0},
  {0xd0cc5ec8, 77, 0, 0},
  {0xd0df726e, 180, 0, 0},
  {0xd0e53454, 160, 0, 0},
  {0xd0eb749f, 92, 0, 0},
  {0xd0f70e36, 156, 2, 0},
  {0xd131bf15, 34, 1, 8},
  {0xd1397940, 89, 0, 0},
  {0xd152fb02, 173, 1, 790},
  {0xd153caf6, 88, 2, 90}, // Swords and Serpents (E) [!]
  {0xd161888b, 84, 2, 0},
  {0xd1691028, 117, 0, 0},
  {0xd175b0cb, 191, 0, 0},
  {0xd188963d, 40, 1, 0},
  {0xd18a3dde, 126, 0, 0},
  {0xd18e6be3, 142, 1, 1289},
  {0xd19addeb, 126, 1, 490},
  {0xd19dcb2b, 123, 1, 991},
  {0xd1bde95c, 11, 1, 0},
  {0xd1d0496f, 186, 1, 1193},
  {0xd1d1b61e, 92, 0, 0},
  {0xd1e50064, 21, 0, 0},
  {0xd1f7df3a, 178, 0, 0},
  {0xd1fb1533, 156, 1, 90},
  {0xd202612b, 89, 0, 0},
  {0xd2038fc5, 89, 0, 0},
  {0xd20bb617, 92, 2, 0},
  {0xd2121f97, 100, 1, 1288},
  {0xd21da4f7, 56, 2, 0},
  {0xd24de7f9, 76, 0, 0},
  {0xd255c7a3, 179, 0, 0},
  {0xd2562072, 4, 1, 392},
  {0xd2574720, 34, 1, 1288},
  {0xd2699893, 117, 0, 0},
  {0xd26efd78, 126, 1, 1188},
  {0xd273b409, 160, 1, 1092},
  {0xd2754add, 0, 1, 0},
  {0xd27b9d50, 104, 0, 0},
  {0xd29db3c7, 155, 0, 0},
  {0xd2bc86f3, 93, 0, 0},
  {0xd2e775d8, 174, 0, 0},
  {0xd308d52c, 29, 1, 987},
  {0xd313adfd, 38, 0, 0},
  {0xd31dc910, 34, 0, 0},
  {0xd31e4238, 126, 1, 686},
  {0xd3428e2e, 65, 1, 991},
  {0xd343c66a, 24, 0, 0},
  {0xd359e02e, 140, 0, 0},
  {0xd377cc36, 31, 1, 0},
  {0xd383dc94, 97, 1, 490},
  {0xd386c2bb, 24, 0, 0},
  {0xd3a265ff, 38, 0, 0},
  {0xd3a91b41, 0, 1, 0},
  {0xd3bff72e, 104, 1, 288},
  {0xd3d248c9, 89, 0, 0},
  {0xd3ee724e, 164, 0, 0},
  {0xd4042702, 97, 0, 0},
  {0xd415571f, 0, 1, 0},
  {0xd4304457, 34, 1, 192},
  {0xd445f698, 126, 1, 0},
  {0xd4611b79, 104, 1, 992},
  {0xd467c0cc, 100, 0, 0},
  {0xd48b14ed, 92, 0, 0},
  {0xd4a443d0, 92, 0, 0},
  {0xd4a70d2b, 74, 1, 1291},
  {0xd4a76b07, 15, 1, 0},
  {0xd4d02ebb, 155, 0, 0},
  {0xd4d9e21a, 126, 1, 1085},
  {0xd4dc528d, 34, 1, 192},
  {0xd4e4afcd, 62, 0, 0},
  {0xd4e8ec0b, 165, 1, 992},
  {0xd4f018f5, 126, 1, 1290},
  {0xd4f5287b, 77, 1, 89},
  {0xd52ebaa6, 100, 2, 0},
  {0xd532e98f, 126, 0, 0},
  {0xd534c98e, 165, 1, 690},
  {0xd548307f, 126, 1, 1085},
  {0xd54f21c7, 38, 0, 0},
  {0xd54f5da9, 144, 0, 0},
  {0xd554c455, 97, 0, 0},
  {0xd5566edf, 113, 1, 1292},
  {0xd568563f, 34, 0, 0},
  {0xd5883d6b, 166, 1, 0},
  {0xd58d14e3, 126, 1, 0},
  {0xd58fec16, 160, 1, 887},
  {0xd5941aa9, 120, 0, 0},
  {0xd5c64257, 126, 1, 1085},
  {0xd5c71458, 65, 1, 988},
  {0xd5d7eac4, 126, 0, 0},
  {0xd5f95da7, 0, 1, 0},
  {0xd60e10aa, 24, 0, 0},
  {0xd63b30f5, 74, 1, 1291},
  {0xd653e27b, 120, 0, 0},
  {0xd65c0697, 104, 1, 891},
  {0xd666560f, 117, 0, 0},
  {0xd679627a, 104, 1, 1092},
  {0xd68a6f33, 136, 0, 0},
  {0xd6ae4ab5, 120, 1, 1290},
  {0xd6cc1f78, 92, 0, 0},
  {0xd6d9de37, 3, 1, 592},
  {0xd6da2a1e, 117, 0, 0},
  {0xd6f7383e, 100, 2, 0},
  {0xd711ea15, 126, 1, 1088},
  {0xd71746d4, 156, 0, 0},
  {0xd7215873, 55, 0, 0},
  {0xd72fb9c8, 4, 1, 1292},
  {0xd738c059, 24, 1, 890},
  {0xd73aa04c, 127, 1, 693},
  {0xd745d7cb, 24, 2, 0},
  {0xd74b2719, 126, 1, 1188},
  {0xd7679a0e, 165, 2, 0},
  {0xd76a57bf, 173, 2, 0},
  {0xd770c1a9, 126, 1, 787},
  {0xd777d47d, 25, 0, 0},
  {0xd7794afc, 126, 1, 593},
  {0xd77b73c8, 34, 1, 690},
  {0xd78729fd, 92, 1, 0},
  {0xd78f47ae, 91, 1, 792},
  {0xd7961e01, 100, 0, 0},
  {0xd7a97b38, 161, 0, 0},
  {0xd7aa0b6d, 117, 0, 0},
  {0xd7ae93df, 0, 1, 0},
  {0xd7b35f7d, 100, 2, 0},
  {0xd7c4816b, 182, 1, 389},
  {0xd7cb398f, 165, 0, 0},
  {0xd7e29c03, 24, 1, 690},
  {0xd7f6320c, 74, 1, 0},
  {0xd7fabac1, 100, 0, 0},
  {0xd7fb558e, 83, 0, 0},
  {0xd7fda595, 117, 0, 0},
  {0xd821a1c6, 165, 0, 0},
  {0xd8230d0e, 153, 1, 492},
  {0xd836a90b, 126, 1, 389},
  {0xd83c671a, 104, 1, 1187},
  {0xd8447fa4, 92, 1, 90},
  {0xd84663b8, 178, 0, 0},
  {0xd84cda4c, 117, 0, 0},
  {0xd852c2f7, 149, 0, 0},
  {0xd8578bfd, 100, 1, 393},
  {0xd858033d, 143, 0, 0},
  {0xd85d26a4, 179, 0, 0},
  {0xd8748e0a, 92, 0, 0},
  {0xd878ebf5, 165, 0, 0},
  {0xd88293fa, 128, 0, 0},
  {0xd88d48d7, 160, 1, 192},
  {0xd89b40c8, 24, 0, 0},
  {0xd8a0c10c, 100, 0, 0},
  {0xd8e641a3, 32, 0, 0},
  {0xd8ee1e43, 32, 0, 0},
  {0xd8ee7669, 6, 1, 1290},
  {0xd8eff0df, 100, 0, 0},
  {0xd8f651e2, 77, 0, 0},
  {0xd907b2e8, 117, 0, 0},
  {0xd9084936, 176, 0, 0},
  {0xd920f9df, 160, 0, 0},
  {0xd923eb5b, 34, 0, 0},
  {0xd9350367, 147, 0, 0},
  {0xd9612836, 25, 0, 0},
  {0xd9736a6d, 2, 1, 492},
  {0xd97595a3, 92, 0, 0},
  {0xd979c8b7, 69, 0, 0},
  {0xd97c31b0, 117, 0, 0},
  {0xd9803a35, 117, 0, 0},
  {0xd98138c4, 34, 0, 0},
  {0xd988bdc0, 156, 0, 0},
  {0xd98e3e31, 0, 1, 0},
  {0xd99a2087, 100, 0, 0},
  {0xd9bb572c, 123, 1, 790},
  {0xd9c093b1, 160, 0, 0},
  {0xd9e8e747, 0, 1, 0},
  {0xd9f1e47c, 34, 0, 0},
  {0xda05d696, 126, 0, 0},
  {0xda122635, 93, 0, 0},
  {0xda23166a, 0, 1, 0},
  {0xda2cb59a, 104, 1, 1090},
  {0xda40e4a2, 120, 0, 0},
  {0xda4de327, 46, 0, 0},
  {0xda690d17, 20, 0, 0},
  {0xda6ccd81, 92, 0, 0},
  {0xda731f14, 133, 0, 0},
  {0xda8b5993, 117, 0, 0},
  {0xdaadebfa, 179, 0, 0},
  {0xdab84a9c, 160, 1, 0},
  {0xdaba9e8e, 142, 1, 1290},
  {0xdad34ee6, 191, 0, 0},
  {0xdaeb93ed, 100, 0, 0},
  {0xdaee19f2, 92, 1, 788},
  {0xdaf9d7e3, 46, 1, 587},
  {0xdb05106e, 24, 0, 0},
  {0xdb0b7db0, 100, 0, 0},
  {0xdb0c3656, 0, 1, 0},
  {0xdb0c7019, 0, 1, 0},
  {0xdb196068, 160, 0, 0},
  {0xdb2d711d, 117, 0, 0},
  {0xdb479677, 55, 0, 0},
  {0xdb511963, 54, 0, 0},
  {0xdb5169dc, 160, 0, 0},
  {0xdb53a88d, 92, 0, 0},
  {0xdb564628, 126, 0, 0},
  {0xdb70a67c, 30, 1, 990},
  {0xdb770ca7, 92, 1, 788},
  {0xdb844a39, 100, 0, 0},
  {0xdb9418e8, 100, 0, 0},
  {0xdb99d0cb, 33, 1, 0},
  {0xdb9dcf89, 77, 1, 189},
  {0xdba3a02e, 89, 0, 0},
  {0xdbb06a25, 24, 1, 1289},
  {0xdbbd1de9, 0, 1, 0},
  {0xdbdefceb, 37, 0, 0},
  {0xdbece74f, 25, 0, 0},
  {0xdbf90772, 143, 1, 1087},
  {0xdc02f095, 104, 1, 790},
  {0xdc1e07d2, 190, 0, 0},
  {0xdc270779, 166, 1, 0},
  {0xdc2a266c, 10, 1, 0},
  {0xdc33aed8, 2, 0, 0},
  {0xdc4da5d4, 46, 1, 687},
  {0xdc52bf0c, 24, 0, 0},
  {0xdc5930ab, 83, 0, 0},
  {0xdc75732f, 20, 0, 0},
  {0xdc87f63d, 114, 1, 89},
  {0xdcb71ca6, 159, 0, 0},
  {0xdcb7c0a1, 160, 0, 0},
  {0xdcb94341, 0, 1, 0},
  {0xdcb972ce, 24, 0, 0},
  {0xdcba4a78, 84, 2, 0},
  {0xdcbcc7a2, 0, 1, 0},
  {0xdcd8d6f4, 39, 0, 0},
  {0xdcdf06de, 117, 0, 0},
  {0xdcea8dc4, 117, 0, 0},
  {0xdcf5fa30, 46, 0, 0},
  {0xdcfd85fc, 0, 1, 0},
  {0xdd29fd59, 117, 0, 0},
  {0xdd454208, 117, 0, 0},
  {0xdd516f89, 57, 1, 490},
  {0xdd53c4ae, 100, 0, 0},
  {0xdd564975, 20, 1, 190},
  {0xdd6a44b5, 156, 0, 0},
  {0xdd8ed0f7, 24, 0, 0},
  {0xdda9f780, 92, 0, 0},
  {0xddad5880, 84, 0, 0},
  {0xddd90c39, 120, 1, 1290},
  {0xdddc649b, 117, 0, 0},
  {0xdde40381, 199, 0, 0},
  {0xdde46aad, 45, 0, 0},
  {0xddf29542, 4, 1, 987},
  {0xddfb8dd5, 159, 0, 0},
  {0xde182c88, 46, 1, 790},
  {0xde1d762f, 0, 1, 0},
  {0xde2070ab, 117, 0, 0},
  {0xde25b90f, 100, 1, 487},
  {0xde395efd, 100, 0, 0},
  {0xde7e6767, 87, 1, 93},
  {0xde8c89ab, 126, 1, 1290},
  {0xde8fd935, 126, 1, 1189},
  {0xde9c9c64, 160, 0, 0},
  {0xdea0d843, 57, 2, 0},
  {0xdea62da3, 24, 0, 0},
  {0xdeb21c9e, 177, 0, 0},
  {0xdebea5a6, 165, 0, 0},
  {0xdeddd5e5, 164, 0, 0},
  {0xdee059e7, 114, 1, 390},
  {0xdefd7665, 115, 1, 93},
  {0xdefe6e8d, 90, 0, 0},
  {0xdf16890a, 107, 0, 0},
  {0xdf189d34, 100, 0, 0},
  {0xdf28a039, 165, 1, 787},
  {0xdf3776c6, 21, 0, 0},
  {0xdf38e21c, 48, 0, 0},
  {0xdf3e45d2, 100, 0, 0},
  {0xdf3eb610, 100, 0, 0},
  {0xdf43e073, 196, 1, 0}, // "Proto"
  {0xdf58fc5a, 126, 1, 1085},
  {0xdf64963b, 115, 1, 190},
  {0xdf67daa1, 126, 1, 1085},
  {0xdfa111f1, 153, 1, 993},
  {0xdfc0ce21, 23, 0, 0},
  {0xdfd70e27, 24, 1, 988},
  {0xdfd9a2ee, 126, 0, 0},
  {0xdff6cf19, 46, 1, 1187},
  {0xe02133ac, 147, 0, 0},
  {0xe03d1e02, 66, 0, 0},
  {0xe042a0ef, 0, 1, 0},
  {0xe054a381, 15, 1, 0},
  {0xe06a060b, 67, 1, 91},
  {0xe086e68e, 117, 0, 0},
  {0xe08c8a60, 39, 0, 0},
  {0xe095c3f2, 160, 1, 893},
  {0xe09d2cd0, 92, 1, 91},
  {0xe0ac6242, 100, 2, 0},
  {0xe0bc622d, 160, 1, 1190},
  {0xe0cbc2ba, 120, 0, 0},
  {0xe0fe0a18, 100, 1, 1292},
  {0xe0fffbd2, 34, 2, 0},
  {0xe12bb5fb, 158, 1, 1192},
  {0xe1383deb, 100, 0, 0},
  {0xe1404f15, 24, 0, 0},
  {0xe145b441, 68, 1, 692},
  {0xe14a7971, 0, 1, 0},
  {0xe1526228, 117, 0, 0},
  {0xe15c973d, 94, 0, 0},
  {0xe16bb5fe, 89, 0, 0},
  {0xe170404c, 24, 0, 0},
  {0xe1789032, 38, 0, 0},
  {0xe17e3b54, 196, 1, 894},
  {0xe180297f, 104, 1, 289},
  {0xe18ab050, 24, 0, 0},
  {0xe19293a2, 2, 0, 0},
  {0xe19a2473, 164, 0, 0},
  {0xe19ee99c, 100, 1, 192},
  {0xe1a2e5b5, 117, 0, 0},
  {0xe1a987e7, 46, 0, 0},
  {0xe1acc990, 72, 0, 0},
  {0xe1b260da, 165, 0, 0},
  {0xe1c03eb6, 78, 0, 0},
  {0xe1c41d7c, 142, 1, 990},
  {0xe1cefa12, 126, 2, 0},
  {0xe1dc9b54, 195, 0, 0},
  {0xe1e307f1, 126, 0, 0},
  {0xe20779c6, 77, 0, 0},
  {0xe20fca45, 156, 0, 0},
  {0xe2281986, 34, 0, 0},
  {0xe2313813, 34, 1, 390},
  {0xe23e6fac, 77, 0, 0},
  {0xe2428b3c, 166, 1, 90},
  {0xe24483b1, 165, 0, 0},
  {0xe24704c9, 160, 0, 0},
  {0xe24df353, 193, 0, 0},
  {0xe266f83a, 77, 0, 0},
  {0xe28f2596, 117, 0, 0},
  {0xe292aa10, 57, 1, 1087},
  {0xe294c86f, 159, 0, 0},
  {0xe2a79a57, 69, 0, 0},
  {0xe2b43a68, 12, 1, 489},
  {0xe2c0a2be, 156, 0, 0},
  {0xe2c4edce, 156, 1, 990},
  {0xe2e92524, 150, 0, 0},
  {0xe2eca5ce, 24, 0, 0},
  {0xe305202e, 149, 0, 0},
  {0xe30552db, 32, 0, 0},
  {0xe30736d7, 29, 1, 987},
  {0xe30b2bcf, 180, 0, 0},
  {0xe30b7f64, 89, 0, 0},
  {0xe333ffa1, 72, 0, 0},
  {0xe349af38, 100, 0, 0},
  {0xe35321bc, 166, 1, 0},
  {0xe353969f, 92, 1, 291},
  {0xe3626d51, 92, 0, 0},
  {0xe362ecdc, 192, 0, 0},
  {0xe3664231, 77, 0, 0},
  {0xe36d5991, 0, 1, 0},
  {0xe3765667, 100, 1, 190},
  {0xe37f82cc, 126, 1, 0},
  {0xe386da54, 126, 0, 0},
  {0xe387c77f, 6, 1, 492},
  {0xe3a6d7f6, 0, 1, 0},
  {0xe3b98143, 100, 0, 0},
  {0xe3c5bb3d, 188, 1, 990},
  {0xe3ef6c9a, 126, 0, 0},
  {0xe3f3f6ae, 115, 1, 1288},
  {0xe402b134, 56, 2, 0},
  {0xe40b4973, 117, 0, 0},
  {0xe40b593b, 0, 1, 0},
  {0xe41ae491, 46, 1, 390},
  {0xe429f0d3, 176, 0, 0},
  {0xe43075eb, 117, 0, 0},
  {0xe431136d, 0, 1, 0},
  {0xe44001d8, 201, 0, 0},
  {0xe4541f6d, 0, 1, 0},
  {0xe45485a5, 117, 0, 0},
  {0xe46c156d, 133, 0, 0},
  {0xe476313e, 0, 1, 0},
  {0xe47e9fa7, 160, 0, 0},
  {0xe488a2f0, 166, 1, 0},
  {0xe492d45a, 89, 0, 0},
  {0xe49fc53e, 24, 0, 0},
  {0xe4a1777e, 100, 0, 0},
  {0xe4c04eea, 100, 0, 0},
  {0xe4c1a245, 6, 2, 0},
  {0xe4ceead1, 100, 0, 0},
  {0xe4cf03d3, 24, 0, 0},
  {0xe4cf4e13, 46, 0, 0},
  {0xe4e3d2ed, 117, 0, 0},
  {0xe50a9130, 97, 1, 889},
  {0xe52687d0, 156, 1, 290},
  {0xe53f7a55, 124, 0, 0},
  {0xe54138a9, 126, 2, 0},
  {0xe542e3cf, 100, 1, 192},
  {0xe5703fc4, 77, 1, 1090},
  {0xe575687c, 3, 1, 993},
  {0xe583ec5e, 151, 0, 0},
  {0xe5a8401b, 6, 2, 0},
  {0xe5a972be, 126, 2, 0},
  {0xe5d11921, 117, 0, 0},
  {0xe5d49424, 193, 0, 0},
  {0xe5d93c8f, 156, 1, 292},
  {0xe5edcde3, 126, 1, 1085},
  {0xe5f49166, 4, 1, 1289},
  {0xe60c5258, 20, 0, 0},
  {0xe61e89ee, 156, 0, 0},
  {0xe62a531b, 156, 0, 0},
  {0xe62e3382, 33, 1, 0},
  {0xe63d9193, 133, 0, 0},
  {0xe63f7d0b, 92, 0, 0},
  {0xe641bd98, 0, 1, 0},
  {0xe64b8975, 117, 0, 0},
  {0xe6517826, 0, 1, 0},
  {0xe66bddcf, 97, 0, 0},
  {0xe66e59a0, 77, 0, 0},
  {0xe675ba2a, 120, 0, 0},
  {0xe6779074, 0, 1, 0},
  {0xe699a97c, 24, 0, 0},
  {0xe6a477b2, 4, 1, 987},
  {0xe6b30bb3, 34, 0, 0},
  {0xe6c28c5f, 99, 0, 0},
  {0xe6c9029e, 100, 0, 0},
  {0xe6d73266, 9, 0, 0},
  {0xe6df6616, 100, 0, 0},
  {0xe6f08e93, 126, 1, 387},
  {0xe6f5fdd2, 92, 0, 0},
  {0xe713f464, 156, 0, 0},
  {0xe71d034e, 100, 2, 91}, // Snake's Revenge (E) [!]
  {0xe737a11f, 0, 1, 0},
  {0xe73e7260, 166, 1, 0},
  {0xe74a91bb, 12, 1, 690},
  {0xe74aa15a, 150, 0, 0},
  {0xe77268ec, 156, 0, 0},
  {0xe78a394c, 25, 0, 0},
  {0xe792de94, 2, 0, 0},
  {0xe7a3867b, 53, 0, 0},
  {0xe7b03b03, 24, 0, 0},
  {0xe7ba371d, 34, 1, 789},
  {0xe7baa5f6, 192, 0, 0},
  {0xe7c72dbb, 99, 1, 392},
  {0xe7c981a2, 22, 1, 1192},
  {0xe7da8a04, 153, 1, 292},
  {0xe7ff1a3d, 104, 1, 488},
  {0xe801f662, 160, 0, 0},
  {0xe80b9027, 164, 0, 0},
  {0xe840fd21, 126, 1, 290},
  {0xe84274c5, 92, 0, 0},
  {0xe8456051, 126, 0, 0},
  {0xe84f40e5, 142, 1, 792},
  {0xe86e4df1, 72, 0, 0},
  {0xe87d33a0, 133, 0, 0},
  {0xe880d426, 58, 1, 90},
  {0xe8941598, 166, 1, 0},
  {0xe8973b45, 101, 0, 0},
  {0xe8af6ff5, 178, 0, 0},
  {0xe8b20197, 126, 0, 0},
  {0xe8b4834c, 100, 0, 0},
  {0xe8b7c767, 117, 0, 0},
  {0xe8baa782, 192, 0, 0},
  {0xe8bbe2be, 195, 0, 0},
  {0xe8d26cb0, 126, 1, 0},
  {0xe8f8f7a5, 126, 2, 0},
  {0xe9023072, 92, 0, 0},
  {0xe90507fa, 100, 0, 0},
  {0xe909d14a, 77, 0, 0},
  {0xe90d4a9f, 92, 0, 0},
  {0xe911bcc4, 117, 0, 0},
  {0xe91548d8, 126, 0, 0},
  {0xe9176129, 160, 0, 0},
  {0xe91f17c2, 117, 0, 0},
  {0xe922037c, 117, 0, 0},
  {0xe92aed05, 97, 1, 190},
  {0xe93d07d7, 117, 0, 0},
  {0xe940d56f, 155, 0, 0},
  {0xe943ec4d, 24, 1, 791},
  {0xe949ef8a, 1, 0, 0},
  {0xe94d5181, 128, 0, 0},
  {0xe94e883d, 126, 2, 88}, // Super Mario Bros 2 (E) [!]
  {0xe95454fc, 102, 0, 0},
  {0xe95752bd, 0, 1, 0},
  {0xe98ab943, 158, 1, 1193},
  {0xe9a6c211, 188, 1, 291},
  {0xe9aa0400, 90, 0, 0},
  {0xe9bc16ff, 128, 0, 0},
  {0xe9c387ec, 14, 1, 190},
  {0xe9cf747f, 34, 1, 85},
  {0xe9d1ec63, 92, 0, 0},
  {0xe9d352eb, 4, 2, 0},
  {0xe9e27b7b, 0, 1, 0},
  {0xe9f16673, 130, 1, 0},
  {0xe9f5be99, 0, 1, 0},
  {0xea08700b, 77, 0, 0},
  {0xea113128, 40, 1, 0},
  {0xea2b9d3f, 60, 0, 0},
  {0xea3e78dd, 92, 0, 0},
  {0xea4eb69e, 143, 1, 291},
  {0xea62f88f, 76, 0, 0},
  {0xea74c587, 100, 0, 0},
  {0xea79cc19, 150, 0, 0},
  {0xea812597, 92, 0, 0},
  {0xea89963f, 161, 0, 0},
  {0xea9f7a09, 200, 1, 0},
  {0xeab93cfb, 69, 0, 0},
  {0xead17d60, 166, 1, 0},
  {0xeadf0b97, 134, 0, 0},
  {0xeaf7ed72, 126, 1, 0},
  {0xeafc4944, 100, 0, 0},
  {0xeb0bda7e, 166, 1, 0},
  {0xeb0f903a, 142, 1, 92},
  {0xeb15169e, 131, 1, 690},
  {0xeb297321, 117, 0, 0},
  {0xeb2c8f4d, 34, 0, 0},
  {0xeb465156, 43, 0, 0},
  {0xeb61133b, 160, 1, 1190},
  {0xeb6daea5, 126, 1, 1088},
  {0xeb71a6c5, 34, 1, 392},
  {0xeb75ca30, 117, 0, 0},
  {0xeb764567, 117, 0, 0},
  {0xeb803610, 20, 1, 1191},
  {0xeb807320, 89, 0, 0},
  {0xeb84c54c, 104, 1, 591},
  {0xeb9960ee, 4, 1, 290},
  {0xebac24e9, 124, 0, 0},
  {0xebb1c045, 66, 0, 0},
  {0xebbc6ba8, 77, 0, 0},
  {0xebcf8419, 195, 0, 0},
  {0xebcfe7c5, 4, 1, 1289},
  {0xec04f0da, 155, 0, 0},
  {0xec0fc2de, 74, 1, 989},
  {0xec1c5ad5, 20, 1, 1190},
  {0xec3001c7, 29, 1, 987},
  {0xec461db9, 126, 0, 0},
  {0xec47296d, 147, 0, 0},
  {0xec69adf2, 65, 1, 988},
  {0xec8a884f, 100, 0, 0},
  {0xec968c51, 166, 1, 0},
  {0xeca43006, 126, 1, 1088},
  {0xecbf33ce, 124, 0, 0},
  {0xeccd4089, 92, 1, 1088},
  {0xecdbafa4, 160, 0, 0},
  {0xece525dd, 24, 1, 388},
  {0xece951ea, 138, 0, 0},
  {0xecec80b4, 92, 0, 0},
  {0xecf3964d, 133, 0, 0},
  {0xecfd3c69, 160, 0, 0},
  {0xed0aa77b, 77, 0, 0},
  {0xed2465be, 100, 1, 990},
  {0xed36649f, 89, 0, 0},
  {0xed4fb487, 92, 0, 0},
  {0xed55073f, 142, 1, 1289},
  {0xed588f00, 126, 0, 0},
  {0xed58dddd, 40, 1, 89},
  {0xed620003, 195, 0, 0},
  {0xed67811e, 126, 0, 0},
  {0xed77b453, 87, 2, 0},
  {0xed9a3ac6, 24, 0, 0},
  {0xedc3662b, 160, 1, 589},
  {0xedcf1b71, 153, 1, 690},
  {0xedd6d29b, 68, 1, 692},
  {0xedd7479a, 117, 0, 0},
  {0xeddcc468, 34, 0, 0},
  {0xeddd580b, 156, 1, 1090},
  {0xede55d1c, 178, 0, 0},
  {0xedea9ac6, 193, 1, 390},
  {0xee1b0d52, 92, 0, 0},
  {0xee309168, 0, 1, 0},
  {0xee43153e, 34, 0, 0},
  {0xee60b5d6, 117, 0, 0},
  {0xee64327b, 126, 0, 0},
  {0xee6892eb, 4, 1, 1091},
  {0xee693008, 127, 1, 493},
  {0xee6ef957, 161, 0, 0},
  {0xee711afb, 160, 0, 0},
  {0xee875b41, 133, 0, 0},
  {0xee8c9971, 126, 1, 0},
  {0xee921d8e, 188, 1, 689},
  {0xee993635, 69, 0, 0},
  {0xeeb16683, 117, 0, 0},
  {0xeeb7a62b, 126, 1, 1085},
  {0xeeb9252b, 156, 0, 0},
  {0xeee0c7f8, 57, 2, 0},
  {0xeee3f31c, 126, 0, 0},
  {0xeee6314e, 165, 0, 0},
  {0xeee9a682, 99, 0, 0},
  {0xef0c9672, 116, 1, 291},
  {0xef1c8906, 0, 1, 0},
  {0xef40790c, 160, 1, 1291},
  {0xef4649f6, 191, 0, 0},
  {0xef4f5903, 0, 1, 0},
  {0xef4f9b96, 130, 1, 0},
  {0xef5f1d5f, 93, 0, 0},
  {0xef6278f9, 163, 1, 89},
  {0xef7996bf, 117, 0, 0},
  {0xef7af338, 126, 0, 0},
  {0xefb09075, 34, 1, 989},
  {0xefb1df9e, 92, 0, 0},
  {0xefcc2da9, 100, 0, 0},
  {0xefcf375d, 110, 1, 1090},
  {0xefd4837f, 92, 0, 0},
  {0xefd51f04, 193, 1, 0},
  {0xefeb0c34, 100, 0, 0},
  {0xeffd40c3, 188, 1, 691},
  {0xeffe438f, 117, 0, 0},
  {0xeffeea40, 77, 0, 0},
  {0xf00355a0, 147, 0, 0},
  {0xf00584b6, 188, 1, 691},
  {0xf009ddd2, 24, 1, 492},
  {0xf011e490, 99, 1, 991},
  {0xf0302211, 165, 0, 0},
  {0xf04bc5ef, 160, 0, 0},
  {0xf053ac5f, 97, 0, 0},
  {0xf0552f02, 156, 0, 0},
  {0xf05870d5, 15, 1, 0},
  {0xf0632142, 100, 0, 0},
  {0xf08e8ef0, 192, 0, 0},
  {0xf08ed289, 34, 1, 1291},
  {0xf09d0316, 74, 1, 1090},
  {0xf0a4d2f4, 199, 0, 0},
  {0xf0a5eb24, 0, 1, 0},
  {0xf0ae68c0, 24, 0, 0},
  {0xf0c198ff, 0, 2, 0},
  {0xf0c77dcb, 165, 1, 891},
  {0xf0d02f6a, 42, 0, 0},
  {0xf0dab3d3, 115, 1, 1290},
  {0xf0df311e, 4, 1, 291},
  {0xf0e9971b, 34, 1, 1092},
  {0xf1081b1b, 46, 0, 0},
  {0xf159b106, 37, 0, 0},
  {0xf161a5d8, 34, 0, 0},
  {0xf17486df, 43, 0, 0},
  {0xf18180cb, 81, 0, 0},
  {0xf181c021, 29, 1, 489},
  {0xf18eef69, 188, 1, 289},
  {0xf19a11af, 93, 0, 0},
  {0xf19a4249, 0, 1, 0},
  {0xf1a03778, 24, 0, 0},
  {0xf1bdcf18, 160, 0, 0},
  {0xf1c76aed, 39, 0, 0},
  {0xf1d07e3e, 0, 1, 0},
  {0xf1d861ef, 126, 1, 0},
  {0xf1db1ca5, 129, 2, 0},
  {0xf1e2b9e8, 173, 0, 0},
  {0xf1e6b576, 92, 0, 0},
  {0xf1fed9b8, 43, 1, 889},
  {0xf1ffd9f8, 126, 1, 686},
  {0xf200080d, 0, 1, 0},
  {0xf2099df0, 46, 1, 188},
  {0xf210e68f, 126, 1, 1188},
  {0xf21af99c, 126, 0, 0},
  {0xf225e3f5, 161, 0, 0},
  {0xf23c035c, 38, 0, 0},
  {0xf2594374, 136, 0, 0},
  {0xf2641ad0, 126, 1, 890},
  {0xf280184b, 178, 0, 0},
  {0xf28a5b8d, 153, 2, 0},
  {0xf2a6ace1, 92, 1, 190},
  {0xf2a99db1, 100, 0, 0},
  {0xf2a9f64d, 92, 0, 0},
  {0xf2c4836f, 0, 1, 0},
  {0xf2d03ada, 2, 0, 0},
  {0xf2df30b2, 0, 1, 0},
  {0xf2ec0901, 156, 0, 0},
  {0xf2fc8212, 77, 0, 0},
  {0xf304f1b9, 6, 1, 1289},
  {0xf308e97a, 46, 0, 0},
  {0xf31d36a3, 158, 1, 1091},
  {0xf31dcc15, 121, 0, 0},
  {0xf32748a1, 59, 0, 0},
  {0xf34175b3, 92, 0, 0},
  {0xf34190b4, 34, 0, 0},
  {0xf3473009, 117, 0, 0},
  {0xf35e4442, 122, 0, 0},
  {0xf35f14d3, 43, 1, 390},
  {0xf3623561, 175, 0, 0},
  {0xf37befd5, 76, 1, 792},
  {0xf3808245, 76, 0, 0},
  {0xf3826759, 74, 1, 1190},
  {0xf39fd253, 34, 1, 1088},
  {0xf3a36c5f, 38, 0, 0},
  {0xf3c13f19, 156, 0, 0},
  {0xf3c743aa, 34, 1, 1293},
  {0xf3e394d1, 139, 0, 0},
  {0xf3f1269d, 165, 0, 0},
  {0xf3f3a491, 126, 2, 0},
  {0xf3feb3ab, 164, 0, 0},
  {0xf4024666, 99, 1, 1290},
  {0xf4120e58, 99, 0, 0},
  {0xf4174da2, 156, 0, 0},
  {0xf419b383, 174, 0, 0},
  {0xf41add60, 117, 0, 0},
  {0xf42b0dbd, 46, 1, 1186},
  {0xf435dc0c, 171, 0, 0},
  {0xf4615036, 92, 1, 689},
  {0xf470d4e3, 4, 1, 1291},
  {0xf4c83de3, 24, 0, 0},
  {0xf4cd4998, 99, 0, 0},
  {0xf4d46622, 0, 1, 0},
  {0xf4d70c17, 92, 1, 588},
  {0xf4dd5ba5, 144, 0, 0},
  {0xf4dfdb14, 150, 1, 791},
  {0xf4e5df0e, 128, 0, 0},
  {0xf4fb65dd, 69, 0, 0},
  {0xf511566c, 117, 0, 0},
  {0xf518dd58, 114, 1, 690},
  {0xf51e2ffe, 153, 1, 689},
  {0xf532f09a, 193, 1, 988},
  {0xf5399c3c, 126, 0, 0},
  {0xf540677b, 99, 0, 0},
  {0xf54b34bd, 188, 1, 190},
  {0xf54e1dcc, 92, 1, 788},
  {0xf56a5b10, 126, 1, 686},
  {0xf585407a, 133, 0, 0},
  {0xf59cfc3d, 92, 2, 0},
  {0xf59da73c, 160, 0, 0},
  {0xf5a1b8fb, 87, 2, 0},
  {0xf5bfd030, 100, 2, 0},
  {0xf5d061ca, 156, 1, 987},
  {0xf5dfa4a2, 6, 2, 0},
  {0xf5f435b1, 77, 0, 0},
  {0xf5faae4f, 117, 0, 0},
  {0xf5fe896f, 30, 0, 0},
  {0xf6035030, 100, 1, 288},
  {0xf60f6667, 100, 0, 0},
  {0xf6139ee9, 174, 0, 0},
  {0xf613a8f9, 114, 1, 389},
  {0xf6271a51, 100, 0, 0},
  {0xf633813d, 34, 0, 0},
  {0xf635c594, 156, 0, 0},
  {0xf6419d79, 100, 0, 0},
  {0xf64cb545, 92, 0, 0},
  {0xf651398d, 160, 1, 490},
  {0xf66ec512, 126, 0, 0},
  {0xf6751d3d, 54, 0, 0},
  {0xf6898a59, 158, 1, 1192},
  {0xf699ee7e, 166, 1, 0},
  {0xf6aea515, 0, 1, 0},
  {0xf6b24806, 0, 1, 0},
  {0xf6b9799c, 126, 1, 991},
  {0xf6de2aa2, 100, 0, 0},
  {0xf6fa4453, 4, 1, 790},
  {0xf714fae3, 99, 0, 0},
  {0xf71a9931, 4, 2, 0},
  {0xf732c8fd, 33, 1, 0},
  {0xf735d926, 100, 0, 0},
  {0xf74dfc91, 74, 1, 390},
  {0xf751f337, 46, 0, 0},
  {0xf760f1cb, 97, 0, 0},
  {0xf76c815e, 160, 1, 989},
  {0xf786d602, 4, 1, 588},
  {0xf79a75d7, 126, 1, 1294},
  {0xf79d684a, 2, 0, 0},
  {0xf7b852e4, 37, 0, 0},
  {0xf7bc7104, 43, 1, 389},
  {0xf7d20181, 93, 0, 0},
  {0xf7d51d87, 2, 0, 0},
  {0xf7de03e0, 181, 0, 0},
  {0xf7e07b83, 63, 0, 0},
  {0xf7e84558, 77, 1, 1092},
  {0xf7f9785a, 82, 1, 991},
  {0xf808af60, 156, 0, 0},
  {0xf80bdc50, 160, 0, 0},
  {0xf81e0fed, 68, 1, 489},
  {0xf83c4c49, 97, 1, 1290},
  {0xf85084a3, 25, 0, 0},
  {0xf85bbf3e, 126, 1, 0}, // "Proto"
  {0xf85e264d, 160, 0, 0},
  {0xf865ec8d, 53, 0, 0},
  {0xf86d8d8a, 126, 0, 0},
  {0xf885d931, 75, 0, 0},
  {0xf89170c5, 144, 0, 0},
  {0xf89300fb, 178, 0, 0},
  {0xf8a713be, 0, 1, 0},
  {0xf8b6cb9c, 95, 0, 0},
  {0xf8d53171, 147, 0, 0},
  {0xf8da2506, 24, 0, 0},
  {0xf8f2b56b, 12, 1, 489},
  {0xf90ae80e, 0, 1, 0},
  {0xf919795d, 52, 2, 0},
  {0xf91bac83, 65, 1, 1093},
  {0xf927fa43, 76, 0, 0},
  {0xf92be3ec, 166, 1, 0},
  {0xf92be7f2, 115, 1, 591},
  {0xf95d26a9, 134, 0, 0},
  {0xf9622bfa, 126, 0, 0},
  {0xf96d07c8, 173, 0, 0},
  {0xf982cdf5, 34, 0, 0},
  {0xf989296c, 190, 0, 0},
  {0xf98ee444, 43, 0, 0},
  {0xf99e37eb, 20, 1, 190},
  {0xf9b4c729, 117, 0, 0},
  {0xf9c22e00, 117, 0, 0},
  {0xf9c7e1b2, 0, 1, 0},
  {0xf9d5ae4a, 83, 0, 0},
  {0xf9fd48cd, 77, 1, 194},
  {0xfa014ba1, 100, 2, 0},
  {0xfa258f2f, 48, 0, 0},
  {0xfa2a8a8b, 124, 0, 0},
  {0xfa322d23, 29, 1, 987},
  {0xfa3bfc11, 115, 1, 1090},
  {0xfa43146b, 29, 1, 489},
  {0xfa46c411, 126, 0, 0},
  {0xfa4b1d72, 148, 0, 0},
  {0xfa6d4281, 32, 0, 0},
  {0xfa704c86, 160, 0, 0},
  {0xfa7ee642, 34, 2, 0},
  {0xfa8339a5, 100, 1, 192},
  {0xfa839dc9, 22, 1, 1192},
  {0xfa918f22, 102, 0, 0},
  {0xfaa957b1, 160, 1, 391},
  {0xfaeb390b, 133, 0, 0},
  {0xfaf04375, 95, 0, 0},
  {0xfaf48d27, 0, 1, 0},
  {0xfaf802d1, 39, 0, 0},
  {0xfaff4177, 39, 0, 0},
  {0xfb03a704, 34, 0, 0},
  {0xfb1c0551, 150, 0, 0},
  {0xfb26ff02, 69, 0, 0},
  {0xfb4092fa, 46, 1, 1292},
  {0xfb40d76c, 127, 2, 0},
  {0xfb5f213e, 41, 0, 0},
  {0xfb69c131, 40, 1, 0},
  {0xfb72e586, 0, 1, 0},
  {0xfb7f90fa, 100, 0, 0},
  {0xfb847d1a, 139, 0, 0},
  {0xfb8672a2, 171, 0, 0},
  {0xfb8a9b80, 102, 0, 0},
  {0xfb98d46e, 126, 1, 1085},
  {0xfba98643, 117, 0, 0},
  {0xfbaab554, 68, 1, 989},
  {0xfbd87b3e, 191, 0, 0},
  {0xfbf8a785, 65, 1, 988},
  {0xfbfc6a6c, 100, 2, 0},
  {0xfc00a282, 92, 0, 0},
  {0xfc201c46, 97, 0, 0},
  {0xfc2b7cfa, 97, 1, 1290},
  {0xfc3e5c86, 34, 1, 287},
  {0xfc5783a7, 34, 1, 194},
  {0xfc6201e7, 164, 0, 0},
  {0xfc8f6a1c, 183, 0, 0},
  {0xfcb13110, 46, 0, 0},
  {0xfcb5cb1e, 174, 0, 0},
  {0xfcbf28b1, 100, 0, 0},
  {0xfcd772eb, 91, 2, 0},
  {0xfcdaca80, 160, 0, 0},
  {0xfcde8825, 97, 0, 0},
  {0xfce408a4, 165, 1, 1187},
  {0xfcebcc5f, 126, 0, 0},
  {0xfcfa4c1e, 44, 0, 0},
  {0xfd0299c3, 46, 0, 0},
  {0xfd17e704, 57, 1, 1191},
  {0xfd214c58, 117, 0, 0},
  {0xfd3c3147, 100, 0, 0},
  {0xfd3fc292, 89, 0, 0},
  {0xfd45e9c1, 23, 0, 0},
  {0xfd529896, 173, 0, 0},
  {0xfd5d5c55, 0, 1, 0},
  {0xfd65afff, 68, 1, 290},
  {0xfd719491, 0, 1, 0},
  {0xfd7dd2b0, 77, 0, 0},
  {0xfd7e9a7e, 127, 2, 0},
  {0xfd8d6c75, 115, 1, 1090},
  {0xfd9ad6a2, 34, 1, 1287},
  {0xfda76f70, 120, 0, 0},
  {0xfdb1917e, 166, 0, 0},
  {0xfdb8aa9a, 89, 0, 0},
  {0xfdbca5d1, 117, 0, 0},
  {0xfddf2135, 34, 0, 0},
  {0xfde14cce, 44, 0, 0},
  {0xfde1c7ed, 74, 1, 1090},
  {0xfde61002, 92, 0, 0},
  {0xfde79681, 156, 0, 0},
  {0xfe18e6b6, 2, 0, 0},
  {0xfe2dab28, 156, 1, 987},
  {0xfe3488d1, 99, 0, 0},
  {0xfe35d144, 126, 0, 0},
  {0xfe364be5, 20, 0, 0},
  {0xfe37b30d, 128, 0, 0},
  {0xfe387fe5, 89, 0, 0},
  {0xfe4e5b11, 202, 0, 0},
  {0xfe4ed42b, 191, 0, 0},
  {0xfe58928a, 68, 1, 489},
  {0xfe5f17f0, 126, 1, 692},
  {0xfe94066d, 147, 0, 0},
  {0xfe99bbed, 16, 0, 0},
  {0xfeac6916, 156, 0, 0},
  {0xfecf92c8, 0, 1, 0},
  {0xfed27aca, 160, 0, 0},
  {0xfefe9064, 0, 1, 0},
  {0xff0bd357, 34, 0, 0},
  {0xff1412ea, 100, 0, 0},
  {0xff24d794, 126, 1, 1085},
  {0xff2c8ee4, 156, 0, 0},
  {0xff5135a3, 126, 0, 0},
  {0xff5bc685, 100, 0, 0},
  {0xff6621ce, 100, 0, 0},
  {0xff743e38, 0, 1, 0},
  {0xffbef374, 100, 0, 0},
  {0xffd9db04, 147, 0, 0},
  {0xffecf645, 46, 1, 790},
  {0xffef86c8, 126, 0, 0}
};

static nes_file_t type;

static const st_getopt2_t ines_usage[] =
  {
    {NULL, 0, 0, 0, NULL, "iNES header", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

static const st_getopt2_t unif_usage[] =
  {
    {NULL, 0, 0, 0, NULL, "UNIF header", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

static const st_getopt2_t pasofami_usage[] =
  {
    {NULL, 0, 0, 0, NULL, "Pasofami file", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

static const st_getopt2_t fds_usage[] =
  {
    {NULL, 0, 0, 0, NULL, "Famicom Disk System file (diskimage)", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };

static st_ines_header_t ines_header;
static st_unif_header_t unif_header;
static st_smc_header_t ffe_header;

#if     UNIF_REVISION > 7
static const char unif_ucon64_sig[] =
  "uCON64" WRTR_MARKER_S UCON64_VERSION_S WRTR_MARKER_S CURRENT_OS_S;
#else
static const char unif_ucon64_sig[] = STD_COMMENT UCON64_VERSION_S " " CURRENT_OS_S;
#endif
static const int unif_prg_ids[] = {PRG0_ID, PRG1_ID, PRG2_ID, PRG3_ID,
                                   PRG4_ID, PRG5_ID, PRG6_ID, PRG7_ID,
                                   PRG8_ID, PRG9_ID, PRGA_ID, PRGB_ID,
                                   PRGC_ID, PRGD_ID, PRGE_ID, PRGF_ID};
static const int unif_pck_ids[] = {PCK0_ID, PCK1_ID, PCK2_ID, PCK3_ID,
                                   PCK4_ID, PCK5_ID, PCK6_ID, PCK7_ID,
                                   PCK8_ID, PCK9_ID, PCKA_ID, PCKB_ID,
                                   PCKC_ID, PCKD_ID, PCKE_ID, PCKF_ID};
static const int unif_chr_ids[] = {CHR0_ID, CHR1_ID, CHR2_ID, CHR3_ID,
                                   CHR4_ID, CHR5_ID, CHR6_ID, CHR7_ID,
                                   CHR8_ID, CHR9_ID, CHRA_ID, CHRB_ID,
                                   CHRC_ID, CHRD_ID, CHRE_ID, CHRF_ID};
static const int unif_cck_ids[] = {CCK0_ID, CCK1_ID, CCK2_ID, CCK3_ID,
                                   CCK4_ID, CCK5_ID, CCK6_ID, CCK7_ID,
                                   CCK8_ID, CCK9_ID, CCKA_ID, CCKB_ID,
                                   CCKC_ID, CCKD_ID, CCKE_ID, CCKF_ID};

static const char *nes_destfname = NULL, *internal_name;
static int rom_size;
static FILE *nes_destfile;


static int
nes_compare (const void *key, const void *found)
{
  /*
    The return statement looks overly complicated, but is really necessary.
    This construct:
      return ((st_nes_data_t *) key)->crc32 - ((st_nes_data_t *) found)->crc32;
    does *not* work correctly for all cases.
  */
  return (int) (((int64_t) ((st_nes_data_t *) key)->crc32 -
                 (int64_t) ((st_nes_data_t *) found)->crc32) / 2);
}


static int
toprint (int c)
{
  return isprint (c) ? c : '-';
}


nes_file_t
nes_get_file_type (void)
{
  return type;
}


static void
remove_destfile (void)
{
  if (nes_destfname)
    {
      printf ("Removing: %s\n", nes_destfname);
      fclose (nes_destfile);                    // necessary under DOS/Win9x for DJGPP port
      remove (nes_destfname);
      nes_destfname = NULL;
    }
}


static unsigned int
read_block (unsigned char **data, unsigned int size, FILE *file, const char *format, ...)
{
  va_list argptr;
  unsigned int real_size;

  va_start (argptr, format);
  if ((*data = (unsigned char *) malloc (size)) == NULL)
    {
      vfprintf (stderr, format, argptr);
      exit (1);
    }
  real_size = fread (*data, 1, size, file);
  if (real_size != size)
    printf ("WARNING: Couldn't read %d bytes, only %d bytes were available\n",
      size, real_size);

  va_end (argptr);
  return real_size;
}


static st_unif_chunk_t *
read_chunk (unsigned long id, unsigned char *rom_buffer, int cont)
/*
  The caller is responsible for freeing the memory for the allocated
  st_unif_chunk_t. It should do that by calling free() with the pointer to
  the st_unif_chunk_t. It should do NOTHING for the struct member `data'.
*/
{
// the DEBUG_READ_CHUNK blocks are left here on purpose, don't remove!
//#define DEBUG_READ_CHUNK
#ifdef  DEBUG_READ_CHUNK
  char id_str[5] = "    ";
#endif
  struct
  {
     unsigned int id;                           // chunk identification string
     unsigned int length;                       // data length, in little endian format
  } chunk_header;
  st_unif_chunk_t *unif_chunk;
  static int pos = 0;

  if (!cont)
    pos = 0; // fseek (file, UNIF_HEADER_LEN, SEEK_SET);

#ifdef  WORDS_BIGENDIAN
  id = bswap_32 (id);                           // swap id once instead of chunk_header.id often
#endif
  do
    {
      // fread (&chunk_header, 1, sizeof (chunk_header), file);
      memcpy (&chunk_header, rom_buffer + pos, sizeof (chunk_header));
      pos += sizeof (chunk_header);
#ifdef  WORDS_BIGENDIAN
      chunk_header.length = bswap_32 (chunk_header.length);
#endif
//      if (feof (file))
//        break;
#ifdef  DEBUG_READ_CHUNK
      memcpy (id_str, &chunk_header.id, 4);
#ifdef  WORDS_BIGENDIAN
      *((int *) id_str) = bswap_32 (*((int *) id_str));
#endif
      printf ("chunk header: id=%s, length=%d\n", id_str, chunk_header.length);
#endif
      if (chunk_header.id != id)
        {
          // (fseek (file, chunk_header.length, SEEK_CUR) != 0) // fseek() clears EOF indicator
          if ((int) (pos + chunk_header.length) >= rom_size)
            break;
          else
            pos += chunk_header.length;
        }
    }
  while (chunk_header.id != id);

  if (chunk_header.id != id || pos >= rom_size) // || feof (file))
    {
#ifdef  DEBUG_READ_CHUNK
      printf ("exit1\n");
#endif
      return (st_unif_chunk_t *) NULL;
    }

  if ((unif_chunk = (st_unif_chunk_t *)
         malloc (sizeof (st_unif_chunk_t) + chunk_header.length)) == NULL)
    {
      fprintf (stderr, "ERROR: Not enough memory for chunk (%d bytes)\n",
        (int) sizeof (st_unif_chunk_t) + chunk_header.length);
      exit (1);
    }
  unif_chunk->id = me2le_32 (chunk_header.id);
  unif_chunk->length = chunk_header.length;
  unif_chunk->data = &((unsigned char *) unif_chunk)[sizeof (st_unif_chunk_t)];

  // fread (unif_chunk->data, 1, chunk_header.length, file);
  memcpy (unif_chunk->data, rom_buffer + pos, chunk_header.length);
  pos += chunk_header.length;
#ifdef  DEBUG_READ_CHUNK
  printf ("exit2\n");
#endif
  return unif_chunk;
#undef  DEBUG_READ_CHUNK
}


static void
write_chunk (st_unif_chunk_t *chunk, FILE *file)
{
#ifdef  WORDS_BIGENDIAN
  int x;
  x = bswap_32 (chunk->id);
  fwrite (&x, 1, sizeof (chunk->id), file);
  x = bswap_32 (chunk->length);
  fwrite (&x, 1, sizeof (chunk->length), file);
#else
  fwrite (&chunk->id, 1, sizeof (chunk->id), file);
  fwrite (&chunk->length, 1, sizeof (chunk->length), file);
#endif
  fwrite (chunk->data, 1, chunk->length, file);
}


static int
parse_info_file (st_dumper_info_t *info, const char *fname)
/*
  Format of info file:

  <string for name><newline>
  <d>[<d>]{-,/}<m>[<m>]{-,/}<y><y>[<y>][<y>]{-,/}<newline>
  <string for agent>[<newline>]

  <newline> can be \n (Unix), \r\n (DOS) or \r (Macintosh)

  examples of valid date lines:
  22-11-1975
  1/1/01
*/
{
#define SIZE_INFO (99 + 10 + 99 + 3 * 2)        // possibly 3 lines in DOS text format
  int n, prev_n;
  char buf[SIZE_INFO] = { 0 }, number[80];      // 4 should be enough, but don't
                                                //  be too sensitive to errors
  memset (info, 0, sizeof (st_dumper_info_t));
  if (ucon64_fread (buf, 0, SIZE_INFO, fname) <= 0)
    return -1;

  for (n = 0; n < 99; n++)
    if (buf[n] == '\n' || buf[n] == '\r')
      break;
  strncpy (info->dumper_name, buf, n);
  info->dumper_name[99] = 0;

  // handle newline, possibly in DOS format
  prev_n = n;
  for (; n < prev_n + 2; n++)
    if (buf[n] != '\n' && buf[n] != '\r')
      break;

  prev_n = n;
  for (; n < prev_n + 2; n++)
    if (buf[n] == '-' || buf[n] == '/')
      break;
  strncpy (number, &buf[prev_n], n - prev_n);
  number[n - prev_n] = 0;
  info->day = (unsigned char) strtol (number, NULL, 10);

  n++;
  prev_n = n;
  for (; n < prev_n + 2; n++)
    if (buf[n] == '-' || buf[n] == '/')
      break;
  strncpy (number, &buf[prev_n], n - prev_n);
  number[n - prev_n] = 0;
  info->month = (unsigned char) strtol (number, NULL, 10);

  n++;
  prev_n = n;
  for (; n < prev_n + 4; n++)
    if (buf[n] == '\n' || buf[n] == '\r')
      break;
  strncpy (number, &buf[prev_n], n - prev_n);
  number[n - prev_n] = 0;
  info->year = (unsigned short) strtol (number, NULL, 10);

  // handle newline, possibly in DOS format
  prev_n = n;
  for (; n < prev_n + 2; n++)
    if (buf[n] != '\n' && buf[n] != '\r')
      break;

  prev_n = n;
  for (; n < prev_n + 99; n++)
    if (buf[n] == '\n' || buf[n] == '\r')
      break;
  strncpy (info->dumper_agent, &buf[prev_n], n - prev_n);
  info->dumper_agent[99] = 0;

  return 0;
#undef  SIZE_INFO
}


static int
nes_ines_unif (FILE *srcfile, FILE *destfile)
{
  int prg_size, chr_size, x;
  unsigned char *prg_data = NULL, *chr_data = NULL, b;
  st_unif_chunk_t unif_chunk;

  // read iNES file
  fread (&ines_header, 1, INES_HEADER_LEN, srcfile);
  if (ines_header.ctrl1 & INES_TRAINER)
    fseek (srcfile, 512, SEEK_CUR);             // discard trainer data (lib_unif does the same)

  prg_size = ines_header.prg_size << 14;
  prg_size = read_block (&prg_data, prg_size, srcfile,
                         "ERROR: Not enough memory for PRG buffer (%d bytes)\n", prg_size);
  chr_size = ines_header.chr_size << 13;
  if (chr_size > 0)
    chr_size = read_block (&chr_data, chr_size, srcfile,
                           "ERROR: Not enough memory for CHR buffer (%d bytes)\n", chr_size);

  // write UNIF file
  memset (&unif_header, 0, UNIF_HEADER_LEN);
  memcpy (&unif_header.signature, UNIF_SIG_S, 4);
  unif_header.revision = me2le_32 (UNIF_REVISION);
  fwrite (&unif_header, 1, UNIF_HEADER_LEN, destfile);

  unif_chunk.id = MAPR_ID;
  if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0)
    {
      fprintf (stderr, "ERROR: No board name specified\n");
      return -1;
    }
  unif_chunk.length = strlen (ucon64.mapr) + 1; // +1 to include ASCII-z
  unif_chunk.data = (void *) ucon64.mapr;
  if (unif_chunk.length > BOARDNAME_MAXLEN)
    {                                           // Should we give a warning?
      unif_chunk.length = BOARDNAME_MAXLEN;
      ((char *) unif_chunk.data)[BOARDNAME_MAXLEN - 1] = 0;
    }                                           // make it an ASCII-z string
  write_chunk (&unif_chunk, destfile);

#if     UNIF_REVISION > 7
  if (ucon64.comment != NULL && strlen (ucon64.comment) > 0)
    {
      unif_chunk.id = READ_ID;
      unif_chunk.length = strlen (ucon64.comment) + 1; // +1 to include ASCII-z
      unif_chunk.data = (void *) ucon64.comment;
      write_chunk (&unif_chunk, destfile);
    }

  // WRTR chunk can be helpful when debugging
  unif_chunk.id = WRTR_ID;
  unif_chunk.length = strlen (unif_ucon64_sig) + 1;
  unif_chunk.data = (char *) unif_ucon64_sig;
  write_chunk (&unif_chunk, destfile);
#else
  // READ chunk can be helpful when debugging
  unif_chunk.id = READ_ID;
  unif_chunk.length = strlen (unif_ucon64_sig) + 1;
  unif_chunk.data = (char *) unif_ucon64_sig;   // assume ASCII-z (spec is not clear)
  write_chunk (&unif_chunk, destfile);
#endif

  if (UCON64_ISSET (ucon64.tv_standard))
    {
      unif_chunk.id = TVCI_ID;
      unif_chunk.length = 1;
      b = ucon64.tv_standard;                   // necessary for big endian machines
      unif_chunk.data = &b;
      write_chunk (&unif_chunk, destfile);
    }

  if (UCON64_ISSET (ucon64.use_dump_info))
    {
      st_dumper_info_t info;
      if (ucon64.dump_info != NULL && strlen (ucon64.dump_info) > 0)
        {
          if (access (ucon64.dump_info, F_OK) == 0)
            {
              parse_info_file (&info, ucon64.dump_info);
#ifdef  WORDS_BIGENDIAN
              info.year = bswap_16 (info.year);
#endif
              unif_chunk.id = DINF_ID;
              unif_chunk.length = 204;
              unif_chunk.data = &info;
              write_chunk (&unif_chunk, destfile);
            }
          else
            printf ("WARNING: Dumper info file %s does not exist, chunk won't be written\n",
                    ucon64.dump_info);
        }
      else                                      // Is this a warning or an error?
        printf ("WARNING: No dumper info file was specified, chunk won't be written\n");
    }

  if (UCON64_ISSET (ucon64.controller))
    {
      unif_chunk.id = CTRL_ID;
      unif_chunk.length = 1;
      b = ucon64.controller;                    // necessary for big endian machines
      unif_chunk.data = &b;
      write_chunk (&unif_chunk, destfile);
    }

  x = crc32 (0, prg_data, prg_size);
  unif_chunk.id = PCK0_ID;
  unif_chunk.length = 4;
#ifdef  WORDS_BIGENDIAN
  x = bswap_32 (x);
#endif
  unif_chunk.data = &x;
  write_chunk (&unif_chunk, destfile);

  unif_chunk.id = PRG0_ID;                      // How to detect that x > 0 for PRGx?
  unif_chunk.length = prg_size;
  unif_chunk.data = prg_data;
  write_chunk (&unif_chunk, destfile);

  if (chr_size > 0)
    {
      x = crc32 (0, chr_data, chr_size);
      unif_chunk.id = CCK0_ID;
      unif_chunk.length = 4;
#ifdef  WORDS_BIGENDIAN
      x = bswap_32 (x);
#endif
      unif_chunk.data = &x;
      write_chunk (&unif_chunk, destfile);

      unif_chunk.id = CHR0_ID;                  // How to detect that x > 0 for CHRx?
      unif_chunk.length = chr_size;
      unif_chunk.data = chr_data;
      write_chunk (&unif_chunk, destfile);
    }

  b = 0;                                        // this is a dummy
  unif_chunk.id = BATR_ID;
  unif_chunk.length = 1;
  unif_chunk.data = &b;
  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        write_chunk (&unif_chunk, destfile);
    }
  else if (ines_header.ctrl1 & INES_SRAM)
    write_chunk (&unif_chunk, destfile);

  if (UCON64_ISSET (ucon64.vram))
    {
      if (ucon64.vram)
        {
          b = 0;                                // this is a dummy
          unif_chunk.id = VROR_ID;
          unif_chunk.length = 1;
          unif_chunk.data = &b;
          write_chunk (&unif_chunk, destfile);
        }
    }

  unif_chunk.id = MIRR_ID;
  unif_chunk.length = 1;
  if (UCON64_ISSET (ucon64.mirror))
    {
      if (ucon64.mirror > 5)
        {
          ucon64.mirror = 5;
          printf ("WARNING: Invalid mirroring type specified, using \"%d\"\n",
                  ucon64.mirror);
        }
      b = ucon64.mirror;                        // necessary for big endian machines
      unif_chunk.data = &b;
      write_chunk (&unif_chunk, destfile);
    }
  else if (ines_header.ctrl1 & (INES_MIRROR | INES_4SCREEN))
    {
      if (ines_header.ctrl1 & INES_MIRROR)
        b = 1;
      else                                      // it must be INES_4SCREEN
        b = 4;
      unif_chunk.data = &b;
      write_chunk (&unif_chunk, destfile);
    }

  free (prg_data);
  free (chr_data);                              // free() accepts case "chr_data == NULL"
  return 0;
}


static int
nes_unif_unif (unsigned char *rom_buffer, FILE *destfile)
{
  int x, n;
  st_unif_chunk_t *unif_chunk1, unif_chunk2, *unif_chunk3;
  unsigned char b;

  x = me2le_32 (unif_header.revision);
  if (x > UNIF_REVISION)
    printf ("WARNING: The UNIF file is of a revision later than %d (%d), but uCON64\n"
            "         doesn't support that revision yet. Some chunks may be discarded.\n",
            UNIF_REVISION, x);
  unif_header.revision = me2le_32 (UNIF_REVISION);
  memcpy (&unif_header.signature, UNIF_SIG_S, 4);
  fwrite (&unif_header, 1, UNIF_HEADER_LEN, destfile);

  if ((unif_chunk1 = read_chunk (MAPR_ID, rom_buffer, 0)) == NULL || // no MAPR chunk
      (ucon64.mapr != NULL && strlen (ucon64.mapr) > 0))             // MAPR, but has to change
    {
      unif_chunk2.id = MAPR_ID;
      if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0) // unif_chunk1 == NULL
        {
          fprintf (stderr, "ERROR: File has no MAPR chunk, but no board name was specified\n");
          return -1;
        }

      // ucon64.mapr != NULL && strlen (ucon64.mapr) > 0
      unif_chunk2.length = strlen (ucon64.mapr) + 1; // +1 to include ASCII-z
      unif_chunk2.data = (void *) ucon64.mapr;
      if (unif_chunk2.length > BOARDNAME_MAXLEN)
        {
          unif_chunk2.length = BOARDNAME_MAXLEN;
          ((char *) unif_chunk2.data)[BOARDNAME_MAXLEN - 1] = 0;
        }                                       // make it an ASCII-z string
      write_chunk (&unif_chunk2, destfile);
    }
  else                                          // MAPR chunk, but no board name specified
    {
      printf ("WARNING: No board name specified, using old value\n");
      write_chunk (unif_chunk1, destfile);
    }
  free (unif_chunk1);                           // case NULL is valid

#if     UNIF_REVISION > 7
  if (ucon64.comment != NULL && strlen (ucon64.comment) > 0)
    {
      unif_chunk2.id = READ_ID;
      unif_chunk2.length = strlen (ucon64.comment) + 1; // +1 to include ASCII-z
      unif_chunk2.data = (void *) ucon64.comment;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      if ((unif_chunk1 = read_chunk (READ_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if ((unif_chunk1 = read_chunk (WRTR_ID, rom_buffer, 0)) == NULL)
    {
      unif_chunk2.id = WRTR_ID;
      unif_chunk2.length = strlen (unif_ucon64_sig) + 1;
      unif_chunk2.data = (char *) unif_ucon64_sig;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      char ucon64_name[] = "uCON64";
      int sig_added = 0;
      // find uCON64 WRTR chunk and modify it if it is present
      do
        {
          if (!strncmp ((const char *) unif_chunk1->data, ucon64_name, strlen (ucon64_name)))
            {
              unif_chunk1->length = strlen (unif_ucon64_sig) + 1;
              unif_chunk1->data = (char *) unif_ucon64_sig;
              sig_added = 1; // don't break, because we want to preserve other WRTR chunks
            }
          write_chunk (unif_chunk1, destfile);
          free (unif_chunk1);
        }
      while ((unif_chunk1 = read_chunk (WRTR_ID, rom_buffer, 1)) != NULL);

      if (!sig_added)
        {
          unif_chunk2.id = WRTR_ID;
          unif_chunk2.length = strlen (unif_ucon64_sig) + 1;
          unif_chunk2.data = (char *) unif_ucon64_sig;
          write_chunk (&unif_chunk2, destfile);
        }
    }
#else
  if ((unif_chunk1 = read_chunk (READ_ID, rom_buffer, 0)) == NULL)
    {
      unif_chunk2.id = READ_ID;
      unif_chunk2.length = strlen (unif_ucon64_sig) + 1;
      unif_chunk2.data = (char *) unif_ucon64_sig;
      write_chunk (&unif_chunk2, destfile);     // assume ASCII-z (spec is not clear)
    }
  else
    {
      if (!strncmp (unif_chunk1->data, STD_COMMENT, strlen (STD_COMMENT)))
        { // overwrite uCON64 comment -> OS and version match with the used exe
          unif_chunk1->length = strlen (unif_ucon64_sig) + 1;
          unif_chunk1->data = (char *) unif_ucon64_sig;
        }
      write_chunk (unif_chunk1, destfile);
    }
  free (unif_chunk1);
#endif

  if (internal_name != NULL)
    {
      unif_chunk2.id = NAME_ID;
      unif_chunk2.length = strlen (internal_name) + 1;
      unif_chunk2.data = (char *) internal_name;
      write_chunk (&unif_chunk2, destfile);     // assume ASCII-z (spec is not clear)
    }
  else
    {
      if ((unif_chunk1 = read_chunk (NAME_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.tv_standard))
    {
      unif_chunk2.id = TVCI_ID;
      unif_chunk2.length = 1;
      b = ucon64.tv_standard;                   // necessary for big endian machines
      unif_chunk2.data = &b;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      if ((unif_chunk1 = read_chunk (TVCI_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.use_dump_info))
    {
      st_dumper_info_t info;
      if (ucon64.dump_info != NULL && strlen (ucon64.dump_info) > 0)
        {
          if (access (ucon64.dump_info, F_OK) == 0)
            {
              parse_info_file (&info, ucon64.dump_info);
/*
              printf ("Dump info:\n"
                      "  Dumper: %s\n"
                      "  Date: %d-%d-%02d\n"
                      "  Agent: %s\n",
                      info.dumper_name,
                      info.day, info.month, info.year,
                      info.dumper_agent);
*/
#ifdef  WORDS_BIGENDIAN
              info.year = bswap_16 (info.year);
#endif
              unif_chunk2.id = DINF_ID;
              unif_chunk2.length = 204;
              unif_chunk2.data = &info;
              write_chunk (&unif_chunk2, destfile);
            }
          else
            printf ("WARNING: Dumper info file %s does not exist, chunk won't be written\n",
                    ucon64.dump_info);
        }
      else                                      // Is this a warning or an error?
        printf ("WARNING: No dumper info file was specified, chunk won't be written\n");
    }
  else
    {
      if ((unif_chunk1 = read_chunk (DINF_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.controller))
    {
      unif_chunk2.id = CTRL_ID;
      unif_chunk2.length = 1;
      b = ucon64.controller;                    // necessary for big endian machines
      unif_chunk2.data = &b;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      if ((unif_chunk1 = read_chunk (CTRL_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  // copy PRG chunks
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk1 = read_chunk (unif_prg_ids[n], rom_buffer, 0)) != NULL)
        {
          if ((unif_chunk3 = read_chunk (unif_pck_ids[n], rom_buffer, 0)) == NULL)
            {
              x = crc32 (0, (unsigned char *) unif_chunk1->data, unif_chunk1->length);
              unif_chunk2.id = unif_pck_ids[n];
              unif_chunk2.length = 4;
#ifdef  WORDS_BIGENDIAN
              x = bswap_32 (x);
#endif
              unif_chunk2.data = &x;
              write_chunk (&unif_chunk2, destfile);
            }
          else
            {
              x = crc32 (0, (unsigned char *) unif_chunk1->data, unif_chunk1->length);
#ifdef  WORDS_BIGENDIAN
              x = bswap_32 (x);
#endif
              if (x != *((int *) unif_chunk3->data))
                printf ("WARNING: PRG chunk %d has a bad checksum, writing new checksum\n", n);
              unif_chunk3->length = 4;
              unif_chunk3->data = &x;
              write_chunk (unif_chunk3, destfile);
            }
          free (unif_chunk3);
          write_chunk (unif_chunk1, destfile);
        }
      free (unif_chunk1);
    }

  // copy CHR chunks
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk1 = read_chunk (unif_chr_ids[n], rom_buffer, 0)) != NULL)
        {
          if ((unif_chunk3 = read_chunk (unif_cck_ids[n], rom_buffer, 0)) == NULL)
            {
              x = crc32 (0, (unsigned char *) unif_chunk1->data, unif_chunk1->length);
              unif_chunk2.id = unif_cck_ids[n];
              unif_chunk2.length = 4;
#ifdef  WORDS_BIGENDIAN
              x = bswap_32 (x);
#endif
              unif_chunk2.data = &x;
              write_chunk (&unif_chunk2, destfile);
            }
          else
            {
              x = crc32 (0, (unsigned char *) unif_chunk1->data, unif_chunk1->length);
#ifdef  WORDS_BIGENDIAN
              x = bswap_32 (x);
#endif
              if (x != *((int *) unif_chunk3->data))
                printf ("WARNING: CHR chunk %d has a bad checksum, writing new checksum\n", n);
              unif_chunk3->length = 4;
              unif_chunk3->data = &x;
              write_chunk (unif_chunk3, destfile);
            }
          free (unif_chunk3);
          write_chunk (unif_chunk1, destfile);
        }
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        {
          b = 0;                                // this is a dummy
          unif_chunk2.id = BATR_ID;
          unif_chunk2.length = 1;
          unif_chunk2.data = &b;
          write_chunk (&unif_chunk2, destfile);
        }
    }
  else
    {
      if ((unif_chunk1 = read_chunk (BATR_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.vram))
    {
      if (ucon64.vram)
        {
          b = 0;                                // this is a dummy
          unif_chunk2.id = VROR_ID;
          unif_chunk2.length = 1;
          unif_chunk2.data = &b;
          write_chunk (&unif_chunk2, destfile);
        }
    }
  else
    {
      if ((unif_chunk1 = read_chunk (VROR_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  if (UCON64_ISSET (ucon64.mirror))
    {
      if (ucon64.mirror > 5)
        {
          ucon64.mirror = 5;
          printf ("WARNING: Invalid mirroring type specified, using \"%d\"\n",
                  ucon64.mirror);
        }
      unif_chunk2.id = MIRR_ID;
      unif_chunk2.length = 1;
      b = ucon64.mirror;                        // necessary for big endian machines
      unif_chunk2.data = &b;
      write_chunk (&unif_chunk2, destfile);
    }
  else
    {
      if ((unif_chunk1 = read_chunk (MIRR_ID, rom_buffer, 0)) != NULL)
        write_chunk (unif_chunk1, destfile);
      free (unif_chunk1);
    }

  return 0;
}


int
nes_unif (void)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *rom_buffer;
  FILE *srcfile, *destfile;

  if (type != INES && type != UNIF)
    {
      if (type == PASOFAMI)
        fprintf (stderr, "ERROR: Pasofami -> UNIF is currently not supported\n");
      else if (type == FFE)
        fprintf (stderr, "ERROR: FFE -> UNIF is currently not supported\n");
      else if (type == FDS || type == FAM)
        fprintf (stderr, "ERROR: FDS/FAM -> UNIF is currently not supported\n");
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".unf");
  strcpy (src_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }

  /*
    remove possible temp file created by ucon64_file_handler ()
    nes_ines_unif() and nes_unif_unif() might exit() so we use register_func()
  */
  register_func (remove_temp_file);
  nes_destfname = dest_name;
  nes_destfile = destfile;
  register_func (remove_destfile);
  /*
    Converting from UNIF to UNIF should be allowed, because the user might want
    to change some paramaters.
  */
  if (type == INES)
    {
      if ((srcfile = fopen (src_name, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
          return -1;
        }

      if (nes_ines_unif (srcfile, destfile) == -1) // -1 == error
        exit (1);

      fclose (srcfile);
    }
  else if (type == UNIF)
    {
      if ((rom_buffer = (unsigned char *) malloc (rom_size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], rom_size);
          exit (1);
        }
      ucon64_fread (rom_buffer, UNIF_HEADER_LEN, rom_size, src_name);

      if (nes_unif_unif (rom_buffer, destfile) == -1) // -1 == error
        exit (1);

      free (rom_buffer);
    }

  unregister_func (remove_destfile);
  fclose (destfile);
  unregister_func (remove_temp_file);
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static void
set_mapper (st_ines_header_t *header, unsigned int mapper)
{
  header->ctrl1 &= 0x0f;                        // clear mapper bits
  header->ctrl2 &= 0x03;                        // clear reserved and mapper bits

  header->ctrl1 |= mapper << 4;
  header->ctrl2 |= mapper & 0xf0;
  if (mapper > 0xff)                            // we support mapper numbers > 255
    {
      if (mapper > 0xfff)
        {
          fprintf (stderr, "ERROR: Mapper numbers greater than 4095 can't be stored\n");
          exit (1);
        }
      // We can't just clear bits 0 & 1 of ctrl2, because they have their own
      //  meaning. So, a warning is in place here.
      printf ("WARNING: Mapper number is greater than 255\n");
      header->ctrl2 |= (mapper >> 8) & 0xf;
    }
}


static int
nes_ines_ines (FILE *srcfile, FILE *destfile, int deinterleave)
{
  int prg_size, chr_size;
  unsigned char *prg_data = NULL, *chr_data = NULL;

  // read iNES file
  fread (&ines_header, 1, INES_HEADER_LEN, srcfile);
  if (ines_header.ctrl1 & INES_TRAINER)
    {
      fseek (srcfile, 512, SEEK_CUR);           // discard trainer data
      ines_header.ctrl1 &= ~INES_TRAINER;       // clear trainer bit
    }

  prg_size = ines_header.prg_size << 14;
  prg_size = read_block (&prg_data, prg_size, srcfile,
                         "ERROR: Not enough memory for PRG buffer (%d bytes)\n", prg_size);
  chr_size = ines_header.chr_size << 13;
  if (chr_size > 0)
    chr_size = read_block (&chr_data, chr_size, srcfile,
                           "ERROR: Not enough memory for CHR buffer (%d bytes)\n", chr_size);

  if (deinterleave)
    /*
      This is a bit of a hack, i.e., putting the following code in this
      function. AFAIK (dbjh) interleaved images contain one big block of data:
      ROM data at even addresses (offsets in the file) and VROM data at odd
      addresses. Currently uCON64 supports deinterleaving of iNES files only,
      so we have to handle the improbable case that the source ROM file
      contains two blocks of interleaved ROM/VROM data.
      Due to the way the data is stored the ROM data will have the same size
      as the VROM data.
    */
    {
      unsigned char *data;
      int size, n = 0, prg = 0, chr = 0;

      size = prg_size + chr_size;
      if ((data = (unsigned char *) malloc (size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
          exit (1);
        }
      memcpy (data, prg_data, prg_size);
      memcpy (data + prg_size, chr_data, chr_size);

      if (prg_size != chr_size)
        {
          free (prg_data);
          free (chr_data);
          prg_size = chr_size = size / 2;
          prg_data = (unsigned char *) malloc (prg_size);
          chr_data = (unsigned char *) malloc (chr_size);
          if (prg_data == NULL || chr_data == NULL)
            {
              fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
              exit (1);
            }
        }

      while (n < size)
        {                                       // deinterleave
          prg_data[prg++] = data[n++];
          chr_data[chr++] = data[n++];
        }
      free (data);
    }

  // write iNES file
  if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0)
    printf ("WARNING: No mapper number specified, using old value\n");
  else                                          // mapper specified
    set_mapper (&ines_header, strtol (ucon64.mapr, NULL, 10));
  memcpy (&ines_header.signature, INES_SIG_S, 4);
  ines_header.prg_size = prg_size >> 14;
  ines_header.chr_size = chr_size >> 13;

  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        ines_header.ctrl1 |= INES_SRAM;
      else
        ines_header.ctrl1 &= ~INES_SRAM;
    }

  if (UCON64_ISSET (ucon64.mirror))
    {
      ines_header.ctrl1 &= ~(INES_MIRROR | INES_4SCREEN); // clear bits
      if (ucon64.mirror == 0)
        ;                                       // default value in ctrl1 (0) is ok
      else if (ucon64.mirror == 1)
        ines_header.ctrl1 |= INES_MIRROR;
      else if (ucon64.mirror == 4)
        ines_header.ctrl1 |= INES_4SCREEN;
      else
        printf ("WARNING: Invalid mirroring type specified, using \"0\"\n");
    }

  ines_header.reserved1 = 0;
  ines_header.reserved2 = 0;
  fwrite (&ines_header, 1, INES_HEADER_LEN, destfile);
  fwrite (prg_data, 1, prg_size, destfile);
  fwrite (chr_data, 1, chr_size, destfile);

  free (prg_data);
  free (chr_data);
  return 0;
}


static int
nes_mapper_number (const char *board_name)
{
  typedef struct
  {
    const char *string;
    int value;
  } st_string_value_t;

  int n;
  st_string_value_t name_to_mapr[] =            // TODO: expand this list
    {
      { "NROM", 0 },
      { "NES-RROM", 0 },
      { "SNROM", 1 },
      { "SOROM", 1 },
      { "SVROM", 1 },
      { "SUROM", 1 },
      { "SAROM", 1 },
      { "SBROM", 1 },
      { "UNROM", 2 },
      { "CNROM", 3 },
      { "TEROM", 4 },
      { "TFROM", 4 },
      { "TGROM", 4 },
      { "TVROM", 4 },
      { "TSROM", 4 },
      { "TQROM", 4 },
      { "TKROM", 4 },
      { "TLSROM", 4 },
      { "DRROM", 4 },
      { "TLROM", 4 },
      { "SL1ROM", 4 },
      { "SL2ROM", 4 },
      { "SL3ROM", 4 },
      { "ELROM", 5 },
      { "ETROM", 5 },
      { "EWROM", 5 },
      { "AOROM", 7 },
      { "PNROM", 9 },
      { NULL, 0 }
    };

  n = 0;
  while (name_to_mapr[n].string != NULL)
    {
      if (!strncmp (board_name, name_to_mapr[n].string, BOARDNAME_MAXLEN - 1))
        return name_to_mapr[n].value;
      n++;
    }

  return -1;
}


static int
nes_unif_ines (unsigned char *rom_buffer, FILE *destfile)
{
  int n, x, prg_size = 0, chr_size = 0;
  st_unif_chunk_t *unif_chunk;

  x = me2le_32 (unif_header.revision);
  if (x > UNIF_REVISION)
    printf ("WARNING: The UNIF file is of a revision later than %d (%d), but uCON64\n"
            "         doesn't support that revision yet. Some chunks may be discarded.\n",
            UNIF_REVISION, x);

  // build iNES header
  memset (&ines_header, 0, INES_HEADER_LEN);
  memcpy (&ines_header.signature, INES_SIG_S, 4);

  if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0)
    {                                           // no mapper specified, try autodetection
      if ((unif_chunk = read_chunk (MAPR_ID, rom_buffer, 0)) != NULL)
        {
          if ((x = nes_mapper_number ((const char *) unif_chunk->data)) == -1)
            {
              printf ("WARNING: Couldn't determine mapper number, writing \"0\"\n");
              x = 0;
            }
          set_mapper (&ines_header, x);
          free (unif_chunk);
        }
      else                                      // no MAPR chunk
        {
          fprintf (stderr, "ERROR: File has no MAPR chunk, but no mapper number was specified\n");
          return -1;
        }
    }
  else                                          // mapper specified
    set_mapper (&ines_header, strtol (ucon64.mapr, NULL, 10));

  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        ines_header.ctrl1 |= INES_SRAM;
      else
        ines_header.ctrl1 &= ~INES_SRAM;
    }
  else
    {
      if ((unif_chunk = read_chunk (BATR_ID, rom_buffer, 0)) != NULL)
        ines_header.ctrl1 |= INES_SRAM;
      free (unif_chunk);
    }

  if (UCON64_ISSET (ucon64.mirror))
    {
      if (ucon64.mirror == 0)
        ;                                       // default value in ctrl1 (0) is ok
      else if (ucon64.mirror == 1)
        ines_header.ctrl1 |= INES_MIRROR;
      else if (ucon64.mirror == 4)
        ines_header.ctrl1 |= INES_4SCREEN;
      else
        printf ("WARNING: Invalid mirroring type specified, using \"0\"\n");
    }
  else if ((unif_chunk = read_chunk (MIRR_ID, rom_buffer, 0)) != NULL)
    {
      switch (*((unsigned char *) unif_chunk->data))
        {
        case 0:                                 // default value in ctrl1 (0) is ok
        case 2:                                 // can't express in iNES terms
        case 3:                                 // idem
        case 5:                                 // idem
          break;
        case 1:
          ines_header.ctrl1 |= INES_MIRROR;
          break;
        case 4:
          ines_header.ctrl1 |= INES_4SCREEN;
          break;
        default:
          printf ("WARNING: Unsupported value in MIRR chunk\n");
          break;
        }
      free (unif_chunk);
    }

  /*
    Determining the PRG & CHR sizes could be done in the copy loops. Doing it
    here is quite inefficient, but now we can write to a zlib stream. zlib
    doesn't support backward seeks in a compressed output stream. A backward
    seek would be necessary if the size calculation would be done in the copy
    loops.
  */
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk = read_chunk (unif_prg_ids[n], rom_buffer, 0)) != NULL)
        prg_size += unif_chunk->length;
      free (unif_chunk);
      if ((unif_chunk = read_chunk (unif_chr_ids[n], rom_buffer, 0)) != NULL)
        chr_size += unif_chunk->length;
      free (unif_chunk);
    }

  // write header
  ines_header.prg_size = prg_size >> 14;        // # 16 kB banks
  ines_header.chr_size = chr_size >> 13;        // # 8 kB banks
  fwrite (&ines_header, 1, INES_HEADER_LEN, destfile);

  // copy PRG data
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk = read_chunk (unif_prg_ids[n], rom_buffer, 0)) != NULL)
        fwrite (unif_chunk->data, 1, unif_chunk->length, destfile);
      free (unif_chunk);
    }

  // copy CHR data
  for (n = 0; n < 16; n++)
    {
      if ((unif_chunk = read_chunk (unif_chr_ids[n], rom_buffer, 0)) != NULL)
        fwrite (unif_chunk->data, 1, unif_chunk->length, destfile);
      free (unif_chunk);
    }

  return 0;
}


int
nes_ines (void)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *rom_buffer;
  FILE *srcfile, *destfile;

  if (type == FFE)
    {
      fprintf (stderr, "ERROR: FFE -> iNES is currently not supported\n");
      return -1;
    }
  else if (type == FDS || type == FAM)
    {
      fprintf (stderr, "ERROR: FDS/FAM -> iNES is not possible\n");
      return -1;
    }

  // Pasofami doesn't fit well in the source -> destination "paradigm"
  if (type == PASOFAMI)
    return nes_j (NULL);

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".nes");
  strcpy (src_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }

  register_func (remove_temp_file);
  nes_destfname = dest_name;
  nes_destfile = destfile;
  register_func (remove_destfile);
  if (type == INES)
    {
      if ((srcfile = fopen (src_name, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
          return -1;
        }

      if (nes_ines_ines (srcfile, destfile, 0) == -1) // -1 == error
        exit (1);                       // calls remove_temp_file() & remove_destfile()

      fclose (srcfile);
    }
  else if (type == UNIF)
    {
      if ((rom_buffer = (unsigned char *) malloc (rom_size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], rom_size);
          exit (1);
        }
      ucon64_fread (rom_buffer, UNIF_HEADER_LEN, rom_size, src_name);

      if (nes_unif_ines (rom_buffer, destfile) == -1) // -1 == error
        exit (1);                       // calls remove_temp_file() & remove_destfile()

      free (rom_buffer);
    }

  unregister_func (remove_destfile);
  fclose (destfile);
  unregister_func (remove_temp_file);
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
nes_pasofami (void)
{
  // nes_s() does iNES -> Pasofami. nes_s() checks for type
  return nes_s ();
}


int
nes_ffe (st_rominfo_t *rominfo)
{
  st_smc_header_t smc_header;
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  int size = ucon64.file_size - rominfo->buheader_len, mapper,
      prg_size, chr_size, new_prg_size = -1;

  if (type != INES)
    {
      fprintf (stderr, "ERROR: Currently only iNES -> FFE is supported\n");
      return -1;
    }

  ucon64_fread (&ines_header, 0, INES_HEADER_LEN, ucon64.rom);

  mapper = ines_header.ctrl1 >> 4 | (ines_header.ctrl2 & 0xf0);
  prg_size = ines_header.prg_size << 14;
  if (prg_size > size)
    prg_size = size;
  chr_size = ines_header.chr_size << 13;

  memset (&smc_header, 0, SMC_HEADER_LEN);

  switch (mapper)
    {
    case 0:
      if ((prg_size == 32 * 1024 || prg_size == 16 * 1024) && chr_size == 8 * 1024)
        {
          if (prg_size == 16 * 1024)
            new_prg_size = 32 * 1024;
          smc_header.emulation1 = 0x80;
          smc_header.emulation2 = 0xea;
        }
      else if (prg_size == 32 * 1024 && chr_size == 16 * 1024)
        {
          smc_header.emulation1 = 0x80;
          smc_header.emulation2 = 0xce;
        }
      break;
    case 2:
    case 71:
      /*
        Some mapper 71 games need to be patched before they work:
        [0x0e] <= 0x40
        [0x12] <= 0x50
      */
      if (prg_size == 128 * 1024 && chr_size == 0)
        {
          smc_header.emulation1 = 0x80;
          smc_header.emulation2 = 0;
        }
      else if (prg_size == 256 * 1024 && chr_size == 0)
        {
          smc_header.emulation1 = 0x80;
          smc_header.emulation2 = 0x40;
        }
      break;
    case 3:
      if (prg_size == 32 * 1024 && chr_size == 32 * 1024)
        {
          smc_header.emulation1 = 0x80;
          smc_header.emulation2 = 0xb7;
        }
      break;
#if 0
    case ?:
      if (prg_size == 128 * 1024 && chr_size == 32 * 1024)
        {
          smc_header.emulation1 = 0x80;
          smc_header.emulation2 = 0x80;
        }
      break;
#endif
    case 6:                                     // falling through
    case 8:                                     // falling through
    case 12:
      /*
        According to kyuusaku the following headers only apply to "trained"
        dumps. "Trainers tell the copier how to play the game and how many rom
        banks there are and where they are on a game by game basis."
        However, I (dbjh) subtracted 0x40 from emulation1 and moved the code
        that sets bit SMC_TRAINER (0x40) to general code.
        Note that we discard trainer data when converting to iNES and UNIF.
      */
      if (size == 1 * MBIT)
        {
          smc_header.emulation1 = 0x80;
          smc_header.emulation2 = 0;
        }
      else if (size == 2 * MBIT)
        {
          smc_header.emulation1 = 0x80;
          smc_header.emulation2 = 0x20;
        }
      else if (size == 3 * MBIT)
        {
          smc_header.emulation1 = 0xa0;
          smc_header.emulation2 = 0x20;
        }
      else if (size == 4 * MBIT)
        {
          smc_header.emulation1 = 0xb0;
          smc_header.emulation2 = 0x20;
        }
      break;
    }

  if (ines_header.ctrl1 & INES_TRAINER)
    smc_header.emulation1 |= SMC_TRAINER;

  smc_header.id1 = 0xaa;
  smc_header.id2 = 0xbb;
#if 0                                           // already set to 0 by memset()
  smc_header.type = 0;
#endif

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".ffe");
  ucon64_file_handler (dest_name, src_name, 0);

  if (new_prg_size == -1)                       // don't resize PRG block
    {
      ucon64_fwrite (&smc_header, 0, SMC_HEADER_LEN, dest_name, "wb");
      fcopy (src_name, rominfo->buheader_len, size, dest_name, "ab");
    }
  else
    {
      unsigned char *prg_data = (unsigned char *) malloc (new_prg_size);
      int offset;

      if (prg_data == NULL)
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], new_prg_size);
          exit (1);
        }
      memset (prg_data, 0, new_prg_size);       // pad with zeroes
      ucon64_fread (prg_data, rominfo->buheader_len, prg_size, src_name);

      // write header
      ucon64_fwrite (&smc_header, 0, SMC_HEADER_LEN, dest_name, "wb");

      // copy trainer data if present
      if (ines_header.ctrl1 & INES_TRAINER)
        {
          fcopy (src_name, rominfo->buheader_len, 512, dest_name, "ab");
          offset = 512;
        }
      else
        offset = 0;

      // write resized PRG block
      ucon64_fwrite (prg_data, SMC_HEADER_LEN + offset, new_prg_size, dest_name, "ab");

      // copy CHR block
      fcopy (src_name, rominfo->buheader_len + offset + prg_size,
              size - offset - prg_size, dest_name, "ab");

      free (prg_data);
    }

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
nes_ineshd (st_rominfo_t *rominfo)
{
  char dest_name[FILENAME_MAX];

  if (type != INES)
    {
      fprintf (stderr, "ERROR: This option is only meaningful for iNES files\n");
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".hdr");
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.rom, rominfo->buheader_start, 16, dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
nes_dint (void)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  FILE *srcfile, *destfile;

  if (type != INES)
    {
      // Do interleaved UNIF or Pasofami images exist?
      fprintf (stderr, "ERROR: Currently only iNES images can be deinterleaved\n");
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".nes");
  strcpy (src_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);
  if ((srcfile = fopen (src_name, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], src_name);
      return -1;
    }
  if ((destfile = fopen (dest_name, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], dest_name);
      return -1;
    }

  register_func (remove_temp_file);
  nes_destfname = dest_name;
  nes_destfile = destfile;
  register_func (remove_destfile);
  // type == INES
  if (nes_ines_ines (srcfile, destfile, 1) == -1) // -1 == error
    exit (1);                           // calls remove_temp_file() & remove_destfile()

  unregister_func (remove_destfile);
  fclose (srcfile);
  fclose (destfile);
  unregister_func (remove_temp_file);
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


static int
parse_prm (st_ines_header_t *header, const char *fname)
/*
  Parse a .PRM file. The format has probably been designed be some mentally
  impaired person. I am not mentally impaired, so I just read the information
  that I can use :-)
  The meaning of the bytes comes from Marat Fayzullin's NESLIST.
*/
{
  unsigned char prm[71];                        // .PRM files are 71 bytes in size

  ucon64_fread (prm, 0, 71, fname);

  if (prm[0] == 'V')                            // mirroring
    header->ctrl1 |= INES_MIRROR;
  else if (prm[0] == 'H')
    header->ctrl1 &= ~INES_MIRROR;

  if (prm[1] == 'T')                            // ROM mapper type (2/4/not specified)
    set_mapper (header, 2);
  else if (prm[1] == 'N')
    set_mapper (header, 4);
  else
    set_mapper (header, 0);

  // ignore VROM mapper type ('T' == mapper 2 or 4, nice design)
  // ignore music mode
  // ignore "something related to graphics"
  // ignore "unknown"
  // ignore "validity?"
  // ignore "IRQ control?"
  // ignore "something related to graphics"
  // ignore "display validity?"
  // ignore speed (NMI) control (always 'S')
  // ignore default sprite size (always 'L')
  // ignore default foreground/background (always 'R')
  // ignore "break order?"

  if (prm[14] == 'E')                           // preserve extension RAM
    header->ctrl1 |= INES_SRAM;
  else
    header->ctrl1 &= ~INES_SRAM;

  // ignore "unknown"
  // ignore "something related to interrupts" (always 'S')

  if (prm[17] != 'M')                           // bank-switched ROM?
    set_mapper (header, 0);

  // ignore 9 unknown bytes
  // ignore "partial horizontal scroll?" (always 'X')
  // ignore "don't scroll up to this scanline?" (always "02")
  // ignore "line to do a scroll in?" (always '2')
  // ignore "comment?" (always 'A')

  return 0;
}


int
nes_j (unsigned char **mem_image)
/*
  The Pasofami format consists of several files:
  - .PRM: header (uCON64 treats it as optional in order to support RAW images)
  - .700: trainer data (optional)
  - .PRG: ROM data
  - .CHR: VROM data (optional)
*/
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *buffer;
  int prg_size = 0, chr_size = 0, write_file = 0, size, bytes_read = 0, nparts = 0;

  if (type != PASOFAMI)
    {
      fprintf (stderr, "ERROR: Only Pasofami files can be joined (for NES)\n");
      return -1;
    }

  if (mem_image == NULL)
    write_file = 1;

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".nes");
  if (write_file)
    ucon64_file_handler (dest_name, NULL, 0);

  // build iNES header
  memset (&ines_header, 0, INES_HEADER_LEN);
  memcpy (&ines_header.signature, INES_SIG_S, 4);

  strcpy (src_name, ucon64.rom);
  set_suffix (src_name, ".prm");
  if (access (src_name, F_OK) == 0)
    {
      parse_prm (&ines_header, src_name);
      nparts++;
    }
  else if (write_file)                          // Don't print this from nes_init()
    printf ("WARNING: No %s, using default values\n", src_name);

  // Don't do this in parse_prm(), because there might be no .PRM file available
  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        ines_header.ctrl1 |= INES_SRAM;
      else
        ines_header.ctrl1 &= ~INES_SRAM;
    }

  if (UCON64_ISSET (ucon64.mirror))
    {
      ines_header.ctrl1 &= ~(INES_MIRROR | INES_4SCREEN); // clear bits
      if (ucon64.mirror == 0)
        ;                                       // default value in ctrl1 (0) is ok
      else if (ucon64.mirror == 1)
        ines_header.ctrl1 |= INES_MIRROR;
      else if (ucon64.mirror == 4)
        ines_header.ctrl1 |= INES_4SCREEN;
      else
        printf ("WARNING: Invalid mirroring type specified, using \"0\"\n");
    }

  strcpy (src_name, ucon64.rom);
  set_suffix (src_name, ".700");
  if (access (src_name, F_OK) == 0 && fsizeof (src_name) >= 512)
    {
      ines_header.ctrl1 |= INES_TRAINER;
      nparts++;
    }

  set_suffix (src_name, ".prg");
  if (access (src_name, F_OK) != 0)             // .PRG file must exist, but
    {                                           //  not for nes_init()
      if (write_file)
        {
          fprintf (stderr, "ERROR: No %s, can't make image without it\n", src_name);
          exit (1);
        }
    }
  else
    {
      prg_size = fsizeof (src_name);
      nparts++;
    }
  ines_header.prg_size = prg_size >> 14;

  set_suffix (src_name, ".chr");
  if (access (src_name, F_OK) == 0)
    {
      chr_size = fsizeof (src_name);
      nparts++;
    }
  ines_header.chr_size = chr_size >> 13;

  if (ucon64.mapr == NULL || strlen (ucon64.mapr) == 0)
    {                                           // maybe .PRM contained mapper
      if (write_file)                           // Don't print this from nes_init()
        printf ("WARNING: No mapper number specified, writing \"%d\"\n",
                (ines_header.ctrl1 >> 4) | (ines_header.ctrl2 & 0xf0));
    }
  else  // mapper specified (override unreliable value from .PRM file)
    set_mapper (&ines_header, strtol (ucon64.mapr, NULL, 10));

  size = prg_size + chr_size + ((ines_header.ctrl1 & INES_TRAINER) ? 512 : 0);
  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      return -1;
    }

  if (ines_header.ctrl1 & INES_TRAINER)
    {
      set_suffix (src_name, ".700");
      ucon64_fread (buffer, 0, 512, src_name);  // use 512 bytes at max
      bytes_read = 512;
    }

  if (prg_size > 0)
    {
      set_suffix (src_name, ".prg");
      ucon64_fread (buffer + bytes_read, 0, prg_size, src_name);
      bytes_read += prg_size;
    }

  if (chr_size > 0)
    {
      set_suffix (src_name, ".chr");
      ucon64_fread (buffer + bytes_read, 0, chr_size, src_name);
    }

  if (write_file)
    {
      ucon64_fwrite (&ines_header, 0, INES_HEADER_LEN, dest_name, "wb");
      ucon64_fwrite (buffer, INES_HEADER_LEN, size, dest_name, "ab");
      printf (ucon64_msg[WROTE], dest_name);
      free (buffer);
    }
  else
    *mem_image = buffer;

  if (!UCON64_ISSET (ucon64.split))
    ucon64.split = nparts;

  return 0;
}


static int
write_prm (st_ines_header_t *header, const char *fname)
{
  unsigned char prm[72] =                       // .PRM files are 71 bytes in size (ASCII-z)
    "          SLR   S          X022A\r\n"
    "1234567890123456789012345678901234\r\n\x1a";
  int mapper;

  if (UCON64_ISSET (ucon64.mirror))
    {
      header->ctrl1 &= ~(INES_MIRROR | INES_4SCREEN); // clear bits
      if (ucon64.mirror == 0)
        ;                                       // default value in ctrl1 (0) is ok
      else if (ucon64.mirror == 1)
        header->ctrl1 |= INES_MIRROR;
// Pasofami only stores horizontal or vertical mirroring types
//    else if (ucon64.mirror == 4)
//      header->ctrl1 |= INES_4SCREEN;
      else
        printf ("WARNING: Invalid mirroring type specified, using \"0\"\n");
    }
  if (header->ctrl1 & INES_MIRROR)              // mirroring
    prm[0] = 'V';
  else
    prm[0] = 'H';

  mapper = (header->ctrl1 >> 4) | (header->ctrl2 & 0xf0);
  if (mapper % 16 == 2)                         // mod of 16 to do the same as NESLIST,
    {                                           //  but I (dbjh) think this is a bug
      prm[1] = 'T';                             // ROM mapper type (2/4/not specified)
      prm[2] = 'T';                             // VROM mapper type (2/4/not specified)
      prm[4] = 'C';                             // "something related to graphics"
    }                                           //  (2/4/not specified)
  else if (mapper % 16 == 4)
    {
      prm[1] = 'N';
      prm[2] = 'T';
      prm[4] = 'N';
    }

  if (UCON64_ISSET (ucon64.battery))
    {
      if (ucon64.battery)
        header->ctrl1 |= INES_SRAM;
      else
        header->ctrl1 &= ~INES_SRAM;
    }
  if (header->ctrl1 & INES_SRAM)                // preserve extension RAM
    prm[14] = 'E';

  if (mapper)                                   // bank-switched ROM?
    prm[17] = 'M';

  // don't write backups of parts, because one name is used
  if (ucon64_fwrite (prm, 0, sizeof (prm), fname, "wb") == -1)
    {
      fprintf (stderr, ucon64_msg[WRITE_ERROR], fname);
      return -1;                                // try to continue
    }
  else
    printf (ucon64_msg[WROTE], fname);

  return 0;
}


int
nes_s (void)
{
  char dest_name[FILENAME_MAX];
  unsigned char *trainer_data = NULL, *prg_data = NULL, *chr_data = NULL;
  int prg_size = 0, chr_size = 0, x;
  FILE *srcfile;

  if (type != INES)
    {
      fprintf (stderr, "ERROR: Currently only iNES -> Pasofami is supported\n");
      return -1;
    }

  if ((srcfile = fopen (ucon64.rom, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.rom);
      return -1;
    }

  // read iNES file
  fread (&ines_header, 1, INES_HEADER_LEN, srcfile);
  if (ines_header.ctrl1 & INES_TRAINER)
    {
      if (read_block (&trainer_data, 512, srcfile,
                      "ERROR: Not enough memory for trainer buffer (%d bytes)\n", 512) != 512)
        {
          fprintf (stderr, "ERROR: %s is not a valid iNES file", ucon64.rom);
          exit (1);
        }
    }
  prg_size = ines_header.prg_size << 14;
  prg_size = read_block (&prg_data, prg_size, srcfile,
                         "ERROR: Not enough memory for PRG buffer (%d bytes)\n", prg_size);
  chr_size = ines_header.chr_size << 13;
  if (chr_size > 0)
    chr_size = read_block (&chr_data, chr_size, srcfile,
                           "ERROR: Not enough memory for CHR buffer (%d bytes)\n", chr_size);

  if (ucon64.mapr != NULL && strlen (ucon64.mapr) > 0) // mapper specified
    {
      x = strtol (ucon64.mapr, NULL, 10);
      if (x == 0 || x == 2 || x == 4)
        set_mapper (&ines_header, x);
      else
        printf ("WARNING: Pasofami can only store mapper numbers 0, 2 or 4; using old value\n");
    }

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".prm");
  ucon64_output_fname (dest_name, 0);
  write_prm (&ines_header, dest_name);

  if (ines_header.ctrl1 & INES_TRAINER)
    {
      set_suffix (dest_name, ".700");
      // don't write backups of parts, because one name is used
      if (ucon64_fwrite (trainer_data, 0, 512, dest_name, "wb") == -1)
        fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name); // try to continue
      else
        printf (ucon64_msg[WROTE], dest_name);
    }

  if (prg_size > 0)
    {
      set_suffix (dest_name, ".prg");
      // don't write backups of parts, because one name is used
      if (ucon64_fwrite (prg_data, 0, prg_size, dest_name, "wb") == -1)
        fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name); // try to continue
      else
        printf (ucon64_msg[WROTE], dest_name);
    }
  else
    printf ("WARNING: No PRG data in %s\n", ucon64.rom);

  if (chr_size > 0)
    {
      set_suffix (dest_name, ".chr");
      // don't write backups of parts, because one name is used
      if (ucon64_fwrite (chr_data, 0, chr_size, dest_name, "wb") == -1)
        fprintf (stderr, ucon64_msg[WRITE_ERROR], dest_name); // try to continue
      else
        printf (ucon64_msg[WROTE], dest_name);
    }

  free (trainer_data);
  free (prg_data);
  free (chr_data);
  fclose(srcfile);
  return 0;
}


int
nes_n (const char *name)
{
  if (type != UNIF)
    {
      fprintf (stderr, "ERROR: This option is only meaningful for UNIF files\n");
      return -1;
    }

  if (name != NULL && strlen (name) > 0)
    internal_name = name;
  else
    internal_name = NULL;

  return nes_unif ();                           // will call nes_unif_unif()
}


int
nes_init (st_rominfo_t *rominfo)
{
  unsigned char magic[15], *rom_buffer;
  int result = -1, size, x, y, n, crc = 0;
  // currently 92 bytes is enough for ctrl_str, but extra space avoids
  //  introducing bugs when controller type text would be changed
  char buf[MAXBUFSIZE], ctrl_str[200], *str, *str_list[8];
  st_unif_chunk_t *unif_chunk, *unif_chunk2;
  st_nes_data_t *info, key;

  internal_name = NULL;                         // reset this var, see nes_n()
  type = PASOFAMI;                              // reset type, see below

  ucon64_fread (magic, 0, 15, ucon64.rom);
  if (memcmp (magic, "NES", 3) == 0)
    /*
      Check for "NES" and not for INES_SIG_S ("NES\x1a"), because there are two
      NES files floating around on the internet with a pseudo iNES header:
      "Home Alone 2 - Lost in New York (U) [b3]" (magic: "NES\x19") and
      "Linus Music Demo (PD)" (magic: "NES\x1b")
    */
    {
      type = INES;
      result = 0;
    }
  else if (memcmp (magic, UNIF_SIG_S, 4) == 0)
    {
      type = UNIF;
      result = 0;
    }
  else if (memcmp (magic, FDS_SIG_S, 4) == 0)
    {
      type = FDS;
      result = 0;

      rominfo->buheader_start = FDS_HEADER_START;
      rominfo->buheader_len = FDS_HEADER_LEN;
      // we use ffe_header to save some space in the exe
      ucon64_fread (&ffe_header, FDS_HEADER_START, FDS_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &ffe_header;
    }
  else if (memcmp (magic, "\x01*NINTENDO-HVC*", 15) == 0) // "headerless" FDS/FAM file
    {
      if (ucon64.file_size % 65500 == 192)
        type = FAM;
      else
        type = FDS;
      result = 0;
    }

  if (type == PASOFAMI)                         // INES, UNIF, FDS and FAM are much
    {                                           //  more reliable than stricmp()s
      str = (char *) get_suffix (ucon64.rom);
      if (!stricmp (str, ".prm") ||
          !stricmp (str, ".700") ||
          !stricmp (str, ".prg") ||
          !stricmp (str, ".chr"))
        {
          type = PASOFAMI;
          result = 0;
        }
      else if (magic[8] == 0xaa && magic[9] == 0xbb)
        {                                       // TODO: finding a reliable means
          type = FFE;                           //  for detecting FFE images
          result = 0;
        }
    }
  if (ucon64.console == UCON64_NES)
    result = 0;

  switch (type)
    {
    case INES:
      rominfo->copier_usage = ines_usage[0].help;
      rominfo->buheader_start = INES_HEADER_START;
      rominfo->buheader_len = INES_HEADER_LEN;
      ucon64_fread (&ines_header, INES_HEADER_START, INES_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &ines_header;
      ucon64.split = 0;                         // iNES files are never split

      sprintf (buf, "Internal size: %.4f Mb\n",
        TOMBIT_F ((ines_header.prg_size << 14) + (ines_header.chr_size << 13)));
      strcat (rominfo->misc, buf);

      sprintf (buf, "Internal PRG size: %.4f Mb\n",     // ROM
        TOMBIT_F (ines_header.prg_size << 14));
      strcat (rominfo->misc, buf);

      sprintf (buf, "Internal CHR size: %.4f Mb\n",     // VROM
        TOMBIT_F (ines_header.chr_size << 13));
      strcat (rominfo->misc, buf);

      x = (ines_header.ctrl1 >> 4) | (ines_header.ctrl2 & 0xf0);
      if (ines_header.ctrl2 & 0xf)
        sprintf (buf, "Memory mapper (iNES): %d (%d)\n", x,
          x | ((ines_header.ctrl2 & 0xf) << 8));
      else
        sprintf (buf, "Memory mapper (iNES): %d\n", x);
      strcat (rominfo->misc, buf);

      if (ines_header.ctrl1 & INES_MIRROR)
        str = "Vertical";
      else if (ines_header.ctrl1 & INES_4SCREEN)
        str = "Four screens of VRAM";
      else
        str = "Horizontal";
      sprintf (buf, "Mirroring: %s\n", str);
      strcat (rominfo->misc, buf);

      sprintf (buf, "Save RAM: %s\n", (ines_header.ctrl1 & INES_SRAM) ? "Yes" : "No");
      strcat (rominfo->misc, buf);

      sprintf (buf, "512-byte trainer: %s\n",
                (ines_header.ctrl1 & INES_TRAINER) ? "Yes" : "No");
      strcat (rominfo->misc, buf);

      sprintf (buf, "VS-System: %s", (ines_header.ctrl2 & 0x01) ? "Yes" : "No");
      strcat (rominfo->misc, buf);
      break;
    case UNIF:
      rominfo->copier_usage = unif_usage[0].help;
      rominfo->buheader_start = UNIF_HEADER_START;
      rominfo->buheader_len = UNIF_HEADER_LEN;
      ucon64_fread (&unif_header, UNIF_HEADER_START, UNIF_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &unif_header;

      rom_size = ucon64.file_size - UNIF_HEADER_LEN;
      if ((rom_buffer = (unsigned char *) malloc (rom_size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], rom_size);
          return -1; //exit (1); please don't use exit () in init
        }
      ucon64_fread (rom_buffer, UNIF_HEADER_LEN, rom_size, ucon64.rom);
      ucon64.split = 0;                         // UNIF files are never split

      x = me2le_32 (unif_header.revision);      // Don't modify header data
      sprintf (buf, "UNIF revision: %d\n", x);
      strcpy (rominfo->misc, buf);

      if ((unif_chunk = read_chunk (MAPR_ID, rom_buffer, 0)) != NULL)
        {
          sprintf (buf, "Board name: %s\n", (char *) unif_chunk->data);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      if ((unif_chunk = read_chunk (READ_ID, rom_buffer, 0)) != NULL)
        {
          sprintf (buf, "Comment: %s\n", (char *) unif_chunk->data);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
#if     UNIF_REVISION > 7
      if ((unif_chunk = read_chunk (WRTR_ID, rom_buffer, 0)) != NULL)
        {
          char ucon64_name[] = "uCON64";
          strcat (rominfo->misc, "Processed by: ");
          y = 0;
          do
            {
              if (y)
                strcat (rominfo->misc, ", ");
              /*
                The format of the uCON64 WRTR chunk is:
                uCON64<marker><version string><marker><OS string>
                but other tools needn't use the same format. We can only be
                sure that the string starts with the tool name.
              */
              y = strlen ((const char *) unif_chunk->data);
              x = 0;
              if (!strncmp ((const char *) unif_chunk->data, ucon64_name, strlen (ucon64_name)))
                {
                  while (x < y)
                    {
                      if (((char *) unif_chunk->data)[x] == WRTR_MARKER)
                        ((char *) unif_chunk->data)[x] = ' ';
                      x++;
                    }
                }
              else
                {
                  while (x < y)
                    {
                      if (((char *) unif_chunk->data)[x] == WRTR_MARKER)
                        {
                          ((char *) unif_chunk->data)[x] = 0;
                          break;
                        }
                      x++;
                    }
                }
              strcat (rominfo->misc, (const char *) unif_chunk->data);
              y = 1;
              free (unif_chunk);
            }
          while ((unif_chunk = read_chunk (WRTR_ID, rom_buffer, 1)) != NULL);
          strcat (rominfo->misc, "\n");
        }
#endif
      if ((unif_chunk = read_chunk (NAME_ID, rom_buffer, 0)) != NULL)
        {
#if 0
          sprintf (buf, "Internal name: %s\n", (char *) unif_chunk->data);
          strcat (rominfo->misc, buf);
#endif
          memcpy (rominfo->name, unif_chunk->data,
                  unif_chunk->length > sizeof rominfo->name ?
                    sizeof rominfo->name : unif_chunk->length);
        }
      free (unif_chunk);
      if ((unif_chunk = read_chunk (TVCI_ID, rom_buffer, 0)) != NULL)
        {
          str_list[0] = "NTSC";
          str_list[1] = "PAL";
          str_list[2] = "NTSC/PAL";
          x = *((unsigned char *) unif_chunk->data);
          sprintf (buf, "Television standard: %s\n", x > 2 ? "Unknown" : str_list[x]);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      if ((unif_chunk = read_chunk (DINF_ID, rom_buffer, 0)) != NULL)
        {
          st_dumper_info_t *info = (st_dumper_info_t *) unif_chunk->data;
          sprintf (buf, "Dump info:\n"
                        "  Dumper: %s\n"
                        "  Date: %d-%d-%02d\n"
                        "  Agent: %s\n",
                        info->dumper_name,
                        info->day, info->month, le2me_16 (info->year),
                        info->dumper_agent);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      if ((unif_chunk = read_chunk (CTRL_ID, rom_buffer, 0)) != NULL)
        {
          str_list[0] = "Regular joypad";
          str_list[1] = "Zapper";
          str_list[2] = "R.O.B.";
          str_list[3] = "Arkanoid controller";
          str_list[4] = "Power pad";
          str_list[5] = "Four-score adapter";
          str_list[6] = "Unknown";              // bit 6 and 7 are reserved
          str_list[7] = str_list[6];
          ctrl_str[0] = 0;

          x = *((unsigned char *) unif_chunk->data);
          y = 0;
          for (n = 0; n < 8; n++)
            if (x & (1 << n))
              {
                if (y)
                  strcat (ctrl_str, ", ");
                strcat (ctrl_str, str_list[n]);
                y = 1;
              }
          sprintf (buf, "Supported controllers: %s\n", ctrl_str);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);

      size = 0;
      // PRG chunk info
      for (n = 0; n < 16; n++)
        {
          if ((unif_chunk = read_chunk (unif_prg_ids[n], rom_buffer, 0)) != NULL)
            {
              crc = crc32 (crc, (unsigned char *) unif_chunk->data, unif_chunk->length);
              size += unif_chunk->length;
              if ((unif_chunk2 = read_chunk (unif_pck_ids[n], rom_buffer, 0)) == NULL)
                str = "not available";
              else
                {
                  x = crc32 (0, (unsigned char *) unif_chunk->data, unif_chunk->length);
#ifdef  WORDS_BIGENDIAN
                  x = bswap_32 (x);
#endif
                  str = (char *)
#ifdef  USE_ANSI_COLOR
                    (ucon64.ansi_color ?
                      ((x == *((int *) unif_chunk2->data)) ?
                        "\x1b[01;32mok\x1b[0m" : "\x1b[01;31mbad\x1b[0m")
                      :
                      ((x == *((int *) unif_chunk2->data)) ? "ok" : "bad"));
#else
                      ((x == *((int *) unif_chunk2->data)) ? "ok" : "bad");
#endif
                }
              sprintf (buf, "PRG%X: %.4f Mb, checksum %s\n", n,
                TOMBIT_F (unif_chunk->length), str);
              strcat (rominfo->misc, buf);
              free (unif_chunk2);
            }
          free (unif_chunk);
        }

      // CHR chunk info
      for (n = 0; n < 16; n++)
        {
          if ((unif_chunk = read_chunk (unif_chr_ids[n], rom_buffer, 0)) != NULL)
            {
              crc = crc32 (crc, (unsigned char *) unif_chunk->data, unif_chunk->length);
              size += unif_chunk->length;
              if ((unif_chunk2 = read_chunk (unif_cck_ids[n], rom_buffer, 0)) == NULL)
                str = "not available";
              else
                {
                  x = crc32 (0, (unsigned char *) unif_chunk->data, unif_chunk->length);
#ifdef  WORDS_BIGENDIAN
                  x = bswap_32 (x);
#endif
                  str = (char *)
#ifdef  USE_ANSI_COLOR
                    (ucon64.ansi_color ?
                      ((x == *((int *) unif_chunk2->data)) ?
                        "\x1b[01;32mok\x1b[0m" : "\x1b[01;31mbad\x1b[0m")
                      :
                      ((x == *((int *) unif_chunk2->data)) ? "ok" : "bad"));
#else
                      ((x == *((int *) unif_chunk2->data)) ? "ok" : "bad");
#endif
                }
              sprintf (buf, "CHR%X: %.4f Mb, checksum %s\n", n,
                TOMBIT_F (unif_chunk->length), str);
              strcat (rominfo->misc, buf);
              free (unif_chunk2);
            }
          free (unif_chunk);
        }
      ucon64.crc32 = crc;
      rominfo->data_size = size;

      // Don't introduce extra code just to make this line be printed above
      //  the previous two line types (PRG & CHR)
      sprintf (buf, "Size: %.4f Mb\n", TOMBIT_F (rominfo->data_size));
      strcat (rominfo->misc, buf);

      x = 0;
      if ((unif_chunk = read_chunk (BATR_ID, rom_buffer, 0)) != NULL)
        x = 1;
      sprintf (buf, "Save RAM: %s\n", x ? "Yes" : "No");
      strcat (rominfo->misc, buf);
      free (unif_chunk);
      if ((unif_chunk = read_chunk (MIRR_ID, rom_buffer, 0)) != NULL)
        {
          str_list[0] = "Horizontal (hard wired)";
          str_list[1] = "Vertical (hard wired)";
          str_list[2] = "All pages from $2000 (hard wired)";
          str_list[3] = "All pages from $2400 (hard wired)";
          str_list[4] = "Four screens of VRAM (hard wired)";
          str_list[5] = "Controlled by mapper hardware";
          x = *((unsigned char *) unif_chunk->data);
          sprintf (buf, "Mirroring: %s\n", x > 5 ? "Unknown" : str_list[x]);
          strcat (rominfo->misc, buf);
        }
      free (unif_chunk);
      x = 0;
      if ((unif_chunk = read_chunk (VROR_ID, rom_buffer, 0)) != NULL)
        x = 1;
      sprintf (buf, "VRAM override: %s", x ? "Yes" : "No");
      strcat (rominfo->misc, buf);
      free (unif_chunk);

      free (rom_buffer);
      break;
    case PASOFAMI:
      /*
        Either a *.PRM header file, a 512-byte *.700 trainer file, a *.PRG
        ROM data file or a *.CHR VROM data file.
      */
      rominfo->copier_usage = pasofami_usage[0].help;
      rominfo->buheader_start = 0;
      strcpy (buf, ucon64.rom);
      set_suffix (buf, ".prm");
      if (access (buf, F_OK) == 0)
        {
          rominfo->buheader_len = fsizeof (buf);
          // we use ffe_header to save some space
          ucon64_fread (&ffe_header, 0, rominfo->buheader_len, buf);
          rominfo->buheader = &ffe_header;
        }
      else
        rominfo->buheader_len = 0;

      /*
        Build a temporary iNES image in memory from the Pasofami files.
        In memory, because we want to be able to display info for Pasofami
        files on read-only filesystems WITHOUT messing with/finding temporary
        storage somewhere. We also want to calculate the CRC and it's handy to
        have the data in memory for that.
        Note that nes_j() wouldn't be much different if ucon64_chksum() would be
        used. This function wouldn't be much different either.
      */
      x = nes_j (&rom_buffer);
      rominfo->data_size = (ines_header.prg_size << 14) + (ines_header.chr_size << 13) +
                             ((ines_header.ctrl1 & INES_TRAINER) ? 512 : 0);
      if (x == 0)
        {                                       // use buf only if it could be allocated
          ucon64.crc32 = crc32 (0, rom_buffer, rominfo->data_size);
          free (rom_buffer);
        }

      sprintf (buf, "Size: %.4f Mb\n", TOMBIT_F (rominfo->data_size));
      strcat (rominfo->misc, buf);

      sprintf (buf, "PRG size: %.4f Mb\n",      // ROM, don't say internal,
        TOMBIT_F (ines_header.prg_size << 14)); //  because it's not
      strcat (rominfo->misc, buf);

      sprintf (buf, "CHR size: %.4f Mb\n",      // VROM
        TOMBIT_F (ines_header.chr_size << 13));
      strcat (rominfo->misc, buf);

      sprintf (buf, "Memory mapper (iNES): %d\n",
        (ines_header.ctrl1 >> 4) | (ines_header.ctrl2 & 0xf0));
      strcat (rominfo->misc, buf);

      sprintf (buf, "Mirroring: %s\n",
        (ines_header.ctrl1 & INES_MIRROR) ? "Vertical" : "Horizontal");
      strcat (rominfo->misc, buf);

      sprintf (buf, "Save RAM: %s\n",
        (ines_header.ctrl1 & INES_SRAM) ? "Yes" : "No");
      strcat (rominfo->misc, buf);

      sprintf (buf, "512-byte trainer: %s",
        (ines_header.ctrl1 & INES_TRAINER) ? "Yes" : "No");
      strcat (rominfo->misc, buf);
      break;
    case FFE:
      if (magic[10] == 1)
        {
          rominfo->buheader_len = SMC_HEADER_LEN;
          strcpy (rominfo->name, "Name: N/A");
          rominfo->console_usage = NULL;
          rominfo->copier_usage = smc_usage[0].help;
          rominfo->maker = "Publisher: You?";
          rominfo->country = "Country: Your country?";
          rominfo->has_internal_crc = 0;
          ucon64.split = 0;                     // RTS files are never split
          strcat (rominfo->misc, "Type: Super Magic Card RTS file\n");
          return 0;                             // rest is nonsense for RTS file
        }

      /*
        512-byte header
        512-byte trainer (optional)
        ROM data
        VROM data (optional)

        It makes no sense to make a temporary iNES image here. It makes sense
        for Pasofami, because there might be a .PRM file and because there is
        still other information about the image structure.
      */
      rominfo->copier_usage = smc_usage[0].help;
      rominfo->buheader_start = SMC_HEADER_START;
      rominfo->buheader_len = SMC_HEADER_LEN;
      ucon64_fread (&ffe_header, SMC_HEADER_START, SMC_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &ffe_header;

      sprintf (buf, "512-byte trainer: %s",
        (ffe_header.emulation1 & SMC_TRAINER) ? "Yes" : "No");
      strcat (rominfo->misc, buf);
      break;
    case FDS:
      rominfo->copier_usage = fds_usage[0].help;
      rominfo->country = "Japan";
      strcat (rominfo->misc, "\n");
      nes_fdsl (rominfo, rominfo->misc);        // will also fill in rominfo->name
      break;
    case FAM:
      rominfo->copier_usage = fds_usage[0].help;
      rominfo->country = "Japan";

      // FAM files don't have a header. Instead they seem to have a 192 byte trailer.
      rominfo->buheader_start = ucon64.file_size - FAM_HEADER_LEN;
      rominfo->buheader_len = FAM_HEADER_LEN;

      // we use ffe_header to save some space
      ucon64_fread (&ffe_header, rominfo->buheader_start, FAM_HEADER_LEN, ucon64.rom);
      rominfo->buheader = &ffe_header;
      strcat (rominfo->misc, "\n");
      nes_fdsl (rominfo, rominfo->misc);        // will also fill in rominfo->name

      rom_size = ucon64.file_size - FAM_HEADER_LEN;
      if ((rom_buffer = (unsigned char *) malloc (rom_size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], rom_size);
          return -1;
        }
      ucon64_fread (rom_buffer, 0, rom_size, ucon64.rom);
      ucon64.crc32 = crc32 (0, rom_buffer, rom_size);
      free (rom_buffer);
      break;
    }

  if (UCON64_ISSET (ucon64.buheader_len))       // -hd, -nhd or -hdn switch was specified
    rominfo->buheader_len = ucon64.buheader_len;

  if (ucon64.crc32 == 0)
    ucon64_chksum (NULL, NULL, &ucon64.crc32, ucon64.rom, rominfo->buheader_len);

  // additional info
  key.crc32 = ucon64.crc32;
  info = (st_nes_data_t *) bsearch (&key, nes_data, sizeof nes_data / sizeof (st_nes_data_t),
                                    sizeof (st_nes_data_t), nes_compare);
  if (info)
    {
      if (info->maker)
        rominfo->maker = NULL_TO_UNKNOWN_S (nes_maker[MIN (info->maker, NES_MAKER_MAX - 1)]);

      rominfo->country = NULL_TO_UNKNOWN_S (nes_country[MIN (info->country, NES_COUNTRY_MAX - 1)]);

      if (info->date)
        {
          int month = info->date / 100, year = info->date % 100;
          char format[80];

          if (month)
            {
              sprintf (format, "\nDate: %%d/19%%d");
              strcat (format, (year == 8 || year == 9) ? "x" : "");
              sprintf (buf, format, month, year);
            }
          else
            {
              sprintf (format, "\nDate: 19%%d");
              strcat (format, (year == 8 || year == 9) ? "x" : "");
              sprintf (buf, format, year);
            }
          strcat (rominfo->misc, buf);
        }
    }

  rominfo->console_usage = nes_usage[0].help;

  return result;
}


static inline char *
to_func (char *s, int len, int (*func) (int))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = func (*p);

  return s;
}


int
nes_fdsl (st_rominfo_t *rominfo, char *output_str)
/*
  Note that NES people prefer $ to signify hexadecimal numbers (not 0x).
  However we will print only addresses that way.
  This code is based on Marat Fayzullin's FDSLIST.
*/
{
  FILE *srcfile;
  unsigned char buffer[58];
  char name[16], str_list_mem[6], *str_list[4], info_mem[MAXBUFSIZE], *info, line[80];
  int disk, n_disks, file, n_files, start, size, x, header_len = 0;

  if (output_str == NULL)
    {
      info = info_mem;
      info[0] = 0;
    }
  else
    info = output_str;

  if ((srcfile = fopen (ucon64.rom, "rb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.rom);
      return -1;
    }

  n_disks = (ucon64.file_size - rominfo->buheader_len) / 65500;
  x = (ucon64.file_size - rominfo->buheader_len) % 65500;
  if (x)
    {
      sprintf (line, "WARNING: %d excessive bytes\n", x);
      strcat (info, line);
    }

  if (type == FDS)
    header_len = rominfo->buheader_len;
  else if (type == FAM)                         // if type == FAM rominfo->buheader_len
    header_len = 0;                             //  contains the length of the trailer
  for (disk = 0; disk < n_disks; disk++)
    {
      // go to the next disk
      fseek (srcfile, disk * 65500 + header_len, SEEK_SET);

      // read the disk header
      if (fread (buffer, 1, 58, srcfile) != 58)
        {
          fprintf (stderr, "ERROR: Can't read disk header\n");
          fclose (srcfile);
          return -1;
        }

      if (memcmp (buffer, "\x01*NINTENDO-HVC*", 15))
        {
          fprintf (stderr, "ERROR: Invalid disk header\n");
          fclose (srcfile);
          return -1;                            // should we return?
        }

      if (buffer[56] != 2)
        strcat (info, "WARNING: Invalid file number header\n");

      memcpy (name, buffer + 16, 4);
      name[4] = 0;
      if (disk == 0 && output_str != NULL)
        memcpy (rominfo->name, name, 4);
      n_files = buffer[57];
      sprintf (line, "Disk: '%-4s'  Side: %c  Files: %d  Maker: 0x%02x  Version: 0x%02x\n",
               name, (buffer[21] & 1) + 'A', n_files, buffer[15], buffer[20]);
      strcat (info, line);

      file = 0;
      while (file < n_files && fread (buffer, 1, 16, srcfile) == 16)
        {
          if (buffer[0] != 3)
            {
              sprintf (line, "WARNING: Invalid file header block ID (0x%02x)\n", buffer[0]);
              strcat (info, line);
            }

          // get name, data location, and size
          strncpy (name, (const char *) buffer + 3, 8);
          name[8] = 0;
          start = buffer[11] + 256 * buffer[12];
          size = buffer[13] + 256 * buffer[14];

          x = fgetc (srcfile);
          if (x != 4)
            {
              sprintf (line, "WARNING: Invalid data block ID (0x%02x)\n", x);
              strcat (info, line);
            }

          str_list[0] = "Code";
          str_list[1] = "Tiles";
          str_list[2] = "Picture";
          if (buffer[15] > 2)
            {
              str_list[3] = str_list_mem;
              sprintf (str_list[3], "0x%02x?", buffer[15]);
              buffer[15] = 3;
            }
          /*
            Some FDS files contain control characters in their names. sprintf()
            won't print those character so we have to use to_func() with
            toprint().
          */
          sprintf (line, "%03d $%02x '%-8s' $%04x-$%04x [%s]\n",
                   buffer[1], buffer[2],
                   to_func (name, strlen (name), toprint),
                   start, start + size - 1, str_list[buffer[15]]);
          strcat (info, line);

          fseek (srcfile, size, SEEK_CUR);
          file++;
        }
      if (disk != n_disks - 1)
        strcat (info, "\n");                    // print newline between disk info blocks
    }

  if (output_str == NULL)
    puts (info);

  fclose (srcfile);
  return 0;
}


int
nes_fds (void)
/*
  This function converts a Famicom Disk System disk image from FAM format to
  FDS format. It does almost the same as -strip apart from three checks
  whether the input file is a valid FAM file.
  The "algorithm" comes from Marat Fayzullin's FAM2FDS.
*/
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], *buffer;
  int n;

  if (type != FAM)
    {
      fprintf (stderr, "ERROR: %s is not a FAM file\n", ucon64.rom);
      return -1;
    }
  if ((buffer = (char *) malloc (65500)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], 65500);
      return -1;
    }

  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".fds");
  strcpy (src_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);

  for (n = 0; n < 4; n++)
    {
      if (ucon64_fread (buffer, n * 65500, 65500, src_name) != 65500)
        break;

      // check disk image for validity
      if (buffer[0] != 1 || buffer[56] != 2 || buffer[58] != 3)
        {
          fprintf (stderr, "ERROR: %s is not a valid FAM file\n", ucon64.rom);
          break;
        }
      // FAM2FDS also does the following:
      //  1 - check if the last chunk is one of a new game (use buffer[16] - buffer[19])
      //  2 - if not, check if buffer[21] (side bit) differs from least significant bit of n
      //  3 - if that is the case _print_ (but don't do anything else) "WRONG" else "OK"

      if (ucon64_fwrite (buffer, n * 65500, 65500, dest_name, n ? "ab" : "wb") != 65500)
        break;
    }
  /*
    FAM2FDS prints an error message if n isn't 4 at this point (a break was
    executed). However, other information seems to indicate that FAM files can
    hold fewer than 4 disk images.
  */

  free (buffer);
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}
