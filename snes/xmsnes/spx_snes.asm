;------------------------------------------------------------------------------------------------------------------------
; Copyright (c) 2007, Mukunda Johnson
; 
; All rights reserved.
; 
; Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
; 
;     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
;     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
;     * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
;
;  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
; "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
; LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
; A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
; PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
; LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
; NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;-----------------------------------------------------------------------------------

.equ REG_APUI00	$2140
.equ REG_APUI01	$2141
.equ REG_APUI02	$2142
.equ REG_APUI03	$2143

.include "memmap.inc"		; replace with your memory definitions

.MACRO SPX_RECEIVE_MESSAGE
	sta spx_message
	and #15
	jsl MessageReceived		; SET THIS TO YOUR MESSAGE HANDLER
.ENDM

.define SPX_STACK_SIZE		$10		; increase stack size if neccesary

.define SPX_XMSOFFSET		$2100

.MACRO SPX_SYNC
	lda spx_validation		; load validation
-	cmp REG_APUI03			; sync
	bne -
.ENDM

.ramsection "spx_var" BANK 0 SLOT 1
spx_spvar:			db
spx_validation:		db
spx_package_adr:	dsb 3

spx_stack:			dsb SPX_STACK_SIZE*4	; FIFO stack
spx_stack_r:		dw						; stack read position
spx_stack_w:		dw						; stack write position

spx_var1:			dw
spx_var2:			dw
spx_var3:			dw
spx_var4:			dw

spx_message:		db

ddd:				db

.ends

.bank 0
.SECTION "SPX_SNES"

spx_binary:
.incbin "spx_binaries\spx_core.bin"
spx_lft:
.incbin "spx_binaries\spx_lft.bin"
spx_aft:
.incbin "spx_binaries\spx_aft.bin"
spx_aftf:
.incbin "spx_binaries\spx_aftf.bin"

.INDEX 16
	
;----------------------------------------------------------------------------------------------------
BootSPC:
	ldx #spx_binary
	lda #:spx_binary

	; x = source address	(word)	16-BIT INDEX
	; a = bank				(byte)	8-BIT AKKU
	
	sei						; disable interrupts during upload
	
	stx spx_var1			; store source address in var1
	sta spx_var1+2			; store bank

	REP #$20				; 16-bit akku
	lda #$BBAA				; Check if SPC is ready
-	cmp REG_APUI00			;
	bne -					;
	
	SEP #$20			; 8-bit akku
	ldy #0				; reset data counter
	lda #0				; reset port0 data (for protocol)
	xba					; swap out
	
	; load first block
	lda #$01				; port1 = NOT 0
	sta REG_APUI01			;
	
	REP #$20				; 16-bit akku
	lda [spx_var1], y		; load data transfer address
	iny						; increase data pointer
	iny						;
	sta REG_APUI02			; ports2/3 = TRANSFER ADDRESS
	
	SEP #$20				; 8-bit akku
	lda #$CC				; port0 = $CC (protocol)
	sta REG_APUI00			;
	
-	cmp REG_APUI00			; wait for SPC reply
	bne -					;
	
	REP #$20				; 16-bit akku
	lda [spx_var1], y		; load data LENGTH
	tax						; transfer to X
	SEP #$20				; 8-bit akku
	iny						; increase data pointer
	iny						; 
	lda [spx_var1], y		; load first byte (real data)
	iny						; increase pointer
	sta REG_APUI01			; port1 = data
	
	lda #0					; port0 = 0 (start sending data)
	sta REG_APUI00			; 
	
-	cmp REG_APUI00			; wait for SPC reply
	bne -					;
	
	dex						; prepare loop
	xba						; swap counter/data
	
scr_data_loop:
	lda [spx_var1], y		; load byte
	iny						; inc pointer
	xba						; swap to counter
	
-	cmp REG_APUI00			; check/wait for SPC reply
	bne -					;
	
	ina						; increase counter
	REP #$20				; port1 = data
	sta REG_APUI00			; port0 = counter
	SEP #$20				;
	
	xba						; swap to data
	
	dex						; decrease loop counter
	bne scr_data_loop		; loop
	
	xba						; swap to counter
	
-	cmp REG_APUI00			; check/wait for SPC reply
	bne -					;
	
	xba						; swap to data
	
	REP #$20				; 16-bit akku
	pha						; preserve
	lda [spx_var1], y		; load next block size
	tax						; transfer to X
	pla						; restore
	SEP #$20				; 8-bit akku
	
	iny						; increase data pointer
	iny						;
	cpx #0					; load another block? (if block size is 0, then no)
	beq scr_terminate		; ...
	
	REP #$20			; yes..
	pha					; preserve
	lda [spx_var1], y	; load something
	tax					; transfer to X
	pla					; restore
	SEP #$20			; 8-bit akku
	
	iny					; increase pointer
	iny					;
	lda #1				; port1 = NOT 0
	sta REG_APUI01		; 
	stx REG_APUI02		; port2/3 = transfer address
	xba					; swap to counter
	ina					; counter += 2, != 0
-	ina					;
	beq -				;

	sta REG_APUI00		; store in port0

-	cmp REG_APUI00		; wait for SPC reply
	bne -				;
	bra scr_data_loop	; jump to data loop
	
scr_terminate:				; no...
	stz REG_APUI01			; port1 = 0
	REP #$20				; 16-bit akku
	pha						; preserve
	lda [spx_var1], y		; load program start address
	tax						; transfer to X
	pla						; restore
	SEP #$20				; 8-bit akku
	
	stx REG_APUI02			; port2/3 = program start address
	xba						; swap to counter
	ina						; counter += 2, != 0
-	ina						; 
	beq -					;

	sta REG_APUI00			; port0 = counter
-	cmp REG_APUI00			; wait for SPC reply
	bne -					;
							; TRANSFER COMPLETE.
	cli						; enable interrupts

	jmp SPX_Init			; Initialize
;-----------------------------------------------------------------------------------------------------
SPX_Init:
	lda #0					; reset validation
	sta spx_validation
	sta spx_stack_w			; reset stack read/write
	sta spx_stack_r
	
	REP #$20
	lda #$FEED				; initial sync
-	cmp REG_APUI00			;
	bne -
	SEP #$20
	RTL

;-----------------------------------------------------------------------------------------------------
SPX_Transfer_XMS:
	; ayy = 24-bit address
	sty spx_var1			; save address
	sta spx_var2
	rep #$20				; 16bit everything
	rep #$10
	lda [spx_var1]			; load length
	tax						; x = length/3
	ldy #SPX_XMSOFFSET
	lda spx_var1
	pha
	sep #$20
	lda spx_var2
	jsl SPX_Transfer

	ply
	rtl
;-----------------------------------------------------------------------------------------------------
.accu 8
SPX_Transfer_LFT:
	sep		#$20				; 8-bit akku
	rep		#$10				; 16-bit index
	ldx		#(768/3)			; set transfer length (bytes/3)
	ldy		#(spx_lft & 65535)	; load snes offset
	phy							; push
	ldy		#$300				; $300-$5FF = linear frequency LUT
	lda		#:spx_lft			; get bank#

	JSL		SPX_Transfer		; transfer data

	ply							; free stack

	lda.b	#$1C				; set table
	sta		REG_APUI02			;
	stz		REG_APUI01			;
	JSL		SPX_SEND			;
	
	RTL							; return

;-----------------------------------------------------------------------------------------------------
SPX_Transfer_AFT:
	sep		#$20				; 8-bit akku
	rep		#$10				; 16-bit index
	ldx		#(768/3)			; set transfer length (bytes/3)
	ldy		#(spx_aft & 65535)	; load snes offset
	phy							; push
	ldy		#$300				; $300-$5FF = amiga period LUT
	lda		#:spx_aft			; get bank#
	JSL		SPX_Transfer		; transfer data
	ply							; free stack
	
	ldx		#1365				; 4096/3, rounded down
	ldy		#(spx_aftf & 65535)	; load snes offset
	phy							; push
	ldy		#$F000				; $F000-$FFFF = amiga->freq LUT
	lda		#:spx_aftf			; get bank#
	JSL		SPX_Transfer		; transfer data
	ply							; free stack
	lda		#$1C				; set table
	sta		REG_APUI02			;
	lda		#$01				;
	sta		REG_APUI01			;
	JSL		SPX_SEND			;
	RTL							; return
	
;-----------------------------------------------------------------------------------------------------
.INDEX 16
.ACCU 8

.MACRO SPX_TRANSFER_INCPOINTER
	iny					; increase pointer
	iny					;
	bpl ++				; check for overflow
	cpy #$8001			; check for an overflow reading
	bne +				; fix data if so
	dey					;
	dey					;
	lda [spx_var1], y	; read low byte
	sta REG_APUI00		; store
	ldy #$0000			; read high byte
	inc spx_var1+2		; next bank
	lda [spx_var1], y	;
	sta REG_APUI01		;
	iny					;
	lda spx_validation	;
	xba					;
	bra ++				;
+						; if not just increase bank#
	inc spx_var1+2		; increase bank#
	ldy #$0000			; reset counter
++
.ENDM

SPX_Transfer_SAMP:
	sei					; $14 = SAMPLE TRANSFER
	sta spx_var1+2		;
	SPX_SYNC			;
	lda #$14			;
	jmp SPX_TRANSFER_MOD;

SPX_Transfer:
	; SPX_Transfer
	; parameters:
	; a = file bank			:8
	; x = length/4			:16
	; y = spc offset		:16
	; stack:1 = snes_offset	:16
	
	; types
	;  0 = xms
	;  1 = freq table
	
	sei						; disable interrupts
	
	sta spx_var1+2			; store bank#
	
	SPX_SYNC				; sync with spc

	lda #$1A				; $1A = GENERIC TRANSFER
SPX_TRANSFER_MOD:
	sta REG_APUI02			; set message type
	
	REP #$20				; set spc write position
	tya						; 
	SEP #$20				;
	sta REG_APUI00			;
	xba						;
	sta REG_APUI01			;
	
	lda spx_validation		; validate data
	eor #128				;
	ora #1
	sta REG_APUI03			;
-	cmp REG_APUI03			; wait for spc to respond
	bne -
	
;	eor #128				; prepare transfer mode
	
	sta spx_validation		; save

	REP #$20		; 16-bit akku
	lda 4, S		; load file offset sp+4

	sec				; set carry
	sbc #$8000		; subtract
	tay				; transfer to y
	SEP #$20		; 8-bit akku
	lda #$80		; set offset in var1
	stz spx_var1	; reset mem pointers
	sta spx_var1+1	;

	REP #$20			; 16-bit akku
	
_stf_start:				; loop:
	
	lda [spx_var1], y	; load data
	sta spx_var3		; save
	
	SPX_TRANSFER_INCPOINTER
	
	sep #$20
	lda [spx_var1], y	; get third byte
	
	iny					; increase pointer
	bpl +
	ldy #$0000
	inc spx_var1+2
+
	sta spx_spvar		; store
	lda spx_validation	; get validation
	eor #128			;
	sta spx_validation	; update
	eor #128			; reverse

	phx					; preserve
	ldx spx_spvar		; get ready
	
-	cmp REG_APUI03		; sync with spc
	bne -				;

	lda spx_var3		; load byte1
	sta REG_APUI00		; store byte1
	lda spx_var3+1		; load byte2
	sta REG_APUI01		; store byte2
	stx REG_APUI02		; store byte3/validation
	plx					; restore
	rep #$20
	
	dex					; decrease counter
	bne _stf_start		; loop until finished

	sep #$20
	
	stz REG_APUI03		; send 0
	stz spx_validation
	lda #0
-	cmp REG_APUI03		; wait for reply
	bne -
	
	cli					; enable interrupts
	RTL					; return --make sure higher function frees stack space
	
;--------------------------------------------------------------------------------------------------------
.index 16
SPX_Queue:
	; a = $00/$01 message
	; x = $02/$03 params
	; accumulator can be 8 or 16 bit, do not read anything with it
	ldy spx_stack_w		; load stack position
	sta spx_stack, y	; store bytes0/1
	iny					; increase pointer
	iny
	txa					; get high word
	sta spx_stack, y	; store bytes 2/3
	iny					; increase pointer
	iny
	cpy #SPX_STACK_SIZE*4	; wrap to stack size
	bcc +
	ldy #0
+
	sty spx_stack_w		; save stack position
	rtl

;---------------------------------------------------------------------------------------------------------
.accu 8
SPX_Routine:
	; get messages
	lda REG_APUI00		; check if port0 is different
	cmp spx_message
	beq +
	SPX_RECEIVE_MESSAGE	; if so then a message was received
+
	lda spx_validation	; check if spc has processed last message
	cmp REG_APUI03
	beq +
	rtl			; not ready
+
	ldy spx_stack_r		; load stack read position
	cpy spx_stack_w		; exit function if it equals write position (no messages)
	bne +
	rtl
+
	lda spx_stack, y	; load byte0
	sta REG_APUI00		; store
	iny
	lda spx_stack, y	; load byte1
	sta REG_APUI01		; store
	iny
	lda spx_stack, y	; load byte2
	sta REG_APUI02		; store
	iny
	lda spx_stack, y	; load byte3
	lda spx_validation	; add validation
	and #128
	eor #128
	ora spx_stack, y	; store
	sta REG_APUI03		; message dispatched
	sta spx_validation	; save validation
	
	iny

	cpy #SPX_STACK_SIZE*4	; wrap read counter to stack size
	bcc +
	ldy #0
+
	sty spx_stack_r		; save

	rtl

SPX_Flush:				; flushes queue
	jsl SPX_Routine		; call routine
	ldy spx_stack_r		; check for more messages
	cpy spx_stack_w
	bne SPX_Flush		; loop
	rtl					; exit
	
SPXM_Play:
	ldx #$1E			; $1E = play message
	jmp SPX_Queue

SPXM_BuildDir:
	ldx #$1B				; $1B = build directory
	jmp SPX_Queue

SPXM_Reset:					; blocking function
	ldx #$1D				; $1D = RESET XMS
	jmp SPX_Queue

SPXM_SetVol:
	; a = volume
	ldx #$18				; $18 = set XM playback volume
	jmp SPX_Queue

SPX_SetVol:
	; a = volume L } 16bit akku
	; b = volume R } resets afterwards
	ldx #$19				; $19 = change main volume
	jmp SPX_Queue

.accu 8
.index 16
SPXS_Play:
	; a = volume [llllrrrr]
	; x = sample#/priority/frequency [0ccfffffssssssss]

	pha			; rearrange data
	rep #$20
	txa
	sep #$20
	pha
	rep #$20
	and #$FF00
	ora #$20	; $20 = play sample
	tax
	sep #$20
	pla
	xba
	pla
	rep #$20
	
	jsl SPX_Queue
	sep #$20
	rtl

SPXS_SetParam:
	; a = index
	; x = param
	xba				; rearrange data
	tax
	xba
	rep #$20
	ldx #$21		; $21 = message
	jsl SPX_Queue	; queue
	sep #$20
	rtl
		
SPX_SEND:					; blocking send
	lda spx_validation		; get validation
	eor #128				; change
	sta REG_APUI03			; set port data
	sta spx_validation		; save validation
-	cmp REG_APUI03			; wait for spc reply
	bne -

	RTL						; exit

SPXP_InstallPackage:
	; x = address (16-bit)	;
	; a = bank#
	stx spx_package_adr		; store offset
	sta spx_package_adr+2	; store bank
	rtl
	
SPXP_LoadSong:
	; x = index (16-bit)
	
	sep #$20						; 8bit akku
	txa								; a=x
	sta $4202						; multiply a*3
	lda #3							; 
	sta $4203		; 8 cycles...	; ...
	
	; setup send
	rep #$20						; read package address

	lda #$10
	nop
	nop
	nop								; small delay
	clc
	adc $4216						; add multiplication result
	tay								; y=result
	
	lda spx_package_adr				; read package address
	adc [spx_package_adr], y		; add song offset
	ora	#$8000						;
	pha								; save result
	sep #$20						; 8bit akku
	lda spx_package_adr+2			; load bank#
	iny
	iny
	adc [spx_package_adr], y		; add song offset
	ply								; restore address	
	
	jsl SPX_Transfer_XMS			; transfer song
	
	; transfer samples
	
	SPX_SYNC
	
	lda #$17						; ask for sample requests
	sta REG_APUI02
	jsl SPX_SEND
--
	
	; message received
	lda REG_APUI02					; satisfy request
	cmp #$9C						; 9c = no more samples
	beq +
	ldx REG_APUI00
	ldy #0
	jsl SPXP_LoadSample				; send sample
	
	jsl SPX_SEND					; sync
	
	jmp --
+
	rtl
	
SPXP_LoadSample:
	; x = index (16-bit)
	; y = spc address, 0=use next available
	
	rep #$20						; 16-bit akku
	txa								; a=x
	sta spx_var1					; save
	asl								; shift
	adc spx_var1					; add (a = index*3 now)
	phy
	ldy #4
	adc [spx_package_adr],y			; add sample table offset
	
	tay
	clc
	lda spx_package_adr
	adc [spx_package_adr], y
	ora #$8000						; wrap around ROM area if overflow
	sta spx_var1
	
	sep #$20
	lda spx_package_adr+2
	iny
	iny
	adc [spx_package_adr], y
	adc #0							; increase on overflow
	sta spx_var2
	
	rep #$20
	
	lda [spx_var1]					; load sample size/3
	tax								; transfer to x
	
	lda spx_var1					; load snes address
	
	ply
	pha
	sep #$20						; 8-bit akku
	lda spx_var2					; load bank#
	
	jsl SPX_Transfer_SAMP			; transfer data
	ply								; restore stack
	
	lda spx_validation				; get sample #
	eor #128
	sta REG_APUI03
-	cmp REG_APUI03
	bne -
	sta spx_validation
	
	lda REG_APUI00
	
	rtl								; return
	
.ENDS
