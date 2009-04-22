/*
ffe.h - General Front Far East copier routines for uCON64

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
#ifndef FFE_H
#define FFE_H

/*
    0      - Low byte of 8 kB page count
             SMD: 16 kB page count
    1      - High byte of 8 kB page count
             SMD: File ID code 0 (3, not: high byte of 16 kB page count)
             Magic Griffin: Emulation mode select, first byte?
    2      - Emulation mode select (SWC/SMC/Magic Griffin, second byte?)
             Bit 7 6 5 4 3 2 1 0
                 x               : 1 = Run in mode 0 (JMP $8000) (higher precedence than bit 1)
                   x             : 0 = Last file of the ROM dump (multi-file)
                                 : 1 = Multi-file (there is another file to follow)
                     x           : SWC & SMC:
                                     0 = SRAM mapping mode 1 (LoROM)
                                     1 = mode 2 (HiROM)
                       x         : SWC & SMC:
                                     0 = DRAM mapping mode 20 (LoROM)
                                     1 = mode 21 (HiROM)
                         x x     : SWC & SMC (SRAM size):
                                     00 = 256 kb, 01 = 64 kb, 10 = 16 kb, 11 = no SRAM
                             x   : SWC & SMC:
                                     0 = Run in mode 3
                                     1 = Run in mode 2 (JMP RESET)
                               x : 0 = Disable, 1 = Enable external cartridge memory
                                   image at bank 20-5F,A0-DF in system mode 2/3)
    3-7    - 0, reserved
    8      - File ID code 1 (0xaa)
    9      - File ID code 2 (0xbb)
    10     - File type; check this byte only if ID 1 & 2 match
             1 : Super Magic Card saver data
             2 : Magic Griffin program (PC-Engine)
             3 : Magic Griffin SRAM data
             4 : SNES program
             5 : SWC & SMC password, SRAM data
             6 : Mega Drive program
             7 : SMD SRAM data
             8 : SWC & SMC saver data
    11-511 - 0, reserved
*/

#ifdef USE_PARALLEL

extern void ffe_init_io (unsigned int port);
extern void ffe_deinit_io (void);
extern void ffe_send_block (unsigned short address, unsigned char *buffer, int len);
extern void ffe_send_block2 (unsigned short address, unsigned char *buffer, int len);
extern void ffe_send_command0 (unsigned short address, unsigned char byte);
extern unsigned char ffe_send_command1 (unsigned short address);
extern void ffe_send_command (unsigned char command_code, unsigned short a, unsigned short l);
extern void ffe_receive_block (unsigned short address, unsigned char *buffer, int len);
extern void ffe_receive_block2 (unsigned short address, unsigned char *buffer, int len);
extern unsigned char ffe_receiveb (void);
extern void ffe_wait_for_ready (void);
extern void ffe_checkabort (int status);

#endif // USE_PARALLEL

#endif // FFE_H
