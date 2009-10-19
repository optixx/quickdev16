rm -rf rom.zip
wine kzip.exe rom /s1 ../roms/qd16boot02.smc
wine DeflOpt.exe /a rom.zip
ruby zip2raw.rb rom.zip
rm -rf rom.zip
