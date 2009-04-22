/*
backup.h - single header file for all backup unit functions

Copyright (c) 2003 NoisyB <noisyb@gmx.net>


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
#ifndef BACKUP_H
#define BACKUP_H
#ifdef  HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  USE_PARALLEL
#include "cd64.h"
#include "cmc.h"
#include "dex.h"
#include "doctor64.h"
#include "doctor64jr.h"
#include "fal.h"
//#include "ffe.h"
#include "fig.h"
#include "gbx.h"
#include "gd.h"
#include "interceptor.h"
#include "lynxit.h"
#include "mccl.h"
#include "mcd.h"
#include "md-pro.h"
#include "mgd.h"
#include "msg.h"
#include "pce-pro.h"
#include "pl.h"
//#include "psxpblib.h"
#include "sflash.h"
#include "smc.h"
#include "smd.h"
#include "smsgg-pro.h"
#include "ssc.h"
#include "swc.h"
#include "ufo.h"
#include "yoko.h"
#include "z64.h"
#endif // USE_PARALLEL
#if     defined USE_PARALLEL || defined USE_USB
#include "f2a.h"
#endif

#endif // BACKUP_H
