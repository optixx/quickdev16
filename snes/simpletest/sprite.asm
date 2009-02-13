;============================================================================
; Includes
;============================================================================

;== Include MemoryMap, Vector Table, and HeaderInfo ==
.INCLUDE "header.inc"

;== Include SNES Initialization routines ==
.INCLUDE "init.inc"
.INCLUDE "LoadGraphics.asm"


;== EQUates ==
.EQU PalNum $0000       ; Use some RAM

;============================================================================
; Main Code
;============================================================================

.MACRO Stall
    .REPT 3
        WAI
    .ENDR
.ENDM

.BANK 0 SLOT 0
.ORG 0
.SECTION "MainCode"

Start:
    InitSNES    ; Clear registers, etc.

    rep #$10
    sep #$20

    lda #%00001001
    sta $2105

    ; Blue Background
    stz $2121
    lda #$40
    sta $2122
    sta $2122
    
    ; Load Palette for our tiles
    LoadPalette SprPal, 128, 16     ; Sprite Palettes start at color 128

    ; Load Tile data to VRAM
    LoadBlockToVRAM Sprite, $0000, $0800

    jsr SpriteInit
    
    lda #($80-16)
    sta $0000
    
    lda #(224/2 - 16)
    sta $0001
    
    stz $0002
    lda #%01110000
    sta $0003
 
    ;lda #%11000000
    ;sta $0100
 
   
    lda #%01010100
    sta $0200
    
    ; Setup Video modes and other stuff, then turn on the screen
    jsr SetupVideo

    lda #$80
    sta $4200       ; Enable NMI


Infinity:
    Stall
    lda PalNum
    clc
    adc #$01
    and #$ff        ; If > palette starting color > 24 (00011100), make 0
    sta PalNum

    jmp Infinity    ; bwa hahahahaha

;============================================================================
SpriteInit:
	php	

	rep	#$30	;16bit mem/A, 16 bit X/Y
	
	ldx #$0000
    lda #$0001
_setoffscr:
    sta $0000,X
    inx
    inx
    inx
    inx
    cpx #$0200
    bne _setoffscr
;==================
	ldx #$0000
	lda #$5555
_clr:
	sta $0200, X		;initialize all sprites to be off the screen
	inx
	inx
	cpx #$0020
	bne _clr
;==================

	plp
	rts
;============================================================================
;============================================================================
; SetupVideo -- Sets up the video mode and tile-related registers
;----------------------------------------------------------------------------
; In: None
;----------------------------------------------------------------------------
; Out: None
;----------------------------------------------------------------------------
SetupVideo:
    php
    
    rep #$10
    sep #$20
    
    stz $2102
    stz $2103
    
    ;*********transfer sprite data

	stz $2102		; set OAM address to 0
	stz $2103

	LDY #$0400
	STY $4300		; CPU -> PPU, auto increment, write 1 reg, $2104 (OAM Write)
	stz $4302
	stz $4303		; source offset
	LDY #$0220
	STY $4305		; number of bytes to transfer
	LDA #$7E
	STA $4304		; bank address = $7E  (work RAM)
	LDA #$01
	STA $420B		;start DMA transfer
	
	lda #%10100000
    sta $2101

    lda #%00010000            ; Enable BG1
    sta $212C
    
    lda #$0F
    sta $2100           ; Turn on screen, full Brightness

    plp
    rts



;============================================================================
VBlank:
    rep #$30        ; A/mem=16 bits, X/Y=16 bits (to push all 16 bits)
    phb
	pha
	phx
	phy
	phd

    sep #$20        ; A/mem=8 bit    
    
    

    stz $2121
    lda PalNum
    sta $2122
    sta $2122


    lda $4210       ; Clear NMI flag
    rep #$30        ; A/Mem=16 bits, X/Y=16 bits 
    
    PLD 
	PLY 
	PLX 
	PLA 
	PLB 

    sep #$20
    RTI

;============================================================================
.ENDS

;============================================================================
; Character Data
;============================================================================
.BANK 1 SLOT 0
.ORG 0
.SECTION "CharacterData"
Sprite:
    .INCBIN "biker.pic"
    
SprPal:
    .INCBIN "biker.clr"

.ENDS
