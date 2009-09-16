/*
pce.c - PC-Engine support for uCON64

Copyright (c) 1999 - 2001 NoisyB <noisyb@gmx.net>
Copyright (c) 2003 - 2004 dbjh


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
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/stat.h>
#include "misc/chksum.h"
#include "misc/misc.h"
#include "misc/string.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "pce.h"
#include "backup/mgd.h"
#include "backup/msg.h"


#define PCENGINE_HEADER_START 0x448
#define PCENGINE_HEADER_LEN (sizeof (st_pce_header_t))


// static unsigned int pcengine_chksum (st_rominfo_t *rominfo);

const st_getopt2_t pcengine_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "PC-Engine (CD Unit/Core Grafx(II)/Shuttle/GT/LT/Super CDROM/DUO(-R(X)))\n"
      "Super Grafx/Turbo (Grafx(16)/CD/DUO/Express)"/*"1987/19XX/19XX NEC"*/,
      NULL
    },
    {
      "pce", 0, 0, UCON64_PCE,
      NULL, "force recognition",
      &ucon64_wf[WF_OBJ_PCE_SWITCH]
    },
    {
      "int", 0, 0, UCON64_INT,
      NULL, "force ROM is in interleaved (bit-swapped) format",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "nint", 0, 0, UCON64_NINT,
      NULL, "force ROM is not in interleaved (bit-swapped) format",
      &ucon64_wf[WF_OBJ_ALL_SWITCH]
    },
    {
      "msg", 0, 0, UCON64_MSG,
      NULL, "convert to Magic Super Griffin/MSG",
      &ucon64_wf[WF_OBJ_PCE_DEFAULT]
    },
    {
      "mgd", 0, 0, UCON64_MGD,
      NULL, "convert to Multi Game Doctor*/MGD2/RAW",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT_NO_SPLIT]
    },
    {
      "swap", 0, 0, UCON64_SWAP,
      NULL, "swap bits of all bytes in file (TurboGrafx-16 <-> PC-Engine)",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE]
    },
    {
      "f", 0, 0, UCON64_F,
      NULL, "fix region protection",
      &ucon64_wf[WF_OBJ_ALL_DEFAULT]
    },
    {
      "multi", 1, 0, UCON64_MULTI,
      "SIZE", "make multi-game file for use with PCE-PRO flash card, truncated\n"
      "to SIZE Mbit; file with loader must be specified first, then\n"
      "all the ROMs, multi-game file to create last",
      &ucon64_wf[WF_OBJ_ALL_INIT_PROBE_STOP]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
};

#define PCE_MAKER_MAX 86
static const char *pce_maker[PCE_MAKER_MAX] =
  {
    NULL, "ACCOLADE", "AICOM", "ARTMIC", "ASK KODANSHA",
    "ASMIK", "ATLUS", "AZUMA", "BIG DON", "BIGCLUB",
    "BIT2", "BULLET PROOF", "CAPCOM", "CINEMAWARE", "COCONUTS",
    "CREAM", "DATA EAST", "DEKA", "DISNEY", "FACE",
    "FUJITV", "FUN PROJECT", "GAMES EXPRESS", "HOMEDATA", "HUDSON",
    "HUDSON V1", "HUDSON V2", "HUMAN", "ICOM", "IGS",
    "IMAGE", "IMAX", "INTEC", "IREM", "KANEKO",
    "KONAMI", "KSS", "LASER SOFT", "LORICIEL", "MAGAE CHIP VERSION",
    "MANLEY & ASSOCIATES", "MASYNYA & NCS", "MASYNYA", "MEDIA RINGS", "NAMCO",
    "NATSUME", "NATSUME", "NAXAT SOFT", "NAXAT", "NCS",
    "NEC / HUDSON", "NEC AVENUE & TAITO", "NEC AVENUE", "NEC HOME ELECTRONICS", "NEC",
    "NHK", "NICHIBUTSU", "NIHON BUSSAN", "PACK-IN-VIDEO", "PALSOFT",
    "PSYGNOSYS", "RANDOM HOUSE", "SALIO", "SEGA", "SEIBU KAIHATSU",
    "SGX", "SPECTRUM HOLOBYTE", "SSL", "SUMMER PROJECT", "SUNRISE",
    "SUNSOFT", "TAISANG VERSION", "TAITO 384K STYLE", "TAITO", "TAKARA",
    "TECHNOS", "TELENET", "TENGEN", "THE HU62680 TEAM", "TITUS",
    "TONKIN HOUSE", "UNI POST", "UPL", "VICTOR", "VIDEO SYSTEM",
    "WOLF TEAM"
  };


typedef struct
{
  uint32_t crc32;
  uint8_t maker;
  const char *serial;
  uint32_t date;
  const char *comment;
} st_pce_data_t;


static const st_pce_data_t pce_data[] =
{
  {0x0038b5b5, 70, "SS90002", 100890, NULL},
  {0x00c38e69, 73, "TP02012", 60790, NULL},
  {0x00f83029, 42, NULL, 0, NULL},
  {0x0112d0c7, 42, "NCS90005", 141290, "1p"},
  {0x013a747f, 24, "HC91051", 61291, "1p"},
  {0x01a76935, 44, "NC63003", 261288, NULL},
  {0x020dc2df, 58, "PV1001", 230389, "3M"},
  {0x0243453b, 24, "HC63016", 90289, "aka MILITARY MADNESS,2p"},
  {0x0258accb, 33, NULL, 0, NULL},
  {0x02a578c5, 24, NULL, 91, "1p"},
  {0x02db6fe5, 24, NULL, 0, NULL},
  {0x02dde03e, 22, "T4955754200915", 0, NULL},
  {0x033e8c4a, 73, "TP02006", 260190, "1p"},
  {0x03883ee8, 48, "NX90003", 60490, "2p"},
  {0x03e28cff, 24, "HC62003", 281287, "1p"},
  {0x04188c5c, 83, "JC63005", 290690, "3p"},
  {0x04bf5eaf, 54, "TGX030064", 0, NULL},
  {0x05054f4f, 12, NULL, 0, NULL},
  {0x0517da65, 35, "KM91001", 151191, NULL},
  {0x05362516, 24, "HC93065", 101293, "5p"},
  {0x053a0f83, 58, NULL, 0, "aka DEEP BLUE KAITEI SHINWA,1p"},
  {0x05453628, 19, "FA02-007", 71290, NULL},
  {0x0590a156, 33, "IC02004", 60790, NULL},
  {0x05a4b72e, 24, NULL, 0, "1p (AKA BLODIA)"},
  {0x07a226fb, 54, "TGX020003", 0, "1p"},
  {0x07bc34dc, 44, "TGX040085", 0, "aka GENPEI TORAMADEN VOLUME 2,1p"},
  {0x088d896d, 54, NULL, 0, "1p"},
  {0x08a09b9a, 24, "HC90030", 270490, "aka BLUE BLINK"},
  {0x09048174, 58, "PV1001", 230389, "3M"},
  {0x09509315, 73, NULL, 0, NULL},
  {0x09a0bfcc, 73, "TP03016", 250191, "4p"},
  {0x09cbb5e6, 84, "VS-90002", 70290, NULL},
  {0x0aa88f33, 22, "T4955754200939", 0, "1p"},
  {0x0ad97b04, 57, NULL, 93, NULL},
  {0x0b7f6e5f, 42, "NCS91002", 290391, "5p"},
  {0x0be0e0a8, 24, "TGX020039", 0, "aka ADVENTURE ISLAND,1p"},
  {0x0d766139, 3, "NX91005", 61291, "1p"},
  {0x0df57c90, 44, NULL, 0, NULL},
  {0x106bb7b2, 70, "SS90003", 121090, "1p"},
  {0x109ba474, 42, "NCS91001", 270491, "aka SHOCKMAN,2p"},
  {0x10b60601, 13, NULL, 290493, "5p"},
  {0x113dd5f0, 24, "HC89019", 70789, "aka BLAZING LAZERS,1p"},
  {0x11a36745, 44, "NC63004", 110888, NULL},
  {0x12c4e6fd, 19, "FA02-007", 71290, NULL},
  {0x13bf0409, 27, "HM89002", 231289, NULL},
  {0x149d0511, 33, "HC63007", 250388, "1p (BOOTLEG TRAINER VERSION)"},
  {0x14daf737, 83, NULL, 0, NULL},
  {0x14fad3ba, 44, NULL, 0, "2p"},
  {0x1555697e, 68, "HC91050", 201291, NULL},
  {0x166a0e44, 48, "NX90004", 200790, "2p"},
  {0x1772a6bc, 73, "TP02007", 230290, NULL},
  {0x1772b229, 32, "IG90001", 0, NULL},
  {0x17a47d0d, 48, "NX90008", 71290, NULL},
  {0x17ba3032, 48, "NX89003", 131089, "1p"},
  {0x1828d2e5, 56, "NB91005", 291191, NULL},
  {0x19ff94e5, 73, "TP03021", 130392, NULL},
  {0x1a8393c6, 44, "NC63002", 150788, "1p"},
  {0x1b1a80a2, 6, "HC63015", 40389, "5p"},
  {0x1b2d0077, 57, "NB96002", 271192, "1p"},
  {0x1b5b1cb1, 56, "NB90003", 280990, NULL},
  {0x1bc36b36, 29, "AI02005", 141290, "aka SINISTRON,1p"},
  {0x1c6ff459, 43, "MR90002", 141290, "1p,linkable"},
  {0x1cad4b7f, 2, "AC89003", 11289, NULL},
  {0x1e1d0319, 73, "NAPH-1009", 0, NULL},
  {0x1e2cbcf8, 0, NULL, 0, "5p"},
  {0x1eb30eeb, 27, "HM89001", 220689, NULL},
  {0x20a7d128, 48, "NX90006", 261090, NULL},
  {0x20ef87fd, 8, "PV-2004", 230390, NULL},
  {0x21b5409c, 24, "HC91039", 210691, NULL},
  {0x231b1535, 73, NULL, 0, NULL},
  {0x23d22d63, 29, "ITGX10001", 0, "aka WORLD BEACH VOLLEYBALL,4p"},
  {0x23ec8970, 23, "HD90001", 100890, NULL},
  {0x2546efe0, 61, "NAPH-1021", 270991, NULL},
  {0x25a02bee, 19, "FA03-009", 120791, "1p"},
  {0x25be2b81, 34, "TGX040051", 0, "2p"},
  {0x25de250a, 24, "HC90030", 270490, "aka BLUE BLINK"},
  {0x25e0f6e9, 5, "AS02002", 130490, "TGX020005"},
  {0x26020c77, 24, NULL, 0, NULL},
  {0x261f1013, 24, "HC90028", 300390, "aka CHEW-MAN-FU,2p"},
  {0x2739b927, 44, NULL, 0, NULL},
  {0x2762792b, 48, NULL, 0, "2p"},
  {0x27a4d11a, 42, "NCS89003", 280389, "2p"},
  {0x283b74e0, 54, NULL, 0, NULL},
  {0x2841fd1e, 24, NULL, 0, NULL},
  {0x284ebe25, 24, NULL, 0, NULL},
  {0x29eec024, 24, "HC92058", 101092, "aka WORLD SPORTS COMPETITION,5p"},
  {0x2a3e08e2, 16, NULL, 0, NULL},
  {0x2b54cba2, 7, "HC63016", 221288, "5p"},
  {0x2b94aedc, 26, "HC91047", 270991, "1p"},
  {0x2bc023fc, 68, "HC91050", 201291, NULL},
  {0x2cb5cd55, 24, "HC89024", 151289, "1p"},
  {0x2cb796e2, 24, "HC89024", 151289, "1p"},
  {0x2cb92290, 33, "IC01002", 11289, "aka MR.HELI's BIG ADVENTURE,1p"},
  {0x2cee30ee, 54, "TGX040077", 0, NULL},
  {0x2db4c1fd, 28, "TGX040076", 0, NULL},
  {0x2df97bd0, 19, NULL, 0, NULL},
  {0x2e5ac9c0, 43, "TGX010031", 0, "1p"},
  {0x2e955051, 80, "TON90002", 140990, NULL},
  {0x2f8935aa, 24, "HC63012", 300888, "aka KEITH COURAGE IN ALPHA ZONES"},
  {0x2fd65312, 9, "BG01004", 220889, NULL},
  {0x3028f7ca, 24, "HC91043", 190791, "1p"},
  {0x30cc3563, 24, "HC91046", 90891, "2p"},
  {0x30d4bd0e, 26, NULL, 0, "1p"},
  {0x31dd1c32, 48, "NX89001", 300589, NULL},
  {0x31e2e7b6, 24, "HC91041", 50491, NULL},
  {0x320f5018, 0, "HC692", 91, "EXTRA BACKUP RAM CARD"},
  {0x3219849c, 44, "NC91005", 271291, NULL},
  {0x345f43e9, 24, "HC90031", 210990, NULL},
  {0x348022f7, 24, "TGX020014", 0, "aka KATO & KEN CHAN"},
  {0x34e089a9, 44, "NC63001", 200588, NULL},
  {0x34fd4ef2, 29, "AI02005", 141290, "aka SINISTRON,1p"},
  {0x364508da, 42, "NCS91002", 290391, "5p"},
  {0x38e2917d, 24, NULL, 0, NULL},
  {0x390710ec, 29, NULL, 0, NULL},
  {0x3920105a, 54, NULL, 0, NULL},
  {0x3aea2f8f, 29, "AI-03004", 60791, "aka TRICKY KICK"},
  {0x3b13af61, 0, "HC89026", 301189, NULL},
  {0x3b3808bd, 24, NULL, 0, "EXTRA SAVE RAM THING BY HUDSON"},
  {0x3e4eaf98, 15, "CC-01001", 81289, NULL},
  {0x3e647d8b, 24, "HC91039", 210691, NULL},
  {0x3e79734c, 33, "ICO3006", 190791, NULL},
  {0x3f982d0f, 1, "ATGX04TUTG", 0, NULL},
  {0x3f9f95a4, 54, NULL, 0, NULL},
  {0x4148fd7c, 14, "CJ92002", 130392, NULL},
  {0x428f36cd, 42, "NCS89002", 230289, "5p"},
  {0x43efc974, 48, "NX90001", 10390, "aka PSYCHOSIS,2p"},
  {0x442405d5, 42, "NCS91003", 270991, NULL},
  {0x44af9bea, 24, "TGX020027", 0, "aka DORAEMON MEIKYU DAISAKUSEN,1p"},
  {0x44e7df53, 11, "MC91002", 240591, NULL},
  {0x44f60137, 32, "IG89002", 0, NULL},
  {0x457f2bc4, 23, "HD91013", 291191, NULL},
  {0x45885afb, 73, NULL, 0, "2p"},
  {0x462256fb, 9, "BG01004", 220889, NULL},
  {0x469a0fdf, 21, "JC63002", 240389, "aka WAR OF THE DEAD,1p"},
  {0x471903c6, 5, "AS01001", 81289, NULL},
  {0x47afe6d7, 16, "TGX040037", 0, NULL},
  {0x4938b8bb, 24, "HC91041", 50491, NULL},
  {0x4a135429, 18, "TGX040066", 0, "1p"},
  {0x4a3df3ca, 44, "NC90003", 270490, NULL},
  {0x4bd38f17, 82, "UP02002", 280990, "1p"},
  {0x4c2126b0, 24, "HC91044", 220291, "1p"},
  {0x4caa6be9, 16, NULL, 0, "1p"},
  {0x4cef0456, 42, "NCS90005", 141290, "1p"},
  {0x4d344c8c, 24, NULL, 0, NULL},
  {0x4d3b0bc9, 44, "NC91001", 150391, NULL},
  {0x4d539c9f, 83, "JC63011", 20891, "1p"},
  {0x4df54b81, 54, "TGX030064", 0, NULL},
  {0x4f2844b0, 80, NULL, 0, NULL},
  {0x4f2bd39f, 71, "H54G-1004", 140789, NULL},
  {0x500472d4, 12, NULL, 0, NULL},
  {0x5157a395, 24, NULL, 0, NULL},
  {0x51e86451, 73, NULL, 0, "2p"},
  {0x52520bc6, 54, NULL, 0, NULL},
  {0x53109ae6, 24, "HC62005", 220188, NULL},
  {0x534e8808, 27, "HM92006", 131192, NULL},
  {0x53b7784b, 43, NULL, 60392, NULL},
  {0x560d2305, 27, "HM91004", 10391, NULL},
  {0x56488b36, 58, "PV-1007", 291191, NULL},
  {0x56739bc7, 45, NULL, 0, NULL},
  {0x574352c6, 50, NULL, 0, NULL},
  {0x57615647, 49, "TGX040087", 0, "2p"},
  {0x57a436a2, 1, NULL, 0, NULL},
  {0x57f183ae, 24, NULL, 89, NULL},
  {0x589d33eb, 83, NULL, 0, NULL},
  {0x595bb22a, 24, "HC63016", 90289, "2p"},
  {0x59d07314, 44, "NC64001", 210489, "1p"},
  {0x59e44f45, 24, "HS93054", 100293, "5p"},
  {0x5c4d1991, 15, "CC-01001", 81289, NULL},
  {0x5c78fee1, 20, "MC66680", 40889, "1p"},
  {0x5cdb3f5b, 70, "SS89001", 170389, "1p"},
  {0x5cf59d80, 35, "KM92004", 280292, NULL},
  {0x5d0e3105, 24, "HE-1097", 60790, "1p"},
  {0x5e4fa713, 24, NULL, 0, NULL},
  {0x5f2c9a45, 22, "T4955754200984", 0, "2p"},
  {0x6069c5e7, 24, "HC62006", 301187, "aka JJ & JEFF"},
  {0x60ecae22, 48, "NX89001", 300589, NULL},
  {0x60edf4e1, 48, "NX63001", 140988, NULL},
  {0x616ea179, 16, "DE90004", 290391, "1p"},
  {0x61a6e210, 48, NULL, 0, NULL},
  {0x61b80005, 58, "PV1003", 221289, "1p"},
  {0x6203de23, 22, NULL, 0, NULL},
  {0x625221a6, 24, "HC90034", 200790, NULL},
  {0x6257cce7, 52, NULL, 0, NULL},
  {0x62654ad5, 73, "TPO1002", 300689, NULL},
  {0x6273a9d4, 44, "TGX020018", 0, "1p"},
  {0x62ec2956, 44, "NC63004", 110888, NULL},
  {0x633a3d48, 73, "TP02010", 310590, "2p"},
  {0x637ba71d, 30, NULL, 0, "BOOTLEG"},
  {0x64301ff1, 24, "TGX040058", 0, "1p"},
  {0x64580427, 52, "H67G-1002", 91288, "1p"},
  {0x647718f9, 35, "KM92003", 210292, NULL},
  {0x65fdb863, 75, "NX90002", 300390, "aka HOT BLOOD HIGHSCHOOL DODGEBALL"},
  {0x67573bac, 24, "HC92052", 240192, NULL},
  {0x67aab7a1, 58, "PV-1005", 141290, "1p"},
  {0x67aede77, 24, NULL, 0, NULL},
  {0x67ec5ec4, 16, "DE90005", 300390, "aka DROP OFF"},
  {0x6923d736, 24, "HC62004", 301087, "1p"},
  {0x6976d5b3, 30, "1992", 0, NULL},
  {0x6a628982, 63, NULL, 0, NULL},
  {0x6b319457, 44, NULL, 0, "1p"},
  {0x6c30f0ac, 30, NULL, 0, "BOOTLEG"},
  {0x6c34aaea, 56, "NB1001", 10290, "1p"},
  {0x6cca614c, 24, "HC90040", 221290, NULL},
  {0x6e297e49, 82, "UP02002", 280990, "1p"},
  {0x6eab778c, 71, "HC63009", 30688, "1p"},
  {0x6f4fd790, 73, "TP02006", 260190, "1p"},
  {0x6fd6827c, 49, "NCS63001", 230988, NULL},
  {0x70749841, 13, NULL, 290493, "5p"},
  {0x70d90e20, 44, "TGX020019", 0, "4p"},
  {0x7146027c, 27, "HM94007", 150194, NULL},
  {0x727f4656, 1, "JC63012", 240792, NULL},
  {0x72814acb, 44, NULL, 0, NULL},
  {0x72a2c22c, 73, "TP01003", 291189, NULL},
  {0x72cb0f9d, 63, "H49G-1001", 141088, NULL},
  {0x72d6860b, 19, NULL, 0, NULL},
  {0x72e00bc4, 44, "NC92003", 250692, NULL},
  {0x73614660, 54, NULL, 0, NULL},
  {0x73e994a0, 48, "NX90001", 10390, "aka PSYCHOSIS,2p"},
  {0x740491c2, 24, "HC92053", 201192, "1p"},
  {0x7424452e, 66, "TGX040067", 0, "1p,linkable"},
  {0x745408ae, 42, "NCS89002", 230289, "5p"},
  {0x74903426, 48, "NX91003", 120491, NULL},
  {0x756a1802, 24, NULL, 0, NULL},
  {0x76164593, 73, "TP02014", 141290, NULL},
  {0x7632db90, 2, "AC89001", 200389, NULL},
  {0x767245cd, 24, "TGX020008", 0, "aka THE KUNG FU,1p"},
  {0x775bd3e1, 73, "TPO3019", 200991, NULL},
  {0x786d9bbd, 73, NULL, 0, "2p"},
  {0x79362389, 12, "NX91002", 220391, "2p"},
  {0x7aa9d4dc, 24, "TGX030010", 0, "aka GUNHED,1p"},
  {0x7acb60c8, 73, "TP03019", 200991, "4p"},
  {0x7b96317c, 24, "HC90034", 200790, NULL},
  {0x7d3e6f33, 24, NULL, 0, NULL},
  {0x7d48d2fc, 73, "TP02015", 180191, "2p"},
  {0x805a34b9, 83, "JC63013", 290493, NULL},
  {0x80c3f824, 44, "NC62001", 0, "aka GHOST TRAVEL STORY"},
  {0x82ae3b16, 12, "JC63004", 230290, "aka TIGER ROAD"},
  {0x82def9ee, 36, "NV91001", 131291, NULL},
  {0x83213ade, 48, "NX90005", 280990, NULL},
  {0x8420b12b, 24, "HC92056", 100792, NULL},
  {0x850829f2, 64, "HC91049", 221191, "1p"},
  {0x85101c20, 52, "NAPH-1007", 220690, NULL},
  {0x854c37b3, 44, "NC89003", 70789, "2p"},
  {0x85a1e7b6, 27, "HM90003", 270490, NULL},
  {0x85aa49d0, 29, "ITGX10007", 0, "aka VIOLENT SOLDIER"},
  {0x85b85ff9, 73, "TP02012", 60790, NULL},
  {0x85cc9b60, 45, NULL, 0, NULL},
  {0x86087b39, 42, "NCS89006", 290989, "2p"},
  {0x8793758c, 44, "NC92002", 70492, "aka Samurai Ghost,1p"},
  {0x87fd22ad, 24, "HC90036", 71290, "5p,linkable"},
  {0x88796264, 24, "TGX020035", 0, "aka BE BALL,2p"},
  {0x8a046cdc, 44, "NC92003", 250692, NULL},
  {0x8aa4b220, 24, "HC90037", 100890, "2p"},
  {0x8acfc8aa, 0, NULL, 0, "2p"},
  {0x8bf34ffa, 24, NULL, 0, NULL},
  {0x8c4588e2, 24, "HC91048", 230891, "2p"},
  {0x8c565cb6, 44, "NC89004", 80989, "1p"},
  {0x8dc0d85f, 73, "TP02015", 180191, "2p"},
  {0x8def5aa1, 24, "HC93062", 250693, NULL},
  {0x8e25dc77, 73, "TP02009", 270390, NULL},
  {0x8e4d75a8, 73, "TP02007", 230290, NULL},
  {0x8e71d4f3, 24, NULL, 0, "aka DRAGON'S CURSE,1p"},
  {0x8f02fd20, 24, "HC89018", 250589, NULL},
  {0x8f4d9f94, 31, "IM92001", 0, "aka THE LOST SUNHEART"},
  {0x90ed6575, 27, "HM89001", 220689, NULL},
  {0x9107bcc8, 52, NULL, 0, NULL},
  {0x91e6896f, 19, NULL, 261090, "FA02-005,1p"},
  {0x92521f34, 54, "TGX040062", 0, "1p"},
  {0x92c919ea, 70, "SS90003", 121090, "1p"},
  {0x93f05168, 24, NULL, 0, NULL},
  {0x93f316f7, 24, "TGX030015", 0, "aka NECTARIS,2p"},
  {0x94c55627, 73, "TP01005", 271289, "1p"},
  {0x951aa310, 24, NULL, 0, NULL},
  {0x951ed380, 44, "NC92001", 100192, "1p"},
  {0x958bcd09, 24, "HC90027", 230390, "1p"},
  {0x95f90dec, 41, NULL, 90, NULL},
  {0x965c95b3, 73, "TP02011", 290690, "1p"},
  {0x968770f6, 71, "HC90028", 300390, "aka CHEW-MAN-FU,2p"},
  {0x968d908a, 83, "JC63008", 290391, NULL},
  {0x9693d259, 60, "CJ0001", 131291, "2p"},
  {0x96e0cd9d, 73, "TP01002", 300689, NULL},
  {0x9759a20d, 54, NULL, 0, NULL},
  {0x97c5ee9a, 83, "JC63009", 141290, NULL},
  {0x9893e0e6, 48, "NX90001", 10390, "aka PSYCHOSIS,2p"},
  {0x9897fa86, 19, "FA02-006", 70990, NULL},
  {0x98b03ec9, 0, NULL, 0, "CRACKED TO WORK ON JAP PC-E"},
  {0x99033916, 43, "MR92005", 60392, NULL},
  {0x9913a9de, 24, "TGX080097", 0, "2p"},
  {0x99496db3, 73, "TP02008", 20390, "1p"},
  {0x99f2865c, 24, "HC89025", 250590, "1p"},
  {0x99f7a572, 37, "TJ03002", 290391, "1p,linkable"},
  {0x9a41c638, 13, NULL, 0, "5p"},
  {0x9abb4d1f, 24, "HC90036", 71290, "5p,linkable"},
  {0x9b5ebc58, 16, "DE64001", 30389, "4p"},
  {0x9bb8d362, 78, NULL, 0, NULL},
  {0x9c49ef11, 26, "HC89022", 171189, "1p"},
  {0x9c7a8ee4, 29, NULL, 0, "aka TRICKY"},
  {0x9d1a0f5a, 25, "HC89022", 171189, "1p"},
  {0x9e86ffb0, 42, "NCS89006", 290989, "2p"},
  {0x9ec6fc6c, 22, NULL, 0, NULL},
  {0x9edc0aea, 73, "TPO2014", 141290, NULL},
  {0x9fb4de48, 33, "ICO3006", 190791, NULL},
  {0xa019b724, 44, NULL, 0, NULL},
  {0xa0c97557, 29, "AI-02001", 90390, "1p"},
  {0xa15a1f37, 44, "NC90008", 111290, "1p"},
  {0xa170b60e, 24, "HC93063", 20493, "2p"},
  {0xa17d4d7e, 24, "HC89019", 70789, "aka BLAZING LAZERS,1p"},
  {0xa2a0776e, 84, "VS-89001", 190689, "1p"},
  {0xa32430d5, 55, "NV92001", 310192, NULL},
  {0xa326334a, 62, "SL01001", 221189, "1p"},
  {0xa3303978, 44, "NC90006", 90990, "1p"},
  {0xa5290dd0, 43, "MR91004", 131291, NULL},
  {0xa586d190, 73, NULL, 0, NULL},
  {0xa594fac0, 80, "TON90003", 121090, NULL},
  {0xa6088275, 73, NULL, 0, NULL},
  {0xa6539306, 80, "TON90004", 91190, NULL},
  {0xa71d70d0, 42, "NCS90001", 260190, NULL},
  {0xa80c565f, 33, "IC02003", 270790, NULL},
  {0xa9084d6e, 42, "NCS89004", 0, NULL},
  {0xa98d276a, 29, "TGX030030", 0, "1p"},
  {0xa9a94e1b, 25, NULL, 0, "1p"},
  {0xa9ab2954, 44, NULL, 0, NULL},
  {0xa9fab7d2, 40, "TGX040069", 0, "1p"},
  {0xab3c5804, 24, "HE-1097", 60790, "1p"},
  {0xad226f30, 73, "TP01005", 271289, "1p"},
  {0xad450dfc, 53, NULL, 0, NULL},
  {0xad6e0376, 83, NULL, 0, "3p"},
  {0xae26f30f, 24, "TGX060078", 0, "1p"},
  {0xae9fe1aa, 33, "TGX040050", 0, NULL},
  {0xaf2dd2af, 24, "HC91045", 50791, "1p"},
  {0xb01ee703, 24, "HC92059", 250992, NULL},
  {0xb01f70c2, 16, "DE89003", 10989, "1p"},
  {0xb0ba689f, 51, NULL, 0, NULL},
  {0xb101b333, 48, "XX-XX-90", 0, NULL},
  {0xb122787b, 24, "HC63010", 80788, NULL},
  {0xb18d102d, 48, "NX91003", 120491, NULL},
  {0xb24e6504, 28, "TGX040054", 0, NULL},
  {0xb268f2a2, 56, NULL, 92, NULL},
  {0xb2ef558d, 73, NULL, 0, NULL},
  {0xb300c5d0, 24, "HC92061", 111292, "5p,linkable"},
  {0xb3eeea2e, 44, "NC91004", 181091, "1p"},
  {0xb486a8ed, 12, "NAPH-1008", 270790, "1p"},
  {0xb4d29e3b, 48, "NX91004", 291191, "1p"},
  {0xb5326b16, 80, NULL, 0, NULL},
  {0xb54debd1, 69, "TGX020001", 0, "aka MAJIN EIYU WATARU,1p"},
  {0xb552c906, 54, "TGX020008", 0, "1p"},
  {0xb630ab25, 24, "HC89024", 151289, "1p"},
  {0xb64de6fd, 48, NULL, 0, NULL},
  {0xb74ec562, 17, "DE90006", 80191, NULL},
  {0xb77f2e2f, 43, NULL, 0, NULL},
  {0xb866d282, 22, "T4955754200946", 0, NULL},
  {0xb926c682, 44, "NC90001", 160390, "1p"},
  {0xb9899178, 44, "NC90007", 280990, NULL},
  {0xb99a85b6, 64, "HC91049", 221191, "1p"},
  {0xb9dfc085, 27, "HM89002", 231289, NULL},
  {0xba4d0dd4, 73, "TP03018", 90891, NULL},
  {0xbb3ca04a, 3, "NX91005", 61291, "1p"},
  {0xbb654d1c, 24, "HC92057", 70892, "2p"},
  {0xbb761f3b, 19, "FA02-006", 70990, NULL},
  {0xbc655cf3, 5, "AS01001", 81289, NULL},
  {0xbe62eef5, 16, "DE89002", 10489, NULL},
  {0xbe850530, 29, NULL, 270790, "aka SONIC SPIKE"},
  {0xbe8b6e3b, 58, "PV-1008", 0, NULL},
  {0xbe990010, 48, NULL, 0, NULL},
  {0xbf3e2cc7, 19, "FA64-0001", 10389, NULL},
  {0xbf52788e, 83, "JC63006", 300390, NULL},
  {0xbf797067, 12, "JC63004", 230290, "aka TIGER ROAD"},
  {0xc02b1b59, 34, NULL, 0, NULL},
  {0xc0905ca9, 74, "HC63008", 220488, "aka VICTORY LIFE,5p"},
  {0xc0af0947, 42, NULL, 0, NULL},
  {0xc150637a, 0, "BG01003", 280689, NULL},
  {0xc159761b, 40, NULL, 0, NULL},
  {0xc2287894, 58, NULL, 0, NULL},
  {0xc267e25d, 73, "TP01003", 291189, NULL},
  {0xc28b0d8a, 33, "IC03005", 130391, "2p"},
  {0xc3212c24, 81, "PJ91001", 60491, NULL},
  {0xc356216b, 2, NULL, 0, "2p"},
  {0xc42b6d76, 70, "SS89002", 221289, "1p"},
  {0xc4eb68a5, 44, "NC63004", 110888, NULL},
  {0xc5fdfa89, 24, "HC89020", 80889, "2p"},
  {0xc614116c, 58, "PV1003", 221289, "1p"},
  {0xc6f764ec, 22, "T4955754200922", 0, "1p"},
  {0xc6fa6373, 24, "HC90032", 180191, "1p"},
  {0xc7327632, 42, "NCS90002", 230290, NULL},
  {0xc74ffbc9, 77, "TG90001", 100890, "1p"},
  {0xc7847df7, 24, NULL, 0, NULL},
  {0xc81d0371, 48, NULL, 0, "aka PARANOIA"},
  {0xc8a412e1, 33, NULL, 0, "1p"},
  {0xc8c084e3, 44, "NC89003", 70789, "2p"},
  {0xc8c7d63e, 63, "HG8G-1006", 290989, "aka ALTERED BEAST,1p"},
  {0xc90971ba, 44, "NC90007", 280990, NULL},
  {0xc9d7426a, 48, "NX89002", 100889, "4p"},
  {0xca12afba, 2, "AC90001", 211290, "1p"},
  {0xca68ff21, 42, "NCS89005", 190489, NULL},
  {0xca72a828, 63, "NAPH-1011", 280990, NULL},
  {0xcaad79ce, 73, "TP01004", 221289, NULL},
  {0xcab21b2e, 10, "NX89004", 260190, "1p"},
  {0xcacc06fb, 6, "BG01003", 280689, "aka LEGENDARY AXE II"},
  {0xcae1f5db, 67, "TGX040072", 0, "1p"},
  {0xcc799d92, 11, "MC91002", 240591, NULL},
  {0xcc7d3eeb, 73, NULL, 0, NULL},
  {0xce2e4f9f, 84, "VS-90002", 70290, NULL},
  {0xcec3d28a, 33, "HC63007", 250388, "1p"},
  {0xcf73d8fc, 24, "HC89017", 270489, "1p"},
  {0xcfc5a395, 73, "TP02013", 30890, "aka TOUR OF HELL"},
  {0xcfcfc7be, 44, "NC89003", 70789, "2p"},
  {0xcfec1d6a, 19, "TGX040090", 0, NULL},
  {0xd0c250ca, 19, "FA01-002", 230689, NULL},
  {0xd15cb6bb, 12, "HE93002", 120693, "2p"},
  {0xd20f382f, 79, "NX91001", 150391, NULL},
  {0xd233c05a, 48, "NX90008", 71290, NULL},
  {0xd329cf9a, 24, "HC89020", 80889, "2p"},
  {0xd3fd6971, 24, "HC90037", 100890, "2p"},
  {0xd4c5af46, 6, NULL, 0, NULL},
  {0xd50ff730, 52, "H54G-1005", 250889, NULL},
  {0xd5c782f2, 24, NULL, 0, "1p"},
  {0xd5ce2d5f, 24, "HC89021", 150989, NULL},
  {0xd634d931, 54, NULL, 0, NULL},
  {0xd6fc51ce, 22, "T4955754200984", 0, "2p"},
  {0xd7921df2, 52, "H54G-1003", 270189, NULL},
  {0xd7cfd70f, 56, "NB91004", 120791, NULL},
  {0xd8373de6, 84, "VS-90003", 191090, NULL},
  {0xd9e1549a, 24, "HC90040", 221290, NULL},
  {0xda059c9b, 83, "JC63008", 290391, NULL},
  {0xdb872a64, 77, "TTGX20001", 0, "1p"},
  {0xdc268242, 54, "TGX020008", 0, "1p"},
  {0xdc47434b, 29, "AI-02002", 20690, "1p"},
  {0xdc760a07, 24, "HC89023", 311089, "aka CRATERMAZE,1p"},
  {0xdca24a76, 63, "H49G-1001", 141088, NULL},
  {0xdcd3e602, 19, "FA01-002", 230689, NULL},
  {0xdcf3675c, 44, NULL, 261288, NULL},
  {0xdd0ebf8c, 59, "PL91001", 151191, "1p"},
  {0xdd175efd, 82, "UP01001", 190190, NULL},
  {0xdd1d1035, 42, "NCS89005", 190489, NULL},
  {0xdd35451d, 54, NULL, 0, NULL},
  {0xddc3e809, 63, "NAPH-1015", 71290, "1p"},
  {0xde963b7e, 39, "H54G-1004", 140789, NULL},
  {0xdf10c895, 82, NULL, 0, NULL},
  {0xdf804dc7, 70, "SS90001", 20390, "1p"},
  {0xdfd4593a, 33, "IC04007", 21092, "1p"},
  {0xe14dee08, 56, "NB89002", 140990, NULL},
  {0xe1a73797, 70, NULL, 0, NULL},
  {0xe203f223, 63, "NAPH-1016", 211290, NULL},
  {0xe3284ba7, 24, "TGX040084", 0, "1p"},
  {0xe4124fe0, 33, NULL, 0, "1p"},
  {0xe415ea19, 24, "TGX040080", 0, "1p"},
  {0xe439f299, 24, "HC90028", 300390, "aka CHEW-MAN-FU,2p"},
  {0xe4b3415c, 83, "JC63006", 300390, NULL},
  {0xe5b6b3e5, 22, "T4955754200977", 0, NULL},
  {0xe5e7b8b7, 12, "H54G-1004", 140789, NULL},
  {0xe6458212, 73, NULL, 0, "2p"},
  {0xe6c93ae7, 47, "NX90007", 211290, NULL},
  {0xe6ee1468, 58, "PV1004", 270790, NULL},
  {0xe6f16616, 54, "PCE-SC1", 0, NULL},
  {0xe749a22c, 72, "TP03021", 130392, NULL},
  {0xe7529890, 83, "JC63013", 290493, NULL},
  {0xe84890a5, 73, NULL, 0, NULL},
  {0xe8702d51, 33, "IC04007", 21092, "1p"},
  {0xe87190f1, 6, "AT03001", 41091, "aka SOMER ASSAULT,1p"},
  {0xe88987bb, 27, "HM91005", 300891, NULL},
  {0xea324f07, 24, "HC89018", 250589, NULL},
  {0xeb4e600b, 33, "IC63001", 140189, "1p"},
  {0xeb923de5, 73, "TP01004", 221289, NULL},
  {0xeb9452f0, 42, NULL, 0, NULL},
  {0xed3a71f8, 24, "TGX040079", 0, "1p"},
  {0xeda32d95, 54, "TGX040056", 0, "1p"},
  {0xee156721, 43, "MR91003", 80391, "1p"},
  {0xeeb6dd43, 80, NULL, 0, NULL},
  {0xeecfa5fd, 38, "TGX040061", 0, "4p"},
  {0xf0227837, 6, "TGX040089", 0, "aka MESOPOTAMIA"},
  {0xf022be13, 2, "AC89002", 280789, NULL},
  {0xf0ab946e, 25, "HC91047", 270991, "1p"},
  {0xf0cc3363, 84, NULL, 0, NULL},
  {0xf207ecae, 33, "HC63009", 30688, "1p"},
  {0xf2e46d25, 4, "AK91002", 221191, NULL},
  {0xf2e6856d, 22, NULL, 0, NULL},
  {0xf370b58e, 54, "TGX040060", 0, NULL},
  {0xf4148600, 19, "FA02-004", 290690, "1p"},
  {0xf42aa73e, 73, "TP02010", 310590, "2p"},
  {0xf45afbca, 73, NULL, 0, NULL},
  {0xf46298e3, 83, "JC63005", 290690, "3p"},
  {0xf5b90d55, 19, "FA01-003", 241189, "1p"},
  {0xf70112e5, 32, "IG89001", 11189, NULL},
  {0xf79657dd, 24, "HC63015", 40389, "5p"},
  {0xf860455c, 68, "HC92060", 41292, NULL},
  {0xf8861456, 70, "SS90002", 100890, NULL},
  {0xf8f85eec, 44, "NC90004", 290690, "1p"},
  {0xf91b055f, 70, "SS90001", 20390, "1p"},
  {0xf999356f, 24, "HC63014", 181188, NULL},
  {0xfaa6e187, 76, NULL, 0, NULL},
  {0xfae0fc60, 67, "TGX040072", 0, "1p"},
  {0xfaecce20, 35, "KM91002", 61291, "2p"},
  {0xfb0fdcfe, 4, "AK90001", 200490, NULL},
  {0xfb37ddc4, 43, NULL, 0, "aka SOHKO BAN WORLD"},
  {0xfba3a1a4, 71, "NX90006", 261090, NULL},
  {0xfde08d6d, 48, "NX91002", 220391, "2p"},
  {0xff898f87, 52, "NAPH-1010", 310890, NULL},
  {0xffd92458, 22, "T4955754200953", 0, "1p"}
};


typedef struct st_pce_header
{
  char pad[48];
} st_pce_header_t;

st_pce_header_t pce_header;


static int
pce_compare (const void *key, const void *found)
{
  /*
    The return statement looks overly complicated, but is really necessary.
    This construct:
      return ((st_pce_data_t *) key)->crc32 - ((st_pce_data_t *) found)->crc32;
    does *not* work correctly for all cases.
  */
  return (int) (((int64_t) ((st_pce_data_t *) key)->crc32 -
                 (int64_t) ((st_pce_data_t *) found)->crc32) / 2);
}


static void
swapbits (unsigned char *buffer, int size)
{
  int n, byte, bit;

  for (n = 0; n < size; n++)
    {
      byte = 0;
      for (bit = 0; bit < 8; bit++)
        if (buffer[n] & (1 << bit))
          byte |= 1 << (7 - bit);
      buffer[n] = byte;
    }
}


// header format is specified in src/backup/ffe.h
int
pcengine_msg (st_rominfo_t *rominfo)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *rom_buffer = NULL;
  st_msg_header_t header;
  int size = ucon64.file_size - rominfo->buheader_len;

  if (rominfo->interleaved)
    if (!(rom_buffer = (unsigned char *) malloc (size)))
      {
        fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
        return -1;
      }

  memset (&header, 0, MSG_HEADER_LEN);
  header.size = size / 8192;
  header.emulation = size == 3 * MBIT ? 1 : 0;
  header.id1 = 0xaa;
  header.id2 = 0xbb;
  header.type = 2;

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  set_suffix (dest_name, ".msg");
  ucon64_file_handler (dest_name, src_name, 0);

  ucon64_fwrite (&header, 0, MSG_HEADER_LEN, dest_name, "wb");
  if (rominfo->interleaved)
    {
      // Magic Super Griffin files should not be "interleaved"
      ucon64_fread (rom_buffer, rominfo->buheader_len, size, src_name);
      swapbits (rom_buffer, size);
      ucon64_fwrite (rom_buffer, MSG_HEADER_LEN, size, dest_name, "ab");
      free (rom_buffer);
    }
  else
    fcopy (src_name, rominfo->buheader_len, size, dest_name, "ab");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


// see src/backup/mgd.h for the file naming scheme
int
pcengine_mgd (st_rominfo_t *rominfo)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *rom_buffer = NULL;
  int size = ucon64.file_size - rominfo->buheader_len;

  if (!rominfo->interleaved)
    if (!(rom_buffer = (unsigned char *) malloc (size)))
      {
        fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
        return -1;
      }

  strcpy (src_name, ucon64.rom);
  mgd_make_name (ucon64.rom, UCON64_PCE, size, dest_name);
  ucon64_file_handler (dest_name, src_name, OF_FORCE_BASENAME);

  // bit-swapping images for the MGD2 only makes sense for owners of a TG-16
  //  (American version of the PCE)
  if (!rominfo->interleaved)
    {
      ucon64_fread (rom_buffer, rominfo->buheader_len, size, src_name);
      swapbits (rom_buffer, size);
      ucon64_fwrite (rom_buffer, 0, size, dest_name, "wb");
      free (rom_buffer);
    }
  else
    fcopy (src_name, rominfo->buheader_len, size, dest_name, "wb");

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();

  mgd_write_index_file ((char *) basename2 (dest_name), 1);
  return 0;
}


int
pcengine_swap (st_rominfo_t *rominfo)
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX];
  unsigned char *rom_buffer;
  int size = ucon64.file_size - rominfo->buheader_len;

  if (!(rom_buffer = (unsigned char *) malloc (size)))
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      return -1;
    }

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);

  if (rominfo->buheader_len)                    // copy header (if present)
    fcopy (src_name, 0, rominfo->buheader_len, dest_name, "wb");

  ucon64_fread (rom_buffer, rominfo->buheader_len, size, src_name);
  swapbits (rom_buffer, size);
  ucon64_fwrite (rom_buffer, rominfo->buheader_len, size, dest_name,
            rominfo->buheader_len ? "ab" : "wb");
  free (rom_buffer);

  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return 0;
}


int
pcengine_f (st_rominfo_t *rominfo)
/*
  Region protection codes are found in (American) TurboGrafx-16 games. It
  prevents those games from running on a PC-Engine. One search pattern seems
  sufficient to fix/crack all TG-16 games. In addition to that, the protection
  code appears to be always somewhere in the first 32 kB.
*/
{
  char src_name[FILENAME_MAX], dest_name[FILENAME_MAX], buffer[32 * 1024];
  int bytesread, n;

  puts ("Attempting to fix region protection code...");

  strcpy (src_name, ucon64.rom);
  strcpy (dest_name, ucon64.rom);
  ucon64_file_handler (dest_name, src_name, 0);
  fcopy (src_name, 0, ucon64.file_size, dest_name, "wb"); // no copy if one file

  if ((bytesread = ucon64_fread (buffer, rominfo->buheader_len, 32 * 1024, src_name)) <= 0)
    return -1;

  // '!' == ASCII 33 (\x21), '*' == 42 (\x2a)
  if (rominfo->interleaved)
    n = change_mem (buffer, bytesread, "\x94\x02\x0f", 3, '*', '!', "\x01", 1, 0);
  else
    n = change_mem (buffer, bytesread, "\x29\x40\xf0", 3, '*', '!', "\x80", 1, 0);

  ucon64_fwrite (buffer, rominfo->buheader_len, 32 * 1024, dest_name, "r+b");

  printf ("Found %d pattern%s\n", n, n != 1 ? "s" : "");
  printf (ucon64_msg[WROTE], dest_name);
  remove_temp_file ();
  return n;
}


static void
write_game_table_entry (FILE *destfile, int file_no, int totalsize)
{
  int n;
  unsigned char name[0x1c];
  const char *p;

  fseek (destfile, 0xb000 + (file_no - 1) * 0x20, SEEK_SET);
  fputc (0xff, destfile);                       // 0x0 = 0xff

  memset (name, ' ', 0x1c);
  p = basename2 (ucon64.rom);
  n = strlen (p);
  if (n > 0x1c)
    n = 0x1c;
  memcpy (name, p, n);
  for (n = 0; n < 0x1c; n++)
    {
      if (!isprint ((int) name[n]))
        name[n] = '.';
      else
        name[n] = toupper (name[n]);            // loader only supports upper case characters
    }
  fwrite (name, 1, 0x1c, destfile);             // 0x1 - 0x1c = name
  fputc (0, destfile);                          // 0x1d = 0
  fputc (totalsize / MBIT, destfile);           // 0x1e = bank code
  fputc (0, destfile);                          // 0x1f = flags (x, D (reserved), x, x, x, x, x, x)
}


int
pcengine_multi (int truncate_size, char *fname)
{
#define BUFSIZE (32 * 1024)
  int n, n_files, file_no, bytestowrite, byteswritten, totalsize = 0, done,
      truncated = 0, paddedsize, org_do_not_calc_crc = ucon64.do_not_calc_crc;
  struct stat fstate;
  FILE *srcfile, *destfile;
  char destname[FILENAME_MAX];
  unsigned char buffer[BUFSIZE];

  if (truncate_size == 0)
    {
      fprintf (stderr, "ERROR: Can't make multi-game file of 0 bytes\n");
      return -1;
    }

  if (fname != NULL)
    {
      strcpy (destname, fname);
      n_files = ucon64.argc;
    }
  else
    {
      strcpy (destname, ucon64.argv[ucon64.argc - 1]);
      n_files = ucon64.argc - 1;
    }

  ucon64_file_handler (destname, NULL, OF_FORCE_BASENAME);
  if ((destfile = fopen (destname, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], destname);
      return -1;
    }

  printf ("Creating multi-game file for PCE-PRO: %s\n", destname);

  file_no = 0;
  for (n = 1; n < n_files; n++)
    {
      if (access (ucon64.argv[n], F_OK))
        continue;                               // "file" does not exist (option)
      stat (ucon64.argv[n], &fstate);
      if (!S_ISREG (fstate.st_mode))
        continue;
      if (file_no == 32)                        // loader + 31 games
        {
          printf ("WARNING: A multi-game file can contain a maximum of 31 games. The other files\n"
                  "         are ignored.\n");
          break;
        }

      ucon64.console = UCON64_UNKNOWN;
      ucon64.rom = ucon64.argv[n];
      ucon64.file_size = fsizeof (ucon64.rom);
      // DON'T use fstate.st_size, because file could be compressed
      ucon64.rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
                                       ucon64.buheader_len : 0;
      ucon64.rominfo->interleaved = UCON64_ISSET (ucon64.interleaved) ?
                                       ucon64.interleaved : 0;
      ucon64.do_not_calc_crc = 1;
      if (pcengine_init (ucon64.rominfo) != 0)
        printf ("WARNING: %s does not appear to be a PC-Engine ROM\n", ucon64.rom);

      if ((srcfile = fopen (ucon64.rom, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], ucon64.rom);
          continue;
        }
      if (ucon64.rominfo->buheader_len)
        fseek (srcfile, ucon64.rominfo->buheader_len, SEEK_SET);

      if (file_no == 0)
        {
          printf ("Loader: %s\n", ucon64.rom);
          if (ucon64.file_size - ucon64.rominfo->buheader_len != 7 * 8192)
            printf ("WARNING: Are you sure %s is a loader binary?\n", ucon64.rom);
        }
      else
        {
          printf ("ROM%d: %s\n", file_no, ucon64.rom);
          write_game_table_entry (destfile, file_no, totalsize);
          fseek (destfile, totalsize, SEEK_SET); // restore file pointer
        }
      file_no++;

      done = 0;
      byteswritten = 0;                         // # of bytes written per file
      while (!done)
        {
          bytestowrite = fread (buffer, 1, BUFSIZE, srcfile);
          if (totalsize + bytestowrite > truncate_size)
            {
              bytestowrite = truncate_size - totalsize;
              done = 1;
              truncated = 1;
              printf ("Output file is %d Mbit, truncating %s, skipping %d bytes\n",
                      truncate_size / MBIT, ucon64.rom, ucon64.file_size -
                        ucon64.rominfo->buheader_len - (byteswritten + bytestowrite));
            }
          totalsize += bytestowrite;
          if (bytestowrite == 0)
            done = 1;
          fwrite (buffer, 1, bytestowrite, destfile);
          byteswritten += bytestowrite;
        }
      fclose (srcfile);
      if (truncated)
        break;

      // games have to be aligned to (start at) a 1 Mbit boundary
      paddedsize = (totalsize + MBIT - 1) & ~(MBIT - 1);
//      printf ("paddedsize: %d (%f); totalsize: %d (%f)\n",
//              paddedsize, paddedsize / (1.0 * MBIT), totalsize, totalsize / (1.0 * MBIT));
      if (paddedsize > totalsize)
        {
          if (paddedsize > truncate_size)
            {
              truncated = 1;                    // not *really* truncated
              paddedsize = truncate_size;
            }

          memset (buffer, 0, BUFSIZE);
          while (totalsize < paddedsize)
            {
              bytestowrite = paddedsize - totalsize > BUFSIZE ?
                              BUFSIZE : paddedsize - totalsize;
              fwrite (buffer, 1, bytestowrite, destfile);
              totalsize += bytestowrite;
            }
        }
      if (truncated)
        break;
    }
  // fill the next game table entry
  fseek (destfile, 0xb000 + (file_no - 1) * 0x20, SEEK_SET);
  fputc (0, destfile);                          // indicate no next game
  fclose (destfile);
  ucon64.console = UCON64_PCE;
  ucon64.do_not_calc_crc = org_do_not_calc_crc;

  return 0;
}


static int
pcengine_check (unsigned char *buf, unsigned int len)
// This function was contributed by Cowering. Comments are his unless stated
//  otherwise.
{
  int i, f1 = 0, f2 = 0, f3 = 0, f4 = 0, f5 = 0;

  // don't crash for too small (non-PCE) files - dbjh
  if (len < 0x200)
    return 0;

  // Directly look for swapped dead moon (U). Lots of zeroes at file start
  //  fakes out header detector.
  // We probably don't need this (no "header detector") - dbjh
  if (buf[0] == 0x1e && buf[1] == 0x2a && buf[2] == 0x95)
    return 1;
  if (buf[0] == 0x12 && buf[1] == 0xe0 && buf[16] == 0x4a) // F-1 Pilot
    return 1;
  if (buf[0] == 0x20 && buf[1] == 0x81 && buf[16] == 0x81) // Cyber Knight
    return 1;
  if (buf[0] == 0x03 && buf[1] == 0x03 && buf[16] == 0x02) // Gaia no monsho
    return 1;
#if 0 // no reason to not recognise this dump - dbjh
  if (len == 0x40400) // BIOS with 2 headers on front, deleted from database now
    return 0;
#endif
/*
  // two headers on a CD-ROM BIOS (bad dump)
  if (len == 0x40200 && buf[0x200] == 0x32 && buf[0x210] == 0xa0 && buf[0x220] == 0xf7)
    return 1;
*/
  // super mario pirate conversion
  if (len > 65000 && !strncmp ((const char*) &buf[0x77f0], "JGGKGKGGGGGGJGJG", 16))
    return 1;
  if (buf[0x20] == 0x7c && buf[0x21] == 0xe3 && buf[0x22] == 0xe6) // 5 in 1
    return 1;
  if (buf[0x10] == 0xc7 && buf[0x30] == 0xe9 && buf[0x50] == 0xa1) // Sekikehara
    return 1;
  if (!strncmp ((const char *) &buf[0], "!BM FORMAT!", 11)) // boxy boy (U)
    return 1;
  if (!strncmp ((const char *) &buf[0], "PUSH RUN BUTTON", 15)) // boxy boy (J)
    return 1;
  // US CD sys 2.01
  if (len > 0x3ffc7 && !strncmp ((const char *) &buf[0x3ffb6], "PC Engine CD-ROM", 16))
    return 1;

  for (i = 0 ; i < 0x200; i++)
    {
      if (buf[i] == 0x00)
        f1++;
      if (buf[i] == 0xff)
        f4++;
      if (buf[i] == 0x78)
        f2 = 1;
      if (buf[i] == 0xa9)
        f3 = 1;
      if (buf[i] == 0x85 || buf[i] == 0x8d)
        f5 = 1;
    }
  if ((f4 > 200) || ((f1 > 200) && (f2 + f3 != 2)))
    return 0;
  if (f2 + f3 + f5 >= 2)
    return 1;
  return 0;
}


int
pcengine_init (st_rominfo_t *rominfo)
{
  int result = -1, size, swapped, x;
  unsigned char *rom_buffer;
  st_pce_data_t *info, key;

  x = ucon64.file_size % (16 * 1024);
  rominfo->buheader_len = UCON64_ISSET (ucon64.buheader_len) ?
    ucon64.buheader_len : (ucon64.file_size > x ? x : 0);

  size = ucon64.file_size - rominfo->buheader_len;
  if (!(rom_buffer = (unsigned char *) malloc (size)))
    {
      fprintf (stderr, ucon64_msg[ROM_BUFFER_ERROR], size);
      return -1;
    }
  ucon64_fread (rom_buffer, rominfo->buheader_len, size, ucon64.rom);

  if (pcengine_check (rom_buffer, size) == 1)
    result = 0;

  swapped = -1;
  /*
    29 40 f0 => unhacked
    29 40 80 => U.S.A. -> Jap hack
    94 02 0f => bit-swapped and unhacked
    94 02 01 => bit-swapped and hacked

    According to Cowering 2 or 3 games don't use these opcodes to check if a
    Japanese game is running on an American console. If they are present then
    it is in the first 500 (or so) bytes.
    The check for "A..Z" is a fix for Puzznic (J). The check for "HESM" a fix
    for:
    Fire Pro Wrestling - 2nd Bout Sounds
    Fire Pro Wrestling 3 - Legend Bout Sounds
  */
  x = size > 32768 ? 32768 : size;
  if ((memmem2 (rom_buffer, x, "\x94\x02\x0f", 3, 0) ||
       memmem2 (rom_buffer, x, "\x94\x02\x01", 3, 0)) &&
       memmem2 (rom_buffer, x, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26, 0) == 0 &&
       memcmp (rom_buffer, "HESM", 4))
    swapped = 1;
  if (UCON64_ISSET (ucon64.interleaved))
    swapped = ucon64.interleaved;

  if ((result == -1 && swapped != 0) || swapped == 1)
    {                                   // don't swap the bits if -nint is specified
      if (!UCON64_ISSET (ucon64.do_not_calc_crc) || swapped == 1)
        ucon64.fcrc32 = crc32 (0, rom_buffer, size);
      swapbits (rom_buffer, size);
      if (pcengine_check (rom_buffer, size) == 1)
        {
          swapped = 1;
          result = 0;
        }
      if (swapped != 1)
        {
          ucon64.crc32 = ucon64.fcrc32;
          ucon64.fcrc32 = 0;
        }
    }
  if (swapped != -1)
    rominfo->interleaved = swapped;

  if (ucon64.console == UCON64_PCE)
    result = 0;

  rominfo->header_start = PCENGINE_HEADER_START;
  rominfo->header_len = PCENGINE_HEADER_LEN;
  rominfo->console_usage = pcengine_usage[0].help;
  rominfo->copier_usage = rominfo->buheader_len ? msg_usage[0].help : mgd_usage[0].help;

  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      if (!ucon64.crc32)
        ucon64.crc32 = crc32 (0, rom_buffer, size);
      // additional info
      key.crc32 = ucon64.crc32;
      info = (st_pce_data_t *) bsearch (&key, pce_data,
                                        sizeof pce_data / sizeof (st_pce_data_t),
                                        sizeof (st_pce_data_t), pce_compare);
      if (info)
        {
          if (info->maker)
            rominfo->maker = NULL_TO_UNKNOWN_S (pce_maker[MIN (info->maker,
                                                               PCE_MAKER_MAX - 1)]);

          if (info->serial)
            if (info->serial[0])
              {
                strcat (rominfo->misc, "\nSerial: ");
                strcat (rominfo->misc, info->serial);
              }

          if (info->date)
            {
              int day = info->date / 10000, month = (info->date % 10000) / 100,
                  year = info->date % 100;
              char date[80];

              date[0] = 0;
              if (day)
                sprintf (date, "\nDate: %d/%d/19%d", day, month, year);
              else if (month)
                sprintf (date, "\nDate: %d/19%d", month, year);
              else if (year)
                sprintf (date, "\nDate: 19%d", year);
              strcat (rominfo->misc, date);
            }

          if (info->comment)
            if (info->comment[0])
              {
                strcat (rominfo->misc, "\nComment: ");
                strcat (rominfo->misc, info->comment);
              }
        }
    }

  free (rom_buffer);
  return result;
}
