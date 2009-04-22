/*
dllinit.c - DLL initialization code

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

#if     defined __CYGWIN__ && defined DLOPEN
#include <cygwin/cygwin_dll.h>

DECLARE_CYGWIN_DLL(DllMain);
#endif


#include <stdio.h>
#include <string.h>
#include "libdiscmage.h"


#if     defined __CYGWIN__ || defined _WIN32
#include <windows.h>

#ifdef  __cplusplus
extern "C" BOOL WINAPI DllMain (HINSTANCE h, DWORD reason, LPVOID ptr);
#endif

/*
  When this function is explicitly specified as entry point (for example by
  using the option /ENTRY of link.exe) printf() does not produce output when
  called from this function.
*/
BOOL WINAPI
DllMain (HINSTANCE h, DWORD reason, LPVOID ptr)
{
  (void) ptr;                                   // warning remover
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls ((HMODULE) h);
      break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    }
  return TRUE;
}

#elif   defined DJGPP
#include "dxedll_pub.h"                         // for st_symbol_t
#include "dxedll_priv.h"                        // must be included after headers
#include "map.h"                                //  of external libraries!


int dxe_init (void);
void *dxe_symbol (char *symbol_name);

st_symbol_t import_export =
{
  dxe_init, dxe_symbol, sizeof (st_symbol_t),
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  {0, NULL, NULL, 0, 0, 0, NULL, 0}, {0, NULL, NULL, 0, 0, 0, NULL, 0},
  {0, NULL, NULL, 0, 0, 0, NULL, 0}, NULL, NULL, NULL, 0
};
st_map_t *symbol;


int
dxe_init (void)
{
  symbol = map_create (17);
  symbol->cmp_key = (int (*) (void *, void *)) strcmp; // How beautiful! ;-)

  symbol = map_put (symbol, "dm_get_version", dm_get_version);
  symbol = map_put (symbol, "dm_get_version_s", dm_get_version_s);
  symbol = map_put (symbol, "dm_set_gauge", dm_set_gauge);

  symbol = map_put (symbol, "dm_open", dm_open);
  symbol = map_put (symbol, "dm_reopen", dm_reopen);
  symbol = map_put (symbol, "dm_fdopen", dm_fdopen);
  symbol = map_put (symbol, "dm_close", dm_close);
  symbol = map_put (symbol, "dm_nfo", dm_nfo);

  symbol = map_put (symbol, "dm_read", dm_read);
  symbol = map_put (symbol, "dm_write", dm_write);

  symbol = map_put (symbol, "dm_disc_read", dm_disc_read);
  symbol = map_put (symbol, "dm_disc_write", dm_disc_write);

  symbol = map_put (symbol, "dm_toc_read", dm_toc_read);
  symbol = map_put (symbol, "dm_toc_write", dm_toc_write);

  symbol = map_put (symbol, "dm_cue_read", dm_cue_read);
  symbol = map_put (symbol, "dm_cue_write", dm_cue_write);

  symbol = map_put (symbol, "dm_rip", dm_rip);

  return 0;
}


/*
  Normally, the code that uses a dynamic library knows what it wants, i.e., it
  searches for specific symbol names. So, for a program that uses a DXE the
  code could look something like:
    void *handle;
    int (*function1) (int);

    handle = open_module (MODULE_NAME);
    function1 = ((st_symbol_t *) handle)->function1;

  However, by adding a symbol loading function, st_symbol_t doesn't have to be
  updated if the DXE should export more or other symbols, which makes using a
  DXE less error prone. Changing st_symbol_t would also require a recompile of
  the code that uses the DXE (which is a *bad* thing).
  A symbol loading function also makes an "extension API" a bit more elegant,
  because the extension functions needn't be hardcoded in st_symbol_t;
*/
void *
dxe_symbol (char *symbol_name)
{
  return map_get (symbol, symbol_name);
}

#elif   defined __unix__

// The functions _init() and _fini() seem to be reserved on GNU/Linux
#if 0
void _init (void)
{
}


void _fini (void)
{
}
#endif

#endif
