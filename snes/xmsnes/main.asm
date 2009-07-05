; XMSNES EXAMPLE DEMO!

.define nsongs 5			; number of songs in package

.include "memmap.inc"		; memory map stuff
.include "cheader.inc"		; snes rom header
.include "InitSNES.asm"		; snes initialization code

.include "snes.inc"			; snes register definitions

.BANK 1

.SECTION "GRAPHICS"			; include graphics (bank1)
.include "graphics\gfx_window.inc"
.include "graphics\gfx_font.inc"
.include "graphics\gfx_bg.inc"
.include "graphics\gfx_bg2.inc"
;.include "graphics\gfx_bub.inc"
.ENDS

.BANK 2 SLOT 3				; include songs/samples (bank2)
.SECTION "SPX_PACKAGE"
TEST_PACKAGE:
.incbin "test.xmp"
.ENDS

.ramsection "globals" BANK 0 SLOT 1
bg_ripple:			db		; for bg effect
bg_rippleL:			db

scr_fade:			db		; for fading screen
scr_flash:			dsb 3	; for flashing screen

joypad:				dw		; joypad state
joypadc:			dw		; joypad 'clicks'
joypadl:			dw		; last joypad state

svar1:				dw		; general purpose
svar2:				dw

current_song:		dw

music_vol:			dw		; 8.8 fixed

custom_string:		dsb 32

effect1:			db		; sound effect indexes
effect2:			db
effect3:			db
effect4:			db

inst_timer:			dw

.ends

.BANK 0

.SECTION "MAIN"

Main:
	InitSNES		; initialize everything
	REP #$10		; 16bit index
	SEP #$20		; 8bit accu
	
	lda #$80		; turn off screen
	sta REG_INIDISP
	
	lda #%00001001	; set display mode
	sta REG_BGMODE	; mode 1
	
	jsl LoadBG			; load background bg
	jsl BuildWindow		; build the window thingy
	jsl LoadFont		; load the font
	jsl ClearAllText	; reset the text layer
	
	;ldx #STR_SPCDATA	; draw "SPC Ports"
	;ldy #14*32+12
	jsl DrawText
	
	lda #%0110		; blend the window with the background
	sta REG_TM		; enable bg1/2 main screen
	lda #%0000		; enable bg0 subscreen
	sta REG_TD		;
	
	lda #%01100011	; add bg0+backdrop+bg1 and half the result
	sta REG_CGADSUB
	lda #%00000010
	sta REG_CGWSEL
	
	;---------------------------------------------------------
	; SETUP XMSNES
	;jsl BootSPC				; send code and initialize
	;jsl SPX_Transfer_LFT	; transfer frequency table (linear mode)
	
	;ldx #TEST_PACKAGE		; load package
	;lda #:TEST_PACKAGE		; 
	;jsl SPXP_InstallPackage	;
	
	;ldx #0					; load song 0
	;jsl ChangeSong

	; song is playing now...
	;---------------------------------------------------------
	
	lda #%10100001	; enable vblank irq, enable joypad
	sta REG_NMITIMEN
	
	cli				; enable interrupts
	wai
	
	lda #0			; reset darkness
	sta scr_fade	;
	sta REG_INIDISP	;
	
	ldx #22000		; set playback volume
	stx music_vol	;
    ;jsl SPXM_SetVol	;
	
_mainloop:

	;-----------------------------------
	;         SCREEN FADE-IN

	lda scr_fade
	cmp #255
	beq +
	clc
	adc #2
	bcc ++
	lda #255
++
	sta scr_fade
	lsr
	lsr
	lsr
	lsr
	sta REG_INIDISP
+	;--------------------------------------
	
	
	jsl UpdateBG		; Update the background effect
	;jsl DrawSPCData		; Read SPC Ports and display them

	bit inst_timer+1	; timer to hide instructions
	bmi ++				;
	rep #$20			;
	lda inst_timer		;
	dea					;
	bpl +				;
	ldy #(25*32)		;
	ldx #32				;
	jsl ClearText		;
+						;
	sta inst_timer		;
	sep #$20			;
++						;
	
	;jsl SPX_Routine		; call this every frame, or every so often
	
	;--------------------------------------
	; slide music volume to full

	rep #$20
	lda music_vol
	cmp #$FFFF
	beq +
	adc #44
	bcc ++
	lda #$FFFF
++
	sta music_vol
	sep #$20
	xba
;	jsl SPXM_SetVol
+	;----------------------------------------
	
	;---------------------------------------
	; if user presses left, decrease song#, load new song

	sep #$20
	lda joypadc+1
	and #1
	beq +
	ldx current_song
	inx
	cpx #nsongs
	bcc ++
	ldx #nsongs-1
++
	jsl ChangeSong
+	;-------------------------------------------------
	; if user presses right, increase song#, load new song

	lda joypadc+1
	and #%10
	beq +
	ldx current_song
	dex
	bpl ++
	ldx #0
++
	jsl ChangeSong
+	;--------------------------------------------------

	;------------------------------------------------
	; if user presses A, play effect1, full volume, freq:4

	lda joypadc
	and #%10000000
	beq +
	; A was pressed
	lda #$04	; freq/priority
	xba
	lda effect1	; sample#
	beq +
	tax
	lda #$77	; volume
	jsl SPXS_Play
+	;-----------------------------------------------
	; if user presses B, play effect2, full volume, freq:4

	lda joypadc+1
	and #%10000000
	beq +
	; B was pressed
	lda #$04	; freq/priority
	xba
	lda effect2	; sample#
	beq +
	tax
	lda #$77	; volume
	jsl SPXS_Play
+	;-----------------------------------------------
	; if user presses X, play effect3, full volume, freq:4
	lda joypadc
	and #%1000000
	beq +
	; X was pressed
	lda #$04	; freq/priority
	xba
	lda effect3	; sample#
	beq +
	tax
	lda #$77	; volume
	jsl SPXS_Play
+	;-----------------------------------------------
	; if user presses Y, play effect4, full volume, freq:3

	lda joypadc+1
	and #%1000000
	beq +
	; Y was pressed
	lda #$03	; freq/priority
	xba
	lda effect4	; sample#
	beq +
	tax
	lda #$77	; volume
	jsl SPXS_Play
+	;------------------------------------------------
	
	ldx #0
	stx joypadc
	wai					; wait for vblank
	jmp _mainloop		; loop

;---------------------------------------------------------
; this function gets called when the snes receives a song message from the spc
MessageReceived:
	; a = message
	cmp #1
	bne +
	jsl FlashScreen		; #1 = flash screen message
+
	rtl
;---------------------------------------------------------

FlashScreen:
	lda #16
	sta scr_flash
	rtl

;---------------------------------------------------------

cs_jumptable:
.dw	CS_WAR
.dw CS_ACID
.dw CS_JOURNEY
.dw CS_SATELLITE
.dw CS_RUSINA

ChangeSong:
	; x = song
	stx current_song			; save song#
	
	stz effect1					; zero out sound effect indexes
	stz effect2
	stz effect3
	stz effect4
	
	jsl SPXM_Reset				; queue reset playback
	jsl SPX_Flush				; flush
	
	rep #$20					; get song*2
	lda current_song
	asl
	tax
	sep #$20
	jmp (cs_jumptable,x)		; and jump
	
CS_WAR:
	ldx #STR_WAR_AUTHOR			; get author string
	phx
	ldx #STR_WAR_TITLE			; song title
	jmp CS_BEGINDRAWING
CS_ACID:						; other songs
	ldx #STR_ACID_AUTHOR		; etc
	phx
	ldx #STR_ACID_TITLE
	jmp CS_BEGINDRAWING
CS_JOURNEY:
	ldx #STR_JOURNEY_AUTHOR
	phx
	ldx #STR_JOURNEY_TITLE
	jmp CS_BEGINDRAWING
CS_SATELLITE:
	ldx #STR_SATELLITE_AUTHOR
	phx
	ldx #STR_SATELLITE_TITLE
	jmp CS_BEGINDRAWING
CS_RUSINA:
	ldx #STR_RUSINA_AUTHOR
	phx
	ldx #STR_RUSINA_TITLE
	jmp CS_BEGINDRAWING
CS_BEGINDRAWING:

	phx					;preserve

	ldy #(8*32)					; clear text
	ldx #32
	jsl ClearText

	ldy #(10*32)
	ldx #32
	jsl ClearText
	
	plx					;restore
	
	ldy #16+(8*32)				; draw title
	jsl DrawCentered
	plx
	ldy #16+(10*32)				; draw author
	jsl DrawCentered

	ldx current_song			; get song#
	jsl SPXP_LoadSong			; transfer song
	
	ldx current_song			; check if song is 4 (has sound effects)
	cpx #4						; 
	bne +

	; load some sfx
	
	ldx #0						; hh.wav
	ldy #0
	jsl SPXP_LoadSample
	sta effect1
	ldx #1						; ow.wav
	ldy #0
	jsl SPXP_LoadSample
	sta effect2
	ldx #2						; sd.wav
	ldy #0
	jsl SPXP_LoadSample
	sta effect3
	ldx #3						; OWWW.wav
	ldy #0
	jsl SPXP_LoadSample
	sta effect4

	; be sure not to load too much stuff
	ldy #(25*32)
	ldx #32
	jsl ClearText	
	ldx #STR_INST1				; tell the user to push buttons
	ldy #(25*32)+16
	jsl DrawCentered
	bra ++
+
	ldy #(25*32)
	ldx #32
	jsl ClearText
	ldx #STR_INST2				; tell the user to push buttons
	ldy #(25*32)+16
	jsl DrawCentered
++
	
	ldx #300
	stx inst_timer

	jsl SPXM_Play				; start playing song
	rtl							; return

;-------------------------------------------------------------------------------
; TEXT RENDERING STUFF
	
DrawText:
	; x = source
	; y = offset
	stz REG_VMAIN		; setup vram increment stuff
	rep #$20			; set destination address
	tya					;
	ora #($9000/2)		;
	sta REG_VMADDL		;
	tay					;
	sec					;
	sep #$20			;
_dt_loop:
	lda $0000, x		;
	beq _dt_exit		; exit when 0
	inx					; increment counter
	sbc #32				; viewable ascii characters start at 32
	bpl +			; check for newline
	tya				; newline
	adc #32			; edit vram address
	sta REG_VMADDL	;
	tay				;
	sec				;
	bra _dt_loop	; loop
+
	sta REG_VMDATAL		; store value
	bra _dt_loop		; loop
_dt_exit:
	rtl					; end

ClearText:
	; x = length
	; y = offset

	stz REG_VMAIN		; setup vram pointer
	rep #$20			;
	tya					;
	ora #($9000/2)		;
	sta REG_VMADDL		;
	sep #$20
	lda #$00			; start clearing memory

_ct_loop:
	sta REG_VMDATAL		; store...
	dex					; count...
	bne _ct_loop		; loop...
_ct_exit:
	sep #$20			; restore 8-bit accu
	rtl					; end

ClearAllText:
	lda #%10000000		; setup vram pointer
	sta REG_VMAIN		;
	ldx #($9000/2)
	stx REG_VMADDL
	
	ldx #$100|(0<<10)|(1<<13)			; $100 = blank tile
	ldy #1024			; 1024 = 32*32 tiles
-
	stx REG_VMDATAL		; store value
	dey					; count..
	bne -				; loop
	rtl					; end

DrawCentered:
	; x = string address
	; y = line
	sty svar2
	stx svar1
	jsl strlen
	rep #$20
	txa
	sec
	sbc svar1
	lsr
	clc
	eor #$FFFF
	inc a
	adc svar2
	tay
	ldx svar1
	sep #$20
	jmp DrawText

DrawSPCData:
	lda #0
	xba

	ldx #0
	stx svar1
-
	lda REG_APUI00, x
	inx
	stx svar2
	pha
	lsr
	lsr
	lsr
	lsr
	clc
	tay
	lda HEX2ASCII, y
	ldx svar1
	sta custom_string,x
	inx
	pla
	and #$0F
	tay
	lda HEX2ASCII, y
	sta custom_string,x
	inx
	lda #32
	sta custom_string,x
	inx
	stx svar1
	ldx svar2
	cpx #4
	bne -
	ldy #11+(16*32)
	ldx #custom_string
	jsl DrawText
	rtl

;---------------------------------------------------------------------------------------------------------------------------------------------------------------------
; INITIALIZATION / EFFECTS

LoadBub:
	lda #%10000000
	sta REG_VMAIN
	ldx #$

BuildWindow:
	lda #%10000000		; setup vram pointer
	sta REG_VMAIN
	ldx #$7C00/2
	stx REG_VMADDL
	ldx #0
	REP #$20
-
	lda.l gfx_window, x	; load graphics
	sta REG_VMDATAL
	inx
	inx
	cpx #$120
	bne -
	SEP #$20
	LDA #-1				; setup bg attributes
	sta REG_BG0VOFS
	stz REG_BG0VOFS
	
	
	lda #($11<<2)
	sta REG_BG0SC

	ldy #($8800)/2
	sty REG_VMADDL

	ldx #464
	ldy #1024
-	stx REG_VMDATAL
	dey
	bne -

	ldy #($8980)/2
	sty REG_VMADDL
	ldx #1000|(2<<10)
	ldy #32
-	stx REG_VMDATAL
	dey
	bne -
	ldx #1000|(2<<10)
	ldy #32*11
-	stx REG_VMDATAL
	dey
	bne -
	ldx #1000|(2<<10)
	ldy #32
-	stx REG_VMDATAL
	dey
	bne -

	lda #32				; load palette
	sta REG_CGADD
	ldx #gfxp_window
	ldy #32
	lda #(REG_CGDATA&255)
	xba
	lda #:gfxp_window
	jsl DMA_TRANSFER
	rtl

LoadFont:
	lda #%10000000		; setup vram pointer
	sta REG_VMAIN
	ldy #($7000/2)
	sty REG_VMADDL
	ldx #0
	rep #$20
	
-
	lda.l gfx_font, x	; load byte
	sta REG_VMDATAL		; store
	inx					; count
	inx
	cpx #$600			; transfer $600*2 bytes
	bne -
	sep #$20			; setup palette
	lda #0
	sta REG_CGADD
	sta REG_CGDATA
	sta REG_CGDATA
	lda #$FF		; white
	sta REG_CGDATA
	lda #$7F
	sta REG_CGDATA
	lda #%00000000		; black
	sta REG_CGDATA
	sta REG_CGDATA
	
	lda #-1				; setup bg
	sta REG_BG2VOFS
	stz REG_BG2VOFS
	lda #($12<<2)		; source = $9000
	sta REG_BG2SC
	
	lda #$03			; set character offset
	sta REG_BG23NBA
	
	rtl

LoadBG:
	lda #%10000000
	sta REG_VMAIN
	ldx #0
	stx REG_VMADDL
	
	ldx #$1000
	; transfer data
	REP #$20
	
-
	lda.l gfx_bg-$1000, x
	sta REG_VMDATAL
	inx
	inx
	bpl -
	

	ldx #0
	stx REG_VMADDL
-	lda.l gfx_bg2, x
	sta REG_VMDATAL
	inx
	inx
	cpx #96
	bne -

	sep #$20
	
	lda #-1
	sta REG_BG1VOFS
	stz REG_BG1VOFS
	
	lda #($10<<2)
	sta REG_BG1SC
	; setup bg
	ldy #$8000/2
	sty REG_VMADDL
	
	ldy #(1<<10)
-
	sty REG_VMDATAL
	iny
	cpy #896|(1<<10)
	bne -
	
	ldy #$8000/2
	sty REG_VMADDL
	ldy #32*5
	ldx #2|(3<<10)
-
	stx REG_VMDATAL
	dey
	bne -
	
	ldy #32
	ldx #1|(3<<10)
-	stx REG_VMDATAL
	dey
	bne -

	ldy #$84C0/2
	sty REG_VMADDL
	ldy #32
	ldx #0|(3<<10)
-	stx REG_VMDATAL
	dey
	bne -

	ldy #32*8
	ldx #2|(3<<10)
-
	stx REG_VMDATAL
	dey
	bne -

	lda #48
	sta REG_CGADD

	ldx #gfxp_bg2
	ldy #32
	lda #(REG_CGDATA & 255)
	xba
	lda #:gfxp_bg2
	jsl DMA_TRANSFER
	
	rtl
	
BG_PALETTE:
;.dw 0,0,1024,1024,2048,2048,2081,3105,3105,4129,4129,4129,5153,5153,6177,6210,6210,6210,6177,5153,5153,4129,4129,4129,3105,3105,2081,2048,2048,1024,1024,0,0,0,1024,1024,2048,2048,2081,3105,3105,4129,4129,4129,5153,5153,6177,6210,
;.dw 6276,6276,7300,7300,8324,8324,8357,9381,9381,10405,10405,10405,11429,11429,12453,12486,12486,12486,12453,11429,11429,10405,10405,10405,9381,9381,8357,8324,8324,7300,7300,6276,6276,6276,7300,7300,8324,8324,8357,9381,9381,10405,10405,10405,11429,11429,12453,12486,
.dw 4162,5186,5186,6243,6243,7267,8291,8291,9348,9348,10372,11396,11396,12453,12453,13477,14501,13477,12453,12453,11396,11396,10372,9348,9348,8291,8291,7267,6243,6243,5186,5186,4162,5186,5186,6243,6243,7267,8291,8291,9348,9348,10372,11396,11396,12453,12453,13477,
;.dw 2116,2117,2117,3174,3174,3175,3176,3176,4233,4233,4234,4235,4235,5292,5292,5293,5294,5293,5292,5292,4235,4235,4234,4233,4233,3176,3176,3175,3174,3174,2117,2117,2116,2117,2117,3174,3174,3175,3176,3176,4233,4233,4234,4235,4235,5292,5292,5293, ;red
UpdateBG:

	clc
	lda bg_rippleL
	adc #99
	sta bg_rippleL
	bcs +
	rtl
+
	
	lda bg_ripple
	inc a
	rep #$20
	and #31
	sta bg_ripple
	asl
	tay
	sep #$20
	
	lda #0
	sta REG_CGADD
	ldx #16
	
	
	lda scr_flash
	rep #$20
	and #31
	asl
	asl
	asl
	asl
	asl
	ora scr_flash
	asl
	asl
	asl
	asl
	asl
	ora scr_flash
	sta scr_flash+1
	sep #$20

	cmp #0
	beq +
	dec scr_flash
	dec scr_flash
	dec scr_flash
	dec scr_flash
+
	clc
	lda BG_PALETTE, y
	adc scr_flash+1
	sta REG_CGDATA
	lda BG_PALETTE+1, y
	adc scr_flash+2
	sta REG_CGDATA
	lda #16
	sta REG_CGADD

-
	lda BG_PALETTE, y
	adc scr_flash+1
	sta REG_CGDATA
	
	lda BG_PALETTE+1, y
	adc scr_flash+2
	sta REG_CGDATA
	iny
	iny
	dex
	bne -
	
	rtl

;-----------------------------------------------------------------------------------------------------------------------------------------------------------------
; DMA
	
DMA_TRANSFER:
	; x = src
	; y = length
	; a = bank#
	; b = dest
	stz REG_DMAP0		; set mode
	stx REG_A1T0L		; set source
	sta REG_A1B0		; set bank#
	xba					; 
	sta REG_BBAD0		; set dest
	sty REG_DAS0L		; set #bytes
	lda #1				; start transfer
	sta REG_MDMAEN
	rtl					; end

;----------------------------------------------------------------------------------------------------------------------------------------------------------
; INTERRUPTS
.index 16
.accu 8

VBlank:
	sei
	rep #$20
	pha				; preserve a
	
	lda joypad		; update last joypad state
	sta joypadl		; 

	sep #$20
	lda #1
-
	bit $4212		; check if joypad is ready
	bne -			; wait...
	rep #$20
	lda REG_JOY1L	; load joystate
	sta joypad		; save to memory
	eor joypadl		; mask with old state
	and joypad		; mask some more..
	sta joypadc		; store button 'clicks'
	
	sep #$20
	lda REG_TIMEUP	; do something
	rep #$20
	pla				; restore a
	rti				; exit

;--------------------------------------------------------------------------------------
; misc

strlen:
	; x = str address
	; returns x = str address + length
-
	lda $0000, x
	beq +
	inx
	bra -
+
	rtl

;-------------------------------------------------------------------------------------
; STRINGS
STR_HELP:
.DB	"Press left/right to", 1
.db "change songs",0

STR_SPCDATA:
.db "SPC Ports",0

HEX2ASCII:
.db 48,49,50,51,52,53,54,55,56,57,65,66,67,68,69,70

STR_WAR_TITLE:
.db "War in Middle Earth", 0
STR_WAR_AUTHOR:
.db "by Skaven", 0

STR_ACID_TITLE:
.db "Acidjazzed Evening", 0
STR_ACID_AUTHOR:
.db "by Tempest", 0

STR_JOURNEY_TITLE:
.db "Your Journey Awaits", 0
STR_JOURNEY_AUTHOR:
.db "by AlexG", 0

STR_SATELLITE_TITLE:
.db "Satellite One",0
STR_SATELLITE_AUTHOR:
.db "by Purple Motion",0

STR_RUSINA_TITLE:
.db "Rusinahumppa",0
STR_RUSINA_AUTHOR:
.db "by Croaker",0

STR_INST1:
.db "Press A/B/X/Y for SFX",0

STR_INST2
.db "Change songs with left/right",0

.ENDS
