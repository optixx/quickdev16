
# microcontroller and project specific settings
TARGET = usbload
F_CPU = 20000000UL
MCU = atmega168

SRC = usbload.c
ASRC = usbdrv/usbdrvasm.S interrupts.S
OBJECTS += $(patsubst %.c,%.o,${SRC})
OBJECTS += $(patsubst %.S,%.o,${ASRC})
HEADERS += $(shell echo *.h)
# CFLAGS += -Werror
LDFLAGS += -L/usr/local/avr/avr/lib
CFLAGS += -Iusbdrv -I.
CFLAGS += -DHARDWARE_REV=$(HARDWARE_REV)
ASFLAGS += -x assembler-with-cpp
ASFLAGS += -Iusbdrv -I.

# use own linkerscript, for special interrupt table handling
LDFLAGS += -T ./ldscripts/avr5.x

# no safe mode checks
AVRDUDE_FLAGS += -u

# set name for dependency-file
MAKEFILE = Makefile

# bootloader section start
# (see datasheet)
ifeq ($(MCU),atmega168)
	# atmega168 with 1024 words bootloader:
	# bootloader section starts at 0x1c00 (word-address) == 0x3800 (byte-address)
	BOOT_SECTION_START = 0x3800
else ifeq ($(MCU),atmega88)
	# atmega88 with 1024 words bootloader:
	# bootloader section starts at 0xc00 (word-address) == 0x1800 (byte-address)
	BOOT_SECTION_START = 0x1800
endif

LDFLAGS += -Wl,--section-start=.text=$(BOOT_SECTION_START)
CFLAGS += -DBOOT_SECTION_START=$(BOOT_SECTION_START)


include avr.mk

.PHONY: all

all: $(TARGET).hex $(TARGET).lss
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo -n "size is: "
	@$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2
	@echo "==============================="

$(TARGET): $(OBJECTS) $(TARGET).o

%.o: $(HEADERS)

.PHONY: install lock fuses-atmega168-unzap bootstrap

# install: program-serial-$(TARGET) program-serial-eeprom-$(TARGET)
install: program-isp-$(TARGET)

lock:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U lock:w:0x2f:m

fuses-atmega168-unzap:
	echo "sck 5" | $(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -F -u -t
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U lfuse:w:0xe7:m -U hfuse:w:0xd5:m -U efuse:w:0x00:m
	echo "sck 0.2" | $(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -F -u -t

fuses-atmega168-rumpus:
	echo "sck 5" | $(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -F -u -t
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U lfuse:w:0xe7:m -U efuse:w:0x00:m
	echo "sck 0.2" | $(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -F -u -t

bootstrap: fuses-atmega168-unzap install lock

.PHONY: clean clean-$(TARGET) clean-uploadtest

clean: clean-$(TARGET) clean-uploadtest

clean-$(TARGET):
	$(RM) $(TARGET) $(OBJECTS)

clean-uploadtest:
	rm -f datatestfile{512,14k}.raw

.PHONY: depend test uploadtest

depend:
	$(CC) $(CFLAGS) -M $(CDEFS) $(CINCS) $(SRC) >> $(MAKEFILE).dep

datatestfile14k.raw:
	dd if=/dev/urandom of=datatestfile14k.raw bs=1 count=14336

datatestfile512.raw:
	dd if=/dev/urandom of=datatestfile512.raw bs=1 count=512

test/test.hex:
	$(MAKE) -C test test.hex

uploadtest: datatestfile14k.raw datatestfile512.raw
	$(AVRDUDE) -p $(AVRDUDE_MCU) -c usbasp -P usb -U flash:w:datatestfile14k.raw -U eeprom:w:datatestfile512.raw
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U flash:v:datatestfile14k.raw -U eeprom:v:datatestfile512.raw

-include $(MAKEFILE).dep
