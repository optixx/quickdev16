;this nmi uses a maximum of 20 scanlines while streaming brr frames to the spc
NMI:
	php
	rep #$39
	pha
	phb
	phd
	phx
	phy
	lda.w #$0000
	tcd
	sep #$20
	
	lda.l $004210			;reset nmi flag		
	lda #$81
	pha
	plb
	stz.w $4201			;clear iobit

	lda.b HdmaFlags		;check which hdma channels need to be activated
	and #$fc		;exclude channel #0,1(reserved for normal dma)
	sta.w $420C		;set hdma channels and disable dma channel


	rep #$31
	lda W12SEL
	sta $2123
	lda W1L
	sta $2126
	lda W2L
	sta $2128
	lda WBGLOG
	sta $212a
	lda WMS
	sta $212e

	sep #$20
	lda WOBJSEL
	sta $2125
	lda.b Mosaic
	sta.w $2106

	sep #$20	
  	ldx FrameCounterLo	;increment word framecounter
  	inx				;
  	stx FrameCounterLo		;

  	lda ScreenMode		;set screenmode and bg sizes
  	sta $2105			;
  	lda MainScreen		;setup main and subscreen
  	sta $212c			;
  	lda SubScreen		;setup main and subscreen
  	sta $212d			;
		;
  	lda BGTilesVram12		;set offsets in vram for tiles
  	sta $210B			;of bg1 and bg2
  	lda BGTilesVram34		;set offsets in vram for tiles
  	sta $210C			;of bg1 and bg2
  	lda BG1TilemapVram	;set offset of bg1 tilemap in  vram
  	sta $2107			;
  	lda BG2TilemapVram	;set offset of bg2 tilemap in  vram
  	sta $2108			;
  	lda BG3TilemapVram	;set offset of bg3 tilemap in  vram
  	sta $2109			;
  	lda BG4TilemapVram	;set offset of bg3 tilemap in  vram
  	sta $210a			;

	lda.w CGWsel		;colour add/sub config
	sta $2130
	lda.w CgadsubConfig
	sta $2131
	lda.w FixedColourB
	and.b #%00011111
	ora.b #%10000000
	sta $2132
	lda.w FixedColourG
	and.b #%00011111
	ora.b #%01000000
	sta $2132
	lda.w FixedColourR
	and.b #%00011111
	ora.b #%00100000
	sta $2132

  	lda BG1HOfLo		;set bg1 h-offset
	sta $210d			;
  	lda BG1HOfHi		;
	sta $210d			;
  	lda BG1VOfLo		;set bg1 v-offset
	sta $210e			;
  	lda BG1VOfHi		;
	sta $210e			;
  	lda BG2HOfLo		;set bg2 h-offset
	sta $210f			;
  	lda BG2HOfHi		;
	sta $210f			;
  	lda BG2VOfLo		;set bg2 v-offset
	sta $2110			;
  	lda BG2VOfHi		;
	sta $2110			;

	lda ObjSel
	sta $2101


;this (writing to regs $4209,$4207,$4200) somehow breaks vblank flag setting in reg $4210,$4200)
	lda.b IrqRoutineNumberBuffer
	sta.b IrqRoutineNumber		;if this is zero, irqs are disabled
	beq NmiDisableHIrq

	rep #$31			;store current h-counter in reg
	lda.b IrqVCounter
	sta.w $4209			;v
	lda.b IrqHCounter
	sta.w $4207			;h

	sep #$20
	lda.b InterruptEnableFlags
	ora.b #%00110000		;enable v and h irq, will take effect next frame. irq is only triggered if both positions match
	sta.b InterruptEnableFlags
	sta.w $4200			;should be ok. hope it breaks nothing

	bra NmiHIrqDone

NmiDisableHIrq:
	sep #$20
	lda.b InterruptEnableFlags
	and.b #%11001111		;disable h-irq
	sta.b InterruptEnableFlags
	sta.w $4200

NmiHIrqDone:


	sep #$20
	lda.b #$80
	sta.w $2100

	
	
	
;	jsr ProcessHdmaList
	jsr IrqCheckTilemapFlags
	jsr ProcessDmaFifo
	jsr IrqBrightness
;	jsr DMATileMapToVramBG3	

;moved to irq. that way, screen might turn black during long transfers, but at least the data gets uploaded reliably 	
  	lda ScreenBrightness	;setup screen brightness
  	and.b #$7f
  	sta $2100	

	
	jsr Random		;update random numbers
	jsr SpcHandlerMain



	rep #$31
	lda.b CheckJoypadMode
	and.w #%11
	asl a
	tax
	sep #$20
	jsr (CheckJoypadModeLUT,x)
	
	rep #$39
	ply
	plx
	pld
	plb
	pla
	plp
	rts
	rti				;return from nmi
	
