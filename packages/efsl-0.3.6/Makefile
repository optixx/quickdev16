include conf/config.makefile

linux: efsl-base efsl-fs-vfat efsl-hwd-linuxfile
avr: efsl-base efsl-fs-vfat efsl-prot-sdspi efsl-hwd-atmega_sd

efsl-base:
	make -C src/base/
	cp src/base/efsl-base.a lib/libefsl-base.a

efsl-fs-vfat:
	make -C src/fs/vfat/
	cp src/fs/vfat/efsl-fs-vfat.a lib/libefsl-fs-vfat.a

efsl-prot-sdspi:
	make -C src/protocols/sdcard_spi/
	cp src/protocols/sdcard_spi/efsl-prot-sdspi.a lib/libefsl-prot-sdspi.a

efsl-hwd-linuxfile:
	make -C src/hwdrivers/linuxfile/
	cp src/hwdrivers/linuxfile/efsl-hwd-linuxfile.a lib/libefsl-hwd-linuxfile.a

efsl-hwd-atmega_sd:
	make -C src/hwdrivers/atmega_spi/
	cp src/hwdrivers/atmega_spi/efsl-hwd-atmega_spi.a lib/libefsl-hwd-atmega_spi.a

clean:
	make -C src/base/ clean
	make -C src/fs/vfat/ clean
	make -C src/hwdrivers/linuxfile/ clean
	make -C src/hwdrivers/atmega_spi clean
	rm -rf lib/*.a
