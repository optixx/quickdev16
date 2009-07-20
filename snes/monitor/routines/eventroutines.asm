EventPtTable:
	.dw EventRoutine0
	.dw EventRoutine1
	.dw EventRoutine2
	.dw EventRoutine3

	
;boot init, also debug menu check		
EventRoutine0:
	rep #$31
	lda.w #200
	sta.w GravityCutOffYPos

	sep #$20
	lda.b #0
	sta.b ScreenBrightness
	lda.b #$80
	sta.l $2100

	jsr ROMToWRAM				;upload
	jsr InitOam
	jsr ResetScrollOffsets
	jsr InitDmaFifo
	jsr ClearColObjList
	jsr ClearZBuffer
	
	stz.b FocusScreenFlags
	lda.b #0
	sta.b CheckJoypadMode		;set joypad check to 8 players
	lda.b #$80
	sta.w IrqBrightnessIncDec
	lda.b #20
	sta.w MaxGravObjCount


	lda.b #0				;load bgmode config #1
	jsr SetBGMode
	jsr DMATilesToVramBG3
	jsr ClearBg3TilemapBuffer		;cls
	ldx.w #0
	jsr LoadTextString		;print "startup ok"
	
	stz.b LoadMenuDoInit
	lda.b #$0f
	sta.b ScreenBrightness
	inc.b CurrentEvent
	rts

;debug menu play
EventRoutine1:
	jsr ObjectProcessor
	ldx.w #0
	jsr LoadMenuFile

	ldx.w #1
	jsr LoadTextString		;print virqs
	ldx.w #4
	jsr LoadTextString		;print extirqs
	ldx.w #5
	jsr LoadTextString		;print $00:3000
	
	rts
	


;debug audio menu init	
EventRoutine2:
	sep #$20
	jsr ClearBg3TilemapBuffer		;cls
	ldx.w #3
	jsr LoadTextString		;print audio menu
	stz.b LoadMenuDoInit
	inc CurrentEvent

;debug audio menu play	
EventRoutine3:
	jsr ObjectProcessor

	ldx.w #1
	jsr LoadMenuFile
	ldx.w #13
	jsr LoadTextString		;print timecode
	ldx.w #14
	jsr LoadTextString
	ldx.w #30
	jsr LoadTextString		;print channel volume output
	ldx.w #31
	jsr LoadTextString	
	rts
		