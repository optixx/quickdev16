# Programmer used for In System Programming
ISP_PROG = usbasp
# device the ISP programmer is connected to
ISP_DEV = 
# Programmer used for serial programming (using the bootloader)
SERIAL_PROG = avr109
# device the serial programmer is connected to
SERIAL_DEV = /dev/ttyS0

# programs
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AS = avr-as
CP = cp
RM = rm -f
AVRDUDE = avrdude
AVRDUDE_BAUDRATE = 19200
SIZE = avr-size

-include $(CURDIR)/config.mk

# flags for avrdude
ifeq ($(MCU),atmega8)
	AVRDUDE_MCU=m8
endif
ifeq ($(MCU),atmega48)
	AVRDUDE_MCU=m48
endif
ifeq ($(MCU),atmega88)
	AVRDUDE_MCU=m88
endif
ifeq ($(MCU),atmega168)
	AVRDUDE_MCU=m168
endif
ifeq ($(MCU),atmega644)
	AVRDUDE_MCU=m644
endif


AVRDUDE_FLAGS += -p $(AVRDUDE_MCU) 

# flags for the compiler
CFLAGS += -g -Os -finline-limit=800 -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu99
ASFLAGS += -g -mmcu=$(MCU) -DF_CPU=$(F_CPU)

# flags for the linker
LDFLAGS += -mmcu=$(MCU)

ifneq ($(DEBUG),)
	CFLAGS += -Wall -W -Wchar-subscripts -Wmissing-prototypes
	CFLAGS += -Wmissing-declarations -Wredundant-decls
	CFLAGS += -Wstrict-prototypes -Wshadow -Wbad-function-cast
	CFLAGS += -Winline -Wpointer-arith -Wsign-compare
	#CFLAGS += -Wunreachable-code -Wdisabled-optimization -Werror
	CFLAGS += -Wunreachable-code -Wdisabled-optimization
	CFLAGS += -Wcast-align -Wwrite-strings -Wnested-externs -Wundef
	CFLAGS += -Wa,-adhlns=$(basename $@).lst
	CFLAGS += -DDEBUG
endif

all:

$(OBJECTS):

clean:
	$(RM) *.hex *.eep.hex *.o *.lst *.lss

.PHONY: all clean interactive-isp interactive-serial launch-bootloader

flash: 
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -U flash:w:$<

flash-eeprom-%: %.eep.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -c $(ISP_PROG) -P $(ISP_DEV) -U eeprom:w:$<

%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<
