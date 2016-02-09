/*
nds.c - Nintendo DS support for uCON64

Copyright (c) 2005 NoisyB


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
#include "misc/archive.h"
#include "misc/chksum.h"
#include "misc/file.h"
#include "misc/misc.h"
#include "ucon64_misc.h"
#include "console/console.h"
#include "console/nds.h"
#include "backup/backup.h"
#include "backup/nfc.h"


#define NDS_NAME_LEN 12
#define NDS_HEADER_START 0
#define NDS_HEADER_LEN (sizeof (st_nds_header_t))
#define NDS_LOGODATA_LEN 156


static int nds_chksum (void);


static st_ucon64_obj_t nds_obj[] =
  {
    {0, WF_DEFAULT},
    {UCON64_NDS, WF_SWITCH}
  };

const st_getopt2_t nds_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Nintendo DS"/*"2005 Nintendo http://www.nintendo.com"*/,
      NULL
    },
    {
      UCON64_NDS_S, 0, 0, UCON64_NDS,
      NULL, "force recognition",
      &nds_obj[1]
    },
    {
      "n", 1, 0, UCON64_N,
      "NEW_NAME", "change internal ROM name to NEW_NAME",
      &nds_obj[0]
    },
    {
      "logo", 0, 0, UCON64_LOGO,
      NULL, "restore ROM logo character data",
      &nds_obj[0]
    },
    {
      "chk", 0, 0, UCON64_CHK,
      NULL, "fix ROM header checksum",
      &nds_obj[0]
    },
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


typedef struct st_nds_header
{
  char title[NDS_NAME_LEN];
  char gamecode[3];
  char game_id_country;
  unsigned char maker_high;
  unsigned char maker_low;
  unsigned char unitcode;
  unsigned char devicetype;             // type of device in the game card
  unsigned char devicecap;              // device capacity (1<<n Mbit)
  unsigned char pad1[9];
  unsigned char romversion;
  unsigned char reserved2;
  uint32_t arm9_rom_offset;
  uint32_t arm9_entry_address;
  uint32_t arm9_ram_address;
  uint32_t arm9_size;
  uint32_t arm7_rom_offset;
  uint32_t arm7_entry_address;
  uint32_t arm7_ram_address;
  uint32_t arm7_size;
  uint32_t fnt_offset;
  uint32_t fnt_size;
  uint32_t fat_offset;
  uint32_t fat_size;
  uint32_t arm9_overlay_offset;
  uint32_t arm9_overlay_size;
  uint32_t arm7_overlay_offset;
  uint32_t arm7_overlay_size;
  uint32_t rom_control_info1;
  uint32_t rom_control_info2;
  uint32_t banner_offset;
  uint16_t secure_area_crc;
  uint16_t rom_control_info3;
  uint32_t pad2[4];                     // pad1[2] & pad1[3] are used as ID for homebrewed games
  uint32_t application_end_offset;      // rom size
  uint32_t rom_header_size;
  uint32_t pad3[14];
  unsigned char logo[NDS_LOGODATA_LEN];
  uint16_t logo_crc;
  uint16_t header_crc;
  uint32_t pad4[4];
  unsigned char zero[144];
} st_nds_header_t;


static st_nds_header_t nds_header;


const unsigned char nds_logodata[NDS_LOGODATA_LEN] =
{
  0x24, 0xff, 0xae, 0x51, 0x69, 0x9a, 0xa2, 0x21,
  0x3d, 0x84, 0x82, 0x0a, 0x84, 0xe4, 0x09, 0xad,
  0x11, 0x24, 0x8b, 0x98, 0xc0, 0x81, 0x7f, 0x21,
  0xa3, 0x52, 0xbe, 0x19, 0x93, 0x09, 0xce, 0x20,
  0x10, 0x46, 0x4a, 0x4a, 0xf8, 0x27, 0x31, 0xec,
  0x58, 0xc7, 0xe8, 0x33, 0x82, 0xe3, 0xce, 0xbf,
  0x85, 0xf4, 0xdf, 0x94, 0xce, 0x4b, 0x09, 0xc1,
  0x94, 0x56, 0x8a, 0xc0, 0x13, 0x72, 0xa7, 0xfc,
  0x9f, 0x84, 0x4d, 0x73, 0xa3, 0xca, 0x9a, 0x61,
  0x58, 0x97, 0xa3, 0x27, 0xfc, 0x03, 0x98, 0x76,
  0x23, 0x1d, 0xc7, 0x61, 0x03, 0x04, 0xae, 0x56,
  0xbf, 0x38, 0x84, 0x00, 0x40, 0xa7, 0x0e, 0xfd,
  0xff, 0x52, 0xfe, 0x03, 0x6f, 0x95, 0x30, 0xf1,
  0x97, 0xfb, 0xc0, 0x85, 0x60, 0xd6, 0x80, 0x25,
  0xa9, 0x63, 0xbe, 0x03, 0x01, 0x4e, 0x38, 0xe2,
  0xf9, 0xa2, 0x34, 0xff, 0xbb, 0x3e, 0x03, 0x44,
  0x78, 0x00, 0x90, 0xcb, 0x88, 0x11, 0x3a, 0x94,
  0x65, 0xc0, 0x7c, 0x63, 0x87, 0xf0, 0x3c, 0xaf,
  0xd6, 0x25, 0xe4, 0x8b, 0x38, 0x0a, 0xac, 0x72,
  0x21, 0xd4, 0xf8, 0x07
};


int
nds_n (st_ucon64_nfo_t *rominfo, const char *name)
{
  char buf[NDS_NAME_LEN], dest_name[FILENAME_MAX];

  memset (buf, 0, NDS_NAME_LEN);
  strncpy (buf, name, NDS_NAME_LEN);
  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");
  ucon64_fwrite (buf, NDS_HEADER_START + rominfo->backup_header_len, NDS_NAME_LEN,
                 dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
nds_logo (st_ucon64_nfo_t *rominfo)
{
  char dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");
  ucon64_fwrite (nds_logodata, NDS_HEADER_START + rominfo->backup_header_len + 192,
                 NDS_LOGODATA_LEN, dest_name, "r+b");

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
nds_chk (st_ucon64_nfo_t *rominfo)
{
  unsigned char *p = NULL;
  char dest_name[FILENAME_MAX];

  strcpy (dest_name, ucon64.fname);
  ucon64_file_handler (dest_name, NULL, 0);
  fcopy (ucon64.fname, 0, ucon64.file_size, dest_name, "wb");

  p = (unsigned char *) &rominfo->current_internal_crc;
  ucon64_fwrite (p, NDS_HEADER_START + rominfo->backup_header_len + 0x15e,
    2, dest_name, "r+b");

  dumper (stdout, p, 2, NDS_HEADER_START + rominfo->backup_header_len + 0x15e, DUMPER_HEX);

  printf (ucon64_msg[WROTE], dest_name);
  return 0;
}


int
nds_init (st_ucon64_nfo_t *rominfo)
{
  int result = -1, value, pos;
  char buf[144];

  rominfo->backup_header_len = UCON64_ISSET (ucon64.backup_header_len) ?
    ucon64.backup_header_len : 0;

  ucon64_fread (&nds_header, NDS_HEADER_START + rominfo->backup_header_len,
                NDS_HEADER_LEN, ucon64.fname);

  // identify the ROM by the zero area
  memset (&buf, 0, 144);
  if (!memcmp (nds_header.zero, buf, 144))
    result = 0;
  else
    result = -1;
  if (ucon64.console == UCON64_NDS)
    result = 0;

  rominfo->header_start = NDS_HEADER_START;
  rominfo->header_len = NDS_HEADER_LEN;
  rominfo->header = &nds_header;

  // internal ROM name
  strncpy (rominfo->name, (const char *) nds_header.title, NDS_NAME_LEN);
  rominfo->name[NDS_NAME_LEN] = 0;

  // ROM maker
  {
    int ih = nds_header.maker_high <= '9' ?
               nds_header.maker_high - '0' : nds_header.maker_high - 'A' + 10,
        il = nds_header.maker_low <= '9' ?
               nds_header.maker_low - '0' : nds_header.maker_low - 'A' + 10;
    value = ih * 36 + il;
  }
  if (value < 0 || value >= NINTENDO_MAKER_LEN)
    value = 0;
  rominfo->maker = NULL_TO_UNKNOWN_S (nintendo_maker[value]);

  // ROM country
  rominfo->country =
    (nds_header.game_id_country == 'J') ? "Japan/Asia" :
    (nds_header.game_id_country == 'E') ? "U.S.A." :
    (nds_header.game_id_country == 'P') ? "Europe, Australia and Africa" :
    "Unknown country";

  // misc stuff
  pos = strlen (rominfo->misc);
  pos += sprintf (rominfo->misc + pos, "Version: v1.%d\n", nds_header.romversion);
  pos += sprintf (rominfo->misc + pos, "Unit code: 0x%02x\n", nds_header.unitcode);
  pos += sprintf (rominfo->misc + pos, "Device type: 0x%02x\n", nds_header.devicetype);
  pos += sprintf (rominfo->misc + pos, "Device capacity: %d Mb\n", 1 < nds_header.devicecap);

  pos += sprintf (rominfo->misc + pos, "Logo data: ");
  if (memcmp (nds_header.logo, nds_logodata, NDS_LOGODATA_LEN) == 0)
    {
#ifdef  USE_ANSI_COLOR
      if (ucon64.ansi_color)
        pos += sprintf (rominfo->misc + pos, "\x1b[01;32mOK\x1b[0m");
      else
#endif
        pos += sprintf (rominfo->misc + pos, "OK");
    }
  else
    {
#ifdef  USE_ANSI_COLOR
      if (ucon64.ansi_color)
        pos += sprintf (rominfo->misc + pos, "\x1b[01;31mBad\x1b[0m");
      else
#endif
        pos += sprintf (rominfo->misc + pos, "Bad");
    }

  // internal ROM crc
  if (!UCON64_ISSET (ucon64.do_not_calc_crc) && result == 0)
    {
      rominfo->has_internal_crc = 1;
      rominfo->internal_crc_len = 1;
      rominfo->current_internal_crc = nds_chksum ();

      rominfo->internal_crc = nds_header.header_crc;
      rominfo->internal_crc2[0] = 0;
    }

  rominfo->console_usage = nds_usage[0].help;
  rominfo->backup_usage = (!rominfo->backup_header_len ? nfc_usage[0].help : unknown_backup_usage[0].help);

  return result;
}


static int
nds_chksum (void)
// Note that this function only calculates the checksum of the internal header
{
  return (~chksum_crc16 (0, &nds_header, 0x15e)) & 0xffff;
}
