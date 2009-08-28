/*
cdi.h - DiscJuggler/CDI image support for libdiscmage

Copyright (c) 2002 NoisyB (noisyb@gmx.net)
based on specs and code by Dext


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
#ifndef CDI_H
#define CDI_H
extern int cdi_init (dm_image_t *image);
extern int cdi_track_init (dm_track_t *track, FILE *fh);

#endif // CDI_H
