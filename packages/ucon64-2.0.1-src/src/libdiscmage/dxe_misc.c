/*
dxe_misc.c - miscellaneous functions for the grand libdiscmage DXE hack

Copyright 2003 - 2005 dbjh


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

/*
  The original reason this file was created was because we can do the
  (re)definition of the names of the buffered file I/O functions only once.
  For a DXE they would have to be (re)defined twice if we want to be able to
  use the zlib & unzip code in misc_z.c; once to substitute the names to make
  code use the import/export "table" and once to make code use the f*2()
  functions in misc_z.c.
*/

#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "dxedll_pub.h"


extern st_symbol_t import_export;
int errno = 0; // TODO: verify how dangerous this is (is it?)


int
open (const char *filename, int flags, ...)
{
  // NOTE: We do the same as djlsr205.zip/src/libc/posix/fcntl/open.c.
  mode_t mode = *(&flags + 1);
  return import_export.open (filename, flags, mode);
}


int
printf (const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = import_export.vfprintf (&import_export.__dj_stdout, format, argptr);
  va_end (argptr);
  return n_chars;
}


int
fprintf (FILE *file, const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = import_export.vfprintf (file, format, argptr);
  va_end (argptr);
  return n_chars;
}


int
vfprintf (FILE *file, const char *format, va_list argptr)
{
  return import_export.vfprintf (file, format, argptr);
}


int
sprintf (char *buffer, const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = import_export.vsprintf (buffer, format, argptr);
  va_end (argptr);
  return n_chars;
}


int
snprintf (char *buffer, size_t size, const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = import_export.vsnprintf (buffer, size, format, argptr);
  va_end (argptr);
  return n_chars;
}


int
vsprintf (char *buffer, const char *format, va_list argptr)
{
  return import_export.vsprintf (buffer, format, argptr);
}


int
vsnprintf (char *buffer, size_t size, const char *format, va_list argptr)
{
  return import_export.vsnprintf (buffer, size, format, argptr);
}


int
puts (const char *str)
{
  return import_export.puts (str);
}


int
fputs (const char *str, FILE *file)
{
  return import_export.fputs (str, file);
}


int
sscanf (const char *buffer, const char *format, ...)
{
  va_list argptr;
  int n_chars;

  va_start (argptr, format);
  n_chars = import_export.vsscanf (buffer, format, argptr);
  va_end (argptr);
  return n_chars;
}


int
vsscanf (const char *buffer, const char *format, va_list argptr)
{
  return import_export.vsscanf (buffer, format, argptr);
}


FILE *
fopen (const char *filename, const char *mode)
{
  return import_export.fopen (filename, mode);
}


FILE *
fdopen (int fd, const char *mode)
{
  return import_export.fdopen (fd, mode);
}


FILE *
popen (const char *command, const char *mode)
{
  return import_export.popen (command, mode);
}


int
fclose (FILE *file)
{
  return import_export.fclose (file);
}


int
pclose (FILE *stream)
{
  return import_export.pclose (stream);
}


int
fseek (FILE *file, long offset, int mode)
{
  return import_export.fseek (file, offset, mode);
}


long
ftell (FILE *file)
{
  return import_export.ftell (file);
}


void
rewind (FILE *file)
{
  import_export.rewind (file);
}


size_t
fread (void *buffer, size_t size, size_t number, FILE *file)
{
  return import_export.fread (buffer, size, number, file);
}


size_t
fwrite (const void *buffer, size_t size, size_t number, FILE *file)
{
  return import_export.fwrite (buffer, size, number, file);
}


int
fgetc (FILE *file)
{
  return import_export.fgetc (file);
}


char *
fgets (char *buffer, int size, FILE *file)
{
  return import_export.fgets (buffer, size, file);
}


int
feof (FILE *file)
{
  return import_export.feof (file);
}


int
fputc (int character, FILE *file)
{
  return import_export.fputc (character, file);
}


int
fflush (FILE *file)
{
  return import_export.fflush (file);
}


int
ferror (FILE *file)
{
  return import_export.ferror (file);
}


int
rename (const char *oldname, const char *newname)
{
  return import_export.rename (oldname, newname);
}


int
remove (const char *filename)
{
  return import_export.remove (filename);
}


void
clearerr (FILE *file)
{
  import_export.clearerr (file);
}


void
free (void *mem)
{
  import_export.free (mem);
}


void *
malloc (size_t size)
{
  return import_export.malloc (size);
}


void *
calloc (size_t n_elements, size_t size)
{
  return import_export.calloc (n_elements, size);
}


void *
realloc (void *mem, size_t size)
{
  return import_export.realloc (mem, size);
}


void
exit (int code)
{
  import_export.exit (code);
}


long
strtol (const char *str, char **endptr, int base)
{
  return import_export.strtol (str, endptr, base);
}


char *
getenv (const char *name)
{
  return import_export.getenv (name);
}


void
srand (unsigned seed)
{
  import_export.srand (seed);
}


int
rand (void)
{
  return import_export.rand ();
}


int
atoi (const char *str)
{
  return import_export.atoi (str);
}


int
memcmp(const void *str1, const void *str2, size_t size)
{
  return import_export.memcmp (str1, str2, size);
}


void *
memcpy (void *dest, const void *src, size_t size)
{
  return import_export.memcpy (dest, src, size);
}


void *
memset (void *mem, int value, size_t size)
{
  return import_export.memset (mem, value, size);
}


void *
memchr (const void *str, int character, size_t size)
{
  return import_export.memchr (str, character, size);
}


int
strcmp (const char *str1, const char *str2)
{
  return import_export.strcmp (str1, str2);
}


char *
strcpy (char *dest, const char *src)
{
  return import_export.strcpy (dest, src);
}


char *
strncpy (char *dest, const char *src, size_t n)
{
  return import_export.strncpy (dest, src, n);
}


char *
strcat (char *str1, const char *str2)
{
  return import_export.strcat (str1, str2);
}


char *
strncat (char *str1, const char *str2, size_t n)
{
  return import_export.strncat (str1, str2, n);
}


char *
stpcpy (char *dest, const char *src)
{
  return import_export.stpcpy (dest, src);
}


int
strcasecmp (const char *str1, const char *str2)
{
  return import_export.strcasecmp (str1, str2);
}


int
strncasecmp (const char *str1, const char *str2, size_t n)
{
  return import_export.strncasecmp (str1, str2, n);
}


char *
strchr (const char *str, int character)
{
  return import_export.strchr (str, character);
}


char *
strrchr (const char *str, int character)
{
  return import_export.strrchr (str, character);
}


char *
strpbrk (const char *str, const char *set)
{
  return import_export.strpbrk (str, set);
}


size_t
strspn (const char *str, const char *set)
{
  return import_export.strspn (str, set);
}


size_t
strcspn (const char *str, const char *set)
{
  return import_export.strcspn (str, set);
}


size_t
strlen (const char *str)
{
  return import_export.strlen (str);
}


char *
strstr (const char *str1, const char *str2)
{
  return import_export.strstr (str1, str2);
}


char *
strdup (const char *str)
{
  return import_export.strdup (str);
}


char *
strtok (char *str1, const char *str2)
{
  return import_export.strtok (str1, str2);
}


char *
strerror (int error)
{
  return import_export.strerror (error);
}


#undef  tolower
int
tolower (int character)
{
  return import_export.tolower (character);
}


#undef  toupper
int
toupper (int character)
{
  return import_export.toupper (character);
}


#undef  isupper
int
isupper (int character)
{
  return import_export.isupper (character);
}


DIR *
opendir (const char *dirname)
{
  return import_export.opendir (dirname);
}


struct dirent *
readdir (DIR *dir)
{
  return import_export.readdir (dir);
}


int
closedir (DIR *dir)
{
  return import_export.closedir (dir);
}


int
access (const char *filename, int mode)
{
  return import_export.access (filename, mode);
}


ssize_t
read (int fd, void *buffer, size_t size)
{
  return import_export.read (fd, buffer, size);
}


ssize_t
write (int fd, const void *buffer, size_t size)
{
  return import_export.write (fd, buffer, size);
}


int
close (int fd)
{
  return import_export.close (fd);
}


off_t
lseek (int fd, off_t offset, int whence)
{
  return import_export.lseek (fd, offset, whence);
}


int
readlink (const char *filename, char *buffer, size_t size)
{
  return import_export.readlink (filename, buffer, size);
}


int
rmdir (const char *dirname)
{
  return import_export.rmdir (dirname);
}


int
isatty (int fd)
{
  return import_export.isatty (fd);
}


int
chdir (const char *dirname)
{
  return import_export.chdir (dirname);
}


char *
getcwd (char *buffer, size_t size)
{
  return import_export.getcwd (buffer, size);
}


int
getuid (void)
{
  return import_export.getuid ();
}


int
sync (void)
{
  return import_export.sync ();
}


int
truncate (const char *filename, off_t size)
{
  return import_export.truncate (filename, size);
}


int
stat (const char *filename, struct stat *statbuf)
{
  return import_export.stat (filename, statbuf);
}


int
chmod (const char *filename, mode_t mode)
{
  return import_export.chmod (filename, mode);
}


int
mkdir (const char *filename, mode_t mode)
{
  return import_export.mkdir (filename, mode);
}


time_t
time (time_t *t)
{
  return import_export.time (t);
}


double
difftime (time_t time1, time_t time0)
{
  return import_export.difftime (time1, time0);
}


void
delay (unsigned nmillis)
{
  import_export.delay (nmillis);
}


int
__dpmi_int (int vector, __dpmi_regs *regs)
{
  return import_export.__dpmi_int (vector, regs);
}
