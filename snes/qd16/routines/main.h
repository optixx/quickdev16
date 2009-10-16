.MEMORYMAP
DEFAULTSLOT 0
SLOTSIZE $10000
SLOT 0 $0000
.ENDME

.ROMBANKMAP
BANKSTOTAL 2
BANKSIZE $10000
BANKS 2
.ENDRO

.snesheader
id "QD16"
name "QUICKDEV16"
hirom
fastrom
cartridgetype 0
romsize 8
sramsize 0
country 0
licenseecode $33
version 0
.endsnes

.SNESNATIVEVECTOR
COP    EmptyHandler
BRK    EmptyHandler
ABORT  EmptyHandler
NMI    EmptyHandler
UNUSED IrqHookUp
IRQ    EmptyHandler
.ENDNATIVEVECTOR

.SNESEMUVECTOR
COP    EmptyHandler
ABORT  EmptyHandler
NMI    EmptyHandler
RESET  Boot
IRQBRK EmptyHandler
.ENDEMUVECTOR

.EMPTYFILL $ff


	.include "routines/variables.h"
	.include "routines/defines.h"


