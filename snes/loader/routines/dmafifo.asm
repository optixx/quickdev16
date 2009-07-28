/*
todo:
add a special case for uploading 2 vertical rows of the tilemap.
doing this with 32 dma transfers of 4 bytes each is incredibly slow.
better make a tight loop using vram acces 32byte increment that uploads the tilemap rows manually.


guesstimate is that the dma fifo routine can transfer 130 bytes per scanline on average.
normal transfers shouldnt be smaller than 128 bytes or so, otherwise the time needed to set up a dma transfer throws
our timing off way too much.

scanlines 200-261 can be used for uploads.
maximum transfer length should be 2048 bytes.
nmi eats about 8 scanlines.

current guesstimate is 6784/$1a80 bytes per frame.

this routine is used to dma transfer data to vram in bulk.
other routines can queue writes into a fifo buffer and these then get transferred to vram during nmi or forced blank.

;this only works for transfers from a-bus to vram to keep things simple
;a transfer length of 0 marks the end of the buffer

;WARNING!
;this routine MUST have finished till nmi starts or it will produce glitched transfers! no longer true. was caused by some status stuff not being pushed to stack during irq entry.
;overscan is enabled to lengthen the time available for dma'ing stuff from the dma fifo to vram
;glitched transfer risk can be minimized by writing the dma count value last!!

number of scanlines:	irq trigger:	nmi trigger normal:	irq trigger overscan:	
ntsc: 261		200		225			240
pal:  311		200		225			240




maximum number of entries: 128


dma queue format:
offset:	length:	function:
00	2	transfer length
02	2	vram target
04	2	a-bus source offset	
06	1	a-bus source bank

areas needed:
DmaFifoBuffer	ds $1c0


variables needed:
DmaFifoPointer		dw		;relative pointer to current free entry in buffer
DmaFifoSourcePointerLo	dw	
DmaFifoSourcePointerBa	db

fixed variable needed:
.define DmaFifoEntryLength	7

routine features:

InitDmaFifo:
	-clear dma fifo buffer
	
ProcessDmaFifo:
	-setup initial dma regs
	-process the list until a length of 0 is encountered
	-reset DmaFifoPointer to 0

transfer types:

00=void, no transfer
01=normal dma from a-bus adress to vram
02=special manual transfer mode to upload vertical tilemap rows 
03=special transfer,16x16 sprite
04=special transfer, 32x32 sprite
05=special transfer, 64x64 sprite
06=special vwf transfer, converts 2bpp tiles to 4bpp. transfer length is number of tiles, not bytelength

how to write entries into table, m/x=16bit, new:

;transfer type 1, normal a-bus to vram dma:
ldx.b DmaFifoPointer
lda #1					;transfer type
sta.l DmaFifoEntryType,x
sta.l DmaFifoEntryCount,x		;length 4305
sta.l DmaFifoEntryTarget,x		;vram target 2116
sta.l DmaFifoEntrySrcLo,x		;source 4302
sta.l DmaFifoEntrySrcHi,x		;source 4303
sta.l DmaFifoEntrySrcBank,x		;source 4304

	txa						;update fifo entry pointer
	clc
	adc.w #DmaFifoEntryLength
	sta.b DmaFifoPointer

;transfer type 2, upload vertical tilemap row from wram bank 7e to vram
ldx.b DmaFifoPointer
lda #2					;transfer type
sta.l DmaFifoEntryType,x
sta.l DmaFifoEntryTarget,x		;vram target 2116
sta.l DmaFifoEntryCount,x		;number of lines to upload (one line has 32 entries), maximum 32 lines
sta.l DmaFifoEntrySrcLo,x		;source 4302
sta.l DmaFifoEntrySrcHi,x		;source 4303
;sta.l DmaFifoEntrySrcBank,x		;source 4304


	
*/

;waits for all dma transfers to be done and then one frame
WaitDmaTransfersDone:
	php
	rep #$31
WaitDmaTransfersDoneLoop:
	lda.b DmaFifoPointer
	bne WaitDmaTransfersDoneLoop


	plp
	rts

;waits 2 frames to be sure all settings have been written to the video regs
WaitFrame:
;	rts
	php
	rep #$31

WaitFrameIncDone:
	lda.b FrameCounterLo
	cmp.b FrameCounterLo
	beq WaitFrameIncDone
/*
WaitFrameIncDone1:
	lda.b FrameCounterLo
	cmp.b FrameCounterLo
	beq WaitFrameIncDone1
*/

	plp
	rts
	
InitDmaFifo:
	php
	rep #$31
	lda.w #0		;clear word: $0000
	sta.l (DmaFifoPointer+$7e0000)			;reset fifo pointer
	sta.l (DmaFifoPointerIrq+$7e0000)			;reset fifo pointer
	ldy.w #DmaFifoEntryLength*256
	ldx.w #DmaFifoBuffer&$ffff
	jsr ClearWRAM		
	
	
;	sta.l (DmaFifoOverhang+$7e0000)
;	sta.l (DmaFifoTotalBytesTransferred+$7e0000)

	plp
	rts

ProcessDmaFifoTypeLUT:
	.dw ProcessDmaFifoTypeVoid
	.dw ProcessDmaFifoTypeNormalDma
	.dw ProcessDmaFifoTypeVTilemapRow
	.dw ProcessDmaFifoType16x16Sprite
	.dw ProcessDmaFifoType32x32Sprite
	.dw ProcessDmaFifoType64x64Sprite
	.dw ProcessDmaFifoTypeVwf2bppTo4bpp
	.dw UploadBg1Tilemap
	.dw UploadBg2Tilemap

	.dw UploadBg3Tilemap
	.dw NMIUploadPal
	.dw NMIUploadOAM
	.dw ProcessDmaFifoTypeVoid	
	.dw ProcessDmaFifoTypeVoid
	.dw ProcessDmaFifoTypeVoid
	.dw ProcessDmaFifoTypeVoid
	.dw ProcessDmaFifoTypeVoid	

ProcessDmaFifoTypeVoid:
;	sec
	rts

ProcessDmaFifoTypeVwf2bppTo4bpp:
;setup some dma regs that always stay the same on all transfers:	
	lda.w #$1801			;Set the DMA mode (word, normal increment)
	sta.w $4300 
	
	lda.b [DmaFifoSourcePointerLo],y
	and.w #$7f								;128 tiles max(vram vwf buffer size)
	beq ProcessDmaFifoTypeVwf2bppTo4bppAbort	
	
	sta.b TempBufferIrq						;number of tiles to transfer, counter
/*	
	asl a									;multiply by 16(length of one 2bpp tile)
	asl a
	asl a
	asl a
	clc
	adc.b DmaFifoTotalBytesTransferred
	sta.b DmaFifoTotalBytesTransferred
*/
	lda.w #16								;transfer length, one tile
	sta.w $4305
	
	
	
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.b TempBufferIrq+2			;vram target
	sta.w $2116
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.b TempBufferIrq+4			;dma source
	sta.w $4302
	iny
	lda.b [DmaFifoSourcePointerLo],y	;dma source, high byte and bank
	iny
	iny
;	iny
	sta.w $4303

ProcessDmaFifoTypeVwf2bppTo4bppLoop:
	sep #$20
	lda #$80
	sta.w $2115			;set VRAM transfer mode to word-access, increment by 1

	lda.b #$01
	sta.w $420b

	dec.b TempBufferIrq			;counter
	beq ProcessDmaFifoTypeVwf2bppTo4bppAbort		;all tiles uploaded?

	
	rep #$31
	
	lda.b TempBufferIrq+2
;	clc
	adc.w #32/2					;add one 4bpp tile to vram target
	sta.b TempBufferIrq+2
	sta.w $2116
	lda.b TempBufferIrq+4
	clc
	adc.w #16					;add one 2bpp tile to wram source
	sta.b TempBufferIrq+4
	sta.w $4302
	lda.w #16				;transfer length, one line
	sta.w $4305
	bra ProcessDmaFifoTypeVwf2bppTo4bppLoop
	
ProcessDmaFifoTypeVwf2bppTo4bppAbort:
	rts

ProcessDmaFifoType16x16Sprite:
	lda.w #$1801			;Set the DMA mode (word, normal increment)
	sta.w $4300 
	
	lda.w #64				;transfer length, one line
	sta.w $4305
/*
	lda.w #64*8
	clc
	adc.b DmaFifoTotalBytesTransferred
	sta.b DmaFifoTotalBytesTransferred
*/	
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.b TempBufferIrq			;vram target
	sta.w $2116
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.b TempBufferIrq+2			;dma source
	sta.w $4302
	iny
	lda.b [DmaFifoSourcePointerLo],y	;dma source, high byte and bank
	iny
	iny
;	iny
	sta.w $4303
     

	sep #$20
	lda #$80
	sta.w $2115			;set VRAM transfer mode to word-access, increment by 1

	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #64					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #64				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b
	rts

ProcessDmaFifoType32x32Sprite:
	lda.w #$1801			;Set the DMA mode (word, normal increment)
	sta.w $4300 

	lda.w #128				;transfer length, one line
	sta.w $4305
/*
	lda.w #128*8
	clc
	adc.b DmaFifoTotalBytesTransferred
	sta.b DmaFifoTotalBytesTransferred
*/	
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.b TempBufferIrq			;vram target
	sta.w $2116
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.b TempBufferIrq+2			;dma source
	sta.w $4302
	iny
	lda.b [DmaFifoSourcePointerLo],y	;dma source, high byte and bank
	iny
	iny
;	iny
	sta.w $4303
     

	sep #$20
	lda #$80
	sta.w $2115			;set VRAM transfer mode to word-access, increment by 1

	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #128					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #128				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #128					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #128				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #128					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #128				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b	
	rts

;takes 19 lines for 2048 bytes. a single 2048 byte transfer takes 12 lines, so this could be sped up a bit still
ProcessDmaFifoType64x64Sprite:
	lda.w #$1801			;Set the DMA mode (word, normal increment)
	sta.w $4300 

;	lda.b [DmaFifoSourcePointerLo],y
;	beq ProcessDmaFifoTypeNormalDmaAbort
	lda.w #256				;transfer length, one line
	sta.w $4305
/*
	lda.w #256*8
	clc
	adc.b DmaFifoTotalBytesTransferred
	sta.b DmaFifoTotalBytesTransferred
*/	
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.b TempBufferIrq			;vram target
	sta.w $2116
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.b TempBufferIrq+2			;dma source
	sta.w $4302
	iny
	lda.b [DmaFifoSourcePointerLo],y	;dma source, high byte and bank
	iny
	iny
;	iny
	sta.w $4303
     

	sep #$20
	lda #$80
	sta.w $2115			;set VRAM transfer mode to word-access, increment by 1

	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #256					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #256				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #256					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #256				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #256					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #256				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #256					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #256				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #256					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #256				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b
	
	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #256					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #256				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b

	rep #$31
	lda.b TempBufferIrq
;	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBufferIrq
	sta.w $2116
	lda.b TempBufferIrq+2
	clc
	adc.w #256					;add one tileline in source
	sta.b TempBufferIrq+2
	sta.w $4302
	lda.w #256				;transfer length, one line
	sta.w $4305
	sep #$20
	lda.b #$01
	sta.w $420b					
	rep #$31			;clear carry, dont abort processing fifo list
	rts


ProcessDmaFifoTypeNormalDma:
	lda.w #$1801			;Set the DMA mode (word, normal increment)
	sta.w $4300 

	lda.b [DmaFifoSourcePointerLo],y
	beq ProcessDmaFifoTypeNormalDmaAbort
	sta.w $4305
/*	
	cmp.w #DmaFifoMinTransferSize				;check if transfer is lower than 64 and load 64 for counter if it is. otherwise, these transfers screw up our calculation
	bcs ProcessDmaFifoTypeNormalDmaBigEnough

	lda.w #DmaFifoMinTransferSize				;load minimum length
ProcessDmaFifoTypeNormalDmaBigEnough:
	clc
	adc.b DmaFifoTotalBytesTransferred
	sta.b DmaFifoTotalBytesTransferred
*/	
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.w $2116
	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.w $4302
	iny
	lda.b [DmaFifoSourcePointerLo],y
	iny
	iny
;	iny
	sta.w $4303
;	lda #$18  			;Set the destination register (VRAM gate)
;	sta $4301      

	sep #$20
	lda #$80
	sta.w $2115			;set VRAM transfer mode to word-access, increment by 1

	lda.b #$01
	sta.w $420b

	rep #$31			;clear carry, dont abort processing fifo list
	rts

ProcessDmaFifoTypeNormalDmaAbort:	;abort and go to next entry if a transfer length of 0 is encountered
	iny
	iny
	iny
	iny
	iny
	iny
	iny
	rts

ProcessDmaFifoTypeVTilemapRow:
	lda.w #$1801			;Set the DMA mode (word, normal increment)
	sta.w $4300 

	sep #$20
	lda #%10000001
	sta $2115			;set VRAM transfer mode to word-access, increment by 64

	rep #$31
;	lda.b [DmaFifoSourcePointerLo],y	;get number of lines to transfer
;	and #31					;maximum number of lines:32
;	tax

	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	sta.w $2116				;store vram target

	iny
	iny
	lda.b [DmaFifoSourcePointerLo],y
	tax					;source offset in bank $7e
	iny
	iny
	iny
	lda.l $7e0000,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$40,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$80,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$c0,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$100,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$140,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$180,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$1c0,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$200,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$240,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$280,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$2c0,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$300,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$340,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$380,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$3c0,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$400,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$440,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$480,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$4c0,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$500,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$540,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$580,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$5c0,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$600,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$640,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$680,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$6c0,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$700,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$740,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$780,x	
	sta $2118			;write data to vram
	lda.l $7e0000+$7c0,x	
	sta $2118			;write data to vram
	rts
	


/*	
pointer
pointerirq

-if pointer=0, exit immediately
-get pointerirq and do transfer its pointing to, then increase pointerirq by one entry
-if pointerirq is equal or bigger than pointer, clear pointer and pointerirq

*/
ProcessDmaFifo:
	php
	sep #$20
;setup pointer to fifo buffer
	lda.b #$7e
	sta.b DmaFifoSourcePointerBa
	rep #$31
	lda.w #DmaFifoBuffer&$ffff
	sta.b DmaFifoSourcePointerLo
;	ldy.b DmaFifoOverhang			;normally 0. if the list wasnt finished last frame, we finish it now

	ldy.b DmaFifoPointer
	beq ProcessDmaFifoExit

	
ProcessDmaFifoLoop:
	ldy.b DmaFifoPointerIrq	
	lda.b [DmaFifoSourcePointerLo],y	;get transfer type
	phy													;save for later
	
	and.w #$000f								;16 different transfer types only

	asl a
	tax
	iny													;prepare pointer for dma upload routine
	php
	jsr (ProcessDmaFifoTypeLUT,x)
	plp
	
	ply

	lda.w #0
	sta.b [DmaFifoSourcePointerLo],y		;clear this transfer type to be on the safe side
	iny
	iny
	sta.b [DmaFifoSourcePointerLo],y
	iny
	iny
	sta.b [DmaFifoSourcePointerLo],y
	iny
	iny
	sta.b [DmaFifoSourcePointerLo],y
	iny
	iny
	tya
	sta.b DmaFifoPointerIrq						;update process pointer
	cmp.b DmaFifoPointer
	bcc ProcessDmaFifoNotDone					;done when process pointer is bigger than queue pointer			
	
	stz.b DmaFifoPointerIrq						;clear both pointers
	stz.b DmaFifoPointer
	bra ProcessDmaFifoExit


ProcessDmaFifoNotDone:
;	cpy.w #DmaFifoEntryLength*128	;check if whole list was processed
	


;check if scanline 238 has been passed:
	sep #$20

	lda.b #$80
	sta.w $4201							;reset latch
	nop
	nop
	nop
	nop
	lda.w $2137							;latch current scanline
	lda.w $213f							;reset $213d lo/hi byte counter
	lda.w $213d							;get low byte
	xba
	lda.w $213d							;get high byte
	xba	
	rep #$31
	and.w #$1ff
	cmp.w #199
	bcc ProcessDmaFifoExit	;exit if lower than scanline 200 (wrapped around)
	cmp.w #239
	bcc ProcessDmaFifoLoop	;exit if bigger than scanline 237 (nearly at end of irq timeframe)

	
ProcessDmaFifoExit:

;	jsr InitDmaFifo
;	sty.b DmaFifoPointer		;should always be zero
	plp
	rts



/*
UploadBg1TilemapFifo:
	php
	rep #$31
	ldx.b DmaFifoPointer
	lda #1					;transfer type normal dma
	sta.l DmaFifoEntryType,x
	lda.b VramBg1Tilemap
	sta.l DmaFifoEntryTarget,x		;vram target 2116
	lda.w #Bg1MapBuffer & $ffff
	sta.l DmaFifoEntrySrcLo,x		;source 4302
	lda.w #$800
	sta.l DmaFifoEntryCount,x		;length 4305
	sep #$20
	lda.b #(Bg1MapBuffer >> 16)
	sta.l DmaFifoEntrySrcBank,x		;source 4304
	rep #$31
	txa						;update fifo entry pointer
	adc.w #DmaFifoEntryLength
	sta.b DmaFifoPointer


	plp
	rts
*/



NMIUploadPal:
;transfer palette buffer to cgram
	sep #$20
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
;	stz.b NMIPaletteUploadFlag
	rts

NMIUploadOAM:
;transfer sprite buffer to oamram
	sep #$20
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
;	stz.b NMIOamUploadFlag		;clear upload flag
	rts
	
BgTilemapSizeLUT:
	.dw $800
	.dw $1000
	.dw $1000
	.dw $2000
	
;NMIUploadBg1:
UploadBg1Tilemap:
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
;	stz.b NMIBg1UploadFlag
	rts
	
;NMIUploadBg2:	
UploadBg2Tilemap:
;transfer bg2 tilemap to vram if needed:
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
;	stz.b NMIBg2UploadFlag
	rts	


UploadBg3Tilemap:
;transfer bg3 tilemap to vram if needed:
	rep #$31
	lda.b BG3TilemapVram
	and.w #%11					;get tilemap size
	asl a
	tax
	lda.l (BgTilemapSizeLUT+BaseAdress),x
	sta.w $4305   			;Store the size of the data block
	sep #$20
	ldx.b VramBg3Tilemap
	stx.w $2116			;vram adress $0000
	ldx.w #Bg3MapBuffer & $ffff
	stx.w $4302			;Store the data offset into DMA source offset
;	ldx.w #$800
;	stx.w $4305   			;Store the size of the data block
	lda.b #(Bg3MapBuffer >> 16)
	sta.w $4304			;Store the data bank of the source data
	lda.b #$80
	sta.w $2115			;set VRAM transfer mode to word-access, increment by 1
	lda.b #$01			;Set the DMA mode (word, normal increment)
	sta.w $4300       
	lda.b #$18    			;Set the destination register (VRAM gate)
	sta.w $4301      
	lda.b #$01    			;Initiate the DMA transfer
	sta.w $420B
;	stz.b NMIBg3UploadFlag
	rts	







	