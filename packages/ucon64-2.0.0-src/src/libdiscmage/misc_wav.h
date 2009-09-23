/*
misc_wav.h

Copyright (c) 2004 NoisyB (noisyb@gmx.net)


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
#ifndef  MISC_WAV_H
#define  MISC_WAV_H
/*
  misc_wav_header()     read/(over-)write a wav header
                          mode == "r", "rb", "w", "wb", etc...
                          it does NOT insert a wav header
  misc_wav_generator()  generate a SQUARE_WAVE or SINE_WAVE
*/
extern int misc_wav_write_header (FILE *fh, int channels, int freq,
                                  int bytespersecond, int blockalign,
                                  int bitspersample, int data_length);
// TODO: remove these
#define misc_wav_write_header_v2(fh,c,f,bit,dl) misc_wav_write_header(fh, c, f,     (bit*c*f)/8, (bit*c)/8, bit, dl)
#define misc_wav_write_header_v3(fh,dl) misc_wav_write_header        (fh, 2, 44100, 176400,      4,         16,  dl)

                                                                                     
#define SQUARE_WAVE 0
#define SINE_WAVE 1
extern void misc_wav_generator (unsigned char *bit, int bitLength,
                                float volume, int wavType);
#endif // MISC_WAV_H
