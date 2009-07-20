
;init variables:
;a(8bit) is index into bgmode 2byte pointertable
SetBGMode:
	php
	rep #$31				;accu 16bit
	and.w #$000f				;16 valid entries
	asl a 
	tax
	lda.l BgModeLut+BaseAdress,x
;	lda.b UploadBackgroundPointer+1		;get source bank of tilemap
	sta.b SetBGThreeBytePointerLo		;

	sep #$20
	lda.b #(:BgModeLut+BaseAdress>>16)
	sta.b SetBGThreeBytePointerBank		;
	ldy.w #$0000
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b SetIni
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b ScreenMode
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b MainScreen
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b SubScreen
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b CGWsel
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b CgadsubConfig
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b BGTilesVram12
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b BGTilesVram34
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b BG1TilemapVram
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b BG2TilemapVram
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b BG3TilemapVram
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b BG4TilemapVram
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b ObjSel
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	jsr IrqInit
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b W12SEL
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b W34SEL
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b WOBJSEL
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b W1L
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b W1R
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b W2L
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b W2R
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b WBGLOG
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b WOBJLOG
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b WMS
	iny
	lda.b [SetBGThreeBytePointerLo],y		;load byte from table
	sta.b WSS


	

;init vram locations	
	rep #$31
	lda.b BGTilesVram12
	pha
	and.w #$00f0
	xba
	sta.b VramBg2Tiles
	
	pla
	and.w #$000f
	xba
	clc
	asl a
	asl a
	asl a
	asl a
	sta.b VramBg1Tiles

	lda.b BGTilesVram34
	pha
	and.w #$00f0
	xba
	sta.b VramBg4Tiles
	
	pla
	and.w #$000f
	xba
	clc
	asl a
	asl a
	asl a
	asl a
	sta.b VramBg3Tiles

	
	lda.b BG1TilemapVram
	and.w #$00fc
	xba
	sta.b VramBg1Tilemap

	lda.b BG2TilemapVram
	and.w #$00fc
	xba
	sta.b VramBg2Tilemap

	lda.b BG3TilemapVram
	and.w #$00fc
	xba
	sta.b VramBg3Tilemap

	lda.b BG4TilemapVram
	and.w #$00fc
	xba
	sta.b VramBg4Tilemap
	
	lda.b ObjSel
	and.w #%111			;get spritetile base adress
	asl a				;shift left by 5
	asl a
	asl a
	asl a
	asl a
	xba				;shift left by 8
	sta.b SpriteTileOffsetVram
	
	plp
	rts





;**********************************
;Simple Background loader for bg1 and 2
;in:	a,8bit: bit0,1:   bg number, 0-3
;		bit2-4: palette to use
;		bit5:   priority bit
;in:	x,16bit: pointernumber of background
;
;every background consists of 1.tiles,2.tilemap,3.pal+header(relative pointers to all 3 files+eof)
;
;uses: TempBuffer0-8
;**********************************

UploadBackgroundFile:
	php
	sep #$20

	sta.b TempBuffer+2

	rep #$31
;	and.w #$ff
	txa
	sta.b TempBuffer+4			;multiply background number by 3
	asl a
	adc.b TempBuffer+4
	tax
	lda.l UploadBackgroundFileLUT+BaseAdress,x		;get pointer low byte to background
	sta.b TempBuffer+4
	inx
	lda.l UploadBackgroundFileLUT+BaseAdress,x		;get pointer high word to background
	sta.b TempBuffer+5

	sep #$20
	
	sei
	jsr UploadTiles	
	jsr UploadTilemap
	cli

	plp
	rts
	
UploadTiles:
	sep #$20
	ldx.b DmaFifoPointer
	lda #1					;transfer type
	sta.l DmaFifoEntryType,x
	
	lda.b TempBuffer+6			;get source bank

	sta.l DmaFifoEntrySrcBank,x		;source 4304
	
	rep #$31
	ldy.w #$0000
	lda.b [TempBuffer+4],y		;get relative pointer to tiles
	sta.b TempBuffer+7
	adc.b TempBuffer+4			;add file offset

	sta.l DmaFifoEntrySrcLo,x		;source 4302
	
	sec
	iny
	iny
	lda.b [TempBuffer+4],y		;get relative pointer to tilemap
	sbc.b TempBuffer+7		;substract tile position to get tile length

	sta.l DmaFifoEntryCount,x			;length 4305
	
	lda.b TempBuffer+2
	and.w #%11
	asl a
	tay

	lda.w VramBg1Tiles,y					;get vram destination

	sta.l DmaFifoEntryTarget,x		;vram target 2116

	txa						;update fifo entry pointer
	clc
	adc.w #DmaFifoEntryLength
	sta.b DmaFifoPointer

	rts

BgMapBufferLUT:
	.dw Bg1MapBuffer&$ffff
	.dw Bg2MapBuffer&$ffff
	.dw Bg3MapBuffer&$ffff
	.dw Bg4MapBuffer&$ffff

UploadTilemap:
;copy tilemap to wram buffer:
	php
	sep #$20

	phb
	lda.b #$81
	pha
	plb

	lda.b #Bg1MapBuffer >> 16		;store target adress(map buffer) in ram port.
	and.b #$01				;only 1 or 0 valid for banks $7e or $7f
	sta.w $2183

	rep #$31
	lda.b TempBuffer+2
	and.w #%11
	asl a
	tax
	lda.l (BgMapBufferLUT+BaseAdress),x
	
	sta.w $2181	
	sep #$20
	lda.b TempBuffer+2			;save layer bit
;	pha
	
	and.b #$3c				;mask off everything but palette and priority bit
	sta.b TempBuffer+3

	rep #$31				;accu 16bit
	lda.b TempBuffer+5		;get source bank of tilemap
	sta.b ThreeBytePointerHi		;

	ldy.w #$0002
	lda.b [TempBuffer+4],y	;get relative pointer to tilemap
	sta.b TempBuffer+7
	adc.b TempBuffer+4
	sta.b ThreeBytePointerLo
	iny
	iny
	lda.b [TempBuffer+4],y	;get relative pointer to palette(in order to calculate tilemap length)
	
	sec
	sbc.b TempBuffer+7
	sta.b TempBuffer			;store total length for dma transfer
	and.w #$fffe				;mask off bit0 so the next loop never hangs
	
	tax					;get length, store in x
	
	ldy.w #$0000
	sep #$20
	
DMAUploadTilemapLoop:
	lda.b [ThreeBytePointerLo],y		;load word from ThreeBytePointer
	sta.w $2180				;store in bg1 map buffer, low byte
	iny
	lda.b [ThreeBytePointerLo],y		;load word from ThreeBytePointer
	and.b #$c3
	ora.b TempBuffer+3			;or with priority and palette
	sta.w $2180				;store in bg1 map buffer, low byte
	iny
	dex					;word-dec length counter
	dex
	bne DMAUploadTilemapLoop		;done if length counter = 0

;	pla
;	sta.b UploadBackgroundLayer
;	jsr DMAUploadBgTilemap

	rep #$31
	lda.b TempBuffer+2
	and.w #%11
	tax
	inc.b NMIBg1UploadFlag,x		;set corresponding bg tilemap update flag
	sep #$20

DMAUploadPaletteBG1:
	lda.b #PaletteBuffer >> 16		;store target adress(palette buffer) in ram port.
	and.b #$01				;only 1 or 0 valid for banks $7e or $7f
	
	lda.b TempBuffer+2
	bit.b #%10						;do an extra shift if this is bg0 or 1(bg0,1:always 16 color palettes /bg2,3: always 4 colors)
	beq DMAUploadPaletteBG1jaja
	and.b #%00011100			;mask off everything but palette number
	asl a					;multiply by 8(2bytes per color x 4 colors per palette)	
	bra DMAUploadPaletteBGSelectDone

DMAUploadPaletteBG1jaja:
	and.b #%00011100			;mask off everything but palette number
	asl a
	asl a
	asl a					;multiply by 32(2bytes per color x 16 colors per palette)	
	
DMAUploadPaletteBGSelectDone:
	rep #$31
	and.w #$00ff
	tax
	lda.b TempBuffer+5		;get source bank of palette
	sta.b ThreeBytePointerHi		;

	ldy.w #$0004
	lda.b [TempBuffer+4],y	;get relative pointer to palette
	sta.b TempBuffer+7
	adc.b TempBuffer+4
	sta.b ThreeBytePointerLo

	iny
	iny
	lda.b [TempBuffer+4],y	;get relative pointer to EOF(in order to calculate tilemap length)
	
	sec
	sbc.b TempBuffer+7

	sta.b TempBuffer

	ldy.w #$0000				;clear target/source counter						

DMAUploadPaletteBG1Loop:
	sep #$20
	lda.b [ThreeBytePointerLo],y				;load word from ThreeBytePointer

	sta.l PaletteBuffer,x				;store in bg1 map buffer
	rep #$31

	iny
	inx
	dec.b TempBuffer					;word-dec length counter
	bne DMAUploadPaletteBG1Loop		;done if length counter = 0
	
	sep #$20
	inc.b NMIPaletteUploadFlag
	plb
	plp
	rts

	
ClearPalette:
   PHX
   PHP
   REP #$30		; mem/A = 8 bit, X/Y = 16 bit
   SEP #$20
   STZ $2121
   LDX #$0100
ClearPaletteLoop:
   STZ $2122
   DEX
   BNE ClearPaletteLoop

   PLP
   PLX
   RTS
   
ClearPaletteBuffer:
	php
	rep #$31
	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #$200
	ldx.w #PaletteBuffer&$ffff
	jsr ClearWRAM
	
	
	plp
	rts
/*

	ldx.w #$0200
ClearPaletteBufferLoop:
	dex
	stz.w PaletteBuffer&$ffff,x
	cpx.w #$0000
	bne ClearPaletteBufferLoop
	rts
*/
ClearBg1TilemapBuffer:
	php
	rep #$31
	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #$800
	ldx.w #Bg1MapBuffer&$ffff
	jsr ClearWRAM
	plp	
	rts
	


/*
	php
	rep #$30
	lda.w #$0000
	ldx.w #$0800
ClearBg1TilemapBufferLoop:
	sta.l Bg1MapBuffer,x
	dex
	dex
	bne ClearBg1TilemapBufferLoop
	plp
	rts
*/	
ClearBg2TilemapBuffer:
	php
	rep #$31
	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #$800
	ldx.w #Bg2MapBuffer&$ffff
	jsr ClearWRAM
	plp	
	rts

/*
	php
	rep #$30
	lda.w #$0000
	ldx.w #$0800
ClearBg2TilemapBufferLoop:
	sta.l Bg2MapBuffer,x
	dex
	dex	
	bne ClearBg2TilemapBufferLoop
	plp
	rts
*/
ClearBg3TilemapBuffer:
	php
	rep #$31
	sep #$20
	lda.b #2		;clear word: $2480
	ldy.w #$800
	ldx.w #Bg3MapBuffer&$ffff
	jsr ClearWRAM
	plp	
	rts

/*
	php
	rep #$30
	lda.w #$2480				;bg3 tilemap clearing tile, priority set
	ldx.w #$0800
ClearBg3TilemapBufferLoop:
	sta.l Bg3Buffer-2,x
	dex
	dex	
	bne ClearBg3TilemapBufferLoop
	plp
	rts
*/

;uploades any otherwise unfitting piece of gfx-tiles to vram
;in:a,8bit: tileset number
;		y,16bit: vram target
; transfer length is determined by tileset
;uses TempBuffer, TempBuffer+1
GeneralVramUploader:
;	lda.w #(TabTestTile8x8TilesEnd-TabTestTile8x8Tiles)
	php
	sep #$20
	sta.b TempBuffer
	phb
	lda.b #$7e
	pha
	plb
	rep #$31
	sei
	tya
	ldy.b DmaFifoPointer
	sta.w DmaFifoEntryTarget&$ffff,y		;vram target 2116

	lda.b TempBuffer										;multiply by 5
	and.w #$ff
	sta.b TempBuffer
	asl a
	asl a
	clc
	adc.b TempBuffer
	tax
	lda.l (GeneralTilesetsLUT+BaseAdress),x		;get source adress
	sta.b TempBuffer
	
	sta.w DmaFifoEntrySrcLo&$ffff,y		;source 4302
	inx
	lda.l (GeneralTilesetsLUT+BaseAdress),x		;get source adress
	
	sta.w DmaFifoEntrySrcHi&$ffff,y		;source 4303
	
	inx
	inx
	lda.l (GeneralTilesetsLUT+BaseAdress),x		;get source length
	sta.w DmaFifoEntryCount&$ffff,y


	sep #$20
	lda #1					;transfer type normal dma
	sta.w DmaFifoEntryType&$ffff,y
	
	rep #$31
	tya						;update fifo entry pointer
	clc
	adc.w #DmaFifoEntryLength
	sta.b DmaFifoPointer
	cli
	
	
	plb
	plp
	rts	


IncBrightness:
	php
	rep #$30
	sep #$20
	lda FrameCounterLo	;load current frame
	and BrightnessSpeed	;and check selected bits(the more, the slower)
	bne DontIncBrightness	;only change brightness every second frame
	lda ScreenBrightness
	and #$0f			;and only brightness regs
	cmp #$0f			;skip increase
	beq DoneIncBrightness	;if maximum brightness is reached
	inc ScreenBrightness	;change brightness
	plp
	rts	
DontIncBrightness:
	plp
	rts				;do nothing
DoneIncBrightness
	plp
	rts				;and return

DecBrightness:
	lda FrameCounterLo	;load current frame
	and BrightnessSpeed	;and check selected bits(the more, the slower)
	bne DontDecBrightness	;only change brightness every second frame
	lda ScreenBrightness
	and #$0f			;and only brightness regs
	beq DoneDecBrightness
	dec ScreenBrightness	;change brightness
	rts	
DontDecBrightness:
	rts				;do nothing
DoneDecBrightness:
	inc CurrentEvent		;go to next event if done with brightness change
	rts				;and return
DisableScreen:
	php
	sep #$20
	lda.b #%10000000			;force blanking
	sta $2100			;


	plp
	rts
	
EnableScreen:
	php
	sep #$20


	LDA $4210		;reg $4210  - NMI status, reading resets
	cli
	lda ScreenBrightness	;setup screen brightness
	and #$7f			;screen always on and enabled
	sta $2100
	plp
	rts

ResetScrollOffsets:
	php
	rep #$31
	lda.w #$3ff
	sta.w BG1VOfLo		;
	sta.w BG2VOfLo		;

	stz.w BG1HOfLo
	stz.w BG2HOfLo		;reset all bg1/bg2 offsets
	stz.w BG2VOfLo		;

	stz.w ScreenPixelPositionX
	stz.w ScreenPixelPositionY
	plp
	rts

;**********************************
;fade in/fade out
;in:	a/8bit: number of sample when fadeout should start
;event is incremented automatically once fadeout is complete
;**********************************
FadeInFadeOut:
	cpx.w FrameCounterLo
	bpl EventRoutineContinue

	lda #$03
	sta BrightnessSpeed
	jsr DecBrightness
	rts	

EventRoutineContinue:
	lda #$03
	sta BrightnessSpeed
	jsr IncBrightness
	rts
	



