;============================================================================
; Includes
;============================================================================

;== Include MemoryMap, Vector Table, and HeaderInfo ==
.INCLUDE "header.inc"




.SECTION "MAIN" 

.define	dp					$0000 

.define	sineswap			$0002
.define sine_offset			$0003
.define	scrollval			$0005
.define	colbar_offset_1		$0006
.define	colbar_offset_2		$0007
.define	colbar_offset_3		$0008


.define	colbar_offset_table	$0010
.define	colbar_color_table	$0020 
.define	colbar_count		$0030
.define	colbar_color		$0032
.define	colbar_color_ptr	$0034



.define hdma_table0			$0050

.define hdma_table1			$0200
.define	plane_0				$0800
.define	plane_1 			$0c00 
.define	char_data			$2000 
.define	logo_data			$2200


init:   
	sei         	;stop interrupts
	phk             ;get the current bank and store on stack
	plb             ;get value off stack and make it the current
					;programming bank
	clc             ;clear carry bit
	xce             ;native 16 bit mode (no 6502 emulation!) 


	jsr		setup 


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
	
	
	lda     #$01
	sta     $4200           ; enable joypad read (bit one)
	
	lda		#$00 
	sta		$2121
	ldx		#$0000		
	
	
col_loop:
	lda		#$ff
	sta		$2122
	lda		#$7f 
	sta		$2122 
	inx
	cpx		#$00ff
	bne 	col_loop 	

	

	ldx.w	#char_data		; assign vram location 
	stx     $2116           ; writing to $2118/9 will store data here!
	ldx     #$0000

copychar:

	lda.w  	charset,x       ; get character set data (font data)
	sta     $2118         	; store bitplane 1
	stz     $2119           ; clear bitplane 2 and increase vram address
	inx
	cpx     #$0200          ; transfer $0200 bytes
	bne     copychar


	ldx		#$0000
copy_logo:					; copy tile data to vram
	lda.w	optixx_logo,x	; using continuos tile vram pointer
	sta     $2118         	; charset @ $2000
	stz     $2119           ; logo  @ $2200 
	inx
	cpx     #$0500           
	bne     copy_logo

	



init_plane_0:
	ldx.w   #plane_0         ; assign vram location $1000 to $2116/7
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


init_plane_1:				; write optixx logo tiles
	ldx.w   #plane_1          
	stx     $2116
	ldx     #$0000
	lda		#$0040 	

init_plane_1_clear_1:		; 0x0140 tiles clear
	sta		$2118
	stz		$2119
	inx
	cpx     #$0140
	bne  	init_plane_1_clear_1
	ldx     #$0000
	
init_plane_1_loop_1:
	ina 						
	sta     $2118
	stz     $2119           
	inx
	cpx     #$00a0          ; 0x00a0 logo tiles
	bne    	init_plane_1_loop_1 

	ldx     #$0000
	lda     #$0040 

init_plane_1_clear_2:
	sta		$2118
	stz		$2119
	inx
	cpx     #$01a0			; 0x01a0 tiles clear
	bne  	init_plane_1_clear_2
	ldx     #$0000

init_screen:
	lda     #$0f            ; screen enabled, full brightness
	sta     $2100           ; 
	cli                     ; clear interrupt bit

init_scroll:
	lda		#$00
	sta.w	scrollval		


init_sineoffset:
	lda		#$00
	sta.w	sine_offset
	lda		#$00
	sta		sineswap 

	ldx.w	#colbar_1_color_values
	stx		colbar_color_table	
	ldx.w	#colbar_2_color_values
	stx		colbar_color_table + $02 
	ldx.w	#colbar_3_color_values
	stx		colbar_color_table + $04 

	ldx.w	#colbar_1_color_values
	stx		colbar_color_table + $06	
	ldx.w	#colbar_2_color_values
	stx		colbar_color_table + $08 
	ldx.w	#colbar_3_color_values
	stx		colbar_color_table + $0a 

	lda		#$00
	sta		colbar_offset_table	
	lda		#$15
	sta		colbar_offset_table + $01
	lda		#$25
	sta		colbar_offset_table + $02
	lda		#$35
	sta		colbar_offset_table + $03
	lda		#$45
	sta		colbar_offset_table + $04
	lda		#$55
	sta		colbar_offset_table + $05
				


call_hmda_setup:
	jsr 	init_hdma_table0 
	jsr 	init_hdma_table1 

	jmp  intro 

main:
	jsr 	wait_vbl
	jsr		sine_plane 
	jsr		sine_colbar
	jsr		scroll_plane
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
	ldx 	#$f2
mosaic_l:
	jsr		wait_vbl 
	jsr		wait_vbl
	txa		
	sta 	$2106
	sbc 	#$10
	tax
	cmp		#$02
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

sine_plane:
	rep		#$10
	sep		#$20
	
	lda		sine_offset
	ina
	sta		sine_offset
	cmp		#$ff
	bne		sine_plane_c
	lda		#$00
	sta		sine_offset
	lda		sineswap		; check wich sine table ot take
	ina
	sta		sineswap
	cmp		#$04			; after 4 loop reset counter
	bne		sine_plane_c
	lda		#$00
	sta		sineswap 

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
	lda.w	sineswap
	cmp		#$02			; lower 2 use vsine 1
	bmi		sine_load_vsine1
sine_load_vsine2:			; else use vsine 2
	lda.w	vsine_2,y
	bra 	sine_load_done
sine_load_vsine1:
	lda.w	vsine_1,y
sine_load_done:
	adc		#$c8			; shift logo left 	
	sta		hdma_table0+3,x	
	inx
	inx
	cpx		#$00c0			; 64 hdma lines a 3 byte
	bne		sine_plane_l 
	
	rep		#$30
	sep		#$20
	rts 
	

sine_colbar:
	rep		#$10
	sep		#$20

	ldx		#$00

sine_colbar_clear:
	stz		hdma_table1+1,x
	stz		hdma_table1+2,x
	inx
	inx
	inx
	cpx		#$012c
	bne		sine_colbar_clear

	lda		#$00
	sta		colbar_count
	
	ldy		#$0000
	sty		colbar_color
	

sine_colbar_init:
	
	ldy		colbar_color
	
	ldx		colbar_color_table,y
	stx		colbar_color_ptr
	
	lda.w	colbar_count	
	tay
	lda		colbar_offset_table,y
	ina
	sta		colbar_offset_table,y
	cmp		#$ff			; if end of sine table
	bne		sine_colbar_continue
	lda		#$00			; reset table pointer
	sta		colbar_offset_table,y
	
sine_colbar_continue:

	tay
	lda.w	colbarsine_1,y
	tax	
	ldy		#$0000

sine_colbar_loop:
	lda		(colbar_color_ptr),y	
    ora     hdma_table1+1,x
	and     #$ff
	sta		hdma_table1+1,x
	iny
	lda		(colbar_color_ptr),y
	ora		hdma_table1+2,x
	and		#$ff 
	sta		hdma_table1+2,x 
	inx
	inx
	inx
	iny
	;iny 
	cpy		#$0040			; 32 colot lines a 2 byte
	bne		sine_colbar_loop  
	
	ldy		colbar_color
	iny
	iny	
	sty		colbar_color 
	
	lda.w	colbar_count
	ina		
	sta.w	colbar_count
	cmp		#$06
	bne 	sine_colbar_init

sine_colbar_end:
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
	;sta		sine_offset
	lda		#$4c	
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
	;lda		vsine_1,y
	lda		#$00 
	sta		hdma_table0,x
	inx
	lda		#$00
	sta		hdma_table0,x
	inx	
	iny 
	;cpx		#$0183 ; (128 + 1) * 3 = 387 = 0x0183
	cpx		#$00c3 ; (64 + 1) * 3 = 195 = 0x00c0
	bne 	init_hdma_table0_loop
	lda		#$4c
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
  ;lda 	#%00000001
	;sta 	$420c
	rep		#$30
	sep		#$20 
	rts	

init_hdma_table1:
	;rep 	#$10    	
	;sep 	#$30    
	ldx		#$0000
	
init_hdma_tabel1_copy:
	ldy.w		backcolors_color_values,x	
	tya 
	sta			hdma_table1,x
	inx
	cpx			#$0295
	bne			init_hdma_tabel1_copy 
	lda     #$00
	sta     $4310
	lda     #$21
	sta     $4311
	ldx.w  	#backcolors_color_list
	stx     $4312
	lda 		#$00	
	sta     $4314

 	lda     #$02
	sta     $4320
	lda     #$22
	sta     $4321
	;ldx.w  	#backcolors_color_values
	ldx.w		#hdma_table1
	stx     $4322
	lda 		#$00	
	sta     $4324
  lda 		#%00000111
	sta 		$420c
	rep			#$30
	sep			#$20 
	rts	

setup:
   	sep 	#$30     ; x,y,a are 8 bit numbers
   	lda 	#$8f     ; screen off, full brightness
   	sta 	$2100    ; brightness + screen enable register 
   	lda 	#$00     ; 
   	sta 	$2101    ; sprite register (size + address in vram)
   	lda 	#$00
   	sta 	$2102    ; sprite registers (address of sprite memory [oam])
   	sta 	$2103    ;    ""                       ""
   	lda 	#$00     ; mode 0
   	sta 	$2105    ; graphic mode register
   	lda 	#$00     ; no planes, no mosaic
   	sta 	$2106    ; mosaic register
   	lda 	#$00     ; 
   	sta 	$2107    ; plane 0 map vram location
   	lda 	#$00
   	sta 	$2108    ; plane 1 map vram location
   	lda 	#$00
   	sta 	$2109    ; plane 2 map vram location
   	lda 	#$00
   	sta 	$210a    ; plane 3 map vram location
   	lda 	#$00
   	sta 	$210b    ; plane 0+1 tile data location
   	lda 	#$00
   	sta 	$210c    ; plane 2+3 tile data location
   	lda 	#$00
   	sta 	$210d    ; plane 0 scroll x (first 8 bits)
   	sta 	$210d    ; plane 0 scroll x (last 3 bits) #$0 - #$07ff
   	sta 	$210e    ; plane 0 scroll y (first 8 bits)
   	sta 	$210e    ; plane 0 scroll y (last 3 bits) #$0 - #$07ff
   	sta 	$210f    ; plane 1 scroll x (first 8 bits)
   	sta 	$210f    ; plane 1 scroll x (last 3 bits) #$0 - #$07ff
   	sta 	$2110    ; plane 1 scroll y (first 8 bits)
   	sta 	$2110    ; plane 1 scroll y (last 3 bits) #$0 - #$07ff
   	sta 	$2111    ; plane 2 scroll x (first 8 bits)
   	sta 	$2111    ; plane 2 scroll x (last 3 bits) #$0 - #$07ff
   	sta 	$2112    ; plane 2 scroll y (first 8 bits)
   	sta 	$2112    ; plane 2 scroll y (last 3 bits) #$0 - #$07ff
   	sta 	$2113    ; plane 3 scroll x (first 8 bits)
   	sta 	$2113    ; plane 3 scroll x (last 3 bits) #$0 - #$07ff
   	sta 	$2114    ; plane 3 scroll y (first 8 bits)
   	sta 	$2114    ; plane 3 scroll y (last 3 bits) #$0 - #$07ff
   	lda 	#$80     ; increase vram address after writing to $2119
   	sta 	$2115    ; vram address increment register
   	lda 	#$00
   	sta 	$2116    ; vram address low
   	sta 	$2117    ; vram address high
   	sta 	$211a    ; initial mode 7 setting register
   	sta 	$211b    ; mode 7 matrix parameter a register (low)
   	lda 	#$01
   	sta 	$211b    ; mode 7 matrix parameter a register (high)
   	lda 	#$00
   	sta 	$211c    ; mode 7 matrix parameter b register (low)
   	sta 	$211c    ; mode 7 matrix parameter b register (high)
   	sta 	$211d    ; mode 7 matrix parameter c register (low)
   	sta 	$211d    ; mode 7 matrix parameter c register (high)
   	sta 	$211e    ; mode 7 matrix parameter d register (low)
   	lda 	#$01
   	sta 	$211e    ; mode 7 matrix parameter d register (high)
   	lda 	#$00
   	sta 	$211f    ; mode 7 center position x register (low)
   	sta 	$211f    ; mode 7 center position x register (high)
   	sta 	$2120    ; mode 7 center position y register (low)
   	sta 	$2120    ; mode 7 center position y register (high)
   	sta 	$2121    ; color number register ($0-ff)
   	sta 	$2123    ; bg1 & bg2 window mask setting register
   	sta 	$2124    ; bg3 & bg4 window mask setting register
   	sta 	$2125    ; obj & color window mask setting register
   	sta 	$2126    ; window 1 left position register
   	sta 	$2127    ; window 2 left position register
   	sta 	$2128    ; window 3 left position register
   	sta 	$2129    ; window 4 left position register
   	sta 	$212a    ; bg1, bg2, bg3, bg4 window logic register
   	sta 	$212b    ; obj, color window logic register (or,and,xor,xnor)
   	lda 	#$01
   	sta 	$212c    ; main screen designation (planes, sprites enable)
   	lda 	#$00
   	sta 	$212d    ; sub screen designation
   	lda 	#$00
   	sta 	$212e    ; window mask for main screen
   	sta 	$212f    ; window mask for sub screen
   	lda 	#$30
   	sta 	$2130    ; color addition & screen addition init setting
   	lda 	#$00
   	sta 	$2131    ; add/sub sub designation for screen, sprite, color
   	lda 	#$e0
   	sta 	$2132    ; color data for addition/subtraction
   	lda 	#$00
   	sta 	$2133    ; screen setting (interlace x,y/enable sfx data)
   	lda 	#$00
  	sta 	$4200    ; enable v-blank, interrupt, joypad register
   	lda 	#$ff
   	sta 	$4201    ; programmable i/o port
   	lda 	#$00
   	sta 	$4202    ; multiplicand a
   	sta 	$4203    ; multiplier b
   	sta 	$4204    ; multiplier c
   	sta 	$4205    ; multiplicand c
  	sta 	$4206    ; divisor b
   	sta 	$4207    ; horizontal count timer
   	sta 	$4208    ; horizontal count timer msb (most significant bit)
   	sta 	$4209    ; vertical count timer
   	sta 	$420a    ; vertical count timer msb
   	sta 	$420b    ; general dma enable (bits 0-7)
   	sta 	$420c    ; horizontal dma (hdma) enable (bits 0-7)
   	sta 	$420d    ; access cycle designation (slow/fast rom)
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
 	.db		"              _   _             "
 	.db		"   ___  _ __ | |_(_)_  ____  __ "
 	.db		"  / _ \| '_ \| __| \ \/ /\ \/ / "
 	.db		" | (_) | |_) | |_| |>  <  >  <  "
 	.db		"  \___/| .__/ \__|_/_/\_\/_/\_\ "
 	.db		"       |_|                      "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"      OPTIXX ONCE A AGAIN       "
 	.db		"     WITH A CONSOLE RELEASE     "
 	.db		"        WWW.OPTIXX.ORG          "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "
 	.db		"                                "



.INCLUDE "vsine_1.s"

.INCLUDE "vsine_2.s"

.INCLUDE "backcolors.s"

.INCLUDE "colbar_1.s" 

.INCLUDE "colbar_2.s" 

.INCLUDE "colbar_3.s" 


.INCLUDE "colbarsine_1.s" 





.INCLUDE "optixx_logo.s" 

.ENDS



;.BANK $04 SLOT 6
;.ORGA $8000
;long_label:
	nop
	nop
	rts 


;.BANK $01 SLOT 6
;.ORGA $8000
;.BASE $01
;.INCLUDE "music.s" 

;.BANK $02 SLOT 6
;.ORG $0000 
;music_data_1:
;.INCBIN "music1.bin"

;.BANk $03 SLOT 6 
;.ORG $0000 
;music_data_2:
;.INCBIN "music2.bin" 



