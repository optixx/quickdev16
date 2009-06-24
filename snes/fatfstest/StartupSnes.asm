; SNES ROM startup code 

;******************************************************************************
;*** Define a special section in case most of the code is not in bank 0.    ***
;******************************************************************************

;STACK   EQU   $01ff     ;CHANGE THIS FOR YOUR SYSTEM

;STARTUP SECTION OFFSET $008000

CODE

	XDEF  	START
START:
        XREF  	~~main
  	
	sei             ; Disabled interrupts
        clc             ; clear carry to switch to native mode
        xce             ; Xchange carry & emulation bit. native mode
        rep     #$18    ; Binary mode (decimal mode off), X/Y 16 bit
	LONGI	ON
        ldx     #$1FFF  ; set stack to $1FFF
        txs

	rep 	#$30
	longa	on
	longi	on

	; Init data used for heap 
	; see heap definition below
	XREF ~~_heap_top
	XREF ~~_mem_start
	stz	~~_heap_top
	stz	~~_mem_start
	
	XREF	~~preInit
        jsr   	>~~preInit

        sep     #$30    ; X,Y,A are 8 bit numbers
	LONGA	OFF
	LONGI	OFF	
        lda     #$8F    ; screen off, full brightness
        sta     $2100   ; brightness + screen enable register 
        stz     $2101   ; Sprite register (size + address in VRAM) 
        stz     $2102   ; Sprite registers (address of sprite memory [OAM])
        stz     $2103   ;    ""                       ""
        stz     $2105   ; Mode 0, = Graphic mode register
        stz     $2106   ; noplanes, no mosaic, = Mosaic register
        stz     $2107   ; Plane 0 map VRAM location
        stz     $2108   ; Plane 1 map VRAM location
        stz     $2109   ; Plane 2 map VRAM location
        stz     $210A   ; Plane 3 map VRAM location
        stz     $210B   ; Plane 0+1 Tile data location
        stz     $210C   ; Plane 2+3 Tile data location
        stz     $210D   ; Plane 0 scroll x (first 8 bits)
        stz     $210D   ; Plane 0 scroll x (last 3 bits) #$0 - #$07ff
        stz     $210E   ; Plane 0 scroll y (first 8 bits)
        stz     $210E   ; Plane 0 scroll y (last 3 bits) #$0 - #$07ff
        stz     $210F   ; Plane 1 scroll x (first 8 bits)
        stz     $210F   ; Plane 1 scroll x (last 3 bits) #$0 - #$07ff
        stz     $2110   ; Plane 1 scroll y (first 8 bits)
        stz     $2110   ; Plane 1 scroll y (last 3 bits) #$0 - #$07ff
        stz     $2111   ; Plane 2 scroll x (first 8 bits)
        stz     $2111   ; Plane 2 scroll x (last 3 bits) #$0 - #$07ff
        stz     $2112   ; Plane 2 scroll y (first 8 bits)
        stz     $2112   ; Plane 2 scroll y (last 3 bits) #$0 - #$07ff
        stz     $2113   ; Plane 3 scroll x (first 8 bits)
        stz     $2113   ; Plane 3 scroll x (last 3 bits) #$0 - #$07ff
        stz     $2114   ; Plane 3 scroll y (first 8 bits)
        stz     $2114   ; Plane 3 scroll y (last 3 bits) #$0 - #$07ff
        lda     #$80    ; increase VRAM address after writing to $2119
        sta     $2115   ; VRAM address increment register
        stz     $2116   ; VRAM address low
        stz     $2117   ; VRAM address high
        stz     $211A   ; Initial Mode 7 setting register
        stz     $211B   ; Mode 7 matrix parameter A register (low)
        lda     #$01
        sta     $211B   ; Mode 7 matrix parameter A register (high)
        stz     $211C   ; Mode 7 matrix parameter B register (low)
        stz     $211C   ; Mode 7 matrix parameter B register (high)
        stz     $211D   ; Mode 7 matrix parameter C register (low)
        stz     $211D   ; Mode 7 matrix parameter C register (high)
        stz     $211E   ; Mode 7 matrix parameter D register (low)
        sta     $211E   ; Mode 7 matrix parameter D register (high)
        stz     $211F   ; Mode 7 center position X register (low)
        stz     $211F   ; Mode 7 center position X register (high)
        stz     $2120   ; Mode 7 center position Y register (low)
        stz     $2120   ; Mode 7 center position Y register (high)
        stz     $2121   ; Color number register ($0-ff)
        stz     $2123   ; BG1 & BG2 Window mask setting register
        stz     $2124   ; BG3 & BG4 Window mask setting register
        stz     $2125   ; OBJ & Color Window mask setting register
        stz     $2126   ; Window 1 left position register
        stz     $2127   ; Window 2 left position register
        stz     $2128   ; Window 3 left position register
        stz     $2129   ; Window 4 left position register
        stz     $212A   ; BG1, BG2, BG3, BG4 Window Logic register
        stz     $212B   ; OBJ, Color Window Logic Register (or,and,xor,xnor)
        sta     $212C   ; Main Screen designation (planes, sprites enable)
        stz     $212D   ; Sub Screen designation
        stz     $212E   ; Window mask for Main Screen
        stz     $212F   ; Window mask for Sub Screen
        lda     #$30
        sta     $2130   ; Color addition & screen addition init setting
        stz     $2131   ; Add/Sub sub designation for screen, sprite, color
        lda     #$E0
        sta     $2132   ; color data for addition/subtraction
        stz     $2133   ; Screen setting (interlace x,y/enable SFX data)
        stz     $4200   ; Enable V-blank, interrupt, Joypad register
        lda     #$FF
        sta     $4201   ; Programmable I/O port
        stz     $4202   ; Multiplicand A
        stz     $4203   ; Multiplier B
        stz     $4204   ; Multiplier C
        stz     $4205   ; Multiplicand C
        stz     $4206   ; Divisor B
        stz     $4207   ; Horizontal Count Timer
        stz     $4208   ; Horizontal Count Timer MSB (most significant bit)
    stz     $4209   ; Vertical Count Timer
    stz     $420A   ; Vertical Count Timer MSB
    stz     $420B   ; General DMA enable (bits 0-7)
    stz     $420C   ; Horizontal DMA (HDMA) enable (bits 0-7)
    stz     $420D   ; Access cycle designation (slow/fast rom)
    cli             ; Enable interrupts


        ;;lda     #$01
        ;;sta     $4200

	rep     #$30
	longa	on
	longi	on

    jsr   	>~~main
    brk

	XDEF IRQ
IRQ:	
    ;LONGA   ON
    ;LONGI   ON
    rep #$10
    sep #$20
    lda #65
    sta $3000
    rep #$30
    ;LONGA   OFF
    ;LONGI   OFF
    rti


    XREF ~~IRQHandler
	LONGA	ON
	LONGI	ON
	rep	#$30
	pha
	phx
	phy
	jsr	~~IRQHandler
	ply
	plx
	pla
	rti
	
	XDEF	NMI
NMI:
    rep #$30
    lda #66
    sta $3000
    sep     #$30    ; X,Y,A are 8 bit numbers
    rti
	XREF	~~NMIHandler
	LONGA	ON
	LONGI	ON
	rep	#$30
	pha
	phx
	phy
	phd
	phb
	lda	#$0000
	sep     #$30    ; X,Y,A are 8 bit numbers
	LONGA	OFF
	LONGI	OFF
	lda     $4210		; Read NMI 
	LONGA	ON
	LONGI	ON
	rep	#$30
	jsr	~~NMIHandler
	plb
	pld
	ply
	plx
	pla
	rti
  
DIRQ:
	rti

ENDS

;******************************************************************************
;*** Heap definition                                                        ***
;******************************************************************************

DATA

	XDEF	~~heap_start
	XDEF	~~heap_end

~~heap_start:
	WORD	$1e00
~~heap_end:
	WORD	$2000

;******************************************************************************
;*** SNES ROM Registartion Data                                             ***
;******************************************************************************

REGISTRATION_DATA SECTION

MAKER_CODE          FCC	/FF/
GAME_CODE		    FCC	/SMWJ/
FIXED_VALUE0		BYTE	$00, $00, $00, $00, $00, $00, $00
EXPANSION_RAM_SIZE	BYTE	$00
SPECIAL_VERSION		BYTE	$00
CARTRIDGE_TYPE_SUB	BYTE	$00
GAME_TITLE		    FCC	/GAME TITLE          !/
				        ;012345678901234567890;
MAP_MODE            BYTE	$20
CARTRIDGE_SIZE		BYTE	$00
ROM_SIZE            BYTE	$09
RAM_SIZE            BYTE	$00
DESTINATION_CODE	BYTE	$00
FIXED_VALUE1		BYTE	$33
MASK_ROM_VERSION	BYTE	$00
COMPLEMENT_CHECK	BYTE	$00, $00
CHEKSUM             BYTE	$00, $00 

;******************************************************************************
;*** SNES Interrupts and Reset vector                                       ***
;******************************************************************************

VECTORS	SECTION
; Native vector
N_COP   DW   DIRQ
N_BRK   DW   DIRQ
N_ABORT DW   DIRQ        
N_NMI   DW   NMI
N_RSRVD DW   DIRQ
N_IRQ   DW   IRQ
        DS   4
; Emulation vector
E_COP   DW   DIRQ
E_RSRVD DW   DIRQ
E_ABORT DW   DIRQ
E_NMI   DW   DIRQ
E_RESET DW   START
E_IRQ   DW   DIRQ

END

