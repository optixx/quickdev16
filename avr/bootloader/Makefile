
# microcontroller and project specific settings
TARGET = bootloader
F_CPU = 20000000UL
MCU = atmega644

SRC = bootloader.c  
ASRC = usbdrv/usbdrvasm.S interrupts.S
OBJECTS += $(patsubst %.c,%.o,${SRC})
OBJECTS += $(patsubst %.S,%.o,${ASRC})
HEADERS += $(shell echo *.h)
# CFLAGS += -Werror

LDFLAGS += -L/usr/local/avr/avr/lib
CFLAGS += -Iusbdrv -I.
CFLAGS += -DHARDWARE_REV=$(HARDWARE_REV)
CDEFS += -DF_CPU
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
else ifeq ($(MCU),atmega644)
        # atmega644 with 2048 words bootloader:
        # bootloader section starts at 0x7800 (word-address) == 0xF000 (byte-address)
        BOOT_SECTION_START = 0xf000
endif

LDFLAGS += -Wl,--section-start=.text=$(BOOT_SECTION_START) -Wl,-u,vfprintf
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

.PHONY: clean clean-$(TARGET) 

clean: clean-$(TARGET) 

clean-$(TARGET):
	$(RM) $(TARGET) $(OBJECTS)


.PHONY: depend test 

depend:
	$(CC) $(CFLAGS) -M $(CDEFS) $(CINCS) $(SRC) >> $(MAKEFILE).dep

-include $(MAKEFILE).dep
