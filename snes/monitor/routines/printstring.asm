/*
tiny text output system for snes by d4s in 2006
call every frame in nmi, should be executed before global palette upload: DMATileMapToVramBG3

call every time a textstring should be loaded: LoadTextString


text byte commands:

#$00 - terminate string
#$01 - set new offset, only executed if not first byte in string(has 2 additional bytes)
#$02 - set font
#$03 - draw string from special adress, but with fixed length(has 4 additional bytes, first byte length, last 3 bytes string vector, maximum string length: 32 letters)
#$04 - draw string from special adress(has 3 additional bytes for string vector)
#$05 - draw byte in hexadecimal(has 3 additional bytes for string vector)
#$06 - draw byte in binary(has 3 additional bytes for string vector)
#$07 - change palette number(AND'ed with 0x7)
#$08 - draw byte in decimal




variables needed:

Bg3Status			bit 0=font and palette uploaded

PrintStringThreeBytePointerLo
PrintStringThreeBytePointerHi
PrintStringThreeBytePointerBank
CurrentStringTarget
CurrentStringTargetHi
FontSelector
FrameCounterLo
FixedStringLength
PrintStringBuffer0
PrintStringBuffer1
PrintStringBuffer2
PrintStringBuffer3
PrintStringBuffer4
PrintStringBuffer5
PrintStringBuffer6
PrintStringBuffer7
PrintStringBuffer8
PrintStringBuffer9
PrintStringPalette			;real number of palette


ram areas needed:
Bg3Buffer
PaletteBuffer


data needed:
BG38x8FontTiles
BG38x8FontPalette
TextStrings
TextStringPTable
ASCIITable

static vars needed:
FontTileConfig

*/

DMATilesToVramBG3:
	php
	rep #$30
;	sep #$20
	
;	lda.b #$03			;set vram adress of bg3/4 tiles $6000
;	lda.b BGTilesVram34
;	and.w 
;	sta.l $210c
;	lda.b #$30			;bg3 tilemap @ vram $7000, 32x32 tiles
;	sta $2109
;	stz $2112			;set bg3 v-offset to $0000
;	stz $2112

	lda #0
	jsr UploadFontBG3
	jsr ClearBg3TilemapBuffer
	plp
	rts


DMATileMapToVramBG3:
	php
	rep #$30
	sep #$20
	lda.b NMIBg3UploadFlag
	beq DontDMATileMapToVramBG3

	stz.b NMIBg3UploadFlag			;clear upload flag
	ldx.b DmaFifoPointer
	lda #1					;transfer type
	sta.l DmaFifoEntryType,x	
;DMATileMapToVramBG3DontUploadFont:
	lda.b #$7e			;get source bank of tilemap buffer
;	sta.w $4304			;Store the data bank of the source data
	sta.l DmaFifoEntrySrcBank,x		;source 4304
	
	rep #$30			;accu 16bit
	lda.w #Bg3MapBuffer&$ffff		;get source offset
;	sta.w $4302
	sta.l DmaFifoEntrySrcLo,x		;source 4302
	lda.w #$0800
;	sta.w $4305   			;Store the size of the data block
	sta.l DmaFifoEntryCount,x		;length 4305
;	ldx.w #$3000
;	stx.w $2116			;vram adress $7000
	lda.b BG3TilemapVram
	and.w #%11111100		;mask off bg size bits
	xba
;	sta.w $2116
	sta.l DmaFifoEntryTarget,x		;vram target 2116
	
	txa						;update fifo entry pointer
	clc
	adc.w #DmaFifoEntryLength
	sta.b DmaFifoPointer

	
/*
	sep #$20
	lda #$80
	sta $2115			;set VRAM transfer mode to word-access, increment by 1
	lda #$01			;Set the DMA mode (word, normal increment)
	sta $4300       
	lda #$18    			;Set the destination register (VRAM gate)
	sta $4301      
	lda #$01    			;Initiate the DMA transfer
	sta $420B
*/
DontDMATileMapToVramBG3:
	plp
	rts


;in: a,8bit: number of font to upload
UploadFontBG3:
	php
	rep #$31
;	sep #$20
	and.w #$ff	
	asl a
	asl a
	tax					
	lda.l (Bg38x8FontLUT+BaseAdress+2),x	;get font length
	pha
	lda.l (Bg38x8FontLUT+BaseAdress),x		;get font pointer
	pha
	
	ldx.b DmaFifoPointer
	sep #$20
	lda #1					;transfer type
	sta.l DmaFifoEntryType,x

	lda.b #(:Bg38x8FontLUT+BaseAdress>>16)	;get source bank
;	sta $4304			;Store the data bank of the source data
	sta.l DmaFifoEntrySrcBank,x
	rep #$30

;	lda.w #BG38x8FontTiles
;	sta $4302			;Store the data offset into DMA source offset
	pla
	sta.l DmaFifoEntrySrcLo,x		;source 4302

;	lda.w #(BG38x8FontTilesEnd-BG38x8FontTiles)			;get size of font
;	sta $4305 		  	;Store the size of the data block
	pla
	sta.l DmaFifoEntryCount,x		;length 4305

	lda.b BGTilesVram34
	and.w #$0f
	clc
	ror
	ror
	ror
	ror
	ror					;put into highest nibble
	clc
	adc.w #$400				;add $800 for tilemap
;	sta $2116			;vram destination adress $1000(bg1 tilespace)
	sta.l DmaFifoEntryTarget,x		;vram target 2116
/*
	sep #$20
	lda #$80
	sta $2115			;set VRAM transfer mode to word-access, increment by 1
	lda #$01			;Set the DMA mode (word, normal increment)
	sta $4300       
	lda #$18  			;Set the destination register (VRAM gate)
	sta $4301      
	lda #$01    			;Initiate the DMA transfer
	sta $420B
*/

	txa						;update fifo entry pointer
	clc
	adc.w #DmaFifoEntryLength
	sta.b DmaFifoPointer


DMAUploadPaletteBG3:
	lda #(:BG38x8FontPalette+BaseAdress>>16)			;get source bank of tilemaps
	sta PrintStringThreeBytePointerBank		;
	rep #$30				;accu 16bit
	lda.w #BG38x8FontPalette			;get source offset of tilemap
	sta PrintStringThreeBytePointerLo
	lda.w #(BG38x8FontPaletteEnd-BG38x8FontPalette)
	sta.b TempBuffer
;	ldx.w #$0008				;get length
	
	lda.w #FontTileConfig			;get palette number
	and.w #%00011100		
	lsr a					
	lsr a
	asl a
	asl a
	asl a
	tax					;calculate starting offset in palette buffer and put in x
	ldy.w #$0000				;clear target/source counter						
DMAUploadPaletteBG3Loop:
	lda [PrintStringThreeBytePointerLo],y				;load word from PrintStringThreeBytePointer
	sta.w PaletteBuffer&$ffff,x				;store in bg3 pal buffer, palette 1
	iny					;word-inc copy counter
	iny
	inx					;word-dec length counter
	inx
	dec TempBuffer
	dec TempBuffer
	bne DMAUploadPaletteBG3Loop		;done if length counter = 0
	sep #$20
	inc.b NMIPaletteUploadFlag
	plp
	rts


LoadTextString:
	pha
	php
	sep #$20
	phb					;set data bank = wram
	lda.b #$7e
	pha
	plb

	lda.b #FontTileConfig
	and.b #%00011100
	sta.b PrintStringPalette
	lda #(:TextStrings+BaseAdress>>16)			;get source bank of strings
	sta PrintStringThreeBytePointerBank		;
	rep #$30				;accu 16bit
	txa					;get number of string
	asl					;multiply by 2 to get pointertable entry
	tax
	lda.l (TextStringPTable+BaseAdress),x			;get source offset of string
	sta PrintStringThreeBytePointerLo
	ldy.w #$0000

	lda [PrintStringThreeBytePointerLo],y		;get target buffer in wram
	and.w #$fffe				;mask off bit 0 to ensure text is word-formatted properly
	sta.w CurrentStringTarget
	iny					;increment source position, word
	iny			

;process all script commands with jump table.
;if processing a fixed length string, dont execute any script codes at all
LoadTextStringFetchLetter:
	rep #$30

	tya
	cmp.w #$01ff				;maximum string length: 256 bytes
	bcs ScriptCommandExit
	
	sep #$20
	lda #$00				;set upper byte of a to zero
	xba
	
	
	
	lda.b FixedStringLength
	beq NoFixedLengthDynaString
	
	lda.b FixedStringLength
	dec a
	sta.b FixedStringLength
	cmp.b #$01
	bne NoScriptCommand

	jmp ScriptCommandExit

NoFixedLengthDynaString:
	rep #$30
	lda [PrintStringThreeBytePointerLo],y		;check if we are processing a script command
	and.w #$00f0
	bne NoScriptCommand

	lda [PrintStringThreeBytePointerLo],y		;get script command
	and.w #$000f
	clc
	asl a
	tax
	jmp (ScriptCommandPointerTable,x)

NoScriptCommand:
	sep #$20

TextStringNotTerminated:
	lda [PrintStringThreeBytePointerLo],y
	tax
	lda.w FontSelector
	beq TextStringNormalFont

;special 8x16 font

	lda.l (ASCIITable8x16+BaseAdress),x		;get tile value for current letter
	
;	clc
	
	
	xba
	lda.b #FontTileConfig			;palette and mirroring config for font tiles
	and.b #%11100011			;clear old palette number
	ora.b PrintStringPalette		;set palette number
	xba
	rep #$30				;now we have the full tileinfo for the letter
	
	ldx.w CurrentStringTarget	;get current position in tilemap
	sta.l Bg3MapBuffer,x			;store upper tile
	
	clc
	adc.w #$0010				;add 16 to get tile number of lower tile
	sta.l (Bg3MapBuffer+$40),x			;store lower tile
	
	sep #$20
	iny					;advance to next letter
	inx
	inx					;increment pointer in bg1 tilemap
	stx.w CurrentStringTarget
	
	jmp LoadTextStringFetchLetter

ScriptCommandExit:	
	sep #$20
	stz.b FixedStringLength
	stz.w FontSelector		;reset font
	inc.b NMIBg3UploadFlag
	plb
	plp
	pla
	rts


;normal 8x8 font
TextStringNormalFont:
	lda.l (ASCIITable+BaseAdress),x		;get tile value for current letter
;	sec
;	sbc.b #$50
	clc
	adc.b #$30
	xba
	lda.b #FontTileConfig			;palette and mirroring config for font tiles
	bcc TextStringNormalFontNoWrap
	ora.b #1				;set tile number msb

TextStringNormalFontNoWrap:
	and.b #%11100011			;clear old palette number
	ora.b PrintStringPalette		;set palette number
	xba
	rep #$30				;now we have the full tileinfo for the letter
	
	ldx.w CurrentStringTarget		;get current position in tilemap
	sta.l Bg3MapBuffer,x
	
	sep #$20
	iny					;advance to next letter
	inx
	inx					;increment pointer in bg1 tilemap
	stx.w CurrentStringTarget
	
	jmp LoadTextStringFetchLetter

ScriptCommandPrintByte:
	rep #$30
	iny
	lda [PrintStringThreeBytePointerLo],y		;get ThreeByte adress of byte
	pha
	iny
	lda [PrintStringThreeBytePointerLo],y
	sta.w PrintStringThreeBytePointerHi		;and store in ThreeByte pointer
	pla
	sta.w PrintStringThreeBytePointerLo
	ldy.w #$0000				;reset source pointer

	sep #$20
	lda.b #$04				;print 2 letters + overhang
	sta.b FixedStringLength

	lda [PrintStringThreeBytePointerLo],y
	pha
	lsr a					;get upper nibble
	lsr a
	lsr a
	lsr a
	and.b #$0f
	clc
	adc.b #$30				;add ASCII "0" to every letter
	cmp.b #$3a				;check if bigger than ASCII "9"
	bcc PrintByteNotA1

	adc.b #$06				;add until ASCII "A" if bigger than 9(carry always set, so -1)
PrintByteNotA1:
	sta.w PrintStringBuffer0&$ffff		;store in first letter
	pla
	and.b #$0f
	clc
	adc.b #$30				;add ASCII "0" to every letter
	cmp.b #$3a				;check if bigger than ASCII "9"
	bcc PrintByteNotA2

	adc.b #06				;add until ASCII "A" if bigger than 9(carry always set, so -1)
PrintByteNotA2:
	sta.w PrintStringBuffer0&$ffff+1		;store in second letter

	lda.b #$7e				;
	sta.b PrintStringThreeBytePointerBank
	rep #$30
	lda.w #PrintStringBuffer0&$ffff
	sta.b PrintStringThreeBytePointerLo
	jmp LoadTextStringFetchLetter
		

ScriptCommandPrintByteBinary:
	rep #$30
	iny
	lda [PrintStringThreeBytePointerLo],y		;get ThreeByte adress of byte
	pha
	iny
	lda [PrintStringThreeBytePointerLo],y
	sta.b PrintStringThreeBytePointerHi		;and store in ThreeByte pointer
	pla
	sta.b PrintStringThreeBytePointerLo
	ldy.w #$0000				;reset source pointer
	tyx
	
	sep #$20
	lda.b #10				;print 8 letters + overhang
	sta.b FixedStringLength


ScriptCommandBinaryConvLoop:
	rep #$30
	txa
	asl a
	tax
	sep #$20


	lda [PrintStringThreeBytePointerLo],y
	jmp (ScriptCommandBinaryConvJtbl,x)

ScriptCommandBinaryConvJtbl:
	.dw ScriptCommandBinaryConv+0
	.dw ScriptCommandBinaryConv+1
	.dw ScriptCommandBinaryConv+2
	.dw ScriptCommandBinaryConv+3
	.dw ScriptCommandBinaryConv+4
	.dw ScriptCommandBinaryConv+5
	.dw ScriptCommandBinaryConv+6
	.dw ScriptCommandBinaryConv+7
	
ScriptCommandBinaryConv:
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a

	clc
	and.b #01
	adc.b #$30				;add until ASCII "0" or "1", depending on bit0

	pha
	rep #$30
	txa
	lsr a
	tax
	sep #$20
	pla

	sta.w PrintStringBuffer0&$ffff,x
	inx
	cpx.w #$0008
	bne ScriptCommandBinaryConvLoop

	lda.b #$7e				;
	sta.b PrintStringThreeBytePointerBank
	rep #$30
	lda.w #PrintStringBuffer0&$ffff
	sta.b PrintStringThreeBytePointerLo
	jmp LoadTextStringFetchLetter



ScriptCommandGetDynaStringFixLength:
	sep #$20
	iny
	lda [PrintStringThreeBytePointerLo],y		;get string length
	inc a
	inc a
	and.b #$1f					;maximum string length: 32 letters
	sta.b FixedStringLength

	
ScriptCommandGetDynaString:
	rep #$30
	iny
	lda [PrintStringThreeBytePointerLo],y		;get ThreeByte adress of dynamic string
	pha
	iny
	lda [PrintStringThreeBytePointerLo],y
	sta.w PrintStringThreeBytePointerHi		;and store in ThreeByte pointer
	pla
	sta.w PrintStringThreeBytePointerLo
	ldy.w #$0000				;reset source pointer
	jmp LoadTextStringFetchLetter
	
ScriptCommandNewPosition:
	rep #$30
	iny
	lda [PrintStringThreeBytePointerLo],y		;get new offset in bg3 buffer
	and.w #$fffe
	sta.w CurrentStringTarget
	iny
	iny
	lda.w #$0000
	sep #$20
	stz.w FontSelector		;reset font
	jmp LoadTextStringFetchLetter

ScriptCommandNewFont:
	sep #$20
	lda.w FontSelector
	eor.b #$01				;xor bit 0
	sta.w FontSelector		;save to font selection var
	iny					;increment string pointer
	jmp LoadTextStringFetchLetter


ScriptCommandChangePalette:
	sep #$20
	iny
	lda [PrintStringThreeBytePointerLo],y	;get new palette number
	asl a
	asl a
	and.b #%00011100			;maximum number of palettes:8, move to correct location in tile config
	
	sta.b PrintStringPalette
	iny					;increment string pointer
	jmp LoadTextStringFetchLetter





ScriptCommandVoid:
	jmp ScriptCommandExit			;exit if an unimplemented script code is executed
	
StringSpace:
	ldx.w CurrentStringTarget
	inx
	inx					;increment target buffer pointer by 2
	stx.w CurrentStringTarget
	iny					;increment string pointer
	rts
	
HdmaFontPaletteGradient:
	rts


;alternates every 64 frames between 2 consecutive textstrings
;in: x,16bit: number of first textstring
FlashTextString:
	php
	rep #$30
	sep #$20
	lda.w FrameCounterLo
	bit #$20
	beq FlashTextStringShowFirstString

	inx						;get next textstring

FlashTextStringShowFirstString:

	jsr LoadTextString

	plp
	rts


ScriptCommandPointerTable:
	.dw ScriptCommandExit				;0
	.dw ScriptCommandNewPosition
	.dw ScriptCommandNewFont
	.dw ScriptCommandGetDynaStringFixLength
	.dw ScriptCommandGetDynaString
	.dw ScriptCommandPrintByte			;5
	.dw ScriptCommandPrintByteBinary
	.dw ScriptCommandChangePalette
	.dw ScriptCommandPrintByteDecimal
	.dw ScriptCommandVoid
	.dw ScriptCommandVoid				;10
	.dw ScriptCommandVoid
	.dw ScriptCommandVoid
	.dw ScriptCommandVoid
	.dw ScriptCommandVoid
	.dw ScriptCommandVoid


ScriptCommandPrintByteDecimal:
	rep #$30
	iny
	lda [PrintStringThreeBytePointerLo],y		;get ThreeByte adress of byte
	pha
	iny
	lda [PrintStringThreeBytePointerLo],y
	sta.b PrintStringThreeBytePointerHi		;and store in ThreeByte pointer
	pla
	sta.b PrintStringThreeBytePointerLo
	ldy.w #$0000				;reset source pointer
	tyx
	
	sep #$20
	lda.b #5				;print 8 letters + overhang
	sta.b FixedStringLength

;have 3 variables, hexa value, a hexa counter and decimal(word).
;increment decimal word and hexa counter until hexa value is reached.
;if hexa counter=a: add 7 to go to 10

	rep #$31

	stz.b TempBuffer+2			;hex counter
	stz.b TempBuffer+4			;decimal counter
	
	lda [PrintStringThreeBytePointerLo],y
	and.w #$ff
	sta.b TempBuffer			;target hex value
	
	beq DeciConvPrint			;skip counting if zero

ScriptCommandDecimalConvLoop:	
	
	inc.b TempBuffer+4			;inc decimal
	lda.b TempBuffer
	lda.b TempBuffer+4
	and.w #$f
	cmp.w #$a
	bne DeciConvNoOverf1

	lda.b TempBuffer+4
	clc
	adc.w #6
	sta.b TempBuffer+4

DeciConvNoOverf1:
	lda.b TempBuffer+4
	and.w #$f0
	cmp.w #$a0
	bne DeciConvNoOverf2

	lda.b TempBuffer+4
	clc
	adc.w #$60
	sta.b TempBuffer+4



DeciConvNoOverf2:

	dec.b TempBuffer
	bne ScriptCommandDecimalConvLoop

DeciConvPrint:
;print to buffer
	sep #$20
	lda.b TempBuffer+5
	and.b #$f
	sta.b TempBuffer+6
	bne DeciConvPrint1			;dont print leading zero
	
	lda.b #$f0
DeciConvPrint1:
	clc
	adc.b #$30				;add until ASCII "0" or "1", depending on bit0
	sta.w PrintStringBuffer0&$ffff


	lda.b TempBuffer+4
	lsr a
	lsr a
	lsr a
	lsr a
	and.b #$f
	bne DeciConvPrint2			;dont print leading zero
	cmp.b TempBuffer+6
	bne DeciConvPrint2
	
	lda.b #$f0
DeciConvPrint2:

	clc
	adc.b #$30				;add until ASCII "0" or "1", depending on bit0
	sta.w PrintStringBuffer0&$ffff+1

	lda.b TempBuffer+4
	and.b #$f
	clc
	adc.b #$30				;add until ASCII "0" or "1", depending on bit0

	sta.w PrintStringBuffer0&$ffff+2



	
	
	

;	sep #$20
	lda.b #$7e				;
	sta.b PrintStringThreeBytePointerBank
	rep #$30
	lda.w #PrintStringBuffer0&$ffff
	sta.b PrintStringThreeBytePointerLo
	jmp LoadTextStringFetchLetter

MessagePendingDelete:
	php
	rep #$31
;	sep #$20
	lda.w MessageDeleteCounter
	beq MessagePendingDeleteInactive
	cmp.w #1
	bne MessagePendingDec

	ldx.w #52
	jsr LoadTextString		;delete message if counter is 1

MessagePendingDec:	
	dec.w MessageDeleteCounter
MessagePendingDeleteInactive:	
	plp
	rts
	