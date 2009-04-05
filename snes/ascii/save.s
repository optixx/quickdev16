.EMPTYFILL 0
.LOROM
		  

.MEMORYMAP
SLOTSIZE 	$8000
DEFAULTSLOT 0
SLOT 		0 $0000	; ram , direct page   
SLOT 		1 $2000 ; PPU1, APU 
SLOT 		2 $3000 ; SFX, DSP
SLOT		3 $4000 ; Controller
SLOT		4 $4200 ; PPU2, DMA 
SLOT		5 $6000 ; RESERVED
SLOT		6 $8000 ; code segment 
.ENDME

;.ROMBANKSIZE $8000 
;.ROMBANKS $80

.ROMBANKMAP
BANKSTOTAL $80
BANKSIZE $8000
BANKS $80
.ENDRO


.NAME "optixx"

.BANK $00 SLOT 6
.ORG	$0000 
.ORGA	$8000



.SECTION "MAIN" 

.define	dp			$0000 

.define	xstorage1	$0001
.define	xstorage2	$0002
.define sineoffset	$0003
.define	scrollval	$0005
.define	col_reg		$0007
.define hdma_table0	$0010
.define	plane_0		$0400
.define	plane_1 	$0800 
.define	tile 		$2000 


init:   
		sei         	;stop interrupts
		phk             ;get the current bank and store on stack
		plb             ;get value off stack and make it the current
						;programming bank
		clc             ;clear carry bit
		xce             ;native 16 bit mode (no 6502 emulation!) 

		jsr				init_hdma_table0
;==========================================================================
;               start of snes register initialization
;==========================================================================

	   sep #$30     ; x,y,a are 8 bit numbers
	   lda #$8f     ; screen off, full brightness
	   sta $2100    ; brightness + screen enable register 
	   lda #$00     ; 
	   sta $2101    ; sprite register (size + address in vram)
	   lda #$00
	   sta $2102    ; sprite registers (address of sprite memory [oam])
	   sta $2103    ;    ""                       ""
	   lda #$00     ; mode 0
	   sta $2105    ; graphic mode register
	   lda #$00     ; no planes, no mosaic
	   sta $2106    ; mosaic register
	   lda #$00     ; 
	   sta $2107    ; plane 0 map vram location
	   lda #$00
	   sta $2108    ; plane 1 map vram location
	   lda #$00
	   sta $2109    ; plane 2 map vram location
	   lda #$00
	   sta $210a    ; plane 3 map vram location
	   lda #$00
	   sta $210b    ; plane 0+1 tile data location
	   lda #$00
	   sta $210c    ; plane 2+3 tile data location
	   lda #$00
	   sta $210d    ; plane 0 scroll x (first 8 bits)
	   sta $210d    ; plane 0 scroll x (last 3 bits) #$0 - #$07ff
	   sta $210e    ; plane 0 scroll y (first 8 bits)
	   sta $210e    ; plane 0 scroll y (last 3 bits) #$0 - #$07ff
	   sta $210f    ; plane 1 scroll x (first 8 bits)
	   sta $210f    ; plane 1 scroll x (last 3 bits) #$0 - #$07ff
	   sta $2110    ; plane 1 scroll y (first 8 bits)
	   sta $2110    ; plane 1 scroll y (last 3 bits) #$0 - #$07ff
	   sta $2111    ; plane 2 scroll x (first 8 bits)
	   sta $2111    ; plane 2 scroll x (last 3 bits) #$0 - #$07ff
	   sta $2112    ; plane 2 scroll y (first 8 bits)
	   sta $2112    ; plane 2 scroll y (last 3 bits) #$0 - #$07ff
	   sta $2113    ; plane 3 scroll x (first 8 bits)
	   sta $2113    ; plane 3 scroll x (last 3 bits) #$0 - #$07ff
	   sta $2114    ; plane 3 scroll y (first 8 bits)
	   sta $2114    ; plane 3 scroll y (last 3 bits) #$0 - #$07ff
	   lda #$80     ; increase vram address after writing to $2119
	   sta $2115    ; vram address increment register
	   lda #$00
	   sta $2116    ; vram address low
	   sta $2117    ; vram address high
	   sta $211a    ; initial mode 7 setting register
	   sta $211b    ; mode 7 matrix parameter a register (low)
	   lda #$01
	   sta $211b    ; mode 7 matrix parameter a register (high)
	   lda #$00
	   sta $211c    ; mode 7 matrix parameter b register (low)
	   sta $211c    ; mode 7 matrix parameter b register (high)
	   sta $211d    ; mode 7 matrix parameter c register (low)
	   sta $211d    ; mode 7 matrix parameter c register (high)
	   sta $211e    ; mode 7 matrix parameter d register (low)
	   lda #$01
	   sta $211e    ; mode 7 matrix parameter d register (high)
	   lda #$00
	   sta $211f    ; mode 7 center position x register (low)
	   sta $211f    ; mode 7 center position x register (high)
	   sta $2120    ; mode 7 center position y register (low)
	   sta $2120    ; mode 7 center position y register (high)
	   sta $2121    ; color number register ($0-ff)
	   sta $2123    ; bg1 & bg2 window mask setting register
	   sta $2124    ; bg3 & bg4 window mask setting register
	   sta $2125    ; obj & color window mask setting register
	   sta $2126    ; window 1 left position register
	   sta $2127    ; window 2 left position register
	   sta $2128    ; window 3 left position register
	   sta $2129    ; window 4 left position register
	   sta $212a    ; bg1, bg2, bg3, bg4 window logic register
	   sta $212b    ; obj, color window logic register (or,and,xor,xnor)
	   lda #$01
	   sta $212c    ; main screen designation (planes, sprites enable)
	   lda #$00
	   sta $212d    ; sub screen designation
	   lda #$00
	   sta $212e    ; window mask for main screen
	   sta $212f    ; window mask for sub screen
	   lda #$30
	   sta $2130    ; color addition & screen addition init setting
	   lda #$00
	   sta $2131    ; add/sub sub designation for screen, sprite, color
	   lda #$e0
	   sta $2132    ; color data for addition/subtraction
	   lda #$00
	   sta $2133    ; screen setting (interlace x,y/enable sfx data)
	   lda #$00
	   sta $4200    ; enable v-blank, interrupt, joypad register
	   lda #$ff
	   sta $4201    ; programmable i/o port
	   lda #$00
	   sta $4202    ; multiplicand a
	   sta $4203    ; multiplier b
	   sta $4204    ; multiplier c
	   sta $4205    ; multiplicand c
	   sta $4206    ; divisor b
	   sta $4207    ; horizontal count timer
	   sta $4208    ; horizontal count timer msb (most significant bit)
	   sta $4209    ; vertical count timer
	   sta $420a    ; vertical count timer msb
	   sta $420b    ; general dma enable (bits 0-7)
	   sta $420c    ; horizontal dma (hdma) enable (bits 0-7)
	   sta $420d    ; access cycle designation (slow/fast rom)
;===========================================================================
;                        end of init routine
;===========================================================================
	rep     #$30    ; x,y,a fixed -> 16 bit mode
	sep     #$20    ; accumulator ->  8 bit mode

	lda		#(dp+0) ;load direct page 
	tcd 			;store & and assign 

	
	
	lda     #(plane_0>>8)   ; screen map data @ vram location $1000
	sta     $2107           ; plane 0 map location register
	lda     #(plane_1>>8)  	; screen map data @ vram location $1000
	sta     $2108           ; plane 1 map location register
	
	lda     #$22            ; plane 0 and plane 1 tile graphics @ $2000  
	sta     $210b           ; plane 0 tile graphics register
	lda     #$00            ; mode 0 value / tile mode
	sta     $2105           ; graphics mode register
	
	lda     #%00000011      ; 
	sta     $212c           ; plane enable register
	
	
	
	lda     #$00
	sta     $2121           ; set color number to 0 (background)
	lda     #$46            ; blue color, lower 8 bits
	sta     $2122           ; enter color value #$46 to color num. (low)
	lda     #$69            ; blue color, higher 8 bits
	sta     $2122           ; enter color value #$69 to color num. (high)
	lda     #$ff            ; white color, lower 8 bits
	sta     $2122           ; write to next color number (01)
	sta     $2122           ; enter same value to color number (01)

	ldx		#$00 



col_loop:
	txa	
	sta     $2121
	lda		#$00
	sta		$2122
	lda		#$7f 
	sta		$2122 
	txa		
	ina
	tax
	cmp		#$ff
	beq 	col_loop 	

	
	
	lda     #$01
	sta     $4200           ; enable joypad read (bit one)


;==========================================================================
;                      start transfer of graphics to vram
;==========================================================================


	ldx		#(tile+0)		; assign vram location 
	stx     $2116           ; writing to $2118/9 will store data here!
	ldx     #$0000

copychar:

	lda.w   charset,x       ; get character set data (font data)
	sta     $2118         	; store bitplane 1
	stz     $2119           ; clear bitplane 2 and increase vram address
	inx
	cpx     #$0200          ; transfer $0200 bytes
	bne     copychar


init_plane_0:
	ldx     #(plane_0+0)         ; assign vram location $1000 to $2116/7
	stx     $2116
	ldx     #$0000


init_plane_0_loop:
	lda.w  	text_0,x       ; get ascii text data
	and     #$3f            ; we only want the first 64 characters
							; convert ascii to c64 screen code
	sta     $2118
	stz     $2119           ; clear unwanted bits, no h/v flipping
	inx
	cpx     #$0400          ; transfer entire screen
							; $20*$20=$0400  (1024 bytes)
	bne    init_plane_0_loop 


init_plane_1:
	ldx     #(plane_1+0)          ; assign vram location $1000 to $2116/7
	stx     $2116
	ldx     #$0000


init_plane_1_loop:
	lda.w  	text_1,x       ; get ascii text data
	and     #$3f            ; we only want the first 64 characters
							; convert ascii to c64 screen code
	sta     $2118
	stz     $2119           ; clear unwanted bits, no h/v flipping
	inx
	cpx     #$0400          ; transfer entire screen
							; $20*$20=$0400  (1024 bytes)
	bne    	init_plane_1_loop 



init_screen:
	lda     #$0f            ; screen enabled, full brightness
	sta     $2100           ; 
	cli                     ; clear interrupt bit



init_scroll:
	lda		#$00
	sta.w	scrollval		


init_col_reg:
	lda 	#$00
	sta.w 	col_reg 


init_sineoffset:
	lda		#$00
	sta.w	sineoffset


call_hmda_setup:
	jsr 	init_hdma_table0 



	jmp  intro 

main:
	jsr 	wait_vbl
	jsr		sine_plane 
	jsr		scroll_plane
	jsr		cycle_color 
	jsr		joypad
	jmp 	main 

	


; test vertical interrupt 


wait_vbl: 
	lda     $4210           ; check for vertical blank
	and     #$80
	beq   	wait_vbl 
	rts

; joypad poll 

joypad:
	lda     $4212           ; is joypad ready to be read?
	and     #$0001
	bne     joypad          ; no? go back until it is! 
	lda     $4219           ; read joypad high byte
	and     #$10            ; leave only "start" bit
	bne     reset           ; "start" pressed? go to reset
	rts            			; if not then jump back to loop
reset:
	sep     #$30
	lda     #$00
	pha                     ; push #$00 to stack
	plb                     ; pull #$00 from stack and make it the
							; the programming bank
	jmp   	init           	; jump long to $008000



; gfx routine

; intro stuff 

intro:
	rep		#$30
	sep		#$20 
	ldx 	#$f1
mosaic_l:
	jsr		wait_vbl 
	txa		
	sta 	$2106
	sbc 	#$10
	tax
	cmp		#$01
	bne 	mosaic_l
	stz 	$2106

fade:
	ldy		#$0f
fade_dark:
	jsr		wait_vbl
	tya
	sta		$2100
	dey		
	cpy		#$0000
	bne		fade_dark

	ldy		#$0000
fade_light:
	jsr		wait_vbl 
	tya
	sta		$2100
	iny		
	cpy		#$000f
	bne		fade_light 

	jmp 	main 

; scroll loop 

scroll_plane:
	lda.w	scrollval
	sta    	$210e  
	stz    	$210e
	

	adc		#$01
	
	sta.w 	scrollval
	cmp    	#$ff
	beq    	restore_scroll	
	rts
restore_scroll:
	lda 	#$00
	sta.w	scrollval 
	rts 	 

; cycle loop 

cycle_color:
	ldx		#$0000
	lda.w  	col_reg,x
	adc		#$01
	stz 	$2121
	sta		$2122
	stz		$2122
	sta		col_reg
	cmp		#$7f
	bne		cycle_c
	lda		#$0000
	sta		col_reg 
cycle_c:			
	rts 


sine_plane:
	rep		#$10
	sep		#$20
	
	lda		sineoffset
	ina
	sta		sineoffset
	cmp		#$ff
	bne		sine_plane_c
	lda		#$00
	sta		sineoffset
	


sine_plane_c:
	tay 	
	ldx		#$0000 
	
sine_plane_l:	
	iny
	cpy		#$ff
	bne		sine_plane_l_c
	lda		#$00
	tay

sine_plane_l_c:
	inx	
	lda.w	vsine,y
	sta		hdma_table0+3,x	
	inx
	inx
	cpx		#$0180
	bne		sine_plane_l 
	
	rep		#$30
	sep		#$20
	rts 
	



; init hdma list 


init_hdma_table0:
	
	rep 	#$10    	
	sep 	#$20    
	
	ldy		#$0000			
	ldx		#$0000

	;lda		#$00
	;sta		sineoffset
	
	lda		#$30	
	sta		hdma_table0,x
	inx
	lda 	#$00	
	sta		hdma_table0,x
	inx			
	sta		hdma_table0,x
	inx

init_hdma_table0_loop:
	
	lda		#$01	
	sta		hdma_table0,x
	inx
	
	lda		vsine,y
	sta		hdma_table0,x
	inx
	
	lda		#$00
	sta		hdma_table0,x
	inx	
	iny 
	cpx		#$0183 ; (128 + 1) * 3 = 387 = 0x0183
	bne 	init_hdma_table0_loop
	
	lda		#$20
	sta		hdma_table0,x
	inx		
	lda		#$00	
	sta		hdma_table0,x
	inx 
	sta		hdma_table0,x 
	inx	
	sta		hdma_table0,x
	inx
	sta		hdma_table0,x
	
	
	lda     #$02
	sta     $4300
	lda     #$0f
	sta     $4301
	ldx.w	#hdma_table0
	;ldx.w  #test_hmda_table
	stx     $4302
	lda 	#$00	
	sta     $4304

    lda 	#%00000001
	sta 	$420c
		  
	rep		#$30
	sep		#$20 

	rts	


charset:

	.db    $55,$aa,$55,$aa,$55,$aa,$55,$aa ;'@'
	.db    $00,$3c,$66,$7e,$66,$66,$66,$00 ;'a'
	.db    $00,$7c,$66,$7c,$66,$66,$7c,$00 ;'b'
	.db    $00,$3c,$66,$60,$60,$66,$3c,$00 ;'c'
	.db    $00,$78,$6c,$66,$66,$6c,$78,$00 ;'d'
	.db    $00,$7e,$60,$78,$60,$60,$7e,$00 ;'e'
	.db    $00,$7e,$60,$78,$60,$60,$60,$00 ;'f'
	.db    $00,$3c,$66,$60,$6e,$66,$3c,$00 ;'g'
	.db    $00,$66,$66,$7e,$66,$66,$66,$00 ;'h'
	.db    $00,$3c,$18,$18,$18,$18,$3c,$00 ;'i'
	.db    $00,$1e,$0c,$0c,$0c,$6c,$38,$00 ;'j'
	.db    $00,$6c,$78,$70,$78,$6c,$66,$00 ;'k'
	.db    $00,$60,$60,$60,$60,$60,$7e,$00 ;'l'
	.db    $00,$63,$77,$7f,$6b,$63,$63,$00 ;'m'
	.db    $00,$66,$76,$7e,$7e,$6e,$66,$00 ;'n'
	.db    $00,$3c,$66,$66,$66,$66,$3c,$00 ;'o'
	.db    $00,$7c,$66,$66,$7c,$60,$60,$00 ;'p'
	.db    $00,$3c,$66,$66,$66,$3c,$0e,$00 ;'q'
	.db    $00,$7c,$66,$66,$7c,$6c,$66,$00 ;'r'
	.db    $00,$3e,$60,$3c,$06,$66,$3c,$00 ;'s'
	.db    $00,$7e,$18,$18,$18,$18,$18,$00 ;'t'
	.db    $00,$66,$66,$66,$66,$66,$3c,$00 ;'u'
	.db    $00,$66,$66,$66,$66,$3c,$18,$00 ;'v'
	.db    $00,$63,$63,$6b,$7f,$77,$63,$00 ;'w'
	.db    $00,$66,$3c,$18,$3c,$66,$66,$00 ;'x'
	.db    $00,$66,$66,$3c,$18,$18,$18,$00 ;'y'
	.db    $00,$7e,$0c,$18,$30,$60,$7e,$00 ;'z'
	.db    $00,$3c,$30,$30,$30,$30,$3c,$00 ;'['
	.db    $c0,$60,$30,$18,$0c,$06,$03,$00 ;'|'
	.db    $00,$3c,$0c,$0c,$0c,$0c,$3c,$00 ;']'
	.db    $10,$38,$6c,$c6,$00,$00,$00,$00 ;'^'
	.db    $00,$00,$00,$00,$00,$00,$00,$fe ;'_'
	.db    $00,$00,$00,$00,$00,$00,$00,$00 ;' '
	.db    $00,$18,$18,$18,$00,$00,$18,$00 ;'!'
	.db    $00,$66,$66,$00,$00,$00,$00,$00 ;'"'
	.db    $00,$66,$ff,$66,$ff,$66,$00,$00 ;'#'
	.db    $00,$08,$1c,$28,$28,$1c,$08,$00 ;'$'
	.db    $00,$64,$6c,$18,$30,$6c,$4c,$00 ;'%'
	.db    $00,$00,$18,$18,$7e,$18,$18,$00 ;'&'
	.db    $00,$0c,$18,$00,$00,$00,$00,$00 ;'''
	.db    $00,$18,$30,$30,$30,$18,$0c,$00 ;'('
	.db    $00,$18,$0c,$0c,$0c,$18,$30,$00 ;')'
	.db    $00,$66,$3c,$ff,$3c,$66,$00,$00 ;'*'
	.db    $00,$18,$18,$7e,$18,$18,$00,$00 ;'+'
	.db    $00,$00,$00,$00,$00,$18,$18,$30 ;','
	.db    $00,$00,$00,$fe,$00,$00,$00,$00 ;'-'
	.db    $00,$00,$00,$00,$00,$18,$18,$00 ;'.'
	.db    $03,$06,$0c,$18,$30,$60,$c0,$00 ;'/'
	.db    $00,$3c,$66,$6e,$76,$66,$3c,$00 ;'0'
	.db    $00,$18,$38,$18,$18,$18,$7e,$00 ;'1'
	.db    $00,$7c,$06,$0c,$30,$60,$7e,$00 ;'2'
	.db    $00,$7e,$06,$1c,$06,$66,$3c,$00 ;'3'
	.db    $00,$0e,$1e,$36,$7f,$06,$06,$00 ;'4'
	.db    $00,$7e,$60,$7c,$06,$66,$3c,$00 ;'5'
	.db    $00,$3e,$60,$7c,$66,$66,$3c,$00 ;'6'
	.db    $00,$7e,$06,$0c,$0c,$0c,$0c,$00 ;'7'
	.db    $00,$3c,$66,$3c,$66,$66,$3c,$00 ;'8'
	.db    $00,$3c,$66,$3e,$06,$66,$3c,$00 ;'9'
	.db    $00,$00,$18,$00,$00,$18,$00,$00 ;':'
	.db    $00,$00,$18,$00,$00,$18,$18,$30 ;';'
	.db    $18,$18,$18,$18,$18,$18,$18,$00 ;'<'
	.db    $00,$00,$7e,$00,$7e,$00,$00,$00 ;'='
	.db    $18,$18,$0c,$0c,$0c,$0c,$18,$18 ;'>'
	.db    $00,$7c,$06,$0c,$18,$00,$18,$00 ;'?'
	  
		  ; 12345678901234567890123456789012
text_0:
 	.db		"|             _   _            |"
 	.db		"|  ___  _ __ | |_(_)_  ____  __|"
 	.db		"| / _ \| '_ \| __| \ \/ /\ \/ /|"
 	.db		"|| (_) | |_) | |_| |>  <  >  < |"
 	.db		"| \___/| .__/ \__|_/_/\_\/_/\_\|"
 	.db		"|      |_|                     |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|     OPTIXX ONCE A AGAIN      |"
 	.db		"|    WITH A CLASSIS CONSOL     |"
 	.db		"|       PIECE OF CODE...       |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"
 	.db		"|                              |"


text_1:

.REPT 20

 	.db		"|             _   _            |"
 	.db		"|  ___  _ __ | |_(_)_  ____  __|"
 	.db		"| / _ \| '_ \| __| \ \/ /\ \/ /|"
 	.db		"|| (_) | |_) | |_| |>  <  >  < |"
 	.db		"| \___/| .__/ \__|_/_/\_\/_/\_\|"
 	.db		"|      |_|                     |"
 	.db		"|                              |"
 
.ENDR

vsine:
	.db  108,109,111,112,113,114,115,115,116,117,118,119,120,120,121
	.db  122,122,123,123,124,124,125,125,125,126,126,126,127,127,127
	.db  127,127,127,127,127,127,127,127,126,126,126,125,125,125,124
	.db  124,123,123,122,122,121,120,120,119,118,117,116,115,115,114
	.db  113,112,111,109,108,107,106,105,104,103,101,100,99,97,96,95
	.db  93,92,91,89,88,86,85,83,82,80,79,77,76,74,73,71,70,68,67,65
	.db  64,62,60,59,57,56,54,53,51,50,48,47,45,44,42,41,39,38,36,35
	.db  34,32,31,30,28,27,26,24,23,22,21,20,19,18,16,15,14,13,12,12
	.db  11,10,9,8,7,7,6,5,5,4,4,3,3,2,2,2,1,1,1,0,0,0,0,0,0,0,0,0,0
	.db  0,1,1,1,2,2,2,3,3,4,4,5,5,6,7,7,8,9,10,11,12,12,13,14,15,16
	.db  18,19,20,21,22,23,24,26,27,28,30,31,32,34,35,36,38,39,41,42
	.db  44,45,47,48,50,51,53,54,56,57,59,60,62,64,65,67,68,70,71,73
	.db  74,76,77,79,80,82,83,85,86,88,89,91,92,93,95,96,97,99,100,101
	.db  103,104,105,106,107
vsine_end:
.ENDS



.BANK $04 SLOT 6
.ORGA $8000
long_label:
	nop
	nop
	rts 




.BANK $01 SLOT 6
.ORGA $8000
.BASE $01
.INCLUDE "music.s" 

.BANK $02 SLOT 6
.ORG $0000 
music_data_1:
.INCBIN "music1.bin"

.BANk $03 SLOT 6 
.ORG $0000 
music_data_2:
.INCBIN "music2.bin" 



