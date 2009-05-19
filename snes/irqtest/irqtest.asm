;============================================================================
; Includes
;============================================================================

;== Include MemoryMap, Vector Table, and HeaderInfo ==
.INCLUDE "header.inc"

;== Include SNES Initialization routines ==
.INCLUDE "init.inc"
.INCLUDE "LoadGraphics.asm"
.INCLUDE "debug.asm"

;============================================================================
; Main Code
;============================================================================

.EQU PalNum $0000       ; Use some RAM

.BANK 0 SLOT 0
.ORG 0
.SECTION "MainCode"

Start:
    InitSNES    ; Clear registers, etc.

    ; Load Palette for our tiles
    LoadPalette OptixxPalette, 0, 16

    ; Load Tile data to VRAM
	;LoadBlockToVRAM TilesData, $0000, $0020	; 2 tiles, 2bpp, = 32 bytes
	;LoadBlockToVRAM OptixxData, $0000, 0xa00	; 160 tiles, 2bpp, = 2560 bytes
	;LoadBlockToVRAM OptixxData, $0000, 0x1e00	; 480 tiles, 2bpp, = 7680 bytes
	LoadBlockToVRAM OptixxData, $0000, 0x3c00	; 960 tiles, 2bpp, = 15360 bytes

    lda #$80
    sta $2115
    ;ldx #$0800	; 5AF
    ldx #$4000	; 5AF
    stx $2116
    
	ldx #$0
Start_do:    
	stx $2118
	inx
	cpx #960
	bne Start_do 
   
    ; Setup Video modes and other stuff, then turn on the screen
    jsr SetupVideo

    prints "Init done"
    
    lda #$81
    sta $4200

Infinity:
    lda PalNum
    clc
    adc #$01
    and #$ff        ; If > palette starting color > 24 (00011100), make 0
    sta PalNum
    jmp Infinity    ; bwa hahahahaha


;============================================================================
; SetupVideo -- Sets up the video mode and tile-related registers
;----------------------------------------------------------------------------
; In: None
;----------------------------------------------------------------------------
; Out: None
;----------------------------------------------------------------------------
SetupVideo:
    php

    lda #$00
    sta $2105           ; Set Video mode 0, 8x8 tiles, 4 color BG1/BG2/BG3/BG4

    ;lda #$08            ; Set BG1's Tile Map offset to $0800 (Word address)
    lda #$40            ; Set BG1's Tile Map offset to $2000 (Word address)
    sta $2107           ; And the Tile Map size to 32x32

    stz $210B           ; Set BG1's Character VRAM offset to $0000 (word address)

    lda #$01            ; Enable BG1
    sta $212C

    lda #$FF
    sta $210E
    sta $210E

    lda #$0F
    sta $2100           ; Turn on screen, full Brightness

    plp
    rts

;.ENDS

;.SECTION "IRQHandlers" 

COPHandler:
    prints "COPHandler"
    rti

BRKHandler:
    prints "BRKHandler"
    rti
ABRTHandler:
    prints "ABRTHandler"
    rti
NMIHandler:
    ;prints "NMIHandler"
    rti

IRQHandler:
    stz $2121
    lda PalNum
    sta $2122
    sta $2122
    prints "IRQHandler"
    rti


str_COP:
    .db "COP",10,0
str_ABORT:
    .db "ABORT",10,0
str_NMI:
    .db "NMI",10,0
str_RESET:
    .db "RESET",10,0
str_IRQBRK:
    .db "IRQBRK",10,0


;============================================================================
.ENDS

;============================================================================
; Character Data
;============================================================================

.BANK 0 SLOT 0
.SECTION "CharacterData02"
    .INCLUDE "optixx.inc"
.ENDS
