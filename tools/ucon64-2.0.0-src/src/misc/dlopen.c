/*
dlopen.c - DLL support code

Copyright (c) 2002 - 2004 dbjh


This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  DJGPP
#include <sys/dxe.h>
#elif   defined __unix__ || defined __APPLE__   // Mac OS X actually; __unix__ is also
#include <dlfcn.h>                              //  defined under Cygwin (and DJGPP)
#elif   defined _WIN32
#include <windows.h>
#elif   defined __BEOS__
#include <image.h>
#include <Errors.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef  DJGPP
#include "dxedll_pub.h"
#include "map.h"


#define INITIAL_HANDLE 1
static st_map_t *dxe_map;
extern int errno;


void
uninit_func (void)
{
  fprintf (stderr, "An uninitialized member of the import/export structure was called!\n"
                   "Update dlopen.c/open_module()\n");
  exit (1);
}
#endif


void *
open_module (char *module_name)
{
  void *handle;
#ifdef  DJGPP
  static int new_handle = INITIAL_HANDLE;
  int n, m;
  st_symbol_t *sym = _dxe_load (module_name);
  /*
    _dxe_load() doesn't really return a handle. It returns a pointer to the one
    symbol a DXE module can export.
  */
  if (sym == 0)
    {
      fprintf (stderr, "Error while loading DXE module: %s\n", module_name);
      exit (1);
    }

  if (sym->size != sizeof (st_symbol_t))
    {
      fprintf (stderr, "Incompatible DXE module: %s\n", module_name);
      exit (1);
    }

  // initialize the import/export structure

  /*
    Catch calls to uninitialized members in case a new function was added to
    st_symbol_t, but forgotten to initialize here.
  */
  m = sizeof (st_symbol_t) / sizeof (void (*) (void));
  for (n = 0; n < m && ((void (**) (void)) sym)[n] != 0; n++) // Don't overwrite values initialized by DXE
    ;
  for (; n < m; n++)
    ((void (**) (void)) sym)[n] = uninit_func;

  // initialize functions
  sym->printf = printf;
  sym->fprintf = fprintf;
  sym->vfprintf = vfprintf;
  sym->sprintf = sprintf;
  sym->vsprintf = vsprintf;
  sym->puts = puts;
  sym->fputs = fputs;
  sym->sscanf = sscanf;
  sym->vsscanf = vsscanf;
  sym->fopen = fopen;
  sym->fdopen = fdopen;
  sym->popen = popen;
  sym->fclose = fclose;
  sym->pclose = pclose;
  sym->fseek = fseek;
  sym->ftell = ftell;
  sym->rewind = rewind;
  sym->fread = fread;
  sym->fwrite = fwrite;
  sym->fgetc = fgetc;
  sym->fgets = fgets;
  sym->feof = feof;
  sym->fputc = fputc;
  sym->fflush = fflush;
  sym->ferror = ferror;
  sym->rename = rename;
  sym->remove = remove;

  sym->free = free;
  sym->malloc = malloc;
  sym->calloc = calloc;
  sym->realloc = realloc;
  sym->exit = exit;
  sym->strtol = strtol;
  sym->getenv = getenv;
  sym->srand = srand;
  sym->rand = rand;
  sym->atoi = atoi;

  sym->memcpy = memcpy;
  sym->memset = memset;
  sym->strcmp = strcmp;
  sym->strcpy = strcpy;
  sym->strncpy = strncpy;
  sym->strcat = strcat;
  sym->strncat = strncat;
  sym->strcasecmp = strcasecmp;
  sym->strncasecmp = strncasecmp;
  sym->strchr = strchr;
  sym->strrchr = strrchr;
  sym->strpbrk = strpbrk;
  sym->strspn = strspn;
  sym->strcspn = strcspn;
  sym->strlen = strlen;
  sym->strstr = strstr;
  sym->strdup = strdup;
  sym->strtok = strtok;

  sym->tolower = tolower;
  sym->toupper = toupper;
  sym->isupper = isupper;

  sym->opendir = opendir;
  sym->readdir = readdir;
  sym->closedir = closedir;

  sym->access = access;
  sym->rmdir = rmdir;
  sym->isatty = isatty;
  sym->chdir = chdir;
  sym->getcwd = getcwd;
  sym->getuid = getuid;
  sym->sync = sync;
  sym->truncate = truncate;

  sym->stat = stat;
  sym->chmod = chmod;
  sym->mkdir = mkdir;
  sym->time = time;
  sym->delay = delay;
  sym->__dpmi_int = __dpmi_int;

  // initialize variables
  sym->__dj_stdin = __dj_stdin;
  sym->__dj_stdout = __dj_stdout;
  sym->__dj_stderr = __dj_stderr;
  sym->__dj_ctype_flags = __dj_ctype_flags;
  sym->__dj_ctype_tolower = __dj_ctype_tolower;
  sym->__dj_ctype_toupper = __dj_ctype_toupper;
  sym->errno = errno;

  // initialize the DXE module
  sym->dxe_init ();

  if (new_handle == INITIAL_HANDLE)
    dxe_map = map_create (10);
  dxe_map = map_put (dxe_map, (void *) new_handle, sym);
  handle = (void *) new_handle++;
#elif   defined __unix__ || defined __APPLE__   // Mac OS X actually
  /*
    We use dlcompat under Mac OS X simply because it's there. I (dbjh) don't
    want to add extra code only because "using the native api's is the supported
    method of loading dynamically on Mac OS X" (Peter O'Gorman, maintainer of
    dlcompat). Besides, dlcompat has been tested while any new code we add, not.
    RTLD_NOW is ignored by dlcompat (7-12-2003).
  */
  if ((handle = dlopen (module_name, RTLD_LAZY)) == NULL)
    {
      fputs (dlerror (), stderr);
      fputc ('\n', stderr);
      exit (1);
    }
#elif   defined _WIN32
  if ((handle = LoadLibrary (module_name)) == NULL)
    {
      LPTSTR strptr;

      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, GetLastError (),
                     MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPTSTR) &strptr, 0, NULL);
      // Note the construct with strptr. You wouldn't believe what a bunch of
      //  fucking morons those guys at Microsoft are!
      fputs (strptr, stderr);
      LocalFree (strptr);
      exit (1);
    }
#elif   defined __BEOS__
  if ((int) (handle = (void *) load_add_on (module_name)) < B_NO_ERROR)
    {
      fprintf (stderr, "Error while loading add-on image: %s\n", module_name);
      exit (1);
    }
#endif

  return handle;
}


void *
get_symbol (void *handle, char *symbol_name)
{
  void *symptr;
#ifdef  DJGPP
  st_symbol_t *sym = map_get (dxe_map, handle);
  if (sym == NULL)
    {
      fprintf (stderr, "Invalid handle: %x\n", (int) handle);
      exit (1);
    }

  symptr = sym->dxe_symbol (symbol_name);
  if (symptr == NULL)
    {
      fprintf (stderr, "Could not find symbol: %s\n", symbol_name);
      exit (1);
    }
#elif   defined __unix__ || defined __APPLE__   // Mac OS X actually, see
  char *strptr;                                 //  comment in open_module()

  symptr = dlsym (handle, symbol_name);
  if ((strptr = (char *) dlerror ()) != NULL)   // this is "the correct way"
    {                                           //  according to the info page
      fputs (strptr, stderr);
      fputc ('\n', stderr);
      exit (1);
    }
#elif   defined _WIN32
  symptr = (void *) GetProcAddress ((HINSTANCE) handle, symbol_name);
  if (symptr == NULL)
    {
      LPTSTR strptr;

      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, GetLastError (),
                     MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                     (LPTSTR) &strptr, 0, NULL);
      fputs (strptr, stderr);
      LocalFree (strptr);
      exit (1);
    }
#elif   defined __BEOS__
  int status = get_image_symbol ((int) handle, symbol_name,
                                 B_SYMBOL_TYPE_TEXT, &symptr); // B_SYMBOL_TYPE_DATA/B_SYMBOL_TYPE_ANY
  if (status != B_OK)
    {
      fprintf (stderr, "Could not find symbol: %s\n", symbol_name);
      exit (1);
    }
#endif

  return symptr;
}
