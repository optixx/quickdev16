import sqlite3
import os
import re
import string
import stat
import popen2
import glob
import sys
import pprint

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


#Process /Users/david/Devel/arch/snes/roms/Teenage Mutant Ninja Turtles IV - Turtles in Time (U) [!].smc
#0 uCON64 2.0.0 Apple (PPC) 1999-2005
#1 Uses code from various people. See 'developers.html' for more!
#2 This may be freely redistributed under the terms of the GNU Public License
#4 /Users/david/Devel/arch/snes/roms/Teenage Mutant Ninja Turtles IV - Turtles in Time (U) [!].smc
#6 Multi Game Doctor (2)/Multi Game Hunter/MGH
#8 00007fb0  ff ff ff ff  ff ff ff ff  ff ff ff ff  ff ff ff ff  ................
#9 00007fc0  54 2e 4d 2e  4e 2e 54 2e  20 34 20 20  20 20 20 20  T.M.N.T. 4      
#10 00007fd0  20 20 20 20  20 20 00 0a  00 01 a4 00  7c e9 83 16        ......|...
#12 Super Nintendo Entertainment System/SNES/Super Famicom
#13 T.M.N.T. 4           
#14 Konami
#15 U.S.A.
#16 1048576 Bytes (8.0000 Mb)
#18 Padded: Maybe, 105 Bytes (0.0008 Mb)
#19 Interleaved/Swapped: No
#20 Backup unit/emulator header: No
#21 HiROM: No
#22 Internal size: 8 Mb
#23 ROM type: (0) ROM
#24 ROM speed: 200 ns (SlowROM)
#25 SRAM: No
#26 Version: 1.0
#27 Checksum: Ok, 0x1683 (calculated) == 0x1683 (internal)
#28 Inverse checksum: Ok, 0xe97c (calculated) == 0xe97c (internal)
#29 Checksum (CRC32): 0x5940bd99
#31 This ROM has no backup unit header





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




def createdb():
	try:
		os.unlink("roms.sqlite3")
	except:
		pass
	conn = sqlite3.connect('roms.sqlite3')
	c = conn.cursor()
	c.execute('''create table roms
                (
                  file_name     text,
                  file_ext      text,
                  file_size     integer,
                  rom_size      integer,
                  rom_mb        real,
				  rom_padded    integer,
                  rom_trainer   integer,
				  rom_backup    integer,
				  rom_name      text,
                  rom_vendor    text,
                  rom_region    text,
                  rom_hirom     integer,
                  rom_internalsize  integer,
                  rom_type      integer,
                  rom_speed     integer,    
                  rom_sram      integer,
                  rom_version   real,
                  rom_chk       integer,
                  swc_size      integer,    
                  swc_mode      integer,
                  swc_split     text,
                  swc_sram_mode text,
                  swc_dram_mode text,
                  swc_sram_size text

                  )''')
	return conn,c
	
def process(conn,c,file_name,out):
	file_ext      = os.path.splitext(file_name)[1].replace(".",'')
	file_size     = os.stat(file_name)[stat.ST_SIZE]
	rom_size      = 0
	rom_mb        = 0
	rom_padded    = 0
	rom_trainer   = 0
	rom_backup    = 0
	rom_name      = ''
	rom_vendor    = ''
	rom_region    = ''
	rom_hirom     = 0
	rom_internalsize = 0
	rom_type      = 0
	rom_speed     = 0    
	rom_sram      = 0
	rom_version   = 0
	rom_chk       = 0
	swc_size      = 0    
	swc_mode      = 0
	swc_split     = ''
	swc_sram_mode = ''
	swc_dram_mode = ''
	swc_sram_size = ''

	print "-" * 60
	print "Process %s" % file_name
	
	try:
		rom_name      = out[13]
		rom_vendor    = out[14]
		rom_region    = out[15]

		try:
			rom_size      = int(out[16].split(" ")[0])
			rom_mb        = float(re.compile("([\d.]+) Mb").search(out[16]).groups()[0])
		except:
			print "Broken..."
			return
		if not "No" in out[18]: 
			rom_padded = int(re.compile("([\d.]+) Bytes").search(out[18]).groups()[0])
	
		for idx,line in enumerate(out):
			if line is None:
				continue
		
			if "Backup unit/emulator header: Yes" in line:
				rom_backup = int(re.compile("([\d.]+) Bytes").search(line).groups()[0])
			
			if "Intro/Trainer:" in line:
				rom_trainer = int(re.compile("([\d.]+) Bytes").search(line).groups()[0])
		
			if "HiROM: Yes" in line:
				rom_hirom = 1
		
			if "Internal size:" in line:
				rom_internalsize = int(re.compile("([\d.]+) Mb").search(line).groups()[0])
	
			if "ROM type:" in line:
				try:
					rom_type = int(re.compile("([\d]+)").search(line).groups()[0])
				except:
					pass
			if "ROM speed:" in line:
				rom_speed  = int(re.compile("([\d]+) ns").search(line).groups()[0])    
		
			if "SRAM: Yes" in line:
				rom_sram = int(re.compile("([\d]+) kBytes").search(line).groups()[0])
	
			if "Version:" in line:
				rom_version   = float(re.compile("([\d.]+)").search(line).groups()[0])
			if "Checksum: Ok" in line:
				rom_chk = 1
	except:
		for idx,line in enumerate(out):
			if line is None:
				continue
			print idx,line
		sys.exit()

	query = """INSERT INTO roms
	          	VALUES
				(
					?,?,?,?,?,
					?,?,?,?,?,
					?,?,?,?,?,
					?,?,?,?,?,
					?,?,?,?) """ 
					
	data = (file_name,
				    file_ext,
				    file_size,
				    rom_size,
				    rom_mb,
				    rom_padded,
				    rom_trainer,
				    rom_backup,
				    rom_name,
				    rom_vendor,
				    rom_region,
				    rom_hirom,
				    rom_internalsize,
				    rom_type,
				    rom_speed,    
				    rom_sram,
				    rom_version,
				    rom_chk,
				    swc_size,    
				    swc_mode,
				    swc_split,
				    swc_sram_mode,
				    swc_dram_mode,
				    swc_sram_size)
	
	c.execute(query,data)
	conn.commit()


def ucon64_info(filename):
	cmd = "ucon64 --dbuh -snes \"%s\"" % filename
	r, w, e = popen2.popen3(cmd)
	err = e.readlines()
	out = r.readlines()
	r.close()
	e.close()
	w.close()
	if len(err):
		return False,err
	return out,err

def clean(s):
	s = s.replace("\n","")
	if not len(s):
		return None
	return s
	
def main():
	
	conn,c = createdb()
	
	path = sys.argv[1]
	files = glob.glob(path + "/*")
	for filename in files:
		try:
			r,err = ucon64_info(filename)
			if not r:
				print err
				continue
			r = map(clean,r)
			process(conn,c,filename,r)
		except (KeyboardInterrupt, SystemExit):
			print "Saving DB..."
			c.close()
			conn.commit()
			conn.close()
			sys.exit(-1)
	c.close()
	conn.commit()
	conn.close()

if __name__ == "__main__":
	main()




