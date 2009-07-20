/*
;partially clears wram to predefined value
;in: 	a,8bit: number of word to clear memory with. 
;		x,16bit: target word adress in wram bank $7e
;		y,16bit: transfer length
;how to use:
	rep #$31
	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #$200
	ldx.w #PaletteBuffer&$ffff
	jsr ClearWRAM
*/	
	
ClearWRAM:
   php
   phb
   sep #$20
   pha
   lda.b #$80
   pha
   plb
   pla
   REP #$31		; mem/A = 8 bit, X/Y = 16 bit
   
   and.w #$7	;calculate adress of clear pattern word(8 entries max)
   asl a
   adc.w #ClearWramBytePatterns
   sta.w $4312	;dma source
   
   SEP #$20

   lda.b #(:ClearWramBytePatterns+BaseAdress>>16)
   STA $4314         ;Set source bank to $00
   
   stx.w $2181	;store target wram adress in bank $7e
   stz.w $2183	;bank $7e

   LDX #$800a
   STX $4310         ;Set DMA mode to fixed source, WORD to $2180

   sty.w $4315         ;Set transfer size
   LDA #$02
   STA $420B         ;Initiate transfer
   plb
   plp
   RTS

;byte patterns to clear wram with.(8 entries max)   
ClearWramBytePatterns:
	.dw 0			;zeros
	.dw $eaea		;nops
	.dw $2480		;bg3 tilemap clear word
	.dw $00c9		;oam buffer
	.dw $2907		;bg1 tilemap clear

;clears whole vram   
ClearVRAM:
   pha
   phx
   php
   REP #$30		; mem/A = 8 bit, X/Y = 16 bit
   SEP #$20
   
   LDA #$80
   sta.w $2100			;set blanking active
   stz.w $4200			;disable irqs
   STA $2115         ;Set VRAM port to word access
   LDX #$1809
   STX $4310         ;Set DMA mode to fixed source, WORD to $2118/9
   LDX #$0000
   STX $2116         ;Set VRAM port address to $0000
   STX $0000         ;Set $00:0000 to $0000 (assumes scratchpad ram)
   STX $4312         ;Set source address to $xx:0000
   LDA #$00
   STA $4314         ;Set source bank to $00
   LDX #$FFFF
   STX $4315         ;Set transfer size to 64k-1 bytes
   LDA #$02
   STA $420B         ;Initiate transfer
   STZ $2119         ;clear the last byte of the VRAM
   lda.b ScreenBrightness
   sta.w $2100				;reenable screen and irqs
   lda.b InterruptEnableFlags
   sta.w $4200
   plp
   plx
   pla
   RTS

;copy random data to wram
;in:	TempBuffer0-2 - source pointer
;			x							- wram bank $7e target
;			y							- transfer length							
DmaToWRAM:
   php
   phb
   sep #$20
   pha
   lda.b #$80
   pha
   plb
   pla
   REP #$31		; mem/A = 8 bit, X/Y = 16 bit
   
	 lda.b TempBuffer
   sta.w $4312	;dma source
   
   SEP #$20

   lda.b TempBuffer+2
   STA $4314         ;Set source bank to $00
   
   stx.w $2181	;store target wram adress in bank $7e
   stz.w $2183	;bank $7e

   LDX #$8002
   STX $4310         ;Set DMA mode to inc source, WORD to $2180

   sty.w $4315         ;Set transfer size
   LDA #$02
   STA $420B         ;Initiate transfer
   plb
   plp
   RTS

;uploads 1 hirom bank to ram bank $7f  
ROMToWRAM:
   php
   phb
   sep #$20
   pha
   lda.b #$80
   pha
   plb
   pla
   REP #$31		; mem/A = 8 bit, X/Y = 16 bit
   
	stz.w $4315         ;Set transfer size
   stz.w $4312	;dma source
   stz.w $2181	;$7f0000
   SEP #$20

   lda.b #$c0
   STA $4314         ;Set source bank to $00
   
   lda.b #1
   sta.w $2183	;bank $7e

   LDX #$8002
   STX $4310         ;Set DMA mode to inc source, WORD to $2180

   
   LDA #$02
   STA $420B         ;Initiate transfer
   plb
   plp
   RTS   
   
   