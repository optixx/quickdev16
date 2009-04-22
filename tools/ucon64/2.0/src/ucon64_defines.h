/*
ucon64_defines.h - definitions for uCON64

Copyright (c) 2002 - 2003 NoisyB <noisyb@gmx.net>
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
#ifndef UCON64_DEFINES_H
#define UCON64_DEFINES_H

// Please make sure that NO definition except FALSE has 0 as value!
#if     (!defined TRUE || !defined FALSE)
#define FALSE 0
#define TRUE (!FALSE)
#endif

#if     (!defined MIN || !defined MAX)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define LIB_VERSION(ver, rel, seq) (((ver) << 16) | ((rel) << 8) | (seq))
#define NULL_TO_EMPTY(str) ((str) ? (str) : (""))
//#define RANDOM(min, max) ((rand () % (max - min)) + min)
#define OFFSET(a, offset) ((((unsigned char *) &(a)) + (offset))[0])

#define UCON64_UNKNOWN (-1)
#define UCON64_UNKNOWN_S "Unknown"
#define NULL_TO_UNKNOWN_S(str) ((str) ? (str) : (UCON64_UNKNOWN_S))

#define UCON64_VERSION_S "2.0.0"

/* program version counter */
//#define UCON64_VERSION (200)

/* version of config file layout */
#define UCON64_CONFIG_VERSION (207)

#define MBIT (131072)
#define TOMBIT(x) ((int)(x) / MBIT)
#define TOMBIT_F(x) ((float)(x) / MBIT)

#define MAXROMSIZE (512 * MBIT)

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif // MAXBUFSIZE

// maximum number of arguments uCON64 takes from the cmdline
#define UCON64_MAX_ARGS 512

#define UCON64_OPTION (1000)
#define UCON64_CONSOLE (0)

// options (consoles)
// these defines shall not exceed 0xffff since they share the same integer
// with the workflow flags (below) in st_getopt2_t
enum
{
  UCON64_3DO = UCON64_CONSOLE + 1,
  UCON64_ATA,
  UCON64_CD32,
  UCON64_CDI,
  UCON64_COLECO,
  UCON64_DC,
  UCON64_GB,
  UCON64_GBA,
  UCON64_GC,
  UCON64_GEN,
  UCON64_GP32,
  UCON64_INTELLI,
  UCON64_JAG,
  UCON64_LYNX,
  UCON64_MAME,
  UCON64_N64,
  UCON64_NES,
  UCON64_NG,
  UCON64_NGP,
  UCON64_PCE,
  UCON64_PS2,
  UCON64_PSX,
  UCON64_S16,
  UCON64_SAT,
  UCON64_SMS,
  // don't mix the following with UCON64_GG (Game Genie), used only for --mgdgg
  UCON64_GAMEGEAR,
  UCON64_SNES,
  UCON64_SWAN,
  UCON64_VBOY,
  UCON64_VEC,
  UCON64_XBOX,
};

// options
enum
{
  UCON64_1991 = UCON64_OPTION + 1,
  UCON64_A,
  UCON64_B,
  UCON64_B0,
  UCON64_B1,
  UCON64_BAT,
  UCON64_BIN,
  UCON64_BIOS,
  UCON64_BOT,
  UCON64_BS,
  UCON64_C,
  UCON64_CHK,
  UCON64_CODE,
  UCON64_COL,
  UCON64_CRC,
  UCON64_CRCHD,
  UCON64_CRP,
  UCON64_CS,
  UCON64_CTRL,
  UCON64_CTRL2,
  UCON64_CMNT,
  UCON64_DB,
  UCON64_DBS,
  UCON64_DBUH,
  UCON64_DBV,
  UCON64_DINT,
  UCON64_DMIRR,
  UCON64_DNSRT,
  UCON64_DUAL,
  UCON64_DUMPINFO,
  UCON64_E,
  UCON64_EROM,
  UCON64_F,
  UCON64_FDS,
  UCON64_FDSL,
  UCON64_FFE,
  UCON64_FIG,
  UCON64_FIGS,
  UCON64_FILE,
  UCON64_FIND,
  UCON64_FINDI,
  UCON64_FINDR,
  UCON64_FRONTEND,
  UCON64_GBX,
  UCON64_GD3,
  UCON64_GD3S,
  UCON64_GG,
  UCON64_GGD,
  UCON64_GGE,
  UCON64_HD,
  UCON64_HDN,
  UCON64_HELP,
  UCON64_HEX,
  UCON64_HI,
  UCON64_I,
  UCON64_ID,
  UCON64_IDPPF,
  UCON64_INES,
  UCON64_INESHD,
  UCON64_INS,
  UCON64_INSN,
  UCON64_INT,
  UCON64_INT2,
  UCON64_ISPAD,
  UCON64_J,
  UCON64_K,
  UCON64_L,
  UCON64_LNX,
  UCON64_LOGO,
  UCON64_LS,
  UCON64_LSD,
  UCON64_LSRAM,
  UCON64_LSV,
  UCON64_LYX,
  UCON64_MAPR,
  UCON64_MD5,
  UCON64_MGD,
  UCON64_MGDGG,
  UCON64_MGH,
  UCON64_MIRR,
  UCON64_MKA,
  UCON64_MKDAT,
  UCON64_MKI,
  UCON64_MKIP,
  UCON64_MKPPF,
  UCON64_MSG,
  UCON64_MULTI,
//  UCON64_MVS,
  UCON64_N,
  UCON64_N2,
  UCON64_N2GB,
  UCON64_NA,
  UCON64_NBAK,
  UCON64_NBAT,
  UCON64_NBS,
  UCON64_NCOL,
  UCON64_NHD,
  UCON64_NHI,
  UCON64_NINT,
  UCON64_NPPF,
  UCON64_NROT,
  UCON64_NS,
  UCON64_NSWP,
  UCON64_NTSC,
  UCON64_NVRAM,
  UCON64_O,
  UCON64_P,
  UCON64_PAD,
  UCON64_PADHD,
  UCON64_PADN,
  UCON64_PAL,
  UCON64_PARSE,
  UCON64_PASOFAMI,
  UCON64_PATCH,
  UCON64_PATTERN,
  UCON64_POKE,
  UCON64_PORT,
  UCON64_PPF,
  UCON64_PRINT,
  UCON64_Q,
  UCON64_QQ,                                    // already reserved ;-)
  UCON64_REGION,
  UCON64_RENAME,
  UCON64_RROM,
  UCON64_RR83,
  UCON64_RL,
  UCON64_ROM,
  UCON64_ROTL,
  UCON64_ROTR,
  UCON64_RU,
  UCON64_S,
  UCON64_SAM,
  UCON64_SCAN,
  UCON64_SCR,
  UCON64_SGB,
  UCON64_SHA1,
  UCON64_SMC,
  UCON64_SMD,
  UCON64_SMDS,
  UCON64_SRAM,
  UCON64_SSC,
  UCON64_SSIZE,
  UCON64_STP,
  UCON64_STPN,
  UCON64_STRIP,
  UCON64_SWAP,
  UCON64_SWAP2,
  UCON64_SWC,
  UCON64_SWCS,
  UCON64_SWP,
  UCON64_TEST,
  UCON64_UFO,
  UCON64_UFOS,
  UCON64_UNIF,
  UCON64_UNSCR,
  UCON64_USMS,
  UCON64_V,
  UCON64_V64,
  UCON64_VER,
  UCON64_VMS,
  UCON64_VRAM,
  UCON64_XCD64,
  UCON64_XCD64B,
  UCON64_XCD64C,
  UCON64_XCD64E,
  UCON64_XCD64F,
  UCON64_XCD64M,
  UCON64_XCD64P,
  UCON64_XCD64S,
  UCON64_XCMC,
  UCON64_XCMCM,
  UCON64_XCMCT,
  UCON64_XDEX,
  UCON64_XDJR,
  UCON64_XF2A,
  UCON64_XF2AB,
  UCON64_XF2AC,
  UCON64_XF2AMULTI,
  UCON64_XF2AS,
  UCON64_XFAL,
  UCON64_XFALB,
  UCON64_XFALC,
  UCON64_XFALMULTI,
  UCON64_XFALS,
  UCON64_XFALM,                                 // actually only necessary for the Windows
  UCON64_XFIG,                                  //  ports, but might be useful for others too
  UCON64_XFIGS,
  UCON64_XFIGC,
  UCON64_XGBX,
  UCON64_XGBXB,
  UCON64_XGBXS,
  UCON64_XGBXM,
  UCON64_XGD3,
  UCON64_XGD3R,
  UCON64_XGD3S,
  UCON64_XGD6,
  UCON64_XGD6R,
  UCON64_XGD6S,
  UCON64_XGG,
  UCON64_XGGB,
  UCON64_XGGS,
  UCON64_XLIT,
  UCON64_XMCCL,
  UCON64_XMCD,
  UCON64_XMD,
  UCON64_XMDB,
  UCON64_XMDS,
  UCON64_XMSG,
  UCON64_XPCE,
  UCON64_XPL,
  UCON64_XPLI,
  UCON64_XPLM,
  UCON64_XSF,
  UCON64_XSFS,
  UCON64_XSMC,
  UCON64_XSMCR,
  UCON64_XSMD,
  UCON64_XSMDS,
  UCON64_XSWC,
  UCON64_XSWC2,
  UCON64_XSWC_IO,
  UCON64_XSWCR,
  UCON64_XSWCS,
  UCON64_XSWCC,
  UCON64_XV64,
  UCON64_Z64,

  UCON64_FORCE63,
  UCON64_GUI,

  // Keep these (libdiscmage) options separate
  UCON64_DISC = UCON64_OPTION + 250,
  UCON64_MKCUE,
  UCON64_MKSHEET,
  UCON64_MKTOC,
  UCON64_RIP,
  UCON64_BIN2ISO,
  UCON64_ISOFIX,
  UCON64_XCDRW,
  UCON64_CDMAGE
};

/*
  uCON64 workflow flags

  WF_PROBE          probe for console type
  WF_INIT           init ROM info (ucon64_init()) necessary
                      w/o this flag WF_NFO and WF_NFO_AFTER
                      will be ignored
  WF_NFO            show info output before processing rom
  WF_NFO_AFTER      show info output AFTER processing rom
  WF_NO_ROM         for this option no ROM is required
  WF_NO_CRC32       no CRC32 calculation necessary for this option; this
                      overrides even WF_INIT, WF_NFO and WF_NFO_AFTER
  WF_STOP           a "stop" option:
                    - -multi (and -xfalmulti) takes more than one file as
                      argument, but should be executed only once.
                    - stop after sending one ROM to a copier ("multizip")
                    - stop after applying a patch so that the patch file won't
                      be interpreted as ROM
  WF_PAR            this option requires a parallel port
  WF_USB            this option requires a USB port
  WF_SERIAL         this option requires a serial port
  WF_NO_SPLIT       this option does not work with split ROMs
  WF_DEFAULT        same as WF_INIT | WF_PROBE | WF_NFO

  example:
  WF_NFO | WF_MFO_AFTER
                    a ROM is required and info will be shown before and
                    after it has been processed

  important:
    we shift these flags 16 bit because they share the same integer with
    those UCON64_<console> defines in st_getopt2_t
*/
#define WF_DEFAULT   (WF_PROBE | WF_INIT | WF_NFO)
#define WF_PROBE     (1)
#define WF_INIT      (1 << 1)
#define WF_NFO       (1 << 2)
#define WF_STOP      (1 << 3)
#define WF_NFO_AFTER (1 << 4)
#define WF_NO_ROM    (1 << 5)
#define WF_PAR       (1 << 6)
#define WF_USB       (1 << 7)
#define WF_SERIAL    (1 << 8)
#define WF_NO_CRC32  (1 << 9)
#define WF_NO_SPLIT  (1 << 10)
#define WF_SWITCH    (1 << 11)

#endif // UCON64_DEFINES_H
