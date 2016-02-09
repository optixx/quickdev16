/*
uCON64 - a tool to modify video game ROMs and to transfer ROMs to the
different backup units/emulators that exist. It is based on the old uCON but
with completely new source. It aims to support all cartridge consoles and
handhelds like N64, JAG, SNES, NG, GENESIS, GB, LYNX, PCE, SMS, GG, NES and
their backup units

Copyright (c) 1999 - 2005       NoisyB
Copyright (c) 2001 - 2005, 2015 dbjh


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

/*
  First I want to thank SiGMA SEVEN! who was my mentor and taught me how to
  write programs in C.
*/
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <stdlib.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef  _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#endif
#include <sys/stat.h>
#ifdef  _MSC_VER
#pragma warning(pop)
#endif

#ifdef  DEBUG
#ifdef  __GNUC__
#warning DEBUG active
#else
#pragma message ("DEBUG active")
#endif
#endif
#include "misc/archive.h"
#include "misc/file.h"
#include "misc/getopt2.h"                       // st_getopt2_t
#include "misc/misc.h"
#include "misc/parallel.h"
#include "misc/property.h"
#include "misc/term.h"
#include "ucon64.h"
#include "ucon64_dat.h"
#include "ucon64_misc.h"
#include "ucon64_opts.h"
#include "console/atari.h"
#include "console/coleco.h"
#include "console/console.h"
#include "console/dc.h"
#include "console/gb.h"
#include "console/gba.h"
#include "console/genesis.h"
#include "console/jaguar.h"
#include "console/lynx.h"
#include "console/n64.h"
#include "console/nds.h"
#include "console/neogeo.h"
#include "console/nes.h"
#include "console/ngp.h"
#include "console/pce.h"
#include "console/psx.h"
#include "console/sms.h"
#include "console/snes.h"
#include "console/swan.h"
#include "console/vboy.h"
#include "backup/backup.h"
#include "backup/cd64.h"
#include "backup/cmc.h"
#include "backup/dex.h"
#include "backup/doctor64.h"
#include "backup/doctor64jr.h"
#include "backup/f2a.h"
#include "backup/fal.h"
#include "backup/gbx.h"
#include "backup/gd.h"
#include "backup/lynxit.h"
#include "backup/mccl.h"
#include "backup/mcd.h"
#include "backup/md-pro.h"
#include "backup/msg.h"
#include "backup/pce-pro.h"
#include "backup/pl.h"
#include "backup/quickdev16.h"
#include "backup/sflash.h"
#include "backup/smc.h"
#include "backup/smd.h"
#include "backup/smsgg-pro.h"
#include "backup/swc.h"
#include "patch/aps.h"
#include "patch/bsl.h"
#include "patch/gg.h"
#include "patch/ips.h"
#include "patch/patch.h"
#include "patch/ppf.h"


#ifdef  _MSC_VER
// Visual C++ doesn't allow inline in C source code
#define inline __inline
#endif


static void ucon64_exit (void);
static int ucon64_execute_options (void);
static void ucon64_rom_nfo (const st_ucon64_nfo_t *nfo);
static st_ucon64_nfo_t *ucon64_probe (st_ucon64_nfo_t *nfo);
static int ucon64_rom_handling (void);
static int ucon64_process_rom (const char *fname);


st_ucon64_t ucon64;                             // containes ptr to image, dat and nfo

#ifdef  AMIGA
unsigned long __stacksize = 102400;             // doesn't work on PPC? is StormC specific?
//unsigned long __stack = 102400;               // for SAS/C, DICE, GCC etc.?
char vers[] = "$VER: uCON64 "UCON64_VERSION_S" "CURRENT_OS_S" ("__DATE__") ("__TIME__")";
#endif

typedef struct
{
  int val;         // (st_getopt2_t->val)
  char *optarg;    // option argument
  int console;     // the console (st_getopt2_t->object)
  int flags;       // workflow flags (st_getopt2_t->object)
} st_args_t;

static st_args_t arg[UCON64_MAX_ARGS];

static st_getopt2_t options[UCON64_MAX_ARGS];
static const st_getopt2_t lf[] =
  {
    {NULL, 0, 0, 0, NULL, "", NULL},
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  },
  *option[] =
  {
    ucon64_options_usage,
    lf,
    ucon64_padding_usage,
    lf,
    ucon64_dat_usage,
    lf,
    patch_usage,
    bsl_usage,
    ips_usage,
    aps_usage,
    ppf_usage,
    gg_usage,
    lf,
#ifdef  USE_DISCMAGE
    discmage_usage,
    lf,
#endif
    dc_usage,
    lf,
    psx_usage,
#ifdef  USE_PARALLEL
    dex_usage,
#endif
    lf,
    gba_usage,
#if     defined USE_PARALLEL || defined USE_USB
    // f2a_usage has to come before fal_usage in case only USE_USB is defined
    //  (no support for parallel port)
    f2a_usage,
#ifdef  USE_PARALLEL
    fal_usage,
#endif
#endif // USE_PARALLEL || USE_USB
#if 0
    // this backup unit uses CF cards for transfer
    sc_usage,
#endif
    lf,
    nds_usage,
    lf,
    n64_usage,
#ifdef  USE_PARALLEL
    doctor64_usage,
    doctor64jr_usage,
#ifdef  USE_LIBCD64
    cd64_usage,
#endif
    dex_usage,
#endif // USE_PARALLEL
    lf,
    snes_usage,
#ifdef  USE_PARALLEL
    swc_usage,
    gd_usage,
    fig_usage,
    sflash_usage,
//    mgd_usage,
#endif
#ifdef  USE_USB
    quickdev16_usage,
#endif
    lf,
    neogeo_usage,
    lf,
    genesis_usage,
#ifdef  USE_PARALLEL
    smd_usage,
    mdpro_usage,
    mcd_usage,
    cmc_usage,
//    mgd_usage,
#endif
    lf,
    gb_usage,
#ifdef  USE_PARALLEL
    gbx_usage,
    mccl_usage,
#endif
    lf,
    lynx_usage,
#ifdef  USE_PARALLEL
    lynxit_usage,
#endif
    lf,
    pce_usage,
#ifdef  USE_PARALLEL
    msg_usage,
    pcepro_usage,
//    mgd_usage,
#endif
    lf,
    nes_usage,
#ifdef  USE_PARALLEL
    smc_usage,
#endif
    lf,
    sms_usage,
#ifdef  USE_PARALLEL
    smsggpro_usage,
#endif
    lf,
    swan_usage,
    lf,
    jaguar_usage,
    lf,
    ngp_usage,
#ifdef  USE_PARALLEL
    pl_usage,
#endif
    lf,
    atari_usage,
#if 0
// these backup units use audio to transfer ROMs
    cc2_usage,
    spsc_usage,
#endif
    lf,
    coleco_usage,
    lf,
    vboy_usage,
    NULL
  };


static st_ucon64_nfo_t *
ucon64_clear_nfo (st_ucon64_nfo_t *nfo)
{
  if (nfo)
    memset (nfo, 0, sizeof (st_ucon64_nfo_t));

  ucon64.nfo = NULL;
  ucon64.crc32 = ucon64.fcrc32 = 0;             // yes, this belongs here
  nfo->data_size = UCON64_UNKNOWN;

  return nfo;
}


//#define TEST 
#ifdef  TEST
typedef struct
{
  int val;

  const char *cmdline;
  uint32_t crc32;    // crc32 of cmdline's output
} st_ucon64_test_t;


void
ucon64_test (void)
{
// default prepare and cleanup macros
#define TEST_START ""
#define TEST_END   ""
#define TEST_BREAK {0, NULL, 0},
#define TEST_BUG 1
#define TEST_TODO 2

  st_ucon64_test_t test[] =
    {
      {UCON64_1991,	"ucon64 -1991 /tmp/test/test.smd;"
                        "ucon64 -gen test.smd;"
                        "rm test.smd", 0xadc940f4},
      {UCON64_A,	"ucon64 -a", TEST_TODO},
      {UCON64_ATA,	"ucon64 -ata /tmp/test/test.64k", 0x4b3a37d0},
      {UCON64_B,	"ucon64 -b", TEST_TODO},
      {UCON64_B0,	"ucon64 -b0 64 /tmp/test/test.lnx;"
                        "ucon64 test.lnx;"
                        "rm test.lnx", 0xde32e069},
      {UCON64_B1,	"ucon64 -b1 64 /tmp/test/test.lnx;"
                        "ucon64 test.lnx;"
                        "rm test.lnx", 0x6b648320},
      {UCON64_BAT,	"ucon64 -bat /tmp/test/test.nes", TEST_BUG},
      {UCON64_BIN,	"ucon64 -bin", TEST_TODO},
      {UCON64_BIOS,	"ucon64 -bios", TEST_TODO},
      {UCON64_BOT,	"ucon64 -bot=test.bot /tmp/test/test.v64;"
                        "ucon64 -crc test.bot;"
                        "rm test.bot", 1},
      {UCON64_BS,	"ucon64 -bs /tmp/test/test.smc", 0x18910ac6},
      {UCON64_C,	"ucon64 -c /tmp/test/test.txt /tmp/test/12345678.abc", 0x2284888d},
#if 0
      {UCON64_CC2,      "ucon64 -cc2 /tmp/test/test.16k;"
                        "ucon64 -crc test.wav;" // crc should be 0xc5cdd20f
                        "rm test.wav", 3},
#endif
      {UCON64_CHK,	"ucon64 -chk /tmp/test/test.smc;"
                        "ucon64 test.smc;"
                        "rm test.smc", 0x3fa1e89a},
      {UCON64_CMNT,	"ucon64 -cmnt", TEST_TODO},
      {UCON64_CODE,	"ucon64 -code /tmp/test/test.txt", TEST_BUG},
      {UCON64_COL,	"ucon64 -col 0xff00", 0xd4f45031},
      {UCON64_COLECO,	"ucon64 -coleco /tmp/test/test.1mb", 0x2fb8741c},
      {UCON64_CRC,	"ucon64 -crc /tmp/test/test.2kb", 0xd17fda4a},
      {UCON64_CRP,	"ucon64 -crp", TEST_TODO},
      {UCON64_CS,	"ucon64 -cs /tmp/test/test.txt /tmp/test/12345678.abc", 0xd6e61833},
      {UCON64_CTRL,	"ucon64 -ctrl", TEST_TODO},
      {UCON64_CTRL2,	"ucon64 -ctrl2", TEST_TODO},
      {UCON64_DB,	"ucon64 -db", TEST_TODO},
      {UCON64_DBS,	"ucon64 -dbs", TEST_TODO},
      {UCON64_DBUH,	"ucon64 -dbuh", TEST_TODO},
      {UCON64_DBV,	"ucon64 -dbv", TEST_TODO},
      {UCON64_DC,	"ucon64 -dc /tmp/test/test.1mb", 0x14c9d369},
      {UCON64_DFIND,	"ucon64 -dfind \"97 98 99 100\" /tmp/test/test.txt", 0xd7aed3fd},
      {UCON64_DFINDR,	"ucon64 -dfindr \"1 2 3 4\" /tmp/test/test.txt", 0x88a6f737},
      {UCON64_DINT,	"ucon64 -dint /tmp/test/test.txt;"
                        "ucon64 -crc test.txt;"
                        "rm test.txt", 0xe60a7df5},
      {UCON64_DMIRR,	"ucon64 -dmirr", TEST_TODO},
      {UCON64_DNSRT,	"ucon64 -dnsrt", TEST_TODO},
      {UCON64_DUAL,	"ucon64 -dual", TEST_TODO},
      {UCON64_DUMPINFO,	"ucon64 -dumpinfo", TEST_TODO},
      {UCON64_E,	"ucon64 -e", TEST_TODO},
      {UCON64_EROM,	"ucon64 -erom", TEST_TODO},
      {UCON64_F,	"ucon64 -f", TEST_TODO},
      {UCON64_FDS,	"ucon64 -fds", TEST_TODO},
      {UCON64_FDSL,	"ucon64 -fdsl", TEST_TODO},
      {UCON64_FFE,	"ucon64 -ffe", TEST_TODO},
      {UCON64_FIG,	"ucon64 -fig", TEST_TODO},
      {UCON64_FIGS,	"ucon64 -figs", TEST_TODO},
      {UCON64_FIND,	"ucon64 -find \"abcd\" /tmp/test/test.txt", 0xd7aed3fd},
      {UCON64_FINDI,	"ucon64 -findi \"ABcD\" /tmp/test/test.txt", 0x10e913cd},
      {UCON64_FINDR,	"ucon64 -findr \"1234\" /tmp/test/test.txt", 0x1b1284d8},
      {UCON64_GB,	"ucon64 -gb /tmp/test/test.1mb", 0xf050caa1},
      {UCON64_GBA,	"ucon64 -gba /tmp/test/test.1mb", 0x5253861d},
      {UCON64_GBX,	"ucon64 -gbx", TEST_TODO},
      {UCON64_GD3,	"ucon64 -gd3", TEST_TODO},
      {UCON64_GD3S,	"ucon64 -gd3s", TEST_TODO},
      {UCON64_GEN,	"ucon64 -gen /tmp/test/test.1mb", 0xaa1f503c},
      {UCON64_GG,	"ucon64 -gg", TEST_TODO},
      {UCON64_GGD,	"ucon64 -ggd", TEST_TODO},
      {UCON64_GGE,	"ucon64 -gge", TEST_TODO},
      {UCON64_HD,	"ucon64 -snes -hd /tmp/test/test.1mb", 0x9ea45865},
      {UCON64_HDN,	"ucon64 -snes -hdn=1024 /tmp/test/test.1mb", 0x37812a26},
      {UCON64_HEX,	"ucon64 -hex /tmp/test/test.txt", 0x90d0b764},
      {UCON64_HFIND,	"ucon64 -hfind \"? 68 ?? 6a\" /tmp/test/test.txt", 0xdfa06028},
      {UCON64_HFINDR,	"ucon64 -hfindr \"01 02 03 04\" /tmp/test/test.txt", 0x88a6f737},
      {UCON64_HI,	"ucon64 -snes -hi /tmp/test/test.1mb", 0x086266b1},
      {UCON64_I,	"ucon64 -i", TEST_TODO},
      {UCON64_IDNUM,	"ucon64 -idnum", TEST_TODO},
      {UCON64_IDPPF,	"ucon64 -idppf", TEST_TODO},
      {UCON64_INES,	"ucon64 -ines", TEST_TODO},
      {UCON64_INESHD,	"ucon64 -ineshd", TEST_TODO},
      {UCON64_INS,      "ucon64 -ins /tmp/test/test.txt;"
                        "ucon64 -crc test.txt 2>&1;"
                        "rm test.txt", 0xa87abae3},
      {UCON64_INSN,     "ucon64 -insn=512 /tmp/test/test.txt;"
                        "ucon64 -crc test.txt 2>&1;"
                        "rm test.txt", 0xa87abae3},
      {UCON64_INT,	"ucon64 -int /tmp/test/test.z64", 0x1d23f41c},
      {UCON64_INT2,	"ucon64 -int2 /tmp/test/test.v64", 1},
      {UCON64_ISPAD,	"ucon64 -ispad /tmp/test/test.2kb", 0xa5e38fbd},
      {UCON64_J,	"ucon64 -j", TEST_TODO},
      {UCON64_JAG,	"ucon64 -jag /tmp/test/test.1mb", 0x2d1f3594},
      {UCON64_K,	"ucon64 -k", TEST_TODO},
      {UCON64_L,	"ucon64 -l", TEST_TODO},
      {UCON64_LNX,	"ucon64 -lnx", TEST_TODO},
      {UCON64_LOGO,	"ucon64 -logo /tmp/test/test.gba", TEST_TODO},
      {UCON64_LS,	"ucon64 -ls /tmp/test/*", TEST_BUG},
      {UCON64_LSD,	"ucon64 -lsd /tmp/test/*", TEST_BUG},
      {UCON64_LSRAM,	"ucon64 -lsram", TEST_TODO},
      {UCON64_LSV,	"ucon64 -lsv", TEST_TODO},
      {UCON64_LYNX,	"ucon64 -lynx /tmp/test/test.1mb", 0x78e02858},
      {UCON64_LYX,	"ucon64 -lyx", TEST_TODO},
      {UCON64_MAPR,	"ucon64 -mapr", TEST_TODO},
      {UCON64_MD5,	"ucon64 -md5", TEST_TODO},
      {UCON64_MGD,	"ucon64 -mgd", TEST_TODO},
      {UCON64_MGDGG,	"ucon64 -mgdgg", TEST_TODO},
      {UCON64_MIRR,	"ucon64 -mirr", TEST_TODO},
      {UCON64_MKA,	"ucon64 -mka", TEST_TODO},
      {UCON64_MKDAT,	"ucon64 -mkdat", TEST_TODO},
      {UCON64_MKI,	"ucon64 -mki=/tmp/test/test.txt /tmp/test/test2.txt;"
                        "ucon64 -crc test2.ips;"
                        "rm test2.ips", 0xe2b26d35},
      {UCON64_MKIP,	"ucon64 -mkip", TEST_TODO},
      {UCON64_MKPPF,	"ucon64 -mkppf", TEST_TODO},
      {UCON64_MSG,	"ucon64 -msg", TEST_TODO},
      {UCON64_MULTI,	"ucon64 -multi", TEST_TODO},
      {UCON64_N,	"ucon64 -n", TEST_TODO},
      {UCON64_N2,	"ucon64 -n2", TEST_TODO},
      {UCON64_N2GB,	"ucon64 -n2gb", TEST_TODO},
      {UCON64_N64,	"ucon64 -n64 /tmp/test/test.1mb", 0x5eedaf08},
      {UCON64_NA,	"ucon64 -na", TEST_TODO},
      {UCON64_NBAT,	"ucon64 -nbat", TEST_TODO},
      {UCON64_NBS,	"ucon64 -nbs", TEST_TODO},
      {UCON64_NCOL,	"ucon64 -ncol -snes /tmp/test/test.1mb", 0xf3091231},
      {UCON64_NDS,	"ucon64 -nds", TEST_TODO},
      {UCON64_NES,	"ucon64 -nes", TEST_TODO},
      {UCON64_NG,	"ucon64 -ng", TEST_TODO},
      {UCON64_NGP,	"ucon64 -ngp", TEST_TODO},
      {UCON64_NHD,	"ucon64 -nhd /tmp/test/test.smc", 0x1a5cc5d4},
      {UCON64_NHI,	"ucon64 -snes -nhi /tmp/test/test.1mb", 0xf3091231},
      {UCON64_NINT,	"ucon64 -nint", TEST_TODO},
      {UCON64_NPPF,	"ucon64 -nppf", TEST_TODO},
      {UCON64_NROT,	"ucon64 -nrot", TEST_TODO},
      {UCON64_NS,	"ucon64 -ns", TEST_TODO},
      {UCON64_NTSC,	"ucon64 -ntsc", TEST_TODO},
      {UCON64_NVRAM,	"ucon64 -nvram", TEST_TODO},
      {UCON64_P,	"ucon64 -p", TEST_TODO},
      {UCON64_PAD,	"ucon64 -pad", TEST_TODO},
      {UCON64_PADN,	"ucon64 -padn", TEST_TODO},
      {UCON64_PAL,	"ucon64 -pal", TEST_TODO},
      {UCON64_PARSE,	"ucon64 -parse", TEST_TODO},
      {UCON64_PASOFAMI,	"ucon64 -pasofami", TEST_TODO},
      {UCON64_PATCH,	"ucon64 -patch", TEST_TODO},
      {UCON64_PATTERN,	"ucon64 -pattern", TEST_TODO},
      {UCON64_PCE,	"ucon64 -pce", TEST_TODO},
      {UCON64_POKE,	"ucon64 -poke", TEST_TODO},
      {UCON64_PPF,	"ucon64 -ppf", TEST_TODO},
      {UCON64_PRINT,	"ucon64 -print /tmp/test/test.txt", 0x5c4acd52},
      {UCON64_PSX,	"ucon64 -psx /tmp/test/test.1mb", 0x79b34e40},
      {UCON64_R83,      "cp /tmp/test/1234567890.abcd .;"
                        "ucon64 -r83 1234567890.abcd;"
                        "rm 12345eeb.abc", 0x1f791880},
      {UCON64_RDAT,	"ucon64 -rdat", TEST_TODO},
      {UCON64_REGION,	"ucon64 -region", TEST_TODO},
      {UCON64_RJOLIET,  "cp /tmp/test/1234567890123456789012345678901234567890123456789012345678901234567890.abcd .;"
                        "ucon64 -rjoliet 1234567890123456789012345678901234567890123456789012345678901234567890.abcd;"
                        "rm 123456789012345678901234567890123456789012345678901234566f5.abcd", 0xd5cfab05},
      {UCON64_RL,       "cp /tmp/test/12345678.ABC .;"
                        "ucon64 -rl 12345678.ABC;"
                        "rm 12345678.abc", 0x48934d06},
      {UCON64_ROTL,	"ucon64 -rotl", TEST_TODO},
      {UCON64_ROTR,	"ucon64 -rotr", TEST_TODO},
      {UCON64_RROM,	"cp /tmp/test/test.smc .;"
                        "ucon64 -rrom test.smc;"
                        "rm \"Mode 7 interactive de.smc\"", 0xaaa714b6},
      {UCON64_RU,       "cp /tmp/test/12345678.abc .;"
                        "ucon64 -ru 12345678.abc;"
                        "rm 12345678.ABC", 0x0c9f305f},
      {UCON64_S,	"ucon64 -s", TEST_TODO},
      {UCON64_SAM,	"ucon64 -sam", TEST_TODO},
      {UCON64_SC,	"ucon64 -sc", TEST_TODO},
      {UCON64_SCAN,	"ucon64 -scan", TEST_TODO},
      {UCON64_SCR,	"ucon64 -scr", TEST_TODO},
      {UCON64_SGB,	"ucon64 -sgb", TEST_TODO},
      {UCON64_SHA1,	"ucon64 -sha1 /tmp/test/test.txt", 0x65608105},
      {UCON64_SMC,	"ucon64 -smc", TEST_TODO},
      {UCON64_SMD,	"ucon64 -smd", TEST_TODO},
      {UCON64_SMDS,	"ucon64 -smds", TEST_TODO},
      {UCON64_SMS,	"ucon64 -sms /tmp/test/test.1mb", 0x73996f1d},
      {UCON64_SNES,	"ucon64 -snes /tmp/test/test.1mb", 0xf3091231},
      {UCON64_SRAM,	"ucon64 -sram", TEST_TODO},
      {UCON64_SSC,	"ucon64 -ssc", TEST_TODO},
      {UCON64_SSIZE,	"ucon64 -ssize", TEST_TODO},
      {UCON64_STP,	"ucon64 -stp /tmp/test/test.64k;"
                        "ucon64 -crc test.64k;"
                        "rm test.64k", 0xe1a4cd85},
      {UCON64_STPN,	"ucon64 -stpn=512 /tmp/test/test.64k;"
                        "ucon64 -crc test.64k;"
                        "rm test.64k", 0xe1a4cd85},
      {UCON64_STRIP,	"ucon64 -strip=512 /tmp/test/test.64k;"
                        "ucon64 -crc test.64k;"
                        "rm test.64k", 0xe1a4cd85},
      {UCON64_SWAN,	"ucon64 -swan /tmp/test/test.1mb", 0x8e07f287},
      {UCON64_SWAP,	"ucon64 -swap /tmp/test/test.txt;"
                        "ucon64 -crc test.txt;"
                        "rm test.txt", 0xe60a7df5},
      {UCON64_SWAP2,	"ucon64 -swap2 /tmp/test/test.txt;"
                        "ucon64 -crc test.txt;"
                        "rm test.txt", 0xd5f4368d},
      {UCON64_SWC,	"ucon64 -swc /tmp/test/test.1mb;"
                        "ucon64 test.swc;"
                        "rm test.swc", 0xa9c3730f},
      {UCON64_SWCS,	"ucon64 -swcs", TEST_TODO},
      {UCON64_UFO,	"ucon64 -ufo", TEST_TODO},
      {UCON64_UFOS,	"ucon64 -ufos", TEST_TODO},
      {UCON64_UNIF,	"ucon64 -unif /tmp/test/test.nes", TEST_BUG},
      {UCON64_UNSCR,	"ucon64 -unscr", TEST_TODO},
      {UCON64_USMS,	"ucon64 -usms", TEST_TODO},
      {UCON64_V64,	"ucon64 -v64 /tmp/test/test.z64;"
                        "ucon64 test.v64;"
                        "rm test.v64", 1},
      {UCON64_VBOY,	"ucon64 -vboy /tmp/test/test.1mb", 0x87661ae8},
      {UCON64_VRAM,	"ucon64 -vram", TEST_TODO},
      {UCON64_Z64,	"ucon64 -z64 /tmp/test/test.v64;"
                        "ucon64 test.z64;"
                        "rm test.z64", 1},
TEST_BREAK
      {UCON64_VER,	"ucon64 -version", 0},  // NO TEST: changes always
      {UCON64_V,	"ucon64 -v", 0},        // NO TEST: verbose switch
      {UCON64_Q,	"ucon64 -q", 0},        // NO TEST: quiet switch
      {UCON64_HELP,	"ucon64 -help", 0},     // NO TEST: usage changes always
      {UCON64_R,	"ucon64 -r", 0},        // NO TEST: recursion
      {UCON64_O,        "ucon64 -o", 0},        // NO TEST: output
      {UCON64_NBAK,	"ucon64 -nbak", 0},     // NO TEST: no backup

      {UCON64_ROM,      "ucon64 -rom", 0},      // NO TEST: hidden option or deprecated
      {UCON64_3DO,	"ucon64 -3do", 0},      // NO TEST: hidden option or deprecated
      {UCON64_CRCHD,	"ucon64 -crchd /tmp/test/test.2kb", 0x707bbaf1}, // NO TEST: hidden option or deprecated
      {UCON64_FILE,	"ucon64 -file", 0},     // NO TEST: hidden option or deprecated
      {UCON64_FRONTEND,	"ucon64 -frontend", 0}, // NO TEST: hidden option or deprecated
      {UCON64_GC,	"ucon64 -gc", 0},       // NO TEST: hidden option or deprecated
      {UCON64_GP32,	"ucon64 -gp32", 0},     // NO TEST: hidden option or deprecated
      {UCON64_ID,	"ucon64 -id", 0},       // NO TEST: hidden option or deprecated
      {UCON64_INTELLI,	"ucon64 -intelli", 0},  // NO TEST: hidden option or deprecated
      {UCON64_PS2,	"ucon64 -ps2", 0},      // NO TEST: hidden option or deprecated
      {UCON64_S16,	"ucon64 -s16", 0},      // NO TEST: hidden option or deprecated
      {UCON64_SAT,	"ucon64 -sat", 0},      // NO TEST: hidden option or deprecated
      {UCON64_VEC,	"ucon64 -vec", 0},      // NO TEST: hidden option or deprecated
      {UCON64_XBOX,	"ucon64 -xbox", 0},     // NO TEST: hidden option or deprecated

      {UCON64_BIN2ISO,	"ucon64 -bin2iso", 0},  // NO TEST: discmage
      {UCON64_DISC,	"ucon64 -disc", 0},     // NO TEST: discmage
      {UCON64_ISOFIX,	"ucon64 -isofix", 0},   // NO TEST: discmage
      {UCON64_MKCUE,	"ucon64 -mkcue", 0},    // NO TEST: discmage
      {UCON64_MKSHEET,	"ucon64 -mksheet", 0},  // NO TEST: discmage
      {UCON64_MKTOC,	"ucon64 -mktoc", 0},    // NO TEST: discmage
      {UCON64_RIP,	"ucon64 -rip", 0},      // NO TEST: discmage

      {UCON64_PORT,     "ucon64 -port", 0},     // NO TEST: transfer code
      {UCON64_XCMC,	"ucon64 -xcmc", 0},     // NO TEST: transfer code
      {UCON64_XCMCM,	"ucon64 -xcmcm", 0},    // NO TEST: transfer code
      {UCON64_XCMCT,	"ucon64 -xcmct", 0},    // NO TEST: transfer code
      {UCON64_XDEX,	"ucon64 -xdex", 0},     // NO TEST: transfer code
      {UCON64_XDJR,	"ucon64 -xdjr", 0},     // NO TEST: transfer code
      {UCON64_XF2A,	"ucon64 -xf2a", 0},     // NO TEST: transfer code
      {UCON64_XF2AB,	"ucon64 -xf2ab", 0},    // NO TEST: transfer code
      {UCON64_XF2AC,	"ucon64 -xf2ac", 0},    // NO TEST: transfer code
      {UCON64_XF2AMULTI,	"ucon64 -xf2amulti", 0}, // NO TEST: transfer code
      {UCON64_XF2AS,	"ucon64 -xf2as", 0},    // NO TEST: transfer code
      {UCON64_XFAL,	"ucon64 -xfal", 0},     // NO TEST: transfer code
      {UCON64_XFALB,	"ucon64 -xfalb", 0},    // NO TEST: transfer code
      {UCON64_XFALC,	"ucon64 -xfalc", 0},    // NO TEST: transfer code
      {UCON64_XFALM,	"ucon64 -xfalm", 0},    // NO TEST: transfer code
      {UCON64_XFALMULTI,	"ucon64 -xfalmulti", 0}, // NO TEST: transfer code
      {UCON64_XFALS,	"ucon64 -xfals", 0},    // NO TEST: transfer code
      {UCON64_XFIG,	"ucon64 -xfig", 0},     // NO TEST: transfer code
      {UCON64_XFIGC,	"ucon64 -xfigc", 0},    // NO TEST: transfer code
      {UCON64_XFIGS,	"ucon64 -xfigs", 0},    // NO TEST: transfer code
      {UCON64_XGBX,	"ucon64 -xgbx", 0},     // NO TEST: transfer code
      {UCON64_XGBXB,	"ucon64 -xgbxb", 0},    // NO TEST: transfer code
      {UCON64_XGBXM,	"ucon64 -xgbxm", 0},    // NO TEST: transfer code
      {UCON64_XGBXS,	"ucon64 -xgbxs", 0},    // NO TEST: transfer code
      {UCON64_XGD3,	"ucon64 -xgd3", 0},     // NO TEST: transfer code
      {UCON64_XGD3R,	"ucon64 -xgd3r", 0},    // NO TEST: transfer code
      {UCON64_XGD3S,	"ucon64 -xgd3s", 0},    // NO TEST: transfer code
      {UCON64_XGD6,	"ucon64 -xgd6", 0},     // NO TEST: transfer code
      {UCON64_XGD6R,	"ucon64 -xgd6r", 0},    // NO TEST: transfer code
      {UCON64_XGD6S,	"ucon64 -xgd6s", 0},    // NO TEST: transfer code
      {UCON64_XGG,	"ucon64 -xgg", 0},      // NO TEST: transfer code
      {UCON64_XGGB,	"ucon64 -xggb", 0},     // NO TEST: transfer code
      {UCON64_XGGS,	"ucon64 -xggs", 0},     // NO TEST: transfer code
      {UCON64_XLIT,	"ucon64 -xlit", 0},     // NO TEST: transfer code
      {UCON64_XMCCL,	"ucon64 -xmccl", 0},    // NO TEST: transfer code
      {UCON64_XMCD,	"ucon64 -xmcd", 0},     // NO TEST: transfer code
      {UCON64_XMD,	"ucon64 -xmd", 0},      // NO TEST: transfer code
      {UCON64_XMDB,	"ucon64 -xmdb", 0},     // NO TEST: transfer code
      {UCON64_XMDS,	"ucon64 -xmds", 0},     // NO TEST: transfer code
      {UCON64_XMSG,	"ucon64 -xmsg", 0},     // NO TEST: transfer code
      {UCON64_XPCE,	"ucon64 -xpce", 0},     // NO TEST: transfer code
      {UCON64_XPL,	"ucon64 -xpl", 0},      // NO TEST: transfer code
      {UCON64_XPLI,	"ucon64 -xpli", 0},     // NO TEST: transfer code
      {UCON64_XPLM,	"ucon64 -xplm", 0},     // NO TEST: transfer code
      {UCON64_XQD16,	"ucon64 -xqd16", 0},    // NO TEST: transfer code
      {UCON64_XRESET,	"ucon64 -xreset", 0},   // NO TEST: transfer code
      {UCON64_XSF,	"ucon64 -xsf", 0},      // NO TEST: transfer code
      {UCON64_XSFS,	"ucon64 -xsfs", 0},     // NO TEST: transfer code
      {UCON64_XSMC,	"ucon64 -xsmc", 0},     // NO TEST: transfer code
      {UCON64_XSMCR,	"ucon64 -xsmcr", 0},    // NO TEST: transfer code
      {UCON64_XSMD,	"ucon64 -xsmd", 0},     // NO TEST: transfer code
      {UCON64_XSMDS,	"ucon64 -xsmds", 0},    // NO TEST: transfer code
      {UCON64_XSWC,	"ucon64 -xswc", 0},     // NO TEST: transfer code
      {UCON64_XSWC_IO,	"ucon64 -xswc-io", 0},  // NO TEST: transfer code
      {UCON64_XSWC2,	"ucon64 -xswc2", 0},    // NO TEST: transfer code
      {UCON64_XSWCC,	"ucon64 -xswcc", 0},    // NO TEST: transfer code
      {UCON64_XSWCR,	"ucon64 -xswcr", 0},    // NO TEST: transfer code
      {UCON64_XSWCS,	"ucon64 -xswcs", 0},    // NO TEST: transfer code
      {UCON64_XV64,	"ucon64 -xv64", 0},     // NO TEST: transfer code

      {0, NULL, 0}
    };
  int x = 0;
  unsigned int crc = 0;
  char buf[MAXBUFSIZE], fname[FILENAME_MAX];

#ifdef  DEBUG
//#if 1
  // this is why no external script is used for testing
  {
    int c = 0, y = 0;

    // convert (st_getopt2_t **) to (st_getopt2_t *)
    memset (&options, 0, sizeof (st_getopt2_t) * UCON64_MAX_ARGS);
    for (c = x = 0; option[x]; x++)
      for (y = 0; option[x][y].name || option[x][y].help; y++)
        if (c < UCON64_MAX_ARGS)
          {
            memcpy (&options[c], &option[x][y], sizeof (st_getopt2_t));
            c++;
          }

    // do we test ALL options?
    for (x = 0; options[x].name || options[x].help; x++)
      if (options[x].val)
        {
          int found = 0;
          for (y = 0; test[y].val; y++)
            if (options[x].val == test[y].val)
              {
                found = 1;
                break;
              }

          if (!found)
            printf ("option: %4d \"%s\" will NOT be tested\n", options[x].val, options[x].name);
        }
  }
#endif

  for (x = 0; test[x].val; x++)
    {
      FILE *in = NULL, *out = NULL;
      const char *state = NULL;

      // NO testing?
      if (!test[x].cmdline || !test[x].crc32)
        continue;

      crc = 0;
      if (test[x].crc32 != TEST_BUG && test[x].crc32 != TEST_TODO)
        {
          sprintf (buf, "%s", test[x].cmdline);
          if (!(in = popen (buf, "r")))
            {
              fprintf (stderr, "ERROR: cmdline \"%s\"\n", test[x].cmdline);
              continue;
            }

          sprintf (fname, "%d-output.txt", test[x].val);
          out = fopen (fname, "w");

          while ((fgets (buf, MAXBUFSIZE, in)))
            {
              crc = crc32 (crc, (const void *) &buf, strlen (buf));
              fputs (buf, out);
            }
        }

      sprintf (buf, "option: %4d crc: 0x%08x calc: 0x%08x status: ",
        test[x].val,
        test[x].crc32,
        crc);

      if (test[x].crc32 == TEST_BUG)
        state = "BUG!";
      else if (test[x].crc32 == TEST_TODO)
        state = "TODO ";
      else  if (test[x].crc32 == crc)
        state = "OK ";
      else
        state = "BUG?";

      sprintf (strchr (buf, 0), "%5s (%s)\n", state, test[x].cmdline);

      printf (buf); 
      fflush (stdout);

      if (test[x].crc32 != TEST_BUG && test[x].crc32 != TEST_TODO)
        {
          fputs ("^^^ ", out);
          fputs (buf, out);

          fclose (in);
          fclose (out);
        }
    }
  exit (0);
}
#endif


#ifdef  DEBUG
void
ucon64_runtime_debug_output (st_getopt2_t *p)
{
  printf ("{\"%s\", %d, 0, %d, \"%s\", \"%s\", %d}, // console: %d workflow: %d\n",
    p->name,
    p->has_arg,
    p->val,
    p->arg_name,
    p->help ? "usage" : p->help, // i (nb) mean it
//    p->help,
    0,
    p->object ? ((st_ucon64_obj_t *) p->object)->console : 0,
    p->object ? ((st_ucon64_obj_t *) p->object)->flags : 0);
}


static void
ucon64_runtime_debug (void)
{
  int x = 0, y = 0, c = 0;
  (void) x;
  (void) y;
  (void) c;

#if 0
  // how many options (incl. dupes) do we have?
  for (x = y = 0; options[x].name || options[x].help; x++)
    if (options[x].name)
      y++;
  printf ("DEBUG: Total options (with dupes): %d\n", y);
  printf ("DEBUG: UCON64_MAX_ARGS == %d, %s\n", UCON64_MAX_ARGS,
    (y < UCON64_MAX_ARGS ? "good" : "\nERROR: too small; must be larger than options"));
#endif

#if 1
  // list all options as a single st_getopt2_t array
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // how many consoles does uCON64 support?
  for (x = y = 0; options[x].name || options[x].help; x++)
    if (options[x].name && options[x].object)
      if (options[x].val == ((st_ucon64_obj_t *) options[x].object)->console)
        ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options without an object (allowed)
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name && !options[x].object)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options without a console (allowed)
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name && !((st_ucon64_obj_t *) options[x].object)->console)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options without a workflow (allowed)
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name && !((st_ucon64_obj_t *) options[x].object)->flags)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options without a val (NOT allowed)
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name && !options[x].val)
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find options with has_arg but without arg_name AND/OR usage
  // hidden options without arg_name AND usage are allowed
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name &&
        ((!options[x].has_arg && options[x].arg_name) ||
         (options[x].has_arg && !options[x].arg_name) ||
         !options[x].help))
      ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
#endif

#if 0
  // find dupe (NOT a problem) options that have different values for val,
  // flag, and/or object (NOT allowed)
  // getopt1() will always use the 1st option in the array
  // (st_getopt2_t *)->arg_name and (st_getopt2_t *)->help can be as
  // different as you like
  for (x = 0; options[x].name || options[x].help; x++)
    if (options[x].name)
      for (y = 0; options[y].name || options[y].help; y++)
        if (options[y].name && x != y) // IS option
          if (!strcmp (options[y].name, options[x].name))
            if (options[y].has_arg != options[x].has_arg || // (NOT allowed)
                options[y].flag != options[x].flag || // (NOT allowed)
                options[y].val != options[x].val || // (NOT allowed)
//                options[y].arg_name != options[x].arg_name || // (allowed)
//                options[y].help != options[x].help || // (allowed)
                ((st_ucon64_obj_t *) options[y].object)->console != ((st_ucon64_obj_t *) options[x].object)->console // (NOT allowed)
                ((st_ucon64_obj_t *) options[x].object)->flags != ((st_ucon64_obj_t *) options[x].object)->flags) // (NOT allowed)
              {
                fputs ("ERROR: different dupe options found\n  ", stdout);
                ucon64_runtime_debug_output ((st_getopt2_t *) &options[x]);
                fputs ("  ", stdout);
                ucon64_runtime_debug_output ((st_getopt2_t *) &options[y]);
                fputs ("\n\n", stdout);
              }
#endif
  puts ("DEBUG: Sanity check finished");
  fflush (stdout);
}
#endif  // DEBUG


static void
ucon64_exit (void)
{
#ifdef  USE_DISCMAGE
  if (ucon64.discmage_enabled && ucon64.image)
    dm_close ((dm_image_t *) ucon64.image);
#endif

  handle_registered_funcs ();
  fflush (stdout);
}


int
main (int argc, char **argv)
{
  int result = 0;
  int x = 0, y = 0, c = 0;
  const char *p = NULL;
  struct stat fstate;
  struct option long_options[UCON64_MAX_ARGS];

#ifdef  TEST
  if (argc == 1)
    ucon64_test ();
#else
  printf ("uCON64 " UCON64_VERSION_S " " CURRENT_OS_S " 1999-2006,2015\n"
    "Uses code from various people. See 'developers.html' for more!\n"
    "This may be freely redistributed under the terms of the GNU Public License\n\n");
#endif

  if (atexit (ucon64_exit) == -1)
    {
      fputs ("ERROR: Could not register function with atexit()\n", stderr);
      exit (1);
    }

  // flush st_ucon64_t
  memset (&ucon64, 0, sizeof (st_ucon64_t));

  // these members of ucon64 (except fname and fname_arch) don't change per file
  ucon64.argc = argc;
  ucon64.argv = argv;                           // must be set prior to calling
                                                //  ucon64_load_discmage() (for DOS)
  ucon64.fname =
  ucon64.file =
  ucon64.mapr =
  ucon64.comment = "";

  ucon64.fname_arch[0] = 0;

  ucon64.recursive =
  ucon64.parport_needed =
  ucon64.io_mode = 0;

  ucon64.battery =
  ucon64.bs_dump =
  ucon64.backup_header_len =
  ucon64.org_console =
  ucon64.controller =
  ucon64.controller2 =
  ucon64.do_not_calc_crc =
  ucon64.id =
  ucon64.interleaved =
  ucon64.mirror =
  ucon64.part_size =
  ucon64.region =
  ucon64.snes_header_base =
  ucon64.snes_hirom =
  ucon64.tv_standard =
  ucon64.use_dump_info =
  ucon64.vram = UCON64_UNKNOWN;

  ucon64.flags = WF_DEFAULT;

  // ucon64.console does change per file
  ucon64.console = ucon64.org_console;

  // convert (st_getopt2_t **) to (st_getopt2_t *)
  memset (&options, 0, sizeof (st_getopt2_t) * UCON64_MAX_ARGS);
  for (c = x = 0; option[x]; x++)
    for (y = 0; option[x][y].name || option[x][y].help; y++)
      if (c < UCON64_MAX_ARGS)
        {
          memcpy (&options[c], &option[x][y], sizeof (st_getopt2_t));
          c++;
        }

#ifdef  DEBUG
  ucon64_runtime_debug (); // check (st_getopt2_t *) options consistency
#endif

  // configfile handling
#ifdef  __unix__
  // We need to modify the umask, because the configfile is made while we are
  //  still running in root mode. Maybe 0 is even better (in case root did
  //  `chmod +s').
  umask (002);
#endif
  realpath2 (PROPERTY_HOME_RC ("ucon64"), ucon64.configfile);

  result = property_check (ucon64.configfile, UCON64_CONFIG_VERSION, 1);
  if (result == 1) // update needed
    result = ucon64_set_property_array ();
  if (result == -1) // property_check() or update failed
    return -1;

#ifdef  USE_ANSI_COLOR
  // ANSI colors?
  ucon64.ansi_color = get_property_int (ucon64.configfile, "ansi_color");
  // the conditional call to ansi_init() has to be done *after* the check for
  //  the switch --ncol
#endif

  // parallel port?
#if     defined USE_PPDEV || defined AMIGA
  p = get_property (ucon64.configfile, "parport_dev", PROPERTY_MODE_FILENAME);
  x = sizeof (ucon64.parport_dev);
#ifdef  USE_PPDEV
  strncpy (ucon64.parport_dev, p ? p : "/dev/parport0", x)[x - 1] = 0;
#elif   defined AMIGA
  strncpy (ucon64.parport_dev, p ? p : "parallel.device", x)[x - 1] = 0;
#endif
#endif

  p = get_property (ucon64.configfile, "parport", PROPERTY_MODE_TEXT);
  if (p)
    sscanf (p, "%hx", &ucon64.parport);
  else
    // use -1 (UCON64_UNKNOWN) to force probing if the config file doesn't contain
    //  a parport line
    ucon64.parport = (uint16_t) -1;

  // make backups?
  ucon64.backup = get_property_int (ucon64.configfile, "backups");

  // $HOME/.ucon64/ ?
  p = get_property (ucon64.configfile, "ucon64_configdir", PROPERTY_MODE_FILENAME);
  if (p)
    {
      x = sizeof (ucon64.configdir);
      strncpy (ucon64.configdir, p, x)[x - 1] = 0;
    }
  else
    *ucon64.configdir = 0;

  // DAT file handling
  ucon64.dat_enabled = 0;
  p = get_property (ucon64.configfile, "ucon64_datdir", PROPERTY_MODE_FILENAME);
  if (p)
    {
      x = sizeof (ucon64.datdir);
      strncpy (ucon64.datdir, p, x)[x - 1] = 0;
    }
  else
    *ucon64.datdir = 0;

  // we use ucon64.datdir as path to the dats
  if (!access (ucon64.datdir,
  // !W_OK doesn't mean that files can't be written to dir for Win32 exe's
#if     !defined __CYGWIN__ && !defined _WIN32
                              W_OK |
#endif
                              R_OK | X_OK) &&
      !stat (ucon64.datdir, &fstate) && S_ISDIR (fstate.st_mode))
    ucon64.dat_enabled = 1;

  if (!ucon64.dat_enabled && !access (ucon64.configdir,
#if     !defined __CYGWIN__ && !defined _WIN32
                                                        W_OK |
#endif
                                                        R_OK | X_OK) &&
      !stat (ucon64.configdir, &fstate) && S_ISDIR (fstate.st_mode))
    {
//      fprintf (stderr, "Please move your DAT files from %s to %s\n\n",
//               ucon64.configdir, ucon64.datdir);
      strcpy (ucon64.datdir, ucon64.configdir); // use .ucon64/ instead of .ucon64/dat/
      ucon64.dat_enabled = 1;
    }

#ifdef  USE_DISCMAGE
  // load libdiscmage (should be done before handling the switches (--ver), but
  //  ucon64_usage() has a dependency as well)
  ucon64.discmage_enabled = ucon64_load_discmage ();
#endif

  if (argc < 2)
    {
      ucon64_usage (argc, argv, USAGE_VIEW_SHORT);
      return 0;
    }

  // turn st_getopt2_t into struct option
  getopt2_long_only (long_options, options, UCON64_MAX_ARGS);

  // getopt() is utilized to make uCON64 handle/parse cmdlines in a sane
  //  and expected way
  x = optind = 0;
  memset (&arg, 0, sizeof (st_args_t) * UCON64_MAX_ARGS);
  while ((c = getopt_long_only (argc, argv, "", long_options, NULL)) != -1)
    {
      if (c == '?') // getopt() returns 0x3f ('?') when an unknown option was given
        {
          fprintf (stderr,
               "Try '%s " OPTION_LONG_S "help' for more information.\n",
               argv[0]);
          exit (1);
        }

      if (x < UCON64_MAX_ARGS)
        {
          const st_ucon64_obj_t *o =
            (st_ucon64_obj_t *) getopt2_get_index_by_val (options, c)->object;

          arg[x].console = UCON64_UNKNOWN; // default

          if (o)
            {
              arg[x].flags = o->flags;
              if (o->console)
                arg[x].console = o->console;
            }

          arg[x].val = c;
          arg[x++].optarg = optarg ? optarg : NULL;
        }
      else
        // this shouldn't happen
        exit (1);
    }

#ifdef  DEBUG
  for (x = 0; arg[x].val; x++)
    printf ("%d %s %d %d\n\n",
      arg[x].val,
      arg[x].optarg ? arg[x].optarg : "(null)",
      arg[x].flags,
      arg[x].console);
#endif

  // switches
  for (x = 0; arg[x].val; x++)
    {
      if (arg[x].console != UCON64_UNKNOWN)
        ucon64.org_console = ucon64.console = arg[x].console;
      if (arg[x].flags)
        ucon64.flags = arg[x].flags;
      if (arg[x].val)
        ucon64.option = arg[x].val;
      ucon64.optarg = arg[x].optarg;

//      if (ucon64.flags & WF_SWITCH)
        ucon64_switches (&ucon64);
    }

#ifdef  USE_ANSI_COLOR
  if (ucon64.ansi_color)
    ucon64.ansi_color = ansi_init ();
#endif

  /*
    Call ucon64_dat_indexer() after handling the switches and after calling
    ansi_init() so that the progress bar is displayed correctly (colour/no
    colour).
  */
  if (ucon64.dat_enabled)
    ucon64_dat_indexer ();              // update cache (index) files if necessary

#ifdef  USE_PARALLEL
  /*
    The copier options need root privileges for parport_open(). We can't use
    ucon64.flags & WF_PAR to detect whether a (parallel port) copier option has
    been specified, because another switch might've been specified after -port.
  */
  if (ucon64.parport_needed == 1)
    {
      ucon64.parport = parport_open (ucon64.parport);
      if (register_func (parport_close) == -1)
        {
          fputs ("ERROR: Could not register function with register_func()\n", stderr);
          exit (1);
        }
    }
#endif
#if     defined __unix__ && !defined __MSDOS__
  /*
    We can drop privileges after we have set up parallel port access. We cannot
    drop privileges if the user wants to communicate with the USB version of the
    F2A.
    SECURITY WARNING: We stay in root mode if the user specified an F2A option!
    We could of course drop privileges which requires the user to run uCON64 as
    root (not setuid root), but we want to be user friendly. Besides, doing
    things as root is bad anyway (from a security viewpoint).
  */
  if (ucon64.parport_needed != 2
#ifdef  USE_USB
      && !ucon64.usbport
#endif
     )
    drop_privileges ();
#endif // __unix__ && !__MSDOS__

  if (optind == argc)                   // no file was specified (e.g. --db)
    ucon64_execute_options();
  else
    {
      int flags = GETOPT2_FILE_FILES_ONLY;
      if (ucon64.recursive)
        flags |= GETOPT2_FILE_RECURSIVE;
      else 
        {
          /*
            Check if one of the parameters is a directory and if so, set the
            flag GETOPT2_FILE_RECURSIVE_ONCE. This flag makes uCON64 behave
            like version 2.0.0, i.e., specifying a directory is equivalent to
            specifying all files in that directory. In commands:
              ucon64 file dir1 dir2
            is equivalent to:
              ucon64 file dir1\* dir2\*
            Once the flag is set it is not necessary to check the remaining
            parameters.

            TODO: Find a solution for the fact that the stat() implementation
                  of MinGW and VC++ don't accept a path with an ending slash.
          */
          int i = optind;
          for (; i < argc; i++)
            if (!stat (argv[i], &fstate) && S_ISDIR (fstate.st_mode))
              {
                flags |= GETOPT2_FILE_RECURSIVE_ONCE;
                break;
              }
        }
      getopt2_file (argc, argv, ucon64_process_rom, flags);
    }

  return 0;
}


static int
ucon64_process_rom (const char *fname)
{
#ifdef  USE_ZLIB
  int n_entries;
#endif

  // Try to get file status information only if the file exists. We have to
  //  accept non-existing files for the dump options.
  if (access (fname, F_OK) == 0)
    {
      struct stat fstate;
      if (stat (fname, &fstate) == -1)
        return 0;
      if (!S_ISREG (fstate.st_mode))
        return 0;
    }

#ifdef  USE_ZLIB
  n_entries = unzip_get_number_entries (fname);
  if (n_entries != -1)                          // it's a zip file
    {
      for (unzip_current_file_nr = 0; unzip_current_file_nr < n_entries;
           unzip_current_file_nr++)
        {
          ucon64_fname_arch (fname);
          /*
            There seems to be no other way to detect directories in ZIP files
            than by looking at the file name. Paths in ZIP files should contain
            forward slashes. ucon64_fname_arch() changes forward slashes into
            backslashes (DIR_SEPARATORs) when uCON64 is compiled with Visual
            C++ or MinGW so that basename2() always produces a correct base
            name. So, if the entry in the ZIP file is a directory
            ucon64.fname_arch will be an empty string.
          */
          if (ucon64.fname_arch[0] == 0)
            continue;

          ucon64.fname = fname;

          ucon64_execute_options();

          if (ucon64.flags & WF_STOP)
            break;
        }
      unzip_current_file_nr = 0;
      ucon64.fname_arch[0] = 0;

      if (ucon64.flags & WF_STOP)
        return 1;
    }
  else
#endif
    {
      ucon64.fname = fname;

      ucon64_execute_options();
      if (ucon64.flags & WF_STOP)
        return 1;
    }

  return 0;
}


static int
ucon64_execute_options (void)
/*
  Execute all options for a single file.
  Please, if you experience problems then try your luck with the flags in
  ucon64_misc.c/ucon64_wf[] before changing things here or in
  ucon64_rom_handling().
*/
{
  int c = 0, result = 0, x = 0, opts = 0;

  // these members of ucon64 can change per file
  ucon64.dat = NULL;
#ifdef  USE_DISCMAGE
  ucon64.image = NULL;
#endif
  ucon64.nfo = NULL;

  ucon64.split = UCON64_UNKNOWN;

  ucon64.file_size =
  ucon64.crc32 =
  ucon64.fcrc32 = 0;

  ucon64.console = ucon64.org_console;
  
  for (x = 0; arg[x].val; x++)
    if (!(arg[x].flags & WF_SWITCH))
      {
        if (ucon64.console == UCON64_UNKNOWN)
          ucon64.console = arg[x].console;
        ucon64.flags = arg[x].flags;
        ucon64.option = arg[x].val;
        ucon64.optarg = arg[x].optarg;

        opts++;

        // WF_NO_SPLIT, WF_INIT, WF_PROBE, CRC32, DATabase and WF_NFO
        result = ucon64_rom_handling ();

        if (result == -1) // no rom, but WF_NO_ROM
          return -1;

        if (ucon64_options (&ucon64) == -1)
          {
            const st_getopt2_t *p = getopt2_get_index_by_val (options, c);
            const char *opt = p ? p->name : NULL;

            fprintf (stderr, "ERROR: %s%s encountered a problem\n",
                             opt ? (!opt[1] ? OPTION_S : OPTION_LONG_S) : "",
                             opt ? opt : "uCON64");

//            if (p)
//              getopt2_usage (p);

            fputs ("       Is the option you used available for the current console system?\n"
                   "       Please report bugs to ucon64-main@lists.sf.net or http://ucon64.sf.net\n\n",
                   stderr);

            return -1;
          }

        /*
          "stop" options:
          - -multi (and -xfalmulti) takes more than one file as argument, but
            should be executed only once.
          - stop after sending one ROM to a copier ("multizip")
          - stop after applying a patch so that the patch file won't be
            interpreted as ROM
        */
        if (ucon64.flags & WF_STOP)
          break;
      }

  if (!opts) // no options => just display ROM info
    {
      ucon64.flags = WF_DEFAULT;
      // WF_NO_SPLIT WF_INIT, WF_PROBE, CRC32, DATabase and WF_NFO
      if (ucon64_rom_handling () == -1)
        return -1; // no rom, but WF_NO_ROM
    }

  fflush (stdout);

  return 0;
}


static int
ucon64_rom_handling (void)
{
  int no_rom = 0;
  static st_ucon64_nfo_t nfo;
  struct stat fstate;

  ucon64_clear_nfo (&nfo);

  // a ROM (file)?
  if (!ucon64.fname)
    no_rom = 1;
  else if (!ucon64.fname[0])
    no_rom = 1;
  else if (access (ucon64.fname, F_OK | R_OK) == -1 && (!(ucon64.flags & WF_NO_ROM)))
    {
      fprintf (stderr, "ERROR: Could not open %s\n", ucon64.fname);
      no_rom = 1;
    }
  else if (stat (ucon64.fname, &fstate) == -1)
    no_rom = 1;
  else if (S_ISREG (fstate.st_mode) != TRUE)
    no_rom = 1;
#if 0
  // printing the no_rom error message for files of 0 bytes only confuses people
  else if (!fstate.st_size)
    no_rom = 1;
#endif

  if (no_rom)
    {
      if (!(ucon64.flags & WF_NO_ROM))
        {
          fputs ("ERROR: This option requires a file argument (ROM/image/SRAM file/directory)\n",
                 stderr);
          return -1;
        }
      return 0;
    }

  // The next statement is important and should be executed as soon as
  //  possible (and sensible) in this function
  if ((ucon64.file_size = fsizeof (ucon64.fname)) < 0)
    {
      fprintf (stderr, "ERROR: Could not determine size of %s\n", ucon64.fname);
      return -1;
    }
  // We have to do this here, because we don't know the file size until now
  if (ucon64.backup_header_len > ucon64.file_size)
    {
      fprintf (stderr,
               "ERROR: A backup unit header length was specified that is larger than the file\n"
               "       size (%d > %d)\n", ucon64.backup_header_len, ucon64.file_size);
      return -1;
    }

  if (!(ucon64.flags & WF_INIT))
    return 0;

  // Try to find the correct console by analysing the ROM
  if (ucon64.flags & WF_PROBE)
    {
      if (ucon64.nfo)
        {
          // Restore any overrides from st_ucon64_t
          // We have to do this *before* calling ucon64_probe(), *not* afterwards
          if (UCON64_ISSET (ucon64.backup_header_len))
            nfo.backup_header_len = ucon64.backup_header_len;

          if (UCON64_ISSET (ucon64.interleaved))
            nfo.interleaved = ucon64.interleaved;

//          ucon64.nfo = (st_ucon64_nfo_t *) &nfo;
        }
      ucon64.nfo = ucon64_probe (&nfo); // determines console type

#ifdef  USE_DISCMAGE
      // check for disc image only if ucon64_probe() failed or --disc was used
      if (ucon64.discmage_enabled)
//        if (!ucon64.nfo || ucon64.force_disc)
        if (ucon64.force_disc)
          ucon64.image = dm_reopen (ucon64.fname, 0, (dm_image_t *) ucon64.image);
#endif
    }
  // end of WF_PROBE

  // Does the option allow split ROMs?
  if (ucon64.flags & WF_NO_SPLIT)
    /*
      Test for split files only if the console type knows about split files at
      all. However we only know the console type after probing.
    */
    if ((ucon64.console == UCON64_NES || ucon64.console == UCON64_SNES ||
         ucon64.console == UCON64_GEN || ucon64.console == UCON64_NG) &&
        (UCON64_ISSET (ucon64.split) ? ucon64.split : ucon64_testsplit (ucon64.fname, NULL)))
      {
        fprintf (stderr, "ERROR: %s seems to be split. You have to join it first\n",
                 basename2 (ucon64.fname));
        return -1;
      }


  /*
    CRC32

    Calculating the CRC32 checksum for the ROM data of a UNIF file (NES)
    shouldn't be done with ucon64_fcrc32(). nes_init() uses crc32().
    The CRC32 checksum is used to search in the DAT files, but at the time
    of this writing (Februari the 7th 2003) all DAT files contain checksums
    of files in only one format. This matters for SNES and Genesis ROMs in
    interleaved format and Nintendo 64 ROMs in non-interleaved format. The
    corresponding initialization functions calculate the CRC32 checksum of
    the data in the format of which the checksum is stored in the DAT
    files. For these "problematic" files, their "real" checksum is stored
    in ucon64.fcrc32.
  */
  if (ucon64.crc32 == 0 && !ucon64.force_disc && // NOT for disc images
      !(ucon64.flags & WF_NO_CRC32) && ucon64.file_size <= MAXROMSIZE)
    ucon64_chksum (NULL, NULL, &ucon64.crc32, ucon64.fname, ucon64.file_size,
                   ucon64.nfo ? ucon64.nfo->backup_header_len : 0);


  // DATabase
  ucon64.dat = NULL;
  if (ucon64.crc32 != 0 && ucon64.dat_enabled)
    {
      ucon64.dat = ucon64_dat_search (ucon64.crc32, NULL);
      if (ucon64.dat)
        {
          // detected file size must match DAT file size
          int size = ucon64.nfo ?
                       UCON64_ISSET (ucon64.nfo->data_size) ?
                         ucon64.nfo->data_size :
                         ucon64.file_size - ucon64.nfo->backup_header_len :
                       ucon64.file_size;
          if ((int) (((st_ucon64_dat_t *) ucon64.dat)->fsize) != size)
            ucon64.dat = NULL;
        }

      if (ucon64.dat)
        switch (ucon64.console)
          {
          case UCON64_SNES:
          case UCON64_GEN:
          case UCON64_GB:
          case UCON64_GBA:
          case UCON64_N64:
            // These ROMs have internal headers with name, country, maker, etc.
            break;

          default:
            // Use ucon64.dat instead of ucon64.dat_enabled in case the index
            //  file could not be created/opened -> no segmentation fault
            if (ucon64.dat && ucon64.nfo)
              {
                if (!ucon64.nfo->name[0])
                  strcpy (ucon64.nfo->name, NULL_TO_EMPTY (((st_ucon64_dat_t *) ucon64.dat)->name));
                else if (ucon64.console == UCON64_NES)
                  { // override the three-character FDS or FAM name
                    int t = nes_get_file_type ();
                    if (t == FDS || t == FAM)
                      strcpy (ucon64.nfo->name, NULL_TO_EMPTY (((st_ucon64_dat_t *) ucon64.dat)->name));
                  }

                if (!ucon64.nfo->country)
                  ucon64.nfo->country = NULL_TO_EMPTY (((st_ucon64_dat_t *) ucon64.dat)->country);
              }
            break;
          }
    }

  // display info
  if ((ucon64.flags & WF_NFO) && ucon64.quiet < 1)
    ucon64_nfo ();

  return 0;
}


static st_ucon64_nfo_t *
ucon64_probe (st_ucon64_nfo_t *nfo)
{
  typedef struct
    {
      int console;
      int (*init) (st_ucon64_nfo_t *);
      uint32_t flags;
    } st_probe_t;

// auto recognition
#define AUTO 1

  int x = 0;
  st_probe_t probe[] =
    {
      /*
        The order of the init functions is important. snes_init() must be
        called before nes_init(), but after gb_init() and sms_init().
        sms_init() must be called before snes_init(), but after genesis_init().
        There may be more dependencies, so don't change the order unless you
        can verify it won't break anything.
      */
      {UCON64_GBA, gba_init, AUTO},
      {UCON64_N64, n64_init, AUTO},
      {UCON64_GEN, genesis_init, AUTO},
      {UCON64_LYNX, lynx_init, AUTO},
      {UCON64_GB, gb_init, AUTO},
      {UCON64_SMS, sms_init, AUTO},
      {UCON64_COLECO, coleco_init, AUTO},
      {UCON64_SNES, snes_init, AUTO},
      {UCON64_NES, nes_init, AUTO},
      {UCON64_NGP, ngp_init, AUTO},
      {UCON64_SWAN, swan_init, AUTO},
      {UCON64_JAG, jaguar_init, AUTO},
      {UCON64_ATA, atari_init, AUTO},
      {UCON64_NDS, nds_init, AUTO},
      {UCON64_VBOY, vboy_init, 0},
      {UCON64_PCE, pce_init, 0}, // AUTO still works with non-PCE files
      {UCON64_NG, neogeo_init, 0},
      {UCON64_SWAN, swan_init, 0},
      {UCON64_DC, dc_init, 0},
      {UCON64_PSX, psx_init, 0},
#if 0
      {UCON64_GC, NULL, 0},
      {UCON64_GP32, NULL, 0},
      {UCON64_INTELLI, NULL, 0},
      {UCON64_S16, NULL, 0},
      {UCON64_VEC, NULL, 0},
#endif
      {UCON64_UNKNOWN, unknown_console_init, 0},
      {0, NULL, 0}
    };

  if (ucon64.console != UCON64_UNKNOWN)         // force recognition option was used
    {
      for (x = 0; probe[x].console != 0; x++)
        if (probe[x].console == ucon64.console)
          {
            ucon64_clear_nfo (nfo);

            probe[x].init (nfo);

            return nfo;
          }
    }
  else if (ucon64.file_size <= MAXROMSIZE)      // give auto recognition a try
    {
      for (x = 0; probe[x].console != 0; x++)
        if (probe[x].flags & AUTO)
          {
            ucon64_clear_nfo (nfo);

            if (!probe[x].init (nfo))
              {
                ucon64.console = probe[x].console;
                return nfo;
              }
          }
    }

  return NULL;
}


int
ucon64_nfo (void)
{
  puts (ucon64.fname);
  if (ucon64.fname_arch[0])
    printf ("  (%s)\n", ucon64.fname_arch);
  fputc ('\n', stdout);
#ifdef  USE_DISCMAGE
  if (ucon64.console == UCON64_UNKNOWN && !ucon64.image)
#else
  if (ucon64.console == UCON64_UNKNOWN)
#endif
    fprintf (stderr, "%s\n", ucon64_msg[CONSOLE_ERROR]);

  if (ucon64.nfo && ucon64.console != UCON64_UNKNOWN && !ucon64.force_disc)
    ucon64_rom_nfo (ucon64.nfo);

#ifdef  USE_DISCMAGE
  if (ucon64.discmage_enabled && ucon64.image)
    {
      dm_nfo ((dm_image_t *) ucon64.image, ucon64.quiet < 0 ? 1 : 0,
#ifdef  USE_ANSI_COLOR
              ucon64.ansi_color ? 1 :
#endif
                                  0);
      fputc ('\n', stdout);

      return 0; // no crc calc. for disc images and therefore no DAT entry either
    }
#endif
  // Use ucon64.fcrc32 for SNES, Genesis & SMS interleaved/N64 non-interleaved
  if (ucon64.fcrc32 && ucon64.crc32)
    printf ("Search checksum (CRC32): 0x%08x\n"
            "Data checksum (CRC32): 0x%08x\n", ucon64.crc32, ucon64.fcrc32);
  else if (ucon64.fcrc32 || ucon64.crc32)
    printf ("Checksum (CRC32): 0x%08x\n", ucon64.fcrc32 ? ucon64.fcrc32 : ucon64.crc32);

  // The check for the size of the file is made, so that uCON64 won't display a
  //  (nonsense) DAT info line when dumping a ROM (file doesn't exist, so
  //  ucon64.file_size is 0).
  if (ucon64.file_size > 0 && ucon64.dat_enabled && ucon64.dat)
    ucon64_dat_nfo ((st_ucon64_dat_t *) ucon64.dat, 1);

  fputc ('\n', stdout);

  return 0;
}


static inline char *
to_func (char *s, int len, char (*func) (char))
{
  char *p = s;

  for (; len > 0; p++, len--)
    *p = func (*p);

  return s;
}


static inline char
toprint (char c)
{
  if (isprint ((int) c))
    return c;

  // characters that also work with printf()
#ifdef  USE_ANSI_COLOR
  if (c == '\x1b')
    return ucon64.ansi_color ? c : '.';
#endif

  return strchr ("\t\n\r", c) ? c : '.';
}


static void
ucon64_rom_nfo (const st_ucon64_nfo_t *nfo)
{
  unsigned int padded = ucon64_testpad (ucon64.fname),
               intro = ((ucon64.file_size - nfo->backup_header_len) > MBIT) ?
                         ((ucon64.file_size - nfo->backup_header_len) % MBIT) : 0;
  int x, split = (UCON64_ISSET (ucon64.split)) ? ucon64.split :
                   ucon64_testsplit (ucon64.fname, NULL);
  char buf[MAXBUFSIZE];

  // backup unit header
  if (nfo->backup_header && nfo->backup_header_len &&
      nfo->backup_header_len != UNKNOWN_BACKUP_HEADER_LEN)
    {
      dumper (stdout, nfo->backup_header, nfo->backup_header_len,
              nfo->backup_header_start, DUMPER_HEX);
      fputc ('\n', stdout);
    }
  else
    if (nfo->backup_header_len && ucon64.quiet < 0)
      {
        ucon64_dump (stdout, ucon64.fname, nfo->backup_header_start,
                     nfo->backup_header_len, DUMPER_HEX);
        fputc ('\n', stdout);
      }

  // backup unit type?
  if (nfo->backup_usage != NULL)
    {
      puts (nfo->backup_usage);
      fputc ('\n', stdout);
    }

  // ROM header
  if (nfo->header && nfo->header_len)
    {
      dumper (stdout, nfo->header, nfo->header_len,
        nfo->header_start + nfo->backup_header_len, DUMPER_HEX);
      fputc ('\n', stdout);
    }

  // console type
  if (nfo->console_usage != NULL)
    puts (nfo->console_usage);

  // name, maker, country and size
  strcpy (buf, NULL_TO_EMPTY (nfo->name));
  x = UCON64_ISSET (nfo->data_size) ?
    nfo->data_size :
    ucon64.file_size - nfo->backup_header_len;
  printf ("%s\n%s\n%s\n%d Bytes (%.4f Mb)\n\n",
          // some ROMs have a name with control chars in it -> replace control chars
          to_func (buf, strlen (buf), toprint),
          NULL_TO_EMPTY (nfo->maker),
          NULL_TO_EMPTY (nfo->country),
          x,
          TOMBIT_F (x));

  // padded?
  if (!padded)
    puts ("Padded: No");
  else
    printf ("Padded: Maybe, %d Bytes (%.4f Mb)\n", padded, TOMBIT_F (padded));

  // intro, trainer?
  // nes.c determines itself whether or not there is a trainer
  if (intro && ucon64.console != UCON64_NES)
    printf ("Intro/Trainer: Maybe, %d Bytes\n", intro);

  // interleaved?
  if (nfo->interleaved != UCON64_UNKNOWN)
    // printing this is handy for SNES, N64 & Genesis ROMs, but maybe
    //  nonsense for others
    printf ("Interleaved/Swapped: %s\n",
      nfo->interleaved ?
        (nfo->interleaved > 1 ? "Yes (2)" : "Yes") :
        "No");

  // backup unit header?
  if (nfo->backup_header_len)
    printf ("Backup unit/emulator header: Yes, %d Bytes\n",
      nfo->backup_header_len);
  else
// for NoisyB: <read only mode ON>
    puts ("Backup unit/emulator header: No");   // printing No is handy for SNES ROMs
// for NoisyB: <read only mode OFF>

  // split?
  if (split)
    {
      printf ("Split: Yes, %d part%s\n", split, (split != 1) ? "s" : "");
      // nes.c calculates the correct checksum for split ROMs (=Pasofami
      // format), so there is no need to join the files
      if (ucon64.console != UCON64_NES)
        puts ("NOTE: To get the correct checksum the ROM parts must be joined");
    }

  // miscellaneous info
  if (nfo->misc[0])
    {
      strcpy (buf, nfo->misc);
      printf ("%s\n", to_func (buf, strlen (buf), toprint));
    }

  // internal checksums?
  if (nfo->has_internal_crc)
    {
      char *fstr;

      // the internal checksum of GBA ROMS stores only the checksum of the
      //  internal header
      if (ucon64.console != UCON64_GBA)
        fstr = "Checksum: %%s, 0x%%0%dlx (calculated) %%c= 0x%%0%dlx (internal)\n";
      else
        fstr = "Header checksum: %%s, 0x%%0%dlx (calculated) %%c= 0x%%0%dlx (internal)\n";

      sprintf (buf, fstr,
        nfo->internal_crc_len * 2, nfo->internal_crc_len * 2);
#ifdef  USE_ANSI_COLOR
      printf (buf,
        ucon64.ansi_color ?
          ((nfo->current_internal_crc == nfo->internal_crc) ?
            "\x1b[01;32mOK\x1b[0m" : "\x1b[01;31mBad\x1b[0m")
          :
          ((nfo->current_internal_crc == nfo->internal_crc) ? "OK" : "Bad"),
        nfo->current_internal_crc,
        (nfo->current_internal_crc == nfo->internal_crc) ? '=' : '!',
        nfo->internal_crc);
#else
      printf (buf,
        (nfo->current_internal_crc == nfo->internal_crc) ? "OK" : "Bad",
        nfo->current_internal_crc,
        (nfo->current_internal_crc == nfo->internal_crc) ? '=' : '!',
        nfo->internal_crc);
#endif

      if (nfo->internal_crc2[0])
        printf ("%s\n", nfo->internal_crc2);
    }

  fflush (stdout);
}


#ifdef  USE_ZLIB
void
ucon64_fname_arch (const char *fname)
{
  char name[FILENAME_MAX];

  unzFile file = unzOpen (fname);
  unzip_goto_file (file, unzip_current_file_nr);
  unzGetCurrentFileInfo (file, NULL, name, FILENAME_MAX, NULL, 0, NULL, 0);
  unzClose (file);
#if     defined _WIN32 || defined __MSDOS__
  {
    int n, l = strlen (name);
    for (n = 0; n < l; n++)
      if (name[n] == '/')
        name[n] = DIR_SEPARATOR;
  }
#endif
  strncpy (ucon64.fname_arch, basename2 (name), FILENAME_MAX)[FILENAME_MAX - 1] = 0;
}
#endif


void
ucon64_usage (int argc, char *argv[], int view)
{
  int x = 0, y = 0, c = 0, single = 0;
  const char *name_exe = basename2 (argv[0]);
#ifdef  USE_DISCMAGE
  char *name_discmage;
#endif
  (void) argc;                                  // warning remover

#ifdef  USE_ZLIB
  printf ("Usage: %s [OPTION]... [ROM|IMAGE|SRAM|FILE|DIR|ARCHIVE]...\n\n", name_exe);
#else
  printf ("Usage: %s [OPTION]... [ROM|IMAGE|SRAM|FILE|DIR]...\n\n", name_exe);
#endif

  // single usage
  for (x = 0; arg[x].val; x++)
    if (arg[x].console) // IS console
      for (y = 0; option[y]; y++)
        for (c = 0; option[y][c].name || option[y][c].help; c++)
          if (option[y][c].object &&
              ((st_ucon64_obj_t *) option[y][c].object)->console == arg[x].console)
            {
              getopt2_usage (option[y]);
              single = 1;
              break;
            }

  if (!single)
    switch (view)
      {
        case USAGE_VIEW_LONG:
          getopt2_usage (options);
          break;

        case USAGE_VIEW_PAD:
          getopt2_usage (ucon64_padding_usage);
          break;

        case USAGE_VIEW_DAT:
          getopt2_usage (ucon64_dat_usage);
          break;

        case USAGE_VIEW_PATCH:
          getopt2_usage (patch_usage);
          getopt2_usage (bsl_usage);
          getopt2_usage (ips_usage);
          getopt2_usage (aps_usage);
          getopt2_usage (ppf_usage);
          getopt2_usage (gg_usage);
          break;

        case USAGE_VIEW_BACKUP:
//          getopt2_usage (cc2_usage);
#ifdef  USE_LIBCD64
          getopt2_usage (cd64_usage);
#endif
          getopt2_usage (cmc_usage);
          getopt2_usage (dex_usage);
          getopt2_usage (doctor64_usage);
          getopt2_usage (doctor64jr_usage);
          getopt2_usage (f2a_usage);
          getopt2_usage (fal_usage);
          getopt2_usage (fig_usage);
          getopt2_usage (gbx_usage);
          getopt2_usage (gd_usage);
//          getopt2_usage (interceptor_usage);
          getopt2_usage (lynxit_usage);
          getopt2_usage (mccl_usage);
          getopt2_usage (mcd_usage);
          getopt2_usage (mdpro_usage);
//          getopt2_usage (mgd_usage);
          getopt2_usage (msg_usage);
//          getopt2_usage (nfc_usage);
          getopt2_usage (pcepro_usage);
          getopt2_usage (pl_usage);
          getopt2_usage (quickdev16_usage);
//          getopt2_usage (sc_usage);
          getopt2_usage (sflash_usage);
          getopt2_usage (smc_usage);
          getopt2_usage (smd_usage);
          getopt2_usage (smsggpro_usage);
//          getopt2_usage (spsc_usage);
//          getopt2_usage (ssc_usage);
          getopt2_usage (swc_usage);
//          getopt2_usage (ufo_usage);
//          getopt2_usage (yoko_usage);
//          getopt2_usage (z64_usage);
          break;

#ifdef  USE_DISCMAGE
        case USAGE_VIEW_DISC:
          getopt2_usage (discmage_usage);
          break;
#endif

        case USAGE_VIEW_SHORT:
        default:
          getopt2_usage (ucon64_options_usage);
      }

  fputc ('\n', stdout);

  printf ("DATabase: %d known ROMs (DAT files: %s)\n\n",
          ucon64_dat_total_entries (), ucon64.datdir);

#ifdef  USE_DISCMAGE
  name_discmage =
#ifdef  DLOPEN
    ucon64.discmage_path;
#else
#if     defined __MSDOS__
    "discmage.dxe";
#elif   defined __CYGWIN__ || defined _WIN32
    "discmage.dll";
#elif   defined __APPLE__                       // Mac OS X actually
    "libdiscmage.dylib";
#elif   defined __unix__ || defined __BEOS__
    "libdiscmage.so";
#else
    "library";
#endif
#endif

  if (!ucon64.discmage_enabled)
    {
      printf (ucon64_msg[NO_LIB], name_discmage);
      fputc ('\n', stdout);
    }
#endif

  puts ("Please report problems, fixes or ideas to ucon64-main@lists.sf.net or visit\n"
        "http://ucon64.sourceforge.net\n");
}
