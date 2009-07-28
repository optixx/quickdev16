.MEMORYMAP
DEFAULTSLOT 0
SLOTSIZE $10000
SLOT 0 $0000
.ENDME


.ROMBANKSIZE $10000    ; Every ROM bank is 64 KBytes in size, also necessary.
.HIROM
.ROMBANKS 4

.DEFINE HEADER_OFF $8000
.COMPUTESNESCHECKSUM
.EMPTYFILL $ff

	.include "routines/variables.asm"
	.include "routines/defines.asm"


;***********************************************************************************
; setup header
;***********************************************************************************
.define BaseAdress		$c00000


;includes with superfree sections go here:
	.include "routines/miscdata.asm"
	.include "routines/bgmodes.asm"
;	.include "routines/battlefiles.asm"
;	.include "routines/battlezscrolllists.asm"
;	.include "routines/battlezscrolllut.asm"
;	.include "routines/introscene3scrolltable.asm"
	.include "routines/oamvectoranglelut.asm"
	.include "routines/oamobjects.asm"
	.include "routines/oamanimationlists.asm"
;	.include "routines/hdmatables.asm"
	.include "routines/menufiles.asm"	
	.include "routines/textstrings.asm"
	.include "routines/spritedata.asm"
	.include "routines/songs.asm"
	.include "routines/samplepacks.asm"
	.include "routines/backgroundfiles.asm"
;	.include "routines/videoframes.asm"

;includes with fixed sections go here:	
	.include "routines/audiostreams.asm"
;	.include "routines/levelfiles.asm"	

.BANK 0 SLOT 0	; The SLOT 0 may be ommitted, as SLOT 0 is the DEFAULTSLOT
; === Cartridge Header - part 1 - =====================
.ORG    $7FC0 + HEADER_OFF
.DB     "OPTIXX TESTROM       "     ; Title (can't be more than 21 bytes, and should probably be a full 21 bytes)

;       "123456789012345678901"
.ORG    $7FD5 + HEADER_OFF
.DB     $31                         ; Memory Mode   ( $20 = Slow LoRom, $21 = Slow HiRom )

.BANK 0 SLOT 0
; === Cartridge Header - part 2 - =====================
.ORG    $7FD6 + HEADER_OFF
.DB     $00                   ; Contents ( $00 = ROM only, $01 = ROM and RAM, $02 = ROM and Save RAM)
.DB     $08                  ; ROM Size   ( $08 = 2 Mbit, $09 = 4 Mbit, $0A = 8Mbit, $0B = 16Mbit c=32, d=64... etc )
.DB     $00                   ; SRAM Size ( $00 = 0 bits, $01 = 16 kbits, $02 = 32 kbits, $03 = 64 kbits )
.DB     $01                   ; Country ( $01 = USA )
.DB     $33                   ; Licensee Code
.DB     $01                   ; Version
.dw		$0						;checksum
.dw		$ffff					;checksum xor. this must be filled out here, else wla dx thinks its free space, places something there and overwrites it with the checksum later.

.BANK 0 SLOT 0
; === Interrupt Vector Table ====================
.ORG    $7FE4 + HEADER_OFF   ; === Native Mode ===
.DW     EmptyHandler          ; COP
.DW     EmptyHandler          ; BRK
.DW     EmptyHandler          ; ABORT
.DW     NmiHookUp                ; NMI
.DW     $0000                 ; (Unused)
.DW     IrqHookUp          ; IRQ

.ORG    $7FF4 + HEADER_OFF   ; === Emulation Mode ===
.DW     EmptyHandler          ; COP
.DW     $0000                 ; (Unused)
.DW     EmptyHandler          ; ABORT
.DW     EmptyHandler          ; NMI
.DW     Boot           ; RESET
.DW     EmptyHandler          ; IRQ/BRK

; ============================================


	

.bank 0 slot 0
.org $ffa0
EmptyHandler:
	rti

Boot:
	SEI
	CLC
	XCE
	PHK
	PLB
	REP #$30
	SEP #$20
	STZ $4200		;reg $4200  - disable timers, NMI,and auto-joyread
	lda #%00000001
	sta $420d		;set memory mode to fastrom
	jml (HiromStart+BaseAdress) 		;lorom

NmiHookUp:
	rti
IrqHookUp:
	jml (IrqLoader+BaseAdress)



.bank 0 slot 0
.org $0
.Section "Main Code"
;functions to include go here:
	.include "routines/printstring.asm"
	.include "routines/menusystem.asm"
	.include "routines/gfxvrammisc.asm"
	.include "routines/vblanknmi.asm"
;	.include "routines/levelloader.asm"
;	.include "routines/bgscrolling.asm"
	.include "routines/dmafifo.asm"
	.include "routines/irq.asm"
	.include "routines/oammanager.asm"
	.include "routines/oamsubroutines.asm"
;	.include "routines/collisiondetection.asm"
	.include "routines/randomnumbergen.asm"
;	.include "routines/hdmahandler.asm"
;	.include "routines/hdmasubroutines.asm"
	.include "routines/spcinterface.asm"
	.include "routines/menusubroutines.asm"
;	.include "routines/videoplayer.asm"
	.include "routines/memoryclear.asm"
	.include "routines/eventroutines.asm"
	.include "routines/joypadread.asm"	


HiromStart:
	sep #$20
	LDA #$80
	STA $2100		;turn screen off for now, zero brightness

	rep #$31
	lda.w #$0000
	tcd

	lda.w #$01ff
	tcs

	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #$1f0
	ldx.w #0
	jsr ClearWRAM

	lda.b #0		;clear word: $0000
	ldy.w #$6000
	ldx.w #$200
	jsr ClearWRAM

	LDX #$2101

MemClearLoop1:		;regs $2101-$210C
	STZ $00,X		;set Sprite,Character,Tile sizes to lowest, and set addresses to $0000
	INX
	CPX #$210D
	BNE MemClearLoop1

MemClearLoop2:		;regs $210D-$2114
	STZ $00,X		;Set all BG scroll values to $0000
	STZ $00,X
	INX
	CPX #$2115
	BNE MemClearLoop2
	
	LDA #$80		;reg $2115
	STA $2115		; Initialize VRAM transfer mode to word-access, increment by 1
	STZ $2116		;regs $2117-$2117
	STZ $2117		;VRAM address = $0000
	STZ $211A		;clear Mode7 setting
	LDX #$211B

MemClearLoop3:		;regs $211B-$2120
	STZ $00,X		;clear out the Mode7 matrix values
	STZ $00,X
	INX
	CPX #$2121
	BNE MemClearLoop3
	LDX #$2123

MemClearLoop4:		;regs $2123-$2133
	STZ $00,X		;turn off windows, main screens, sub screens, color addition,
	INX			;fixed color = $00, no super-impose (external synchronization),
	CPX #$2134		;no interlaced mode, normal resolution
	BNE MemClearLoop4
	
	LDA #$FF
	STA $4201		;reg $4201  - programmable I/O write port, initalize to allow reading at in-port
	nop
	nop
	LDA $4210		;reg $4210  - NMI status, reading resets
	lda $4211		;irq status reset

;clear all graphics buffers:
	jsr ClearPalette   ;Reset colors
	jsr ClearPaletteBuffer
	jsr ClearBg1TilemapBuffer
	jsr ClearBg2TilemapBuffer
	jsr InitDmaFifo
	jsr InitOam
	jsr ClearColObjList
	jsr ClearZBuffer
;init variables:
	rep #$31
	lda.w #$1234			;seed RNG
	sta.b R1
	lda.w #$55aa
	sta.b R2

	sep #$20
	lda #%00000001		;enable screen, auto joypad
	sta.w InterruptEnableFlags
	sta.w $4200
	lda.w SetIni			;set display mode 
	sta.w $2133			;dont set this during nmi cause if the overscan flag is changed mid-nmi, it might result in screw ups with the nmi timing
	cli
	jsr ROMToWRAM
    jml (go+$7f0000)
go:
    jsr EnableScreen
    jsr SpcPlaySong
/*		
;main loop starts here:
CheckNextFrame:
	ldx.w FrameCounterLo	;load current frame counter
	cpx.w LastFrameLo	;load last frame processed
	beq CheckNextFrame	;check until one frame advances
	stx.w LastFrameLo
*/

;main loop starts here:
CheckNextFrame:

	lda.w $4210				;wait until out of nmi
	bmi CheckNextFrame

CheckNextNMI:	
	lda.w $4210				;wait until in nmi
	bpl CheckNextNMI
	
	jsr NMI

	lda CurrentEvent		;load number of currently active scene
	asl				;multiply number by 2
	rep #$31			;set accu to 16bit
	and.w #$00ff			;only use low byte
	tax				;transfer to x
	
	sep #$20			;reset accu to 8bit
	lda.w SetIni			;set display mode
	sta.l $2133			;dont set this during nmi cause if the overscan flag is changed mid-nmi, it might result in screw ups with the nmi timing


	php
	jsr (EventPtTable,x)	;and jump to the location found at pointertable where x is the pointernumber
	plp
	lda.b #$80
	sta.l $4201
	nop
	nop
	nop
	nop
	lda.l $2137
	lda.l $213f			;reset $213d to low byte
	lda.l $213d			;get current scanline
	sta.w CpuUsageScanline 
	bra CheckNextFrame


.ends
























