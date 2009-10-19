
use_file_names = true
include_file_names = false

loop do


  if ARGF.read(2) != "PK" 
      if ARGF.eof?
        exit
      end
      warn "Input is not a zip file"
      exit 1
  end

  if !(ARGF.read(2) == "\003\004") # file header
    exit
  end
  
  version = ARGF.read(2).unpack("v").first

  # i should use this to sort out some incompatible file formats....

  

  bitflag = ( ARGF.read(2).unpack("v").first)

  use_data_descriptor =  bitflag[3] == 1

  # should check for things that are unsupported for sure....
  c_method = ARGF.read(2).unpack("v").first
  if c_method != 8
      warn "Unknown compression method"
  end
  
  if c_method > 255
    warn "Unsupported compression method"
    exit 1
  end
  last_mod_time = ARGF.read(2) # unsupported until i know how to convert dos to unix time 
  last_mod_date = ARGF.read(2) # unsupported until i know how to convert dos to unix time

  crc_32 = ARGF.read(4).unpack("V").first
  
  size = ARGF.read(4).unpack("V").first
  if use_data_descriptor
    size = :auto
  end

  uncompressed_size = ARGF.read(4).unpack("V").first

  file_name_length = ARGF.read(2).unpack("v").first
  extra_field_length = ARGF.read(2).unpack("v").first

  file_name = ARGF.read(file_name_length)
   ARGF.read(extra_field_length) # extra field.. no use (yet?)
  
  file_name2 = file_name
  if use_file_names
    outfile = nil
    while !outfile
      
      if !(file_name2 =~ /\//)
        used_filename = file_name2
        outfile = File.open("#{file_name2}.deflate","w")
      end
      file_name2 = "zip2gz_file%05i" % rand(100000)
      
    end
  else
    outfile = STDOUT
  end
  
    
  #generate gz header
  #outfile.print "\x1f\x8b"
  #outfile.putc c_method
    
  flags = 0
    
  if include_file_names && !file_name.empty?
    flags |= 0b1000
  end
  
  #outfile.print flags.chr
  #outfile.print [0].pack("V") # no time conversion (yet)
  #outfile.print 2.chr # maybe copy first 3 bits of the zip extra flags? default to maximum compression
  #outfile.print 255.chr # this is stored in the central directory... (same for comments) we don't parse this...
  
  #if include_file_names && !file_name.empty?
  #  outfile.print file_name.tr("\x00"," ")+"\x00"
  #end
  
  if size != :auto
    blocks = size / 0x100
    rest   = size & 0xFF
    
    blocks.times do
      outfile.print ARGF.read(0x100)
    end
    
    outfile.print ARGF.read(rest)
  else
    
    size = -14
    
    buffer = ""
    loop do
      buffer << ARGF.getc
      size += 1
      if buffer.size > 14
        outfile.putc buffer[0]
        buffer[0,1] = ""
      end
    
      if buffer.size == 14
        crc, c_size, u_size, magic_number  = buffer.unpack("VVVa2")
        if c_size == size && c_size <= u_size && magic_number == "PK"
        
          break
        
          ARGF.ungetc "K"
          ARGF.ungetc "P"
          uncompressed_size = u_size
          crc_32 = crc
        end
      end
      
    
    end
  end
  
  #outfile.print [crc_32, uncompressed_size ].pack("VV")
  if file_name.empty?
    warn "Successfully wrote file crc32:%08X length:%i to %s" % [crc_32, size, (outfile == STDOUT ? "stdout" : "#{used_filename}.deflate")]
  else
    warn "Successfully wrote file \"%s\" crc32:%08X length:%i to %s" % [file_name, crc_32, size, (outfile == STDOUT ? "stdout" : "#{used_filename}.deflate")]
  end
  
  if use_file_names
    outfile.close
  end
  
end


