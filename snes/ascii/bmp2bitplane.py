import os
import re
import string
import sys
import binascii
import math

def openfile(filename):
    	return open(filename)


def mk_sint(s):
	a = ord(s[1])
	b = ord(s[0])
	return (a<<8) + b 


def mk_int(s):
	a = ord(s[3])
	b = ord(s[2])
	c = ord(s[1])
	d = ord(s[0])
	return (a<<32) + (b<<16) + (c<<8) + d 


def main():
	file = sys.argv[1] 
	asmfile = string.replace(file,".bmp",".s")
	basename = string.replace(file,".bmp","")
	fp =openfile(file)

	header_offset = 14 + 40 
	data = fp.read()
	fp.close()
	
	type = mk_sint(data[0:2])
	size = mk_int(data[2:6])
	width = mk_int(data[18:22])
	height = mk_int(data[22:26])
	bits_per_pixel = mk_int(data[28:32])
	num_of_colors = mk_int(data[46:50])

	bytes_per_line = width / 8
	bytes_per_line_padded = bytes_per_line + (4-(bytes_per_line % 4))


	header = data[0:header_offset]
	colors = data[header_offset: header_offset + (num_of_colors  * 4)]
	data =   data[header_offset + (num_of_colors  * 4):]	
	
	raw = []
	for i in data:
		raw.append(ord(i))
	
	raw_len = len(raw)
	
	
	

	print "file:\t\t%s" % file
	print "basename:\t%s" % asmfile
	print "header info"
	print "type: \t\t%04X " % type
	print "size: \t\t%i bytes" % size
	print "width: \t\t%i pixel" % width
	print "height: \t%i pixel" % height
	print "bit per pixel:\t%i" % bits_per_pixel
	print "num of colors:\t%i" % num_of_colors
 	print "imagedata: \t%s bytes" % raw_len
	print "per line: \t%i bytes" %  bytes_per_line			
	print "per line pad: \t%i bytes" %  bytes_per_line_padded			
	
	
	
	


	bitplane={}
	for bit in range(0,bits_per_pixel):
		bitplane[bit] = [] 

	
	bytes = [0,0,0,0]
	bit_cnt =0

	cnt=0
	last_progress = 0
	for byte in raw:
		cnt+=1
		progress = cnt / (raw_len/100) 
		
		if (progress%10)==0:
			if (progress>last_progress):
				print "converting bmp to bitplane buffers %i%% done" % (progress)
				last_progress = progress 

		if bits_per_pixel == 4 :
			hi_nibble = (byte >>  4)  
			lo_nibble = byte & 16
			hi_nibble_bin = ''
			
			for nibble in (hi_nibble,lo_nibble): 
				for plane in range(bits_per_pixel-1,-1,-1):
					if (nibble & (1 << plane)):
						bytes[plane] |= 1 << bit_cnt
				
				bit_cnt += 1
				if (bit_cnt==8):
					for i in range(0,bits_per_pixel):
						bitplane[i].append(bytes[i])
					bytes=[0,0,0,0]
					bit_cnt =0

		
		if bits_per_pixel == 1 :
			bitplane[0]  = raw
			break 


 	out='';
	
	for plane in range(0,bits_per_pixel):
		print "bitplane %i has %i bytes " % (plane,len(bitplane[plane]))
		
	x_tiles = width / 8
	y_tiles = height / 8
	cnt=0
	last_progress=0

	fp = open(asmfile,'w')
	tile_cnt = 0
	out ="\n\n\n%s:\n\n" % (basename)
	fp.write(out)
	#for yt in range(0,y_tiles):
	for yt in range(y_tiles-1,-1,-1):			# needed for h flip 
		out ="; 	### row %02x ### \n" % yt 
		fp.write(out)
		progress = cnt / (raw_len/100) 
		if(progress%5)==0:
			print "building asm include  %i%% done" % (progress)
			#last_progress = progress 
			
		for xt in range(0,x_tiles):
			for plane in range(0,bits_per_pixel):	
				out = '	.db	'
				for y in range(((yt+1)*8)-1,(yt*8)-1,-1): 	# needed for h flip 
				#for y in range(yt*8,(yt+1)*8):
					out += "$%02x," % bitplane[plane][(y*x_tiles)+xt]  
					cnt+=1		
				out = out[:-1]
				out  += '	; tile=%02x plane=%02x row=%02x col=%02x \n' % (tile_cnt,plane,yt,xt) 
				fp.write(out)
				tile_cnt+=1
	
	
			
	fp.close()

if __name__ == '__main__':
	if len(sys.argv) is 2:
        	main()
	else:
        	print "usage: %s  in out" % sys.argv[0]




