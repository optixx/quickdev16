
CheckJoypadModeLUT:
	.dw	CheckJoypadSinglePlayer
	.dw CheckJoypad8Player
	.dw CheckJoypadVoid
	.dw CheckJoypadVoid
	
CheckJoypadVoid:
	rts
	

;slow joypad checker, gets data for all 8 joypads with iobit=0.
;writes collected data to 8 individual joypad buffers
CheckJoypad8Player:  	
	lda $4212
	bit #$01
	bne CheckJoypad8Player
	
	rep #$31
	lda.w $4218
	sta.w JoyPortBuffer&$ffff	;port1 pad 1
	lda.w $421c
	sta.w JoyPortBuffer&$ffff+2	;port1 pad 2	
	lda.w $421a
	sta.w JoyPortBuffer&$ffff+8	;port2 pad 1
	lda.w $421e
	sta.w JoyPortBuffer&$ffff+10	;port2 pad 2

;assume we don't need to latch again:
	sep #$20	
  
	lda.b #%11000000	;enable iobit on joyport1&2
	sta.w $4201

	lda.b #1
  	sta.w $4016		;enable latching

  	stz.w $4016		;latching end


	rep #$31  

  	ldx #16			;get 32 bits from every port
CheckJoyPortsLoop3:
	lda.w $4016
      	lsr a			;put bit0 into carry
      	rol.w JoyPortBuffer&$ffff+4		;port 1 pad 3
      	lsr a			;put bit1 into carry
      	rol.w JoyPortBuffer&$ffff+6		;port 1 pad 4
	
	asl a			;get upper byte
	asl a
	xba

      	lsr a			;put bit0 into carry
      	rol.w JoyPortBuffer&$ffff+12		;port 2 pad 3
      	lsr a			;put bit1 into carry
      	rol.w JoyPortBuffer&$ffff+14		;port 2 pad 4
      	dex
  	bne CheckJoyPortsLoop3	

	sep #$20
	lda.b #1
  	sta.w $4016			;enable latching
  	nop					;short delay is needed, otherwise mp5 sometimes doesn't respond in time.
  	nop
	lda.w $4016			;discard first read
	lda.w $4016
	and.b #%00000010	;get data1,2 bits only
;	cmp.b #%10			;mp5 on port1?
	bne Port1MP5Connected
	
	stz.w JoyPortBuffer&$ffff+4		;clear this joypads data if no mp5 connected
	stz.w JoyPortBuffer&$ffff+5		;clear this joypads data if no mp5 connected
	
Port1MP5Connected:
	lda.w $4017			;discard first read
	lda.w $4017
	and.b #%00000010	;get data1,2 bits only
;	cmp.b #%10			;mp5 on port2?
	bne Port2MP5Connected
	
	stz.w JoyPortBuffer&$ffff+$c		;clear this joypads data if no mp5 connected
	stz.w JoyPortBuffer&$ffff+$d		;clear this joypads data if no mp5 connected
	
Port2MP5Connected:	
  	stz.w $4016		;latching end

	rep #$31
	ldx.w #0
CheckJoyPortsTriggerLoop:
	lda.w JoyPortBufferOld&$ffff,x		;get last button state
	eor.w #$ffff				;xor
	sta.w JoyPortBufferTrigger&$ffff,x
	lda.w JoyPortBuffer&$ffff,x
	sta.w JoyPortBufferOld&$ffff,x
	and.w JoyPortBufferTrigger&$ffff,x	;and and only get buttons that werent pressed last frame
	sta.w JoyPortBufferTrigger&$ffff,x	;store in old joypad trigger buffer
	inx
	inx
	cpx.w #8*2				;process 8 entries
	bne CheckJoyPortsTriggerLoop


	sep #$20
	stz.w $4201		;clear iobit again.
	rts


;fast joy1 checker. check this late in nmi so we don't have to wait for auto joypad read to finish:
CheckJoypadSinglePlayer:
	lda $4212
	bit #$01
	bne CheckJoypadSinglePlayer

	rep #$30
	lda.w JoyPortBufferOld&$ffff	;get last button state
	eor.w #$ffff			;xor
	sta.w JoyPortBufferTrigger&$ffff
	lda $4218
	sta.w JoyPortBuffer&$ffff
	sta.w JoyPortBufferOld&$ffff
	and.w JoyPortBufferTrigger&$ffff	;and and only get buttons that werent pressed last frame
	sta.w JoyPortBufferTrigger&$ffff	;store in joypad buffer
	sep #$20
	rts
