/*
IRQ Routine
Features: jumps to different Irq routines depending on variable

Variables needed:
IrqRoutineNumber	db			;
IrqVCounter		dw

if IrqRoutineNumber=0, irq is disabled

irqloader:
actual irq routine

irqinit:
this must be called whenever a different irq is asserted

*/

;in: a,8bit=number of irq routine to set,0 is disable
IrqInit:
	php
	sep #$20
	sta.b IrqRoutineNumber
	sta.b IrqRoutineNumberBuffer
;get initial latch value
	rep #$31
	and.w #%00000111		;maximum number of routines: 8
	asl a				;word pointer
	sta.b TempBufferIrq
	asl a				;h/v latch values, multiply by 6
	clc
	adc.b TempBufferIrq
	
	tax
	
	lda.l (IrqRoutineLUT+BaseAdress+2),x
	sta.b IrqVCounter
	lda.l (IrqRoutineLUT+BaseAdress+4),x
	sta.b IrqHCounter
	
;	sta.w $2109
;	stz.w $2107
	
	plp
	rts

IrqLoader:
;	php				;backup status, then switch to 16bit a/x in order to preserve everything and not just 8bit of the accu
	rep #$39		
	pha
	phx
	phy
	phd
	phb
	
	
	lda.w #$0000
	tcd
;	jml (IrqLoaderHirom + BaseAdress)
IrqLoaderHirom:	
	sep #$20
	lda.b #$81
	pha
	plb
	
	rep #$31
	lda.b IrqRoutineNumber
	and.w #%00000111		;maximum number of routines: 8
	asl a				;word pointer
	sta.b TempBufferIrq
	asl a				;h/v latch values, multiply by 6
	clc
	adc.b TempBufferIrq
	tax
;	php
	jsr (IrqRoutineLUT,x)
 ;	plp



	
	rep #$39
	lda.w $4211			;clear irq flag
	plb
	pld
	ply
	plx
	pla
;	plp
	rti



IrqRoutineLUT:
	.dw IrqRoutine0				;pointer to routine
	.dw 200						;v-counter for this irq
	.dw 130						;h-counter for this irq
	.dw IrqRoutine1
	.dw 200
	.dw 130
	.dw IrqRoutine2
	.dw 143						;v-counter for this irq
	.dw 130						;h-counter for this irq
	.dw IrqRoutine3
	.dw 0						;v-counter for this irq
	.dw 0						;h-counter for this irq
	.dw IrqRoutine4
	.dw 0						;v-counter for this irq
	.dw 0						;h-counter for this irq
	.dw IrqRoutine5
	.dw 0						;v-counter for this irq
	.dw 0						;h-counter for this irq
	.dw IrqRoutine6
	.dw 0						;v-counter for this irq
	.dw 0						;h-counter for this irq
	.dw IrqRoutine7
	.dw 0						;v-counter for this irq
	.dw 0						;h-counter for this irq


IrqRoutine0:
	sep #$20
	lda.b #$2f			;set bg color to red
	sta.w $2132	
	rep #$31
	inc.w ExtIrqCounter
	rts
IrqRoutine1:

	sep #$20			;enable forced blank

	lda.b #$8f			;set bg color to blue
	sta.w $2132
	rep #$31	
	inc.w VIrqCounter

	rts
	
	
IrqRoutine2:
	sep #$20
/*
	lda.b MainScreen						;disable sprites below textbox top
	and.b #%01111
	sta.w $212c
*/	
	lda.b #$40
	dec.b IrqRoutineNumber			;set up dma fifo irq below

	stz.w $210f									;reset bg2 scrolling
	stz.w $210f
	stz.w $2110
	stz.w $2110
	


	lda.b BGTilesVram12					;set bg2 new tile adress
	and.b #$0f
	ora.b #$70
	sta.w $210B

	lda.b BG3TilemapVram				;use bg3 map buffer for bg2 textbox after irq split
	and.b #%11111100						;mask off bg size bits
	sta.w $2108		
	
	rep #$31
	lda.w #200									;set up irq counter for dma fifo
	sta.w $4209
	lda.w #130
	sta.w $4207
	


	

	
	rts
	
IrqRoutine3:	
IrqRoutine4:
IrqRoutine5:
IrqRoutine6:
IrqRoutine7:
	rts

;checks if tilemap upload flags are set and issues one transfer per bg neccessary
;in: 8bit accu	
IrqCheckTilemapFlags:
	lda.b NMIPaletteUploadFlag
	beq IrqDontUploadPal

	stz.w $2121			;upload colour-palette buffer at $7e:0e00 to cg-ram every frame
	ldx #PaletteBuffer&$ffff			;start at color 0 
	stx.w $4302			;Store the data offset into DMA source offset
	ldx #$0200
	stx.w $4305   			;Store the size of the data block
	lda.b #$7e
	sta.w $4304			;Store the data bank holding the tile data
	lda.b #$00			;Set the DMA mode (byte, normal increment)
	sta.w $4300       
	lda.b #$22    			;Set the destination register ( $2122: CG-RAM Write )
	sta.w $4301      
	lda.b #$01    			;Initiate the DMA transfer
	sta.w $420B
	stz.b NMIPaletteUploadFlag
		
	
IrqDontUploadPal:		
	lda.b NMIOamUploadFlag
	beq IrqDontUploadOAM

	
	stz.w $2102			;upload to oam adress 0
	stz.w $2103
	ldx.w #OamBuffer&$ffff			;start at color 0 
	stx.w $4302			;Store the data offset into DMA source offset
	ldx.w #$0220
	stx.w $4305   			;Store the size of the data block
	lda.b #$7e
	sta.w $4304			;Store the data bank holding the tile data
	lda.b #$00			;Set the DMA mode (byte, normal increment)
	sta.w $4300       
	lda.b #$04    			;Set the destination register ( $2122: CG-RAM Write )
	sta.w $4301      
	lda.b #$01    			;Initiate the DMA transfer
	sta.w $420B
	
	stz.b NMIOamUploadFlag
	
	
IrqDontUploadOAM:
	lda.b NMIBg1UploadFlag
	beq IrqDontUploadBg1

	rep #$31
	lda.b BG1TilemapVram
	and.w #%11					;get tilemap size
	asl a
	tax
	lda.l (BgTilemapSizeLUT+BaseAdress),x
	sta.w $4305   			;Store the size of the data block
;transfer bg1 tilemap to vram if needed:
	sep #$20
	ldx.b VramBg1Tilemap
	stx.w $2116			;vram adress $0000
	ldx.w #Bg1MapBuffer & $ffff
	stx.w $4302			;Store the data offset into DMA source offset
	
	lda.b #(Bg1MapBuffer >> 16)
	sta.w $4304			;Store the data bank of the source data
	lda.b #$80
	sta.w $2115			;set VRAM transfer mode to word-access, increment by 1
	lda.b #$01			;Set the DMA mode (word, normal increment)
	sta.w $4300       
	lda.b #$18    			;Set the destination register (VRAM gate)
	sta.w $4301      
	lda.b #$01    			;Initiate the DMA transfer
	sta.w $420B
	stz.b NMIBg1UploadFlag


IrqDontUploadBg1:	
	lda.b NMIBg2UploadFlag
	beq IrqDontUploadBg2

	rep #$31
	lda.b BG2TilemapVram
	and.w #%11					;get tilemap size
	asl a
	tax
	lda.l (BgTilemapSizeLUT+BaseAdress),x
	sta.w $4305   			;Store the size of the data block
	sep #$20
	ldx.b VramBg2Tilemap
	stx.w $2116			;vram adress $0000
	ldx.w #Bg2MapBuffer & $ffff
	stx.w $4302			;Store the data offset into DMA source offset
;	ldx.w #$800
;	stx.w $4305   			;Store the size of the data block
	lda.b #(Bg2MapBuffer >> 16)
	sta.w $4304			;Store the data bank of the source data
	lda.b #$80
	sta.w $2115			;set VRAM transfer mode to word-access, increment by 1
	lda.b #$01			;Set the DMA mode (word, normal increment)
	sta.w $4300       
	lda.b #$18    			;Set the destination register (VRAM gate)
	sta.w $4301      
	lda.b #$01    			;Initiate the DMA transfer
	sta.w $420B
	stz.b NMIBg2UploadFlag

IrqDontUploadBg2:
	lda.b NMIBg3UploadFlag
	beq IrqDontUploadBg3


	ldx.b DmaFifoPointer
	lda.b #9					;transfer type bg3 tilemap
	sta.l DmaFifoEntryType,x

	rep #$31
	txa						;update fifo entry pointer
	clc
	adc.w #DmaFifoEntryLength
	sta.b DmaFifoPointer
	sep #$20
	stz.b NMIBg3UploadFlag

IrqDontUploadBg3:
	rts	


IrqBrightness:
	sep #$20
	lda.w IrqBrightnessIncDec		;if "done"-flag is set, do nothing
	bmi IrqBrightnessExit

	lsr a					;increase or decrease?
	bcc IrqBrightnessDec

	lda.b ScreenBrightness			;done with increasing?
	cmp.b #$f
	beq IrqBrightnessDone

	lda.b ScreenBrightness
	inc a					;decrease
	sta.b ScreenBrightness
	
IrqBrightnessExit:
	rts
	

IrqBrightnessDec:
	lda.b ScreenBrightness			;done with decreasing?
	beq IrqBrightnessDone
	
	lda.b ScreenBrightness
	dec a					;decrease
	sta.b ScreenBrightness

	rts


IrqBrightnessDone:
	lda.w IrqBrightnessIncDec
	ora.b #$80				;set "done"-flag
	sta.w IrqBrightnessIncDec
	rts
	
	
WaitBrightnessDone:
	php
	sep #$20
	
WaitBrightnessLoop:	
	lda.w IrqBrightnessIncDec		;wait for irq to finish brightness inc/dec
	bpl WaitBrightnessLoop
	
	
	plp
	rts
		