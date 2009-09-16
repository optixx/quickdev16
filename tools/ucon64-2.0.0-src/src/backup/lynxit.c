/*
lynxit.c - lynxit support for uCON64

Copyright (c) 1997 - ???? K. Wilkins
Copyright (c) 2002        NoisyB <noisyb@gmx.net>
Copyright (c) 2004        dbjh


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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "console/lynx.h"
#include "lynxit.h"
#include "misc/parallel.h"


const st_getopt2_t lynxit_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Lynxit (Lynx cartridge backup board)"/*"1997 K.Wilkins (selfmade)"*/,
      NULL
    },
#ifdef  USE_PARALLEL
    {
      "xlit", 0, 0, UCON64_XLIT,
      NULL, "receive ROM from Lynxit interface; " OPTION_LONG_S "port=PORT",
//    "                  receives automatically when ROM does not exist\n"
      &ucon64_wf[WF_OBJ_LYNX_STOP_NO_ROM]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
  };


#ifdef USE_PARALLEL

#define MAGIC_STRING           "LYNX"
#define FILE_FORMAT_VERSION    0x0001


//
// Some basic cartidge definitions
//

#define MAX_PAGE_SIZE  0x1000   // Must be 2x largest page

#define CART_PAGE_64K  0x100    // 256 Bytes per page
#define CART_PAGE_128K 0x200    // 512 Bytes per page
#define CART_PAGE_256K 0x400    // 1024 Bytes per page
#define CART_PAGE_512K 0x800    // 2048 Bytes per page

#define BANK0  0
#define BANK1  1


//
// DEFINE CENTRONICS PORT MASKS & PORTS
//

#define    PORT_BASE   0x80     // Keep the power on 8-)
#define    CTRL_STB    0x01
#define    DATA_OUT    0x02
#define    DATA_CLK    0x04
#define    DATA_STB    0x08
#define    DATA_OE     0x10
#define    DATA_LOAD   0x20
#define    PAGE_STB    0x40

unsigned int printer_port = 1, print_data = 0, print_ctrl = 0, print_stat = 0;
char cartname[32], manufname[16];

//
// CONTROL REGISTER DEFINITIONS
//


#define    CTRL_BANK0B     0x01
#define    CTRL_BANK1B     0x02
#define    CTRL_WR_EN      0x04
#define    CTRL_CTR_CLKB   0x40
#define    CTRL_CTR_RST    0x80

#define    CTRL_INACTIVE   (CTRL_BANK0B|CTRL_BANK1B|CTRL_CTR_CLKB)


// Make cart strobes inactive & counter clock & reset inactive 

unsigned char control_register = CTRL_INACTIVE;
static void lynxit_write_control (unsigned char data);


//
// Global control vars
//

static int debug = FALSE, quiet = FALSE, verify = TRUE;


#define  MESSAGE(body) printf body

#define  INPUT(port)        inportb((unsigned short) (port))
#define  OUTPUT(port, data) outportb((unsigned short) (port), (unsigned char) (data))


#if 0
void
usage (void)
{
  MESSAGE (("\nUsage: lynxit [-pX] [-d] [-q] <command> <filename> [cartname] [manuf]\n"));
  MESSAGE (("\n"));
  MESSAGE (("   Commands = read/write/verify/test\n"));
  MESSAGE (("        -pX = Use printer port LPTX: (Default LPT1:)\n"));
  MESSAGE (("        -d  = Debug mode enable\n"));
  MESSAGE (("        -q  = Quiet mode\n"));
  MESSAGE (("        -n  = Don't verify read/write operations\n"));
  MESSAGE (("        -h  = Print this text\n"));
}
#endif


int
ptr_port_init (unsigned int port)
{
#if 0
  if (port < 1 && port > 4)
    return FALSE;

  print_data = *(unsigned int far *) MK_FP (0x0040, 6 + (2 * port));

  if (!print_data)
    return FALSE;
#else
  (void) port;                                  // warning remover
#endif
  print_stat = print_data + 1;
  print_ctrl = print_data + 2;

#if 0
  DEBUG (("\nPrinter port initialised OK: LPT%d\n", port));
  DEBUG (("Data    I/O 0x%04x\n", print_data));
  DEBUG (("Status  I/O 0x%04x\n", print_stat));
  DEBUG (("Control I/O 0x%04x\n\n", print_ctrl));
#endif

  lynxit_write_control (control_register);

  return TRUE;
}


void
lynxit_shift_out_byte (unsigned char data)
{
  unsigned int loop;
  unsigned char outbyte, dbgdata;

  dbgdata = data;

  OUTPUT (print_data, PORT_BASE);       // Set inactive

  for (loop = 0; loop < 8; loop++)
    {
      outbyte = PORT_BASE;
      outbyte |= (data & 0x80) ? DATA_OUT : 0;
      OUTPUT (print_data, outbyte);     // Output data; clock low
      data = data << 1;
      outbyte |= DATA_CLK;
      OUTPUT (print_data, outbyte);     // clock high
    }

  OUTPUT (print_data, PORT_BASE);       // Leave outputs low

#if 0
  DEBUG (("lynxit_shift_out_byte() - Wrote %02x\n", dbgdata));
#endif
}


unsigned char
lynxit_shift_in_byte (void)
{
  unsigned int loop;
  unsigned char data = 0;

  OUTPUT (print_data, PORT_BASE);       // Set inactive

  for (loop = 0; loop < 8; loop++)
    {
      data |= (((INPUT (print_stat)) & 0x80) ? 0 : 1) << (7 - loop);
#if 0
      DEBUG (("Status port returned %02x\n", INPUT (print_stat)));
#endif
      OUTPUT (print_data, PORT_BASE | DATA_CLK);        // clock high
      OUTPUT (print_data, PORT_BASE);   // clock low
    }

#if 0
  DEBUG (("lynxit_shift_in_byte() - Read %02x\n", data));
#endif
  return (data);
}


void
lynxit_write_control (unsigned char data)
{
#if 0
  DEBUG (("lynxit_write_control()  - Set to %02x\n", data));
#endif
  lynxit_shift_out_byte (data);

  OUTPUT (print_data, PORT_BASE);       // clock low; strobe low
  OUTPUT (print_data, PORT_BASE | CTRL_STB);    // clock low; strobe high
  OUTPUT (print_data, PORT_BASE);       // clock low; strobe low

  control_register = data;
}


void
lynxit_write_page (unsigned char page)
{
#if 0
  DEBUG (("lynxit_write_page()  - Set to %02x\n", page));
#endif
  lynxit_shift_out_byte (page);
  lynxit_shift_out_byte (0);

  OUTPUT (print_data, PORT_BASE);       // clock low; strobe low
  OUTPUT (print_data, PORT_BASE | PAGE_STB);    // clock low; strobe high
  OUTPUT (print_data, PORT_BASE);       // clock low; strobe low
}


void
lynxit_counter_reset (void)
{
  lynxit_write_control ((unsigned char) (control_register | CTRL_CTR_RST));
  lynxit_write_control ((unsigned char) (control_register & (CTRL_CTR_RST ^ 0xff)));
}


void
lynxit_counter_increment (void)
{
  lynxit_write_control ((unsigned char) (control_register & (CTRL_CTR_CLKB ^ 0xff)));
  lynxit_write_control ((unsigned char) (control_register | CTRL_CTR_CLKB));
}


unsigned char
cart_read_byte (unsigned int cart)
{
  unsigned char data;

  // Clear relevant cart strobe to activate read

  if (cart == BANK0)
    {
      lynxit_write_control ((unsigned char) (control_register & (0xff ^ CTRL_BANK0B)));
    }
  else
    {
      lynxit_write_control ((unsigned char) (control_register & (0xff ^ CTRL_BANK1B)));
    }

  // Clock byte into shift register with load
  //
  // Note 500ns read cycle for ROM == 5x125ns out cycles


  OUTPUT (print_data, PORT_BASE);       // clock low
  OUTPUT (print_data, PORT_BASE);       // clock low
  OUTPUT (print_data, PORT_BASE);       // clock low
  OUTPUT (print_data, PORT_BASE);       // clock low
  OUTPUT (print_data, PORT_BASE | DATA_LOAD);   // Parallel load
  OUTPUT (print_data, PORT_BASE | DATA_LOAD);   // Parallel load
  OUTPUT (print_data, PORT_BASE);       // Clear load

  // This must be done before strobe is cleared as data will be
  // destoryed bye setting control reg

  data = lynxit_shift_in_byte ();

  // Clear the cartridge strobe

  if (cart == BANK0)
    {
      lynxit_write_control ((unsigned char) (control_register | CTRL_BANK0B));
    }
  else
    {
      lynxit_write_control ((unsigned char) (control_register | CTRL_BANK1B));
    }

#if 0
  DEBUG (("cart_read_byte() - Returning %02x\n", data));
#endif

//   MESSAGE(("%c",data));

  return (data);
}


void
cart_write_byte (unsigned int cart, unsigned char data)
{
#if 0
  DEBUG (("cart_write_byte() - Set to %02x\n", data));
#endif

  // Shift data to correct position

  lynxit_shift_out_byte (data);
  lynxit_shift_out_byte (0);
  lynxit_shift_out_byte (0);

  // Strobe byte to be written into the data register

  OUTPUT (print_data, PORT_BASE);       // clock low; strobe low
  OUTPUT (print_data, PORT_BASE | DATA_STB);    // clock low; strobe high
  OUTPUT (print_data, PORT_BASE);       // clock low; strobe low

  // Assert write enable (active low)

  lynxit_write_control ((unsigned char) (control_register & (0xff ^ CTRL_WR_EN)));

  // Assert output enable

  OUTPUT (print_data, PORT_BASE | DATA_OE);

  // Assert cartridge strobe LOW to write then HIGH

  if (cart == BANK0)
    {
      lynxit_write_control ((unsigned char) (control_register & (0xff ^ CTRL_BANK0B)));
      lynxit_write_control ((unsigned char) (control_register | CTRL_BANK0B));
    }
  else
    {
      lynxit_write_control ((unsigned char) (control_register & (0xff ^ CTRL_BANK1B)));
      lynxit_write_control ((unsigned char) (control_register | CTRL_BANK1B));
    }

  // Clear output enable

  OUTPUT (print_data, PORT_BASE);

  // Clear write enable

  lynxit_write_control ((unsigned char) (control_register | CTRL_WR_EN));

}


void
cart_read_page (unsigned int cart, unsigned int page_number,
                unsigned int page_size, unsigned char *page_ptr)
{
  unsigned int loop;

  lynxit_write_page ((unsigned char) page_number);
  lynxit_counter_reset ();

  for (loop = 0; loop < page_size; loop++)
    {
      *page_ptr = cart_read_byte (cart);
      page_ptr++;
      lynxit_counter_increment ();
    }
}


#if 0
void
cart_write_page (unsigned int cart, unsigned int page_number,
                 unsigned int page_size, unsigned char *page_ptr)
{
}
#endif


int
cart_analyse (int cart)
{
  unsigned char image[MAX_PAGE_SIZE];
  unsigned int page = 0;
  unsigned char test = 0;
  unsigned int loop = 0;

  MESSAGE (("ANALYSE  : BANK%d ", cart));

  for (;;)
    {
      // Read a page - start at zero, try a max of 8 pages

      if (page > 8)
        {
          MESSAGE (("N/A\n"));
          return FALSE;
        }

      cart_read_page (cart, page++, CART_PAGE_512K * 2, image);

      // Explicit check for no bank

      test = 0xff;
      for (loop = 0; loop < CART_PAGE_512K; loop++)
        test &= image[loop];

      if (test == 0xff)
        {
          MESSAGE (("N/A\n"));
          return FALSE;
        }

      // Check bytes are not all same

      for (loop = 1; loop < CART_PAGE_512K; loop++)
        {
          if (image[loop] != image[0])
            break;
        }

      // If we are at end of loop then buffer is all same

      if (loop != CART_PAGE_512K)
        break;
    }

//   {
//       FILE *fp;
//       fp=fopen("TEST.IMG","wb");
//       fwrite(image,sizeof(unsigned char),MAX_PAGE_SIZE,fp);
//       fclose(fp);
//   {

  // Check for 64K Cart

  for (loop = 0; loop < CART_PAGE_64K; loop++)
    {
      if (image[loop] != image[loop + CART_PAGE_64K])
        break;
    }

  if (loop == CART_PAGE_64K)
    {
      MESSAGE (("64K\n"));
      return CART_PAGE_64K;
    }

  // Check for 128K Cart

  for (loop = 0; loop < CART_PAGE_128K; loop++)
    {
      if (image[loop] != image[loop + CART_PAGE_128K])
        break;
    }

  if (loop == CART_PAGE_128K)
    {
      MESSAGE (("128K\n"));
      return CART_PAGE_128K;
    }

  // Check for 256K Cart

  for (loop = 0; loop < CART_PAGE_256K; loop++)
    {
      if (image[loop] != image[loop + CART_PAGE_256K])
        break;
    }

  if (loop == CART_PAGE_256K)
    {
      MESSAGE (("256K\n"));
      return CART_PAGE_256K;
    }

  // Check for 512K Cart

  for (loop = 0; loop < CART_PAGE_512K; loop++)
    {
      if (image[loop] != image[loop + CART_PAGE_512K])
        break;
    }

  if (loop == CART_PAGE_512K)
    {
      MESSAGE (("512K\n"));
      return CART_PAGE_512K;
    }

  // Must be no cart situation -  floating data !!!)

  MESSAGE (("Bad cartridge\n"));

  return FALSE;
}


#define  MAX_ERRORS    16

int
cart_verify (char *filename)
{
  unsigned char image1[MAX_PAGE_SIZE], image2[MAX_PAGE_SIZE];
  int offset = 0;
  unsigned int loop = 0;
  FILE *fp;
  st_lnx_header_t header;
  unsigned int result0 = MAX_ERRORS, result1 = MAX_ERRORS;


#if 0
  DEBUG (("cart_verify() called with <%s>\n\n", filename));
#endif

  if ((fp = fopen (filename, "rb")) == NULL)
    {
//      MESSAGE (("ERROR    : Could not open %s\n", filename));
      MESSAGE ((ucon64_msg[OPEN_READ_ERROR], filename));
      return FALSE;
    }

  if (fread (&header, sizeof (st_lnx_header_t), 1, fp) != 1)
    {
      MESSAGE (("ERROR    : Disk read operation failed on %s\n", filename));
      fclose (fp);
      return FALSE;
    }

  if (strcmp (header.magic, "LYNX") != 0)
    {
      MESSAGE (("ERROR    : %s is not a lynx image\n", filename));
      fclose (fp);
      return FALSE;
    }

  if (header.version != FILE_FORMAT_VERSION)
    {
      MESSAGE (("ERROR    : %s has wrong version information\n", filename));
      fclose (fp);
      return FALSE;
    }

  if (header.page_size_bank0 != cart_analyse (BANK0))
    {
      MESSAGE (("ERROR    : Cartridge BANK0 size mismatch\n"));
      fclose (fp);
      return FALSE;
    }

  if (header.page_size_bank1 != cart_analyse (BANK1))
    {
      MESSAGE (("ERROR    : Cartridge BANK1 size mismatch\n"));
      fclose (fp);
      return FALSE;
    }


  if (header.page_size_bank0)
    {
      for (loop = 0; loop < 256; loop++)
        {
          MESSAGE (("Verifying BANK0: Page <%03d> of <256>", loop + 1));
          cart_read_page (BANK0, loop, header.page_size_bank0, image1);
          MESSAGE (("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"));
          if (fread (image2, sizeof (unsigned char), header.page_size_bank0, fp)
              != (size_t) header.page_size_bank0)
            {
              MESSAGE (("\nERROR    : Disk read operation failed on %s\n",
                        filename));
              fclose (fp);
              return FALSE;
            }

          for (offset = 0; offset < header.page_size_bank0; offset++)
            {
              if (image1[offset] != image2[offset])
                {
                  MESSAGE (("VERIFY   : Mismatch in BANK<0>:PAGE<%02x>:OFFSET<%03x>\n", loop, offset));
                  if (!(--result0))
                    {
                      MESSAGE (("VERIFY   : Too many errors in BANK0, aborting\n"));
                      loop = 256;
                      break;
                    }
                }
            }
        }
    }

  if (header.page_size_bank1)
    {
      for (loop = 0; loop < 256; loop++)
        {
          MESSAGE (("Verifying BANK1: Page <%03d> of <256>", loop + 1));
          cart_read_page (BANK1, loop, header.page_size_bank1, image1);
          MESSAGE (("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"));
          if (fread (image2, sizeof (unsigned char), header.page_size_bank1, fp)
              != (size_t) header.page_size_bank1)
            {
              MESSAGE (("\nERROR    : Disk read operation failed on %s\n",
                        filename));
              fclose (fp);
              return FALSE;
            }

          for (offset = 0; offset < header.page_size_bank0; offset++)
            {
              if (image1[offset] != image2[offset])
                {
                  MESSAGE (("VERIFY   : Mismatch in BANK<1>:PAGE<%02x>:OFFSET<%03x>\n", loop, offset));
                  if (!(--result1))
                    {
                      MESSAGE (("VERIFY   : Too many errors in BANK1, aborting\n"));
                      break;
                    }
                }
            }
        }
    }

  fclose (fp);

  if (result0 != MAX_ERRORS || result1 != MAX_ERRORS)
    {
      MESSAGE (("VERIFY   : FAILED                   \n"));
      return FALSE;
    }
  else
    {
      MESSAGE (("VERIFY   : OK                       \n"));
      return TRUE;
    }
}


int
cart_read (char *filename)
{
  unsigned char image[MAX_PAGE_SIZE];
//  unsigned int page = 0;
  unsigned int loop = 0;
  FILE *fp;
  st_lnx_header_t header;

#if 0
  DEBUG (("read_cart() called with <%s>\n\n", filename));
#endif

  memset (&header, 0, sizeof (st_lnx_header_t));
  strcpy (header.magic, MAGIC_STRING);
  strcpy (header.cartname, cartname);
  strcpy (header.manufname, manufname);
  header.page_size_bank0 = cart_analyse (BANK0);
  header.page_size_bank1 = cart_analyse (BANK1);
  header.version = FILE_FORMAT_VERSION;

  if ((fp = fopen (filename, "wb")) == NULL)
    return FALSE;

  if (fwrite (&header, sizeof (st_lnx_header_t), 1, fp) != 1)
    {
      MESSAGE (("ERROR    : Disk write operation failed on %s\n", filename));
      fclose (fp);
      return FALSE;
    }

  if (header.page_size_bank0)
    {
      for (loop = 0; loop < 256; loop++)
        {
          MESSAGE (("Reading BANK0: Page <%03d> of <256>", loop + 1));
          cart_read_page (BANK0, loop, header.page_size_bank0, image);
          MESSAGE (("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"));
          if (fwrite (image, sizeof (unsigned char), header.page_size_bank0, fp)
              != (size_t) header.page_size_bank0)
            {
              MESSAGE (("\nERROR    : Disk write operation failed on %s\n",
                        filename));
              fclose (fp);
              return FALSE;
            }
        }
    }

  if (header.page_size_bank1)
    {
      for (loop = 0; loop < 256; loop++)
        {
          MESSAGE (("Reading BANK1: Page <%03d> of <256>", loop + 1));
          cart_read_page (BANK1, loop, header.page_size_bank1, image);
          MESSAGE (("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"));
          if (fwrite (image, sizeof (unsigned char), header.page_size_bank1, fp)
              != (size_t) header.page_size_bank1)
            {
              MESSAGE (("\nERROR    : Disk write operation failed on %s\n",
                        filename));
              fclose (fp);
              return FALSE;
            }
        }
    }

  MESSAGE (("READ     : OK                         \n"));

  fclose (fp);

  if (verify)
    return (cart_verify (filename));
  else
    return TRUE;
}


#if 0
int
cart_write (char *filename)
{
  DEBUG (("write_cart() called with <%s>\n\n", filename));
  return TRUE;
}
#endif


int
perform_test (char *testname)
{
  if (strcmp (testname, "LOOPBACK") == 0)
    {
      unsigned int loop;

      for (loop = 0; loop < 256; loop++)
        {
          lynxit_shift_out_byte ((unsigned char) loop);
          lynxit_shift_out_byte (0);
          lynxit_shift_out_byte (0);
          lynxit_shift_out_byte (0);

          if (lynxit_shift_in_byte () != (unsigned char) loop)
            {
              MESSAGE (("LOOPBACK : FAILED\n"));
              return FALSE;
            }
        }
      MESSAGE (("LOOPBACK : OK\n"));
      return TRUE;
    }
  else if (strncmp (testname, "SIZE", 4) == 0)
    {
      if (perform_test ("LOOPBACK") == FALSE)
        return FALSE;
      cart_analyse (BANK0);
      cart_analyse (BANK1);
      return TRUE;
    }
  else if (strncmp (testname, "CART", 4) == 0)
    {
      unsigned int page, offset;
      unsigned char image1[CART_PAGE_128K];
      unsigned char image2[CART_PAGE_128K];
      int result = TRUE;

      if (perform_test ("LOOPBACK") == FALSE)
        return FALSE;
      cart_analyse (BANK0);

      for (page = 0; page < 256; page++)
        {
          MESSAGE (("Testing BANK0: Page <%03d> of <256>", page + 1));

          cart_read_page (BANK0, page, CART_PAGE_128K, image1);
          cart_read_page (BANK0, page, CART_PAGE_128K, image2);

          MESSAGE (("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"));

          for (offset = 0; offset < CART_PAGE_128K; offset++)
            {
              if (image1[offset] != image2[offset])
                {
                  MESSAGE (("CARTRIDGE: Bad read on PAGE %02x               \n", page));
                  result = FALSE;
                  break;
                }
            }
        }
      if (!result)
        {
          MESSAGE (("CARTRIDGE: FAILED                         \n"));
          return FALSE;
        }
      else
        {
          MESSAGE (("CARTRIDGE: OK                             \n"));
          return TRUE;
        }
    }
  else if (strcmp (testname, "INPORT") == 0)
    {
      do
        {
          MESSAGE (("INPORT   : Read %02x\n", INPUT (print_stat)));
        }
      while (!kbhit ());
      return TRUE;
    }
  else if (strcmp (testname, "PAGE") == 0)
    {
      do
        {
          lynxit_write_page (0xaa);
          MESSAGE (("PAGE     : Wrote 0xAA to page\n"));
          getch ();
          lynxit_write_page (0x55);
          MESSAGE (("PAGE     : Wrote 0x55 to page\n"));
        }
      while (getch () != 'q');
      return TRUE;
    }
  else if (strcmp (testname, "COUNTER") == 0)
    {
      int stepmode = TRUE;

      MESSAGE (("\nPress <space> key to step counter U5\n\n"));
      MESSAGE (("  'q' - Quit to DOS\n"));
      MESSAGE (("  's' - Step mode (default)\n"));
      MESSAGE (("  'r' - Run mode\n"));
      MESSAGE (("  'c' - Clear counter\n\n"));

      lynxit_counter_reset ();
      for (;;)
        {
          lynxit_counter_increment ();

          MESSAGE (("COUNTER  : increment\n"));

          if (kbhit () || stepmode)
            {
              switch (getch ())
                {
                case 'q':
                  return TRUE;
                case 's':
                  stepmode = TRUE;
                  break;
                case 'r':
                  stepmode = FALSE;
                  break;
                case 'c':
                  lynxit_counter_reset ();
                  MESSAGE (("COUNTER  : reset\n"));
                  break;
                default:
                  break;
                }
            }
        }
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}



//
//
// MAIN CODE
//
//

static char buf[FILENAME_MAX];


int
lynxit_main (int argc, char **argv)
{
  int loop;

  // Handle argument list
#if 0
  for (loop = 0; loop < argc; loop++)
    strupr (argv[loop]);
#endif

  for (loop = 1; loop < argc; loop++)
    {
      if (argv[loop][0] != '-')
        {
          break;
        }

      if (strlen (argv[loop]) > 3)
        {
//          usage ();
          printf ("\nInvalid argument %d <%s>\n", loop, argv[loop]);
          exit (FALSE);
        }

      switch (argv[loop][1])
        {
        case 'P':
          printer_port = (unsigned int) argv[loop][2] - '0';
          break;
        case 'D':
          debug = TRUE;
          break;
        case 'Q':
          quiet = FALSE;
          break;
        case 'N':
          verify = FALSE;
          break;
        case 'H':
//          usage ();
          exit (FALSE);
        default:
//          usage ();
          printf ("\nUnrecognised argument %d <%s>\n", loop, argv[loop]);
          exit (FALSE);
        }
    }

  // Check there are 2 spare arguments

  if (loop + 1 >= argc)
    {
//      usage ();
      MESSAGE (("\nERROR    : Missing command/filename\n"));
      exit (FALSE);
    }

  // Initialise the printer port

  if (!ptr_port_init (printer_port))
    {
      MESSAGE (("ERROR    : Couldn't initialise printer port LPT%d\n",
                printer_port));
      exit (FALSE);
    }

  // Perform loopback tests to prove operation

  if (!debug && strcmp (argv[loop], "TEST") != 0
      && !perform_test ("LOOPBACK"))
    {
      MESSAGE (("ERROR    : LYNXIT doesn't appear to be working ????\n"));
      MESSAGE (("ERROR    : (Check its plugged in and switched on)\n"));
      exit (FALSE);
    }

  if (strcmp (argv[loop], "READ") == 0)
    {
      if (loop + 3 >= argc)
        {
          MESSAGE (("ERROR    : Missing Cartname/Manufacturer arguments\n"));
          exit (FALSE);
        }
      strcpy (cartname, argv[argc - 2]);
      strcpy (manufname, argv[argc - 1]);
      if (strlen (cartname) > 32 || strlen (manufname) > 16)
        {
          MESSAGE (("ERROR    : Cartname/Manufacturer arguments too long (32/16)\n"));
          exit (FALSE);
        }

      strcpy (buf, argv[++loop]);
      if (cart_read (buf) == FALSE)
        {
          MESSAGE (("ERROR    : Cartridge read failed\n"));
          exit (FALSE);
        }
    }
  else if (strcmp (argv[loop], "WRITE") == 0)
    {
      strcpy (buf, argv[++loop]);
//      cart_write (buf);                       // warning remover
    }
  else if (strcmp (argv[loop], "VERIFY") == 0)
    {
      cart_verify (argv[++loop]);
    }
  else if (strcmp (argv[loop], "TEST") == 0)
    {
      perform_test (argv[++loop]);
    }
  else
    {
//      usage ();
      printf ("\nInvalid command argument - Use READ/WRITE/VERIFY/TEST\n");
      exit (FALSE);
    }

  exit (TRUE);
}


int
lynxit_read_rom (const char *filename, unsigned int parport)
{
  char *argv[128];

  print_data = parport;
  parport_print_info ();

  argv[0] = "ucon64";
  argv[1] = "READ";
  strcpy (buf, filename);
  argv[2] = buf;

  if (lynxit_main (3, argv) != 0)
    {
      fprintf (stderr, ucon64_msg[PARPORT_ERROR]);
      exit (1);
    }
  
  return 0;
}

#endif // USE_PARALLEL
