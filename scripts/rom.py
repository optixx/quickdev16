import sqlite3

# Detect Mirrord Roms
# Rom Type Mapping
#
# 0   ROM only
# 1   ROM and RAM
# 2   ROM and Save RAM
# 3   ROM and DSP1 chip
# 4   ROM, RAM and DSP1 chip
# 5   ROM, Save RAM and DSP1 chip
# 19  ROM and Super FX chip
#227  ROM, RAM and GameBoy data
#246  ROM and DSP2 chip


snes_header_tpl='''
Super Nintendo Entertainment System/SNES/Super Famicom
SNES Tile Demo
Demo or Beta ROM?
Europe, Oceania and Asia
262144 Bytes (2.0000 Mb)

Padded: Maybe, 227296 Bytes (1.7341 Mb)
Interleaved/Swapped: No
Backup unit/emulator header: Yes, 512 Bytes
HiROM: No
Internal size: 2 Mb
ROM type: (0) ROM
ROM speed: 200 ns (SlowROM)
SRAM: No
Version: 1.0
Checksum: Ok, 0x6629 (calculated) == 0x6629 (internal)
Inverse checksum: Ok, 0x99d6 (calculated) == 0x99d6 (internal)
Checksum (CRC32): 0x8e16de1e
'''

swc_header_tpl='''
Backup unit header info (SWC)

00000000  20 00 0c 00  00 00 00 00  aa bb 04 00  00 00 00 00   ...............
00000010  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
00000020  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................

[0-1]    File size: 262144 Bytes (2.0000 Mb) => Matches
[2:7]    Run program in mode: 3
[2:6]    Split: No => Matches
[2:5]    SRAM mapping mode: LoROM => Matches
[2:4]    DRAM mapping mode: LoROM => Matches
[2:3-2]  SRAM size: 0 kB => Matches
[2:1]    Run program in mode: 3
[2:0]    External cartridge memory: Disabled
'''


snes_header_regex='''(?P<line01>.*)
(?P<titel>.*)
(?P<line03>.*)
(?P<region>.*)
'''

a = '''
(?P<titel>[\w]+)
(?P<line03>[\w]+)
(?P<region>[\w]+)
(?P<rom_size>\d+) Bytes ((?P<rom_mb>[\d.]+) Mb)

Padded: Maybe, 227296 Bytes (1.7341 Mb)
Interleaved/Swapped: No
Backup unit/emulator header: Yes, 512 Bytes
HiROM: No
Internal size: 2 Mb
ROM type: (0) ROM
ROM speed: 200 ns (SlowROM)
SRAM: No
Version: 1.0
Checksum: Ok, 0x6629 (calculated) == 0x6629 (internal)
Inverse checksum: Ok, 0x99d6 (calculated) == 0x99d6 (internal)
Checksum (CRC32): 0x8e16de1e
'''


import os
import re
import string
os.unlink("roms.sqlite3")

conn = sqlite3.connect('roms.sqlite3')
c = conn.cursor()
c.execute('''create table roms
                (
                  file_name     text,
                  file_ext      text,
                  file_size     integer,
                  rom_size      integer,
                  rom_mb        real,
                  rom_name      text,
                  rom_vendor    text,
                  rom_region    text,
                  rom_hirom     integer,
                  rom_internalsize  integer,
                  rom_type,     integer,
                  rom_speed     integer,    
                  rom_sram      integer,
                  rom_version   integer,
                  rom_chk       integer,
                  swc_size      integer,    
                  swc_mode      integer,
                  swc_split     text,
                  swc_sram_mode text,
                  swc_dram_mode text,
                  swc_sram_size text

                  )''')

#c.execute("""insert into stocks
#          values ('2006-01-05','BUY','RHAT',100,35.14)""")
conn.commit

c.close()


print re.compile(snes_header_regex,re.M).search(snes_header_tpl).groups()



