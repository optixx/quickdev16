/*
dlopen.h - DLL support code

Copyright (c) 2002, 2015 dbjh


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
#ifndef DLOPEN_H
#define DLOPEN_H

/*
  The next union is a portable means to convert between function and data
  pointers and the only way to silence Visual C++ 2012 other than
    #pragma warning(disable: 4152)
  That is, with /W4.
*/
typedef union u_func_ptr
{
  void (*func_ptr) (void);
  void *void_ptr;
} u_func_ptr_t;

void *open_module (char *module_name);
void *get_symbol (void *handle, char *symbol_name);
void *has_symbol (void *handle, char *symbol_name);

#endif // DLOPEN_H
