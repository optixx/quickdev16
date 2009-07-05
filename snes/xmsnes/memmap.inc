.MEMORYMAP
  SLOTSIZE $8000
  DEFAULTSLOT 0
  SLOT 0 $8000
  SLOT 1 $0000
  SLOT 2 $2000
  SLOTSIZE $40000
  SLOT 3 $8000
.ENDME

.ROMBANKMAP
BANKSTOTAL 3
BANKSIZE $8000
BANKS 2
BANKSIZE $40000
BANKS 1
.ENDRO

.SMC


.SNESHEADER

  ID "SNES"

  NAME "XMSNES               "      ; Title (can't be over 21 bytes)
  ;    "123456789012345678901"              (just a guide)

  SLOWROM                           ; Slow ROM
  LOROM                             ; LO ROM



CARTRIDGETYPE $00             ; $00 = ROM only, see WLA documentation for others
ROMSIZE $08                   ; $08 = 2 Mbits,  see WLA doc for more..
SRAMSIZE $00                  ; No SRAM         see WLA doc for more..
COUNTRY $02                   ; $01 = U.S.  $00 = Japan, that's all I know
LICENSEECODE $00              ; Just use $00
VERSION $00                   ; $00 = 1.00, $01 = 1.01, etc.


.ENDSNES
