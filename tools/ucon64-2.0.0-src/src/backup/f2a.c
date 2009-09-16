/*
f2a.c - Flash 2 Advance support for uCON64

Copyright (c) 2003        Ulrich Hecht <uli@emulinks.de>
Copyright (c) 2003 - 2004 David Voswinkel <d.voswinkel@netcologne.de>
Copyright (c) 2004        NoisyB <noisyb@gmx.net>
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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#if     defined _WIN32 || defined __MSDOS__
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include "misc/bswap.h"
#include "misc/misc.h"
#include "misc/file.h"
#ifdef  USE_ZLIB
#include "misc/archive.h"
#endif
#include "misc/getopt2.h"                       // st_getopt2_t
#include "ucon64.h"
#include "ucon64_misc.h"
#include "f2a.h"
#include "console/gba.h"
#ifdef  USE_USB
#include <stdarg.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include "misc/usb.h"
#endif
#ifdef  USE_PARALLEL
#include "misc/parallel.h"
#endif
#include "misc/property.h"


const st_getopt2_t f2a_usage[] =
  {
    {
      NULL, 0, 0, 0,
      NULL, "Flash 2 Advance (Ultra)"/*"2003 Flash2Advance http://www.flash2advance.com"*/,
      NULL
    },
#if     defined USE_PARALLEL || defined USE_USB
    {
      "xf2a", 0, 0, UCON64_XF2A,
      NULL, "send/receive ROM to/from Flash 2 Advance (Ultra); " OPTION_LONG_S "port=PORT\n"
      "receives automatically (32 Mbits) when ROM does not exist",
      &ucon64_wf[WF_OBJ_GBA_DEFAULT_STOP_NO_ROM]
    },
    {
      "xf2amulti", 1, 0, UCON64_XF2AMULTI, // send only
      "SIZE", "send multiple ROMs to Flash 2 Advance (Ultra); specify a\n"
      "loader in the configuration file; " OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_GBA_DEFAULT_STOP]
    },
    {
      "xf2ac", 1, 0, UCON64_XF2AC,
      "N", "receive N Mbits of ROM from Flash 2 Advance (Ultra);\n" OPTION_LONG_S "port=PORT",
      &ucon64_wf[WF_OBJ_GBA_STOP_NO_ROM]
    },
    {
      "xf2as", 0, 0, UCON64_XF2AS,
      NULL, "send/receive SRAM to/from Flash 2 Advance (Ultra); " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_GBA_STOP_NO_ROM]
    },
    {
      "xf2ab", 1, 0, UCON64_XF2AB,
      "BANK", "send/receive SRAM to/from Flash 2 Advance (Ultra) BANK\n"
      "BANK should be a number >= 1; " OPTION_LONG_S "port=PORT\n"
      "receives automatically when SRAM does not exist",
      &ucon64_wf[WF_OBJ_GBA_STOP_NO_ROM]
    },
#endif
    {NULL, 0, 0, 0, NULL, NULL, NULL}
};


#ifdef  USE_USB

#define EZDEV             "/proc/ezusb/dev0"
#define F2A_FIRM_SIZE     23053
#define CMD_GETINF        0x05          // get info on the system status
#define CMD_MULTIBOOT1    0xff          // boot up the GBA stage 1, no parameters
#define CMD_MULTIBOOT2    0             // boot up the GBA stage 2, f2a_sendmsg_t.size has to be set

#define SENDMSG_SIZE      63            // (sizeof (f2a_sendmsg_t) - 1)

typedef struct
{
  unsigned int command;                 // command to execute, see below
  unsigned int size;                    // size of data block to read/write
  unsigned int pad1[2];
  unsigned int magic;                   // magic number, see below
  unsigned int pad2[3];
  unsigned int unknown;                 // no idea what this is for, seems to have to be 0xa for write, 0x7 for read
  unsigned int address;                 // base address for read/write
  unsigned int sizekb;                  // size of data block to read/write in kB
  unsigned char pad3[5 * 4 /*-1*/];
  /*
    For some reason the original software uses a 63 bytes structure for outgoing
    messages, not 64 as it does for incoming messages, hence the "-1". It all
    seems to work fine with 64 bytes, too, and I therefore suspect this to be a
    bug in the original software.
  */
  // we use SENDMSG_SIZE to solve the problem - dbjh
} /*__attribute__ ((packed))*/ f2a_sendmsg_t;

typedef struct
{
  unsigned char data[64];
} f2a_recvmsg_t;

static int f2a_init_usb (void);
static int f2a_connect_usb (void);
static int f2a_info (f2a_recvmsg_t *rm);
static int f2a_boot_usb (const char *ilclient_fname);
static int f2a_read_usb (int address, int size, const char *filename);
static int f2a_write_usb (int n_files, char **files, int address);

static usb_dev_handle *f2a_handle;
#endif // USE_USB


#ifdef  USE_PARALLEL

#define LOGO_ADDR         0x06000000
#define EXEC_STUB         0x03002000
#define ERASE_STUB        0x03000c00
#define LOGO_SIZE         76800
#define BOOT_SIZE         18432

#define FLIP              1
#define HEAD              1
#define EXEC              1

#define PP_CMD_WRITEROM   0x0a
#define PP_CMD_ERASE      0x0b
#define PP_HEAD_BOOT      0x01

typedef struct
{
  unsigned int pad1[3];
  unsigned int magic;
  unsigned int command;
  unsigned int address;
  unsigned int sizekb;
  unsigned int pad2;
  unsigned int exec;
  unsigned int exec_stub;
  unsigned char pad3[984];
} /*__attribute__ ((packed))*/ f2a_msg_cmd_t; // packed attribute is not necessary

static int f2a_boot_par (const char *ilclient2_fname, const char *illogo_fname);
static int f2a_write_par (int n_files, char **files, unsigned int address);
static int f2a_read_par (unsigned int start, unsigned int size,
                         const char *filename);
//static int f2a_erase_par (unsigned int start, unsigned int size);
static int f2a_send_buffer_par (int cmd, int address,
                                int size, const unsigned char *resource, int head,
                                int flip, unsigned int exec, int mode);
static int f2a_send_cmd_par (int cmd, int address, int size);
static int f2a_exec_cmd_par (int cmd, int address, int size);
static int f2a_receive_data_par (int cmd, int address, int size,
                                 const char *filename, int flip);
static int f2a_send_head_par (int cmd, int size);
static int f2a_send_raw_par (unsigned char *buffer, int len);
static int f2a_receive_raw_par (unsigned char *buffer, int len);
static int f2a_wait_par ();
static int parport_init (int port, int target_delay);
static int parport_init_delay (int target);
static void parport_out31 (unsigned char val);
static void parport_out91 (unsigned char val);
static void parport_nop ();

static int f2a_pport;
#ifdef  DEBUG
static int parport_debug = 1;
#else
static int parport_debug = 0;
#endif
static int parport_nop_cntr;
#endif // USE_PARALLEL


#if     defined USE_PARALLEL || defined USE_USB

#define LOADER_SIZE       32768

#define CMD_WRITEDATA     0x06          // write data to RAM/ROM(USB)/SRAM
#define CMD_READDATA      0x07          // read data from RAM/ROM(USB)/SRAM
#define MAGIC_NUMBER      0xa46e5b91    // needs to be properly set for almost all commands

enum
{
  UPLOAD_FAILED = 0,
  CANNOT_GET_FILE_SIZE,
  UPLOAD_FILE
};

static time_t starttime = 0;
static const char *f2a_msg[] = {
  "ERROR: Upload failed\n",
  "ERROR: Can't determine size of file \"%s\"\n",
  "Uploading %s, %d kB, padded to %d kB\n",
  NULL
};
#endif


#ifdef  USE_USB

int
exec (const char *program, int argc, ...)
/*
  This function is needed in order to execute a program with the same
  permissions as we have. In this way a suid root uCON64 executable can execute
  programs that require root privileges. We cannot use system(), because it
  might drop privileges.
  argc should be the number of _arguments_ (excluding the program name itself).
  DON'T move this function to misc.c. It's only necessary under Linux 2.5 (and
  later) for the USB version of the F2A. It's hard to be more system dependent
  than that. - dbjh
*/
{
  va_list argptr;
  int status;

  argc++;
  va_start (argptr, argc);
  if (fork () == 0)
    {
      int n, size;
      char *argv[argc + 1], *arg;

      size = strlen (program) + 1;
      if ((argv[0] = (char *) malloc (size)) == NULL)
        {
          fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
          exit (1);
        }
      strcpy (argv[0], program);

      for (n = 1; n < argc; n++)
        {
          arg = (char *) va_arg (argptr, char *); // get next argument
          size = strlen (arg) + 1;
          if ((argv[n] = (char *) malloc (size)) == NULL)
            {
              fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
              exit (1);
            }
          strcpy (argv[n], arg);
        }
      argv[n] = 0;

      setuid (geteuid ());
      setgid (getegid ());
      if (execv (argv[0], (char **) argv))
        { // error in info page: return value can be != 1
//          fprintf (stderr, "ERROR: %s\n", strerror (errno));
          exit (1);
        }
    }
  wait (&status);
  va_end (argptr);

  return status;
}


static int
f2a_init_usb (void)
{
  f2a_recvmsg_t rm;
  char iclientu_fname[FILENAME_MAX];

  if (sizeof (f2a_recvmsg_t) != 64)
    {
      fprintf (stderr, "ERROR: The size of f2a_recvmsg_t is not 64 bytes.\n"
                       "       Please correct the source code or send a bug report\n");
      exit (1);
    }
  if (sizeof (f2a_sendmsg_t) != 64)
    {
      fprintf (stderr, "ERROR: The size of f2a_sendmsg_t is not 64 bytes.\n"
                       "       Please correct the source code or send a bug report\n");
      exit (1);
    }

  memset (&rm, 0, sizeof (rm));

  if (f2a_connect_usb ())
    {
      fprintf (stderr, "ERROR: Could not connect to F2A USB linker\n");
      exit (1);                                 // fatal
    }
  f2a_info (&rm);
  if (rm.data[0] == 0)
    {
      get_property_fname (ucon64.configfile, "iclientu", iclientu_fname, "iclientu.bin");
      if (f2a_boot_usb (iclientu_fname))
        {
          fprintf (stderr, "ERROR: Booting GBA client binary was not successful\n");
          exit (1);                             // fatal
        }
      f2a_info (&rm);
    }
  return 0;
}


static int
f2a_connect_usb (void)
{
  int fp, result, firmware_loaded = 0;
  unsigned char f2afirmware[F2A_FIRM_SIZE];
  char f2afirmware_fname[FILENAME_MAX];
  struct usb_bus *bus;
  struct usb_device *dev, *f2adev = NULL;

  get_property_fname (ucon64.configfile, "f2afirmware", f2afirmware_fname, "f2afirm.hex");
  if (ucon64_fread (f2afirmware, 0, F2A_FIRM_SIZE, f2afirmware_fname) <= 0)
    {
      fprintf (stderr, "ERROR: Could not load F2A firmware (%s)\n", f2afirmware_fname);
      exit (1);                                 // fatal
    }

  usb_init ();
  usb_find_busses ();
  usb_find_devices ();
  for (bus = usb_busses; bus; bus = bus->next) // usb_busses is present in libusb
    {
      for (dev = bus->devices; dev; dev = dev->next)
        {
          if (dev->descriptor.idVendor == 0x547 && dev->descriptor.idProduct == 0x2131)
            {
              struct utsname info;
              int version;

              if (uname (&info) == -1)
                {
                  fputs ("ERROR: Could not determine version of the running kernel\n", stderr);
                  return -1;
                }

              version = strtol (&info.release[0], NULL, 10) * 10 +
                        strtol (&info.release[2], NULL, 10);
              // example contents of info.release: "2.4.18-14custom"
              if (version >= 25)                // Linux kernel 2.5 or later
                {
                  // use fxload to upload the F2A firmware
                  char device_path[160];
                  int exitstatus;

                  snprintf (device_path, 160, "/proc/bus/usb/%s/%s",
                            bus->dirname, dev->filename);
                  exitstatus = exec ("/sbin/fxload", 7, "-D", device_path, "-I",
                                     f2afirmware_fname, "-t", "an21",
                                     ucon64.quiet < 0 ? "-vv" : "-v");
                  if (WEXITSTATUS (exitstatus))
                    {
                      char cmd[10 * 80];

                      snprintf (cmd, 10 * 80, ucon64.quiet < 0 ?
                                  "/sbin/fxload -D %s -I %s -t an21 -vv" :
                                  "/sbin/fxload -D %s -I %s -t an21 -v",
                                device_path, f2afirmware_fname);
                      fprintf (stderr, "ERROR: Could not upload EZUSB firmware using fxload. Command:\n"
                                       "       %s\n", cmd);
                      return -1;
                    }
                }
              else
                {
                  int wrote, w;

                  // Linux kernel version 2.4 or older (2.2.16 is supported by
                  //  the EZUSB2131 driver). It is possible to use fxload under
                  //  (later versions of?) Linux 2.4...
                  if ((fp = open (EZDEV, O_WRONLY)) == -1)
                    {
                      fprintf (stderr, "ERROR: Could not upload EZUSB firmware (opening "
                                 EZDEV": %s)\n", strerror (errno));
                      return -1;
                    }

                  // The EZUSB2131 driver (version 1.0) only accepts one line of
                  //  an Intel hex record file at a time...
                  for (wrote = 0; wrote < F2A_FIRM_SIZE; wrote += w)
                    {
                      if ((w = write (fp, f2afirmware + wrote, F2A_FIRM_SIZE - wrote)) == -1)
                        {
                          fprintf (stderr, "ERROR: Could not upload EZUSB firmware (writing "
                                     EZDEV": %s)\n", strerror (errno));
                          return -1;
                        }
                      if (ucon64.quiet < 0)
                        printf ("Wrote %d bytes (%d-%d of %d) to "EZDEV"\n",
                                w, wrote, wrote + w, F2A_FIRM_SIZE);
                    }
                  close (fp);
                }
              firmware_loaded = 1;
              wait2 (2000);                     // give the EZUSB some time to renumerate
              break;
            }
        }
      if (firmware_loaded)
        break;
    }

  usb_find_devices ();
  for (bus = usb_busses; bus; bus = bus->next)
    {
      for (dev = bus->devices; dev; dev = dev->next)
        {
          if (dev->descriptor.idVendor == 0x547 && dev->descriptor.idProduct == 0x1002)
            {
              f2adev = dev;
              break;
            }
        }
      if (f2adev)
        break;
    }

  if (f2adev == NULL)
    {
      fprintf (stderr, "ERROR: Could not find F2A attached to USB\n");
      return -1;
    }

  f2a_handle = usbport_open (f2adev);

  result = usb_claim_interface (f2a_handle, 0x4);
  if (result == -1)
    {
      fprintf (stderr, "ERROR: Could not claim USB interface\n"
                       "       %s\n", usb_strerror ());
      return -1;
    }
  result = usb_claim_interface (f2a_handle, 0x83);
  if (result == -1)
    {
      fprintf (stderr, "ERROR: Could not claim USB interface\n"
                       "       %s\n", usb_strerror ());
      return -1;
    }

  return 0;
}


static int
f2a_info (f2a_recvmsg_t *rm)
{
  f2a_sendmsg_t sm;

  memset (&sm, 0, sizeof (f2a_sendmsg_t));
  memset (rm, 0, sizeof (f2a_recvmsg_t));

  sm.command = me2le_32 (CMD_GETINF);

  if (usbport_write (f2a_handle, (char *) &sm, SENDMSG_SIZE) == -1)
    {
      fprintf (stderr, "ERROR: Could not send info request\n");
      exit (1);
    }
  if (usbport_read (f2a_handle, (char *) rm, sizeof (f2a_recvmsg_t)) == -1)
    {
      fprintf (stderr, "ERROR: Did not receive info request\n");
      exit (1);
    }

#if 0
  {
    unsigned int i;
    for (i = 0; i < (sizeof (f2a_sendmsg_t) / 4); i++)
      printf ("%-2x %08X\n", i, *(((unsigned int *) (&sm)) + i));

    if (ucon64.quiet < 0)
      {
        printf ("info:");
        for (i = 0; i < (sizeof (f2a_sendmsg_t) / 4); i++)
          printf (" %08X", *(((unsigned int *) (rm)) + i));
        fputc ('\n', stdout);
      }
  }
#endif

  return 0;
}


static int
f2a_boot_usb (const char *ilclient_fname)
{
  f2a_sendmsg_t sm;
  unsigned int ack[16], i;
  char ilclient[16 * 1024];

  printf ("Booting GBA\n"
          "Uploading iLinker client\n"
          "Please turn OFF, then ON your GBA with SELECT and START held down\n");

  if (ucon64_fread (ilclient, 0, 16 * 1024, ilclient_fname) <= 0)
    {
      fprintf (stderr, "ERROR: Could not load GBA client binary (%s)\n", ilclient_fname);
      return -1;
    }

  // boot the GBA
  memset (&sm, 0, sizeof (f2a_sendmsg_t));
  sm.command = me2le_32 (CMD_MULTIBOOT1);
  usbport_write (f2a_handle, (char *) &sm, SENDMSG_SIZE);
  sm.command = me2le_32 (CMD_MULTIBOOT2);
  sm.size = me2le_32 (16 * 1024);
  usbport_write (f2a_handle, (char *) &sm, SENDMSG_SIZE);

  // send the multiboot image
  if (usbport_write (f2a_handle, ilclient, 16 * 1024) == -1)
    {
      fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
      return -1;
    }

  if (usbport_read (f2a_handle, (char *) ack, 16 * 4) == -1)
    return -1;

  if (ucon64.quiet < 0)
    {
      printf ("post-boot:");
      for (i = 0; i < 16; i++)
        printf (" %08X", ack[i]);
      fputc ('\n', stdout);
    }

  return 0;
}


static int
f2a_read_usb (int address, int size, const char *filename)
{
  FILE *file;
  int i;
  f2a_sendmsg_t sm;
  char buffer[1024];

  memset (&sm, 0, sizeof (f2a_sendmsg_t));

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      //exit (1); for now, return, although registering usbport_close() is better
      return -1;
    }

  sm.command = me2le_32 (CMD_READDATA);
  sm.magic = me2le_32 (MAGIC_NUMBER);
  sm.unknown = me2le_32 (7);
  sm.address = me2le_32 (address);
  sm.size = me2le_32 (size);
  sm.sizekb = me2le_32 (size / 1024);
  if (usbport_write (f2a_handle, (char *) &sm, SENDMSG_SIZE) == -1)
    return -1;

  for (i = 0; i < size; i += 1024)
    {
      if (usbport_read (f2a_handle, buffer, 1024) == -1)
        {
          fclose (file);
          return -1;
        }
      if (!fwrite (buffer, 1024, 1, file))      // note order of arguments
        {
          fprintf (stderr, ucon64_msg[WRITE_ERROR], filename);
          fclose (file);
          return -1;                            // see comment for fopen() call
        }
      ucon64_gauge (starttime, i + 1024, size);
    }
  fclose (file);
  return 0;
}


static int
f2a_write_usb (int n_files, char **files, int address)
{
  f2a_sendmsg_t sm;
  int i, j, fsize, size, n, is_sram_data = address >= 0xe000000 ? 1 : 0;
  char buffer[1024], loader_fname[FILENAME_MAX];
  unsigned char loader[LOADER_SIZE];
  FILE *file;

  // initialize command buffer
  memset (&sm, 0, sizeof (f2a_sendmsg_t));
  sm.command = me2le_32 (CMD_WRITEDATA);
  sm.magic = me2le_32 (MAGIC_NUMBER);
  sm.unknown = me2le_32 (is_sram_data ? 0x06 : 0x0a); // SRAM => 0x06, ROM => 0x0a

  if (n_files > 1 && !is_sram_data)
    {
      printf ("Uploading multiloader\n");
      get_property_fname (ucon64.configfile, "gbaloader", loader_fname, "loader.bin");
      if (ucon64_fread (loader, 0, LOADER_SIZE, loader_fname) <= 0)
        {
          fprintf (stderr, "ERROR: Could not load loader binary (%s)\n", loader_fname);
          return -1;
        }
#if 0 // just use a correct loader file - dbjh
      ((int *) loader)[0] = me2be_32 (0x2e0000ea); // start address
#endif
      memcpy (loader + 4, gba_logodata, GBA_LOGODATA_LEN); // + 4 for start address

      sm.size = me2le_32 (LOADER_SIZE);
      sm.address = me2le_32 (address);
      sm.sizekb = me2le_32 (LOADER_SIZE / 1024);

      if (usbport_write (f2a_handle, (char *) &sm, SENDMSG_SIZE) == -1)
        return -1;

      if (usbport_write (f2a_handle, (char *) loader, LOADER_SIZE) == -1)
        {
          fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
          return -1;
        }
      address += LOADER_SIZE;
    }
  for (j = 0; j < n_files; j++)
    {
      if ((fsize = fsizeof (files[j])) == -1)
        {
          fprintf (stderr, f2a_msg[CANNOT_GET_FILE_SIZE], files[j]);
          return -1;
        }
      // Round up to 32 kB. FIXME: This has to be 128 kB for Turbo carts
      size = fsize;
      if (size & (32768 - 1))
        size += 32768;
      size &= ~(32768 - 1);
      printf (f2a_msg[UPLOAD_FILE], files[j], fsize / 1024, size / 1024);

      if ((file = fopen (files[j], "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], files[j]);
          //exit (1); for now, return, although registering usbport_close() is better
          return -1;
        }
      clearerr (file);

      sm.size = me2le_32 (size);
      sm.address = me2le_32 (address);
      sm.sizekb = me2le_32 (size / 1024);

      if (usbport_write (f2a_handle, (char *) &sm, SENDMSG_SIZE) == -1)
        return -1;

      for (i = 0; i < size; i += 1024)
        {
          //printf ("writing chunk %d\n", i);
          n = fread (buffer, 1, 1024, file);
          memset (buffer + n, 0, 1024 - n);
          if (ferror (file))
            {
              fputc ('\n', stderr);
              fprintf (stderr, ucon64_msg[READ_ERROR], files[j]);
              fclose (file);
              return -1;                        // see comment for fopen() call
            }
          if (usbport_write (f2a_handle, buffer, 1024) == -1)
            return -1;
          ucon64_gauge (starttime, i + 1024, size);
        }
      fputc ('\n', stdout);                     // start new gauge on new line

      fclose (file);
      address += fsize;
    }

  return 0;
}

#endif // USE_USB


#ifdef  USE_PARALLEL

static int
f2a_init_par (int parport, int parport_delay)
{
  char iclientp_fname[FILENAME_MAX], ilogo_fname[FILENAME_MAX];

  if (parport_init (parport, parport_delay))
    {
      fprintf (stderr, "ERROR: Could not connect to F2A parport linker\n");
      exit (1);                                 // fatal
    }

  get_property_fname (ucon64.configfile, "iclientp", iclientp_fname, "iclientp.bin");
  get_property_fname (ucon64.configfile, "ilogo", ilogo_fname, ""); // "ilogo.bin"
  if (f2a_boot_par (iclientp_fname, ilogo_fname))
    {
      fprintf (stderr, "ERROR: Booting GBA client binary was not successful\n");
      exit (1);                                 // fatal
    }
  return 0;
}


int
parport_init (int port, int target_delay)
{
  f2a_pport = port;
  parport_nop_cntr = parport_init_delay (target_delay);

  parport_print_info ();

#ifndef USE_PPDEV
  outportb ((unsigned short) (f2a_pport + PARPORT_STATUS), 0x01); // clear EPP time flag
#endif

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x04);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_DATA), 0x04);

  parport_out91 (0x47);
  parport_out31 (0x02);
  parport_out91 (0x12);
  parport_out31 (0x01);
  parport_out91 (0x34);
  parport_out31 (0x00);
  parport_out91 (0x56);

  // not parport_out31 (0x02), because extra write to control register
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x03);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_DATA), 0x02);

  // not parport_out91 (0x00), because no write to data register
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x09);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x00);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x00);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x02);
  inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x06);
  inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x00);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x04);
  outportb ((unsigned short) (f2a_pport + PARPORT_EDATA), 0x07);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x02);
  outportb ((unsigned short) (f2a_pport + PARPORT_EDATA), 0x12);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EDATA), 0x34);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x00);
  outportb ((unsigned short) (f2a_pport + PARPORT_EDATA), 0x56);

  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_EADDRESS), 0x02);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x00);
  inportb ((unsigned short) (f2a_pport + PARPORT_EDATA));

  return 0;
}


static int
parport_init_delay (int n_micros)
/*
  We only have millisecond accuracy on DOS, while we have to determine the
  correct initial loop counter value for a number of microseconds. Luckily, the
  function of time against the initial loop counter value is linear (provided
  that the initial counter value is large enough), so we can just divide the
  found loop value by 1000. Of course, in reality we don't get millisecond
  accuracy...
  TODO: Find the equivalent of gettimeofday() for MinGW and Visual C++
*/
{
#define N_CHECKS  10
#define N_HITSMAX 10
#if     defined _WIN32 || defined __MSDOS__
  struct timeb t0, t1;
#else
  struct timeval t0, t1;
#endif
  int n_ticks = 0, n, n_hits = 0, loop = 10000, loop_sum = 0;
  volatile int m;                               // volatile is necessary for Visual C++...

  printf ("Determining delay loop value for %d microseconds...", n_micros);
  fflush (stdout);
  while (n_hits < N_HITSMAX)
    {
      n_ticks = 0;
      for (n = 0; n < N_CHECKS; n++)
        {
          m = loop;
#if     defined _WIN32 || defined __MSDOS__
          ftime (&t0);
#else
          gettimeofday (&t0, NULL);
#endif
          while (m--)
            ;
#if     defined _WIN32 || defined __MSDOS__
          ftime (&t1);
          n_ticks += (t1.time * 1000 + t1.millitm) - (t0.time * 1000 + t0.millitm);
#else
          gettimeofday (&t1, NULL);
          n_ticks += (t1.tv_sec * 1000000 + t1.tv_usec) - (t0.tv_sec * 1000000 + t0.tv_usec);
#endif
        }
      n_ticks /= N_CHECKS;

#ifndef DJGPP
      if (n_ticks - n_micros == 0)              // we are aiming at microsecond accuracy...
#else // DJGPP's runtime system is quite inaccurate under Windows XP
      n = n_ticks - n_micros;
      if (n < 0)
        n = -n;
      if (n <= 1)                               // allow a deviation of 1 ms?
#endif
        {
          n_hits++;
          loop_sum += loop;
          loop -= loop >> 3;                    // force "variation" in hope of better accuracy
          continue;
        }

      if (n_ticks == 0)
        loop <<= 1;
      else
        loop = (int) (n_micros / ((float) n_ticks / loop));
    }

#if     defined _WIN32 || defined __MSDOS__
  n = loop_sum / (1000 * N_HITSMAX);            // divide by 1000
#else
  n = loop_sum / N_HITSMAX;                     // we summed N_HITSMAX loop values
#endif
  printf ("done (%d)\n", n);
  return n;
}


static void
parport_nop ()
{
  volatile int i = parport_nop_cntr;            // volatile is necessary for Visual C++...
  while (i--)
    ;
}


static void
parport_out31 (unsigned char val)
{
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x03);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_DATA), val);
}


static void
parport_out91 (unsigned char val)
{
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x09);
  outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x01);
  outportb ((unsigned short) (f2a_pport + PARPORT_DATA), val);
}


int
f2a_boot_par (const char *iclientp_fname, const char *ilogo_fname)
{
  unsigned char recv[4], iclientp[BOOT_SIZE];

  printf ("Booting GBA\n"
          "Please turn OFF, then ON your GBA with SELECT and START held down\n");

  if (f2a_send_head_par (PP_HEAD_BOOT, 1))
    return -1;
  if (f2a_receive_raw_par (recv, 4))
    return -1;

  if (ilogo_fname[0] != 0)
    {
      unsigned char ilogo[LOGO_SIZE];

      printf ("Uploading iLinker logo\n");
      if (ucon64_fread (ilogo, 0, LOGO_SIZE, ilogo_fname) <= 0)
        {
          fprintf (stderr, "ERROR: Could not load logo file (%s)\n", ilogo_fname);
          return -1;
        }
      if (f2a_send_buffer_par (CMD_WRITEDATA, LOGO_ADDR, LOGO_SIZE, ilogo,
                               0, 0, 0, 0))
        {
          fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
          return -1;
        }
    }

  printf ("Uploading iLinker client\n");
  if (ucon64_fread (iclientp, 0, BOOT_SIZE, iclientp_fname) <= 0)
    {
      fprintf (stderr, "ERROR: Could not load GBA client binary (%s)\n", iclientp_fname);
      return -1;
    }
  if (f2a_send_buffer_par (CMD_WRITEDATA, EXEC_STUB, BOOT_SIZE, iclientp,
                           HEAD, FLIP, EXEC, 0))
    {
      fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
      return -1;
    }
  return 0;
}


int
f2a_write_par (int n_files, char **files, unsigned int address)
{
  int j, fsize, size, is_sram_data = address >= 0xe000000 ? 1 : 0;
  char loader_fname[FILENAME_MAX];
  unsigned char loader[LOADER_SIZE];

  if (n_files > 1 && !is_sram_data)
    {
      printf ("Uploading multiloader\n");
      get_property_fname (ucon64.configfile, "gbaloader", loader_fname, "loader.bin");
      if (ucon64_fread (loader, 0, LOADER_SIZE, loader_fname) <= 0)
        {
          fprintf (stderr, "ERROR: Could not load loader binary (%s)\n", loader_fname);
          return -1;
        }
#if 0 // just use a correct loader file - dbjh
      ((int *) loader)[0] = me2le_32 (0x2e0000ea); // start address
#endif
      if (f2a_send_buffer_par (PP_CMD_WRITEROM, address, LOADER_SIZE, loader,
                               HEAD, FLIP, 0, 0))
        {
          fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
          return -1;
        }
      address += LOADER_SIZE;
    }
  for (j = 0; j < n_files; j++)
    {
      if ((fsize = fsizeof (files[j])) == -1)
        {
          fprintf (stderr, f2a_msg[CANNOT_GET_FILE_SIZE], files[j]);
          return -1;
        }
      size = fsize;
      if (size & (32768 - 1))
        size += 32768;
      size &= ~(32768 - 1);
      printf (f2a_msg[UPLOAD_FILE], files[j], fsize / 1024, size / 1024);
      if (f2a_send_buffer_par (PP_CMD_WRITEROM, address, size,
                               (unsigned char *) files[j], HEAD, FLIP, 0, 1))
        {
          fprintf (stderr, f2a_msg[UPLOAD_FAILED]);
          return -1;
        }

      address += size;
    }
  return 0;
}


int
f2a_erase_par (unsigned int start, unsigned int size)
{
  int end, address;

  f2a_exec_cmd_par (CMD_READDATA, ERASE_STUB, 1024);
  end = start + (size);

  printf ("Erase cart start=0x%08x end=0x%08x\n", start, end);
  for (address = start; address < end; address += 0x40000)
    f2a_send_cmd_par (PP_CMD_ERASE, address, 1024);
  return 0;
}


int
f2a_read_par (unsigned int start, unsigned int size, const char *filename)
{
  f2a_exec_cmd_par (CMD_READDATA, ERASE_STUB, 1024);
  printf ("Reading from cart start=0x%08x size=0x%08x\n", start, size);
  f2a_receive_data_par (CMD_READDATA, start, size, filename, FLIP);
  return 0;
}


#if 0
typedef struct
{
  unsigned char header[16];
  unsigned char command;
  unsigned char unknown;
  unsigned int size;
  unsigned char pad[58];
} __attribute__ ((packed)) f2a_msg_head_t;
#endif

static int
f2a_send_head_par (int cmd, int size)
{
  unsigned char trans[] = { 0xa, 0x8, 0xe, 0xc, 0x2, 0x0, 0x6, 0x4 },
                msg_header[80] = { 0x49, 0x2d, 0x4c, 0x69, 0x6e, 0x6b, 0x65, 0x72,
                                   0x2e, 0x31, 0x30, 0x30, 0x00, 0x00, 0x01, 0xe8 };
//  f2a_msg_head_t msg_header;                  // Don't use structs with misaligned
  unsigned short int s;                         //  members for data streams (we don't
                                                //  want compiler-specific stuff)
//  memcpy (&msg_head, header, 16);             // .head
  msg_header[16] = cmd;                         // .command
  s = size / 1024;
  msg_header[17] =                              // .unknown
    (trans[((s & 255) / 32)] << 4) | (((1023 - (s & 1023)) / 256) & 0x0f);
// msg_header.unknown = 0x82;
  msg_header[18] = (unsigned char) s;           // .size
  msg_header[19] = (unsigned char) (s >> 8);
  memset (&msg_header[20], 0, 80 - 20);

  if (f2a_send_raw_par (msg_header, 80))
    return -1;
  return 0;
}


static int
f2a_exec_cmd_par (int cmd, int address, int size)
{
  unsigned char *buffer;
  f2a_msg_cmd_t msg_cmd;

  memset (&msg_cmd, 0, sizeof (f2a_msg_cmd_t));
  msg_cmd.magic = me2be_32 (MAGIC_NUMBER);
  msg_cmd.command = me2be_32 (cmd);
  msg_cmd.address = me2be_32 (address);
  msg_cmd.sizekb = me2be_32 (size / 1024);

  msg_cmd.exec_stub = me2be_32 (EXEC_STUB);
  msg_cmd.exec = me2be_32 (0x08);
  f2a_send_head_par (CMD_READDATA, size);
  f2a_wait_par ();

  if (parport_debug)
    fprintf (stderr,
             "sending msg_cmd cmd='0x%08x' address='0x%08x' size='0x%08x' %d bytes\n",
             msg_cmd.command, msg_cmd.address, msg_cmd.sizekb,
             (int) sizeof (f2a_msg_cmd_t));


  f2a_send_raw_par ((unsigned char *) &msg_cmd, sizeof (f2a_msg_cmd_t));
//  f2a_wait_par ();
  if ((buffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      exit (1);                                 // not return, caller doesn't handle it
    }
  f2a_receive_raw_par (buffer, size);
  free (buffer);

  return 0;
}


static int
f2a_receive_data_par (int cmd, int address, int size, const char *filename, int flip)
{
  unsigned char buffer[1024], recv[4]; //, *mbuffer;
  int i, j;
  f2a_msg_cmd_t msg_cmd;
  FILE *file;

  memset (&msg_cmd, 0, sizeof (f2a_msg_cmd_t));
  msg_cmd.magic = me2be_32 (MAGIC_NUMBER);
  msg_cmd.command = me2be_32 (cmd);
  msg_cmd.address = me2be_32 (address);
  msg_cmd.sizekb = me2be_32 (size / 1024);

  if (f2a_send_head_par (CMD_READDATA, size))
    return -1;

  if (f2a_receive_raw_par (recv, 4))
    return -1;

  if (parport_debug)
    fprintf (stderr,
             "sending msg_cmd cmd='0x%08x' address='0x%08x' size='0x%08x' %d bytes\n",
             msg_cmd.command, msg_cmd.address, msg_cmd.sizekb,
             (int) sizeof (f2a_msg_cmd_t));

  f2a_send_raw_par ((unsigned char *) &msg_cmd, sizeof (f2a_msg_cmd_t));

  if ((file = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, ucon64_msg[OPEN_WRITE_ERROR], filename);
      //exit (1); return, because the other code does it too...
      return -1;
    }

#if 0
  if ((mbuffer = (unsigned char *) malloc (size)) == NULL)
    {
      fprintf (stderr, ucon64_msg[BUFFER_ERROR], size);
      //exit (1); see comment for fopen() call
      return -1;
    }
  f2a_receive_raw_par (mbuffer, size);
  if (flip)
    for (j = 0; j < size / 4; j++)
      ((int *) mbuffer)[j] = bswap_32 (((int *) mbuffer)[j]);

  if (!fwrite (mbuffer, size, 1, file))         // note order of arguments
    {
      fprintf (stderr, ucon64_msg[WRITE_ERROR], filename);
      fclose (file);
      free (mbuffer);
      return -1;                                // see comment for fopen() call
    }
  free (mbuffer);
#else
  for (i = 0; i < size; i += 1024)
    {
      f2a_receive_raw_par (buffer, 1024);
      if (flip)
        for (j = 0; j < 256; j++)
          ((int *) buffer)[j] = bswap_32 (((int *) buffer)[j]);

      if (!fwrite (buffer, 1024, 1, file))      // note order of arguments
        {
          fprintf (stderr, ucon64_msg[WRITE_ERROR], filename);
          fclose (file);
          return -1;                            // see comment for fopen() call
        }

      if (parport_debug)
        fprintf (stderr, "reading chunk %d of %d\n", (int) (i / 1024) + 1,
                 (int) (size / 1024));
      else
        ucon64_gauge (starttime, i + 1024, size);
    }
  if (!parport_debug)
    fputc ('\n', stdout);
  fclose (file);
#endif

  return 0;
}


static int
f2a_send_cmd_par (int cmd, int address, int size)
{
  unsigned char recv[4];
  f2a_msg_cmd_t msg_cmd;

  memset (&msg_cmd, 0, sizeof (f2a_msg_cmd_t));
  msg_cmd.magic = me2be_32 (MAGIC_NUMBER);
  msg_cmd.command = me2be_32 (cmd);
  msg_cmd.address = me2be_32 (address);
  msg_cmd.sizekb = me2be_32 (size / 1024);

  if (f2a_send_head_par (CMD_WRITEDATA, size))
    return -1;

  if (f2a_receive_raw_par (recv, 4))
    return -1;

  if (parport_debug)
    fprintf (stderr,
             "parport_send_cmd cmd='0x%08x' address='0x%08x' size='0x%08x' %d bytes\n",
             msg_cmd.command, msg_cmd.address, msg_cmd.sizekb,
             (int) sizeof (f2a_msg_cmd_t));

  if (f2a_send_raw_par ((unsigned char *) &msg_cmd, sizeof (f2a_msg_cmd_t)))
    return -1;
  return 0;
}


static int
f2a_send_buffer_par (int cmd, int address, int size, const unsigned char *resource,
                     int head, int flip, unsigned int exec, int mode)
{
  unsigned char recv[4], buffer[1024];
  int i, j;
  f2a_msg_cmd_t msg_cmd;
  FILE *file = NULL;

  memset (&msg_cmd, 0, sizeof (f2a_msg_cmd_t));
  msg_cmd.magic = me2be_32 (MAGIC_NUMBER);
  msg_cmd.command = me2be_32 (cmd);
  msg_cmd.address = me2be_32 (address);
  msg_cmd.sizekb = me2be_32 (size / 1024);
  if (exec)
    {
      msg_cmd.exec_stub = me2be_32 (EXEC_STUB);
      msg_cmd.exec = me2be_32 (0x08);
    }
  if (f2a_send_head_par (CMD_WRITEDATA, size))
    return -1;
  if (f2a_receive_raw_par (recv, 4))
    return -1;

  if (parport_debug)
    fprintf (stderr,
             "parport_send_buffer cmd='0x%08x' address='0x%08x' size='0x%08x' %d bytes\n",
             msg_cmd.command, msg_cmd.address, msg_cmd.sizekb,
             (int) sizeof (f2a_msg_cmd_t));

  if (f2a_send_raw_par ((unsigned char *) &msg_cmd, sizeof (f2a_msg_cmd_t)))
    return -1;

  if (mode == 1)
    {
      if ((file = fopen ((char *) resource, "rb")) == NULL)
        {
          fprintf (stderr, ucon64_msg[OPEN_READ_ERROR], (char *) resource);
          //exit (1); return, because the other code does it too...
          return -1;
        }
      clearerr (file);
    }

  for (i = 0; i < size; i += 1024)
    {
      if (mode == 1)
        {
          j = fread (buffer, 1, 1024, file);
          memset (buffer + j, 0, 1024 - j);
          if (ferror (file))
            {
              fputc ('\n', stderr);
              fprintf (stderr, ucon64_msg[READ_ERROR], (char *) resource);
              fclose (file);
              return -1;
            }
        }
      else
        memcpy (buffer, resource, 1024);

      if (flip)
        for (j = 0; j < 256; j++)
          ((int *) buffer)[j] = bswap_32 (((int *) buffer)[j]);

      if (!i && head)
        for (j = 1; j < GBA_LOGODATA_LEN / 4 + 1; j++) // + 1 for start address
          ((int *) buffer)[j] = bswap_32 (((int *) gba_logodata)[j - 1]);

      if (parport_debug)
        fprintf (stderr, "sending chunk %d of %d\n", (int) (i / 1024) + 1,
                 (int) (size / 1024));
      else
        ucon64_gauge (starttime, i + 1024, size);
      f2a_send_raw_par (buffer, 1024);
      if (mode == 0)
        resource += 1024;
    }
  if (!parport_debug)
    fputc ('\n', stdout);                       // start new gauge on new line

  if (mode == 1)
    fclose (file);

  return 0;
}


#ifdef  DEBUG
static void
parport_dump_byte (unsigned char byte)
{
  char i;

  for (i = 7; i >= 0; i--)
    {
      if ((byte >> i) & 1)
        fprintf (stderr, "1");
      else
        fprintf (stderr, "0");
    }
  fputc ('\n', stderr);
}
#endif


static int
f2a_receive_raw_par (unsigned char *buffer, int len)
{
  int err, i;
  unsigned char *ptr, nibble;

  ptr = buffer;
  if (parport_debug)
    fprintf (stderr, "\nreceive:\n%04x: ", 0);

  *ptr = 0;
  for (err = 0, i = 0; i < len * 2; i++)
    {
      nibble = 0;
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x04);
      parport_nop ();
      while (inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY)
        ;
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x05);
      nibble = inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
      while (!(inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY))
        ;
      if (i % 2)
        {
          *ptr |= (nibble >> 3) & 0x0f;
          if (parport_debug)
            {
              fprintf (stderr, "%02x ", (unsigned char) *ptr);
              if (!(((i / 2) + 1) % 32) && i && (i / 2) < len - 1)
                fprintf (stderr, "\n%04x: ", (i / 2) + 1);
            }
          *ptr = 0;
          ptr++;
        }
      else
        *ptr |= ((nibble >> 3) & 0xf) << 4;
    }
  if (parport_debug)
    fputc ('\n', stderr);

  return err;
}


static int
f2a_send_raw_par (unsigned char *buffer, int len)
{
  int timeout, i;
  unsigned char *pc;

  pc = buffer;
  if (parport_debug)
    fprintf (stderr, "\nsend:\n%04x: ", 0);
  for (i = 0; i < len; i++)
    {
      timeout = 2000;
      if (parport_debug)
        {
          fprintf (stderr, "%02x ", (unsigned char) *pc);
          if (!((i + 1) % 32) && i && i < len - 1)
            fprintf (stderr, "\n%04x: ", i + 1);
        }
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x04);
      parport_nop ();
      while ((inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY) &&
             (timeout--) > 0)
        wait2 (1);
      outportb ((unsigned short) (f2a_pport + PARPORT_DATA), *pc);
      parport_nop ();
      while ((inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY) &&
             (timeout--) > 0)
        wait2 (1);
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x05);
      parport_nop ();
      while ((!(inportb ((unsigned short) (f2a_pport + PARPORT_STATUS)) & PARPORT_IBUSY)) &&
             (timeout--) > 0)
        wait2 (1);
      pc++;
      if (timeout < 0)
        {
          fprintf (stderr, "\nERROR: Time-out\n");
          return -1;
        }
    }
  if (parport_debug)
    fputc ('\n', stderr);

  return 0;
}


static int
f2a_wait_par (void)
{
  int stat;

  while (1)
    {
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x04);
      parport_nop ();
      stat = inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
      if (stat & PARPORT_IBUSY)
        break;
      outportb ((unsigned short) (f2a_pport + PARPORT_CONTROL), 0x05);
      parport_nop ();
      inportb ((unsigned short) (f2a_pport + PARPORT_STATUS));
    }
  return 0;
}
#endif // USE_PARALLEL


#if     defined USE_PARALLEL || defined USE_USB
int
f2a_read_rom (const char *filename, int size)
{
  int offset = 0;

  starttime = time (NULL);
#ifdef  USE_USB
  if (ucon64.usbport)
    {
      f2a_init_usb ();
      f2a_read_usb (0x8000000 + offset * MBIT, size * MBIT, filename);
      usbport_close (f2a_handle);
    }
#endif
#if     defined USE_PARALLEL && defined USE_USB
  else
#endif
#ifdef  USE_PARALLEL
    {
      f2a_init_par (ucon64.parport, 10);
      f2a_read_par (0x08000000 + offset * MBIT, size * MBIT, filename);
    }
#endif
  return 0;
}


int
f2a_write_rom (const char *filename, int size)
{
  int offset = 0, n, n_files, n_files_max = 0, fsize, totalsize = LOADER_SIZE;
  char **files = NULL, *file_mem[1];
  struct stat fstate;

  if (filename)                                 // -xf2a
    {
      files = file_mem;
      files[0] = (char *) filename;
      n_files = 1;
    }
  else                                          // -xf2amulti=SIZE
    {
      n_files = 0;
      for (n = 1; n < ucon64.argc; n++)
        {
          if (access (ucon64.argv[n], F_OK))
            continue;                           // "file" does not exist (option)
          stat (ucon64.argv[n], &fstate);
          if (!S_ISREG (fstate.st_mode))
            continue;

          if (n_files == n_files_max)
            {
              n_files_max += 20;                // allocate mem for 20 extra pointers
              if ((files = (char **) realloc (files, n_files_max * 4)) == NULL)
                {
                  fprintf (stderr, ucon64_msg[BUFFER_ERROR], n_files_max * 4);
                  exit (1);
                }
            }

          fsize = fsizeof (ucon64.argv[n]);
          if (totalsize + fsize > size)
            {
              printf ("WARNING: The sum of the sizes of the files is larger than the specified flash\n"
                      "         card size (%d Mbit). Skipping files, starting with\n"
                      "         %s\n",
                      size / MBIT, ucon64.argv[n]);
              break;
            }
          totalsize += fsize;

          files[n_files] = ucon64.argv[n];
          n_files++;
        }
      if (n_files == 0)
        return -1;
    }

  starttime = time (NULL);
#ifdef  USE_USB
  if (ucon64.usbport)
    {
      f2a_init_usb ();
      f2a_write_usb (n_files, files, 0x8000000 + offset * MBIT);
      usbport_close (f2a_handle);
    }
#endif
#if     defined USE_PARALLEL && defined USE_USB
  else
#endif
#ifdef  USE_PARALLEL
    {
      f2a_init_par (ucon64.parport, 10);
      //f2a_erase_par (0x08000000, size * MBIT);
      f2a_write_par (n_files, files, 0x8000000 + offset * MBIT);
    }
#endif

  if (!filename)
    free (files);

  return 0;
}


int
f2a_read_sram (const char *filename, int bank)
{
  int size;

  if (bank == UCON64_UNKNOWN)
    {
      bank = 1;
      size = 256 * 1024;
    }
  else
    {
      if (bank < 1)
        {
          fprintf (stderr, "ERROR: Bank must be a number larger than or equal to 1\n");
          exit (1);
        }
      size = 64 * 1024;
    }
  bank--;

  starttime = time (NULL);
#ifdef  USE_USB
  if (ucon64.usbport)
    {
      f2a_init_usb ();
      f2a_read_usb (0xe000000 + bank * 64 * 1024, size, filename);
      usbport_close (f2a_handle);
    }
#endif
#if     defined USE_PARALLEL && defined USE_USB
  else
#endif
#ifdef  USE_PARALLEL
    {
      f2a_init_par (ucon64.parport, 10);
      f2a_read_par (0xe000000 + bank * 64 * 1024, size, filename);
    }
#endif
  return 0;
}


int
f2a_write_sram (const char *filename, int bank)
{
  char *files[1] = { (char *) filename };

  // define one bank as a 64 kilobyte unit
  if (bank == UCON64_UNKNOWN)
    bank = 1;
  else
    if (bank < 1)
      {
        fprintf (stderr, "ERROR: Bank must be a number larger than or equal to 1\n");
        exit (1);
      }
  bank--;

  starttime = time (NULL);
#ifdef  USE_USB
  if (ucon64.usbport)
    {
      f2a_init_usb ();
      f2a_write_usb (1, files, 0xe000000 + bank * 64 * 1024);
      usbport_close (f2a_handle);
    }
#endif
#if     defined USE_PARALLEL && defined USE_USB
  else
#endif
#ifdef  USE_PARALLEL
    {
      f2a_init_par (ucon64.parport, 10);
      //f2a_erase_par (0xe000000, size * MBIT);
      f2a_write_par (1, files, 0xe000000 + bank * 64 * 1024);
    }
#endif
  return 0;
}

#endif // defined USE_PARALLEL || defined USE_USB
