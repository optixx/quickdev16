/*
 *
 * PSX Peripheral Bus Library v1.4 17/01/00 Richard Davies
 * <mailto:richard@debaser.force9.co.uk>
 *
 * Revision History:
 * v1.4 - 17/01/00 Win32 / Win32 DLL support, rewrite and bug fixes
 * v1.3 - 21/12/99 Linux support and bug fixes
 * v1.1 - 26/09/99 Minor Controller detection improvements.
 * v1.0 - 17/07/99 Initial release (based on PSXTest v1.1 by me).
 *
 * see psxpblib.h for details.
 *
 */
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <fcntl.h>
#include <ctype.h>
#ifdef  HAVE_DIRENT_H
#include <dirent.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "misc/misc.h"
#include "misc/itypes.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "psxpblib.h"
#include "misc/parallel.h"


#ifdef  USE_PARALLEL

static unsigned char psx_parallel_out_0 = 0xff;
static unsigned char psx_parallel_out_2 = 0x00;


/*
 *
 * sets clock for conport connected to parallel port base
 *
 */
void
psx_clk (int base, int conport, int on)
{
  if (conport == 8)
    {
      if (on)
        {
          /* set controller clock high */
          psx_parallel_out_2 |= LPT_STR;
        }
      else
        {
          /* set controller clock low */
          psx_parallel_out_2 &= ~LPT_STR;
        }

      psx_outportb (base + 2, psx_parallel_out_2);
    }
  else
    {
      if (on)
        {
          /* set controller clock high */
          psx_parallel_out_0 |= LPT_D2;
        }
      else
        {
          /* set controller clock low */
          psx_parallel_out_0 &= ~LPT_D2;
        }

      psx_outportb (base + 0, psx_parallel_out_0);
    }
}


/*
 *
 * sets att for conport connected to parallel port base
 *
 */
void
psx_att (int base, int conport, int on)
{
  /* bits 3-7 base + 0 (pins 5 to 9 parallel port) */
  const int power = LPT_D3 | LPT_D4 | LPT_D5 | LPT_D6 | LPT_D7;
  /* bits 1-6 base + 0 (pins 3, 5, 6, 7 and 8 parallel port) */
  unsigned char att_array[] =
    { LPT_D1, LPT_D1, LPT_D3, LPT_D4, LPT_D5, LPT_D6, LPT_D7 };
  unsigned char att;

  if (conport == 8)
    {
      if (on)
        {
          /* set controller att high */
          psx_parallel_out_2 |= LPT_INI;
        }
      else
        {
          /* set controller att low */
          psx_parallel_out_2 &= ~LPT_INI;
        }

      psx_outportb (base + 2, psx_parallel_out_2);
    }
  else
    {
      /* powers up all parallel port driven conports */
      psx_parallel_out_0 |= power;

      att = att_array[conport - 1];

      if (on)
        {
          /* set controller att high */
          psx_parallel_out_0 |= att;
        }
      else
        {
          /* set controller att low */
          psx_parallel_out_0 &= ~att;
        }

      psx_outportb (base + 0, psx_parallel_out_0);
    }
}


/*
 *
 * sets command for conport connected to parallel port base
 *
 */
void
psx_cmd (int base, int conport, int on)
{
  if (conport == 8)
    {
      if (on)
        {
          /* set controller cmd high */
          psx_parallel_out_2 |= LPT_AUT;
        }
      else
        {
          /* set controller cmd low */
          psx_parallel_out_2 &= ~LPT_AUT;
        }

      psx_outportb (base + 2, psx_parallel_out_2);
    }
  else
    {
      if (on)
        {
          /* set controller cmd high */
          psx_parallel_out_0 |= LPT_D0;
        }
      else
        {
          /* set controller cmd low */
          psx_parallel_out_0 &= ~LPT_D0;
        }

      psx_outportb (base + 0, psx_parallel_out_0);
    }
}


/*
 *
 * tests data for conport connected to parallel port base, returns 1 if high
 *
 */
int
psx_dat (int base, int conport)
{
  if (conport == 2)
    {
      if (psx_inportb (base + 1) & LPT_SEL)
        {
          return 1;
        }
      else
        {
          return 0;
        }
    }
  else
    {
      if (psx_inportb (base + 1) & LPT_ACK)
        {
          return 1;
        }
      else
        {
          return 0;
        }
    }
}


/*
 *
 * tests ack for conport connected to parallel port base, returns 1 if high
 *
 */
int
psx_ack (int base, int conport)
{
  if (conport == 2)
    {
      if (psx_inportb (base + 2) & LPT_ERR)
        {
          return 1;
        }
      else
        {
          return 0;
        }
    }
  else if (conport == 8)
    {
      if (psx_inportb (base + 1) & LPT_SEL)
        {
          return 1;
        }
      else
        {
          return 0;
        }
    }
  else
    {
      if (psx_inportb (base + 1) & LPT_PAP)
        {
          return 1;
        }
      else
        {
          return 0;
        }
    }
}


/*
 *
 * wait for delay * (psx_outportb() execution time) 
 *
 */
void
psx_delay (int base, int delay)
{
  int i;

  for (i = 0; i < delay; i++)
    {
      psx_outportb (base + 0, psx_parallel_out_0);
    }
}


/*
 *
 * send byte as a command to conport connected to parallel port base
 * assumes clock high and the attention for conport
 *
 */
unsigned char
psx_sendbyte (int base, int conport, int delay, unsigned char byte, int wait)
{
  int i;
  unsigned char data;

  data = 0;

  /* for each bit in byte */
  for (i = 0; i < 8; i++)
    {
      psx_delay (base, delay);
      psx_cmd (base, conport, byte & (1 << i)); /* send the (i+1)th bit of byte to any listening controller */
      psx_clk (base, conport, 0);       /* clock low */
      psx_delay (base, delay);
      data |= (psx_dat (base, conport) ? (1 << i) : 0); /* read the (i+1)th bit of data from conport */
      psx_clk (base, conport, 1);       /* clock high */
    }

  /* wait for controller ack */
  for (i = 0; wait && i < 10240 && psx_ack (base, conport); i++)
    ;

  return data;
}


/*
 *
 * sets clock high and gets the attention of conport, use before psx_sendbyte()
 *
 */
void
psx_sendinit (int base, int conport, int delay)
{
//  psx_obtain_io_permission (base);// uCON64 already enabled access to I/O ports
  psx_att (base, conport, 1);   /* set att on for conport */
  psx_clk (base, conport, 1);   /* clock high */
  psx_cmd (base, conport, 1);   /* set command on for conport */
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_att (base, conport, 0);   /* set att off for conport */
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
}


/*
 *
 * use after psx_sendbyte()
 *
 */
void
psx_sendclose (int base, int conport, int delay)
{
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_att (base, conport, 1);   /* set att on for conport */
  psx_cmd (base, conport, 1);   /* set command on for conport */
  psx_clk (base, conport, 1);   /* clock high */
  psx_delay (base, delay);
}


/*
 *
 * send string as a series of commands to conport connected to parallel port base
 *
 */
void
psx_sendstring (int base, int conport, int delay, int string[])
{
  int i;

  psx_sendinit (base, conport, delay);

  /* for each byte in string */
  for (i = 0; string[i + 1] != -1; i++)
    {
      /* send byte i and wait for conport ack */
      psx_sendbyte (base, conport, delay, (unsigned char) string[i], 0);
      psx_delay (base, delay);
    }

  /* send the last byte in string and don't wait for ack */
  psx_sendbyte (base, conport, delay, (unsigned char) string[i], 0);

  psx_sendclose (base, conport, delay);
}


/*
 *
 * tests for the presence of a controller on conport:tap connected to base
 * returns the type if present, otherwise -1
 *
 */
int
psx_controller_detect (int base, int conport, int tap, int delay)
{
  unsigned char ack;
  int type, length;

  psx_sendinit (base, conport, delay);

  psx_sendbyte (base, conport, delay, (unsigned char) tap, 0);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  ack = psx_sendbyte (base, conport, delay, 0x42, 0);
  psx_delay (base, delay);

  psx_sendclose (base, conport, delay);

  type = (ack & 0xf0) >> 4;
  length = 2 * (ack & 0x0f);

  /* check the controller has a legal packet length */
  if (!((length > 0) && (length < PSX_MAX_DATA)))
    return -1;

  /* check the controller has a legal id */
  if (!((type > 0) && (type < 0x0f) && (type != 0x08)))
    return -1;

  return type;
}


/*
 *
 * reads a controller on conport:tap connected to base returns the data
 * if present, otherwise -1
 *
 */
PSX_CON_BUF *
psx_controller_read (int base, int conport, int tap, int delay)
{
  unsigned char ack;
  int i;
  static PSX_CON_BUF psx_con_buf;

  psx_sendinit (base, conport, delay);

  psx_sendbyte (base, conport, delay, (unsigned char) tap, 0);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  ack = psx_sendbyte (base, conport, delay, 0x42, 0);
  psx_delay (base, delay);

  psx_con_buf.type = (ack & 0xf0) >> 4;
  psx_con_buf.length = 2 * (ack & 0x0f);

  /* check the controller has a legal packet length */
  if (!((psx_con_buf.length > 0) && (psx_con_buf.length < PSX_MAX_DATA)))
    {
      psx_sendclose (base, conport, delay);
      return NULL;
    }

  /* check the controller has a legal id */
  if (!
      ((psx_con_buf.type > 0) && (psx_con_buf.type < 0x0f)
       && (psx_con_buf.type != 0x08)))
    {
      psx_sendclose (base, conport, delay);
      return NULL;
    }

  psx_sendbyte (base, conport, delay, 0x00, 0);
  psx_delay (base, delay);
  for (i = 0; i < psx_con_buf.length; i++)
    {
      psx_con_buf.data[i] = psx_sendbyte (base, conport, delay, 0x00, 0);
      psx_delay (base, delay);
    }
  psx_sendclose (base, conport, delay);

  return &psx_con_buf;
}


/*
 *
 * sends force feedback/shock init command sequence to conport:tap on port base
 * (also initialises crash protection for some controllers)
 *
 */
void
psx_controller_vinit (int base, int conport, int tap, int delay)
{
  int i, vibrate_init_string[3][11] =
    {
      {tap, 0x43, 0x00, 0x01, 0x00, 0x01, -1},
      {tap, 0x4d, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x01, -1},
      {tap, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, -1},
    };

  for (i = 0; i < 3; i++)
    {
      psx_delay (base, delay);
      psx_delay (base, delay);
      psx_delay (base, delay);
      psx_sendstring (base, conport, delay, vibrate_init_string[i]);
    }
}


/*
 *
 * sends the dual shock command sequence to conport:tap on port base
 *
 */
void
psx_controller_vshock (int base, int conport, int tap, int delay, int shock,
                       int rumble)
{
  int dualshock_string[7] = { tap, 0x42, 0x00, shock, rumble, 0x01, -1 };

  psx_controller_vinit (base, conport, tap, delay);

  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_delay (base, delay);
  psx_sendstring (base, conport, delay, dualshock_string);
}


/*
 *
 * Reads a single frame (128 bytes) from Memory Card on conport base:tap
 *
 */
unsigned char *
psx_memcard_read_frame (int base, int conport, int tap, int delay, int frame)
{
  int i, xor_val;
  static unsigned char data[128], c_data;
  unsigned char cmd_rstring_hdr[4] = { (0x80 + (unsigned char) tap), 0x52, 0x00, 0x00 },
                chk_rstring_hdr[2] = { 0x5a, 0x5d },
                cmd_rstring_adr[2] = { (frame >> 8) & 0xff, frame & 0xff },
                chk_rstring_ack[1] = { 0x5c },
                chk_rstring_sfl[1] = { 0x5d },
                chk_rstring_efl[1] = { 0x47 };

  psx_sendinit (base, conport, delay);

  /* send header */
  for (i = 0; i < 2; i++)
    {
      psx_sendbyte (base, conport, delay, cmd_rstring_hdr[i], 0);
      psx_delay (base, delay);
      psx_delay (base, delay);
    }

  for (; i < 4; i++)
    {
      c_data = psx_sendbyte (base, conport, delay, cmd_rstring_hdr[i], 0);
      psx_delay (base, delay);
      psx_delay (base, delay);

      if (c_data != chk_rstring_hdr[i - 2])
        {
          psx_sendclose (base, conport, delay);
          return NULL;
        }
    }

  /* send read address */
  for (i = 0; i < 2; i++)
    {
      psx_sendbyte (base, conport, delay, cmd_rstring_adr[i], 0);
      psx_delay (base, delay);
    }

  /* receive command ack (have to wait for this) */
  c_data = psx_sendbyte (base, conport, delay, 0x00, 1);

  if (c_data != chk_rstring_ack[0])
    {
      psx_sendclose (base, conport, delay);
      return NULL;
    }

  /* receive start of data flag */
  i = 0;

  while (c_data != chk_rstring_sfl[0])
    {
      c_data = psx_sendbyte (base, conport, delay, 0x00, 0);
      psx_delay (base, delay);

      i++;

      if (i > 255)
        {
          psx_sendclose (base, conport, delay);
          return NULL;
        }
    }

  /* receive read address */
  for (i = 0; i < 2; i++)
    {
      c_data = psx_sendbyte (base, conport, delay, 0x00, 0);
      psx_delay (base, delay);

      if (c_data != cmd_rstring_adr[i])
        {
          psx_sendclose (base, conport, delay);
          return NULL;
        }
    }

  /* receive data */
  for (i = 0; i < 128; i++)
    {
      data[i] = psx_sendbyte (base, conport, delay, 0x00, 0);
      psx_delay (base, delay);
    }

  /* receive xor */
  c_data = psx_sendbyte (base, conport, delay, 0x00, 0);
  psx_delay (base, delay);

  /* test xor */
  xor_val = 0;

  xor_val ^= cmd_rstring_adr[0];
  xor_val ^= cmd_rstring_adr[1];

  for (i = 0; i < 128; i++)
    xor_val ^= data[i];

  if (xor_val != c_data)
    {
      psx_sendclose (base, conport, delay);
      return NULL;
    }

  /* receive end of data flag */
  c_data = psx_sendbyte (base, conport, delay, 0x00, 0);
  psx_delay (base, delay);

  if (c_data != chk_rstring_efl[0])
    {
      psx_sendclose (base, conport, delay);
      return NULL;
    }

  psx_sendclose (base, conport, delay);

  return data;
}


/*
 *
 * Writes a single frame (128 bytes) to Memory Card on conport base:tap
 *
 */
int
psx_memcard_write_frame (int base, int conport, int tap, int delay, int frame,
                         unsigned char *data_f)
{
  int i, xor_val;
  unsigned char c_data,
                cmd_wstring_hdr[4] = { (0x80 + (unsigned char) tap), 0x57, 0x00, 0x00 },
                chk_wstring_hdr[2] = { 0x5a, 0x5d },
                cmd_wstring_adr[2] = { (frame >> 8) & 0xff, frame & 0xff },
                chk_wstring_emk[2] = { 0x5c, 0x5d },
                chk_wstring_efl[1] = { 0x47 };

  psx_sendinit (base, conport, delay);

  /* send header (have to wait for this) */
  for (i = 0; i < 2; i++)
    {
      psx_sendbyte (base, conport, delay, cmd_wstring_hdr[i], 1);
    }

  for (; i < 4; i++)
    {
      c_data = psx_sendbyte (base, conport, delay, cmd_wstring_hdr[i], 1);

      if (c_data != chk_wstring_hdr[i - 2])
        {
          psx_sendclose (base, conport, delay);
          return -1;
        }
    }

  /* send write address */
  for (i = 0; i < 2; i++)
    {
      psx_sendbyte (base, conport, delay, cmd_wstring_adr[i], 0);
      psx_delay (base, delay);
    }
  /* send data */
  for (i = 0; i < 128; i++)
    {
      psx_sendbyte (base, conport, delay, data_f[i], 0);
      psx_delay (base, delay);
    }

  /* calculate xor */
  xor_val = 0;

  xor_val ^= cmd_wstring_adr[0];
  xor_val ^= cmd_wstring_adr[1];

  for (i = 0; i < 128; i++)
    xor_val ^= data_f[i];

  /* send xor */
  psx_sendbyte (base, conport, delay, (unsigned char) xor_val, 0);
  psx_delay (base, delay);

  /* receive end mark */
  for (i = 0; i < 2; i++)
    {
      c_data = psx_sendbyte (base, conport, delay, 0x00, 1);

      if (c_data != chk_wstring_emk[i])
        {
          psx_sendclose (base, conport, delay);
          return -1;
        }
    }

  /* receive end of data flag */
  c_data = psx_sendbyte (base, conport, delay, 0x00, 1);

  if (c_data != chk_wstring_efl[0])
    {
      psx_sendclose (base, conport, delay);
      return -1;
    }

  psx_sendclose (base, conport, delay);

  return (int) ((unsigned char) xor_val);
}


/*
 *
 * Reads a single block (64 frames) from Memory Card on conport base:tap
 *
 */
unsigned char *
psx_memcard_read_block (int base, int conport, int tap, int delay, int block)
{
  int i, j;
  static unsigned char data_b[8192], *data_f;

  for (i = 0; i < 64; i++)
    {
      data_f =
        psx_memcard_read_frame (base, conport, tap, delay, (block * 64) + i);

      if (data_f != NULL)
        {
          for (j = 0; j < 128; j++)
            data_b[(i * 128) + j] = data_f[j];
        }
      else
        {
          return NULL;
        }
    }

  return data_b;
}


/*
 *
 * Writes a single block (64 frames) to Memory Card on conport base:tap
 *
 */
int
psx_memcard_write_block (int base, int conport, int tap, int delay, int block,
                         unsigned char *data_b)
{
  int i, xor_val;

  for (i = 0; i < 64; i++)
    {
      xor_val =
        psx_memcard_write_frame (base, conport, tap, delay, (block * 64) + i,
                                 &(data_b[128 * i]));

      if (xor_val == -1)
        {
          return -1;
        }
    }

  return 1;
}


/*
 *
 * Reads the info associated with block from the directory
 *
 */
PSX_MCB_INFO_DIR *
psx_mcb_read_dir (int base, int conport, int tap, int delay, int block)
{
  int i, xor_val;
  unsigned char *data_f;
  static PSX_MCB_INFO_DIR mcb_info_dir;

  mcb_info_dir.read = 1;

  /* check memory card state */
  data_f = psx_memcard_read_frame (base, conport, tap, delay, 0);

  if (data_f == NULL)
    {
      mcb_info_dir.read = 0;
      return &mcb_info_dir;
    }

  if ((data_f[0] != 'M') && (data_f[1] != 'C'))
    {
      mcb_info_dir.read = 0;
      return &mcb_info_dir;
    }

  for (i = 2; i < 127; i++)
    {
      if (data_f[i] != 0x00)
        {
          mcb_info_dir.read = 0;
          return &mcb_info_dir;
        }
    }

  if (data_f[127] != 0x0e)
    {
      mcb_info_dir.read = 0;
      return &mcb_info_dir;
    }

  /* read block's directory */
  data_f = psx_memcard_read_frame (base, conport, tap, delay, block);

  if (data_f == NULL)
    {
      mcb_info_dir.read = 0;
      return &mcb_info_dir;
    }

  xor_val = 0;

  for (i = 0; i < 127; i++)
    xor_val ^= data_f[i];

  if (xor_val != data_f[127])
    {
      mcb_info_dir.read = 0;
      return &mcb_info_dir;
    }

  mcb_info_dir.linktype = data_f[0] & 0x0f;

  /* Only test if first link */
  if (mcb_info_dir.linktype == PSX_MCB_LTYPE_FIRST)
    {
      if (data_f[10] != 'B')
        {
          mcb_info_dir.read = 0;
          return &mcb_info_dir;
        }
    }

  mcb_info_dir.state = (data_f[0] >> 4) & 0x0f;

  mcb_info_dir.bytes = (data_f[4] << 16) + (data_f[5] << 8) + data_f[6];

  mcb_info_dir.next = data_f[8];        /* 0 to 14 */

  mcb_info_dir.territory = data_f[11];  /* E, A or I */

  for (i = 0; i < 10; i++)
    mcb_info_dir.code[i] = (char) data_f[12 + i];

  mcb_info_dir.code[i] = '\0';

  for (i = 0; i < 8; i++)
    mcb_info_dir.filename[i] = data_f[22 + i];

  mcb_info_dir.filename[i] = '\0';

  return &mcb_info_dir;
}


/*
 *
 * Reads the info associated with block from it's data
 *
 */
PSX_MCB_INFO_DAT *
psx_mcb_read_dat (int base, int conport, int tap, int delay, int block)
{
  int i, j;
  unsigned char *data_f;
  static PSX_MCB_INFO_DAT mcb_info_dat;

  mcb_info_dat.read = 1;

  if ((block < 1) || (block > 15))
    {
      mcb_info_dat.read = 0;
      return &mcb_info_dat;
    }

  data_f =
    psx_memcard_read_frame (base, conport, tap, delay, (block * 64) + 0);

  if (!data_f)
    {
      mcb_info_dat.read = 0;
      return &mcb_info_dat;
    }

  if ((data_f[0] != 'S') || (data_f[1] != 'C'))
    {
      mcb_info_dat.read = 0;
      return &mcb_info_dat;
    }

  mcb_info_dat.icon_valid = (data_f[2] >> 4) & 0x0f;
  mcb_info_dat.icon_frames = data_f[2] & 0x0f;
  mcb_info_dat.blocks = data_f[3];

  /* bodged character set conversion */
  j = 0;
  for (i = 0; i < 91; i += 2)
    {
      if (data_f[4 + i] != 0x00)
        {
          if (data_f[4 + i] == 0x81)
            {
              if (data_f[5 + i] == 0x7c)
                mcb_info_dat.name[j] = '-';
              else if (data_f[5 + i] == 0x40)
                mcb_info_dat.name[j] = ' ';
              else if (data_f[5 + i] == 0x46)
                mcb_info_dat.name[j] = ':';
              else if (data_f[5 + i] == 0x5e)
                mcb_info_dat.name[j] = '/';
              else if (data_f[5 + i] == 0x49)
                mcb_info_dat.name[j] = '!';
              else if (data_f[5 + i] == 0x93)
                mcb_info_dat.name[j] = '%';
              else if (data_f[5 + i] == 0x68)
                mcb_info_dat.name[j] = '\"';
              else if (data_f[5 + i] == 0x44)
                mcb_info_dat.name[j] = '.';
              else if (data_f[5 + i] == 0x6d)
                mcb_info_dat.name[j] = '[';
              else if (data_f[5 + i] == 0x6e)
                mcb_info_dat.name[j] = ']';
              else if (data_f[5 + i] == 0x69)
                mcb_info_dat.name[j] = '(';
              else if (data_f[5 + i] == 0x6a)
                mcb_info_dat.name[j] = ')';
              else
                mcb_info_dat.name[j] = '?';
            }
          else if (data_f[4 + i] == 0x82)
            {
              if ((data_f[5 + i] > 0x4e) && (data_f[5 + i] < 0x80))
                mcb_info_dat.name[j] = data_f[5 + i] - 0x1f;
              else if ((data_f[4 + i] > 0x80) && (data_f[5 + i] < 0x9a))
                mcb_info_dat.name[j] = data_f[5 + i] - 0x20;
              else
                mcb_info_dat.name[j] = '?';
            }
          else
            {
              mcb_info_dat.name[j] = data_f[4 + i];

              j++;
              if (data_f[5 + i] != 0x00)
                mcb_info_dat.name[j] = data_f[5 + i];
              else
                {
                  mcb_info_dat.name[j] = '\0';
                  i = 91;
                }
            }
        }
      else
        {
          mcb_info_dat.name[j] = '\0';
          i = 91;
        }
      j++;
    }

  return &mcb_info_dat;
}


/*
 *
 * Merges the info associated with block from the directory and it's data
 *
 */
PSX_MCB_INFO *
psx_mcb_info_merge (PSX_MCB_INFO_DIR mcb_info_dir,
                    PSX_MCB_INFO_DAT mcb_info_dat, PSX_MCB_INFO * mcb_info)
{
  mcb_info->read = 1;

  if (!mcb_info_dir.read)
    {
      mcb_info->read = 0;
      return mcb_info;
    }

  if ((mcb_info_dir.linktype == PSX_MCB_LTYPE_FIRST) && (!mcb_info_dat.read))
    {
      mcb_info->read = 0;
      return mcb_info;
    }

  strcpy (mcb_info->filename, mcb_info_dir.filename);
  strcpy (mcb_info->code, mcb_info_dir.code);
  mcb_info->territory = mcb_info_dir.territory;
  mcb_info->bytes = mcb_info_dir.bytes;
  mcb_info->state = mcb_info_dir.state;
  mcb_info->linktype = mcb_info_dir.linktype;
  mcb_info->next = mcb_info_dir.next;
  if (mcb_info_dir.linktype == PSX_MCB_LTYPE_FIRST)
    {
      strcpy (mcb_info->name, mcb_info_dat.name);
      mcb_info->blocks = mcb_info_dat.blocks;
      mcb_info->icon_valid = mcb_info_dat.icon_valid;
      mcb_info->icon_frames = mcb_info_dat.icon_frames;
    }
  else
    {
      mcb_info->name[0] = '\0';
      mcb_info->blocks = 0;
      mcb_info->icon_valid = 0;
      mcb_info->icon_frames = 0;
    }

  return mcb_info;
}


/*
 *
 * Reads the info associated with block from the directory and it's data
 *
 */
PSX_MCB_INFO *
psx_mcb_read_info (int base, int conport, int tap, int delay, int block)
{
  PSX_MCB_INFO_DIR *mcb_info_dir;
  PSX_MCB_INFO_DAT *mcb_info_dat;
  static PSX_MCB_INFO mcb_info;

  mcb_info_dir = psx_mcb_read_dir (base, conport, tap, delay, block);
  mcb_info_dat = psx_mcb_read_dat (base, conport, tap, delay, block);
  return psx_mcb_info_merge (*mcb_info_dir, *mcb_info_dat, &mcb_info);
}

#endif // USE_PARALLEL
