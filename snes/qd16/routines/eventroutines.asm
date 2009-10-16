	.include "routines/main.h"
.section "events" semifree

EventPtTable:
	.dw EventRoutine0
	.dw EventRoutine1
	.dw EventRoutine2
	.dw EventRoutine3
	.dw EventRoutine4
	
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

	
;	jsr InitOam
	jsr InitHdma
	jsr ResetScrollOffsets
	jsr InitDmaFifo
;	jsr ClearColObjList
;	jsr ClearZBuffer
	jsr PalEffectInit

	jsr ClearVRAM
	jsr ClearBg3TilemapBuffer		;cls
	jsr ClearBg2TilemapBuffer		;cls
	jsr ClearBg1TilemapBuffer		;cls
	
	inc.b NMIBg1UploadFlag	
	inc.b NMIBg2UploadFlag
	inc.b NMIBg3UploadFlag
	stz.b FocusScreenFlags
	lda.b #0
	sta.b CheckJoypadMode		;set joypad check to 8 players
	lda.b #$80
	sta.w IrqBrightnessIncDec
	lda.b #20
	sta.w MaxGravObjCount


	lda.b #0				;load bgmode config #1
	jsr SetBGMode
;	jsr DMATilesToVramBG3
;	jsr ClearBg3TilemapBuffer		;cls
	
	lda.b #0
	jsr SpcPlaySong
	
	stz.b LoadMenuDoInit

	inc.b CurrentEvent
	lda.b #13				;window effect
	jsr CreateHdmaEffect
	lda.b #14				;wavy scroll
	jsr CreateHdmaEffect

	rep #$31
	lda.w #%00100100
	ldx.w #0
	ldy.w #0
	jsr UploadBgObject
	lda.w #%00101001+$0100
	ldx.w #0
	ldy.w #0
	jsr UploadBgObject

	lda.w #0
	jsr VwfCreateTextbox
	
	lda.w #11
	jsr PalEffectCreate

	lda.w #12
	jsr PalEffectCreate
	
	lda.w #$0504
	sta.w SpcReportType
	
	jsr SpcSetReportType	
	rts

;wait for rom upload to start
EventRoutine1:
	sep #$20
	lda.w Qd16Flags
	lsr a
	bcc EventNoRomLoad
	
	inc.b CurrentEvent

	rep #$31
	lda.w #1
	jsr VwfCreateTextbox	;print "loading rom"
	sep #$20
		
EventNoRomLoad:	
	jsr ShakeShake	
	jsr QD16CommandPoll
	jsr VwfHandler
	jsr ProcessHdmaList
	jsr PalEffectHandler
	rts

;seek rom header:
EventRoutine2:	
	jsr QD16CheckHeaderValid
	bcc SeekHeaderNotFound

	rep #$31
	lda.l RomHeaderSize,x	;get romsize
	txy
	sec
	sbc.w #8
	and.w #7
	tax
	lda.l RomSizeLUT+BaseAdress,x
	sta.w Qd16Mbit
	tya
	bne SeekHeaderHirom

	lda.w #5
	jsr VwfCreateTextbox	;print lorom header
	bra SeekHeaderNotFound

SeekHeaderHirom:

	lda.w #6
	jsr VwfCreateTextbox	;print hirom header

SeekHeaderNotFound:	
	jsr ShakeShake	
	jsr QD16CommandPoll
	jsr VwfHandler	
	jsr ProcessHdmaList
	jsr PalEffectHandler
	rts

RomSizeLUT:
	.db 2
	.db 4
	.db 8
	.db 16
	.db 32
	.db 64
	.db 0
	.db 0
ShakeShake:
	php
	sep #$20
	lda.b #$0f
	sta.b ScreenBrightness

	lda.l SpcReportBuffer+8
	beq event1Noshake

	lda.b Mosaic
	clc
	adc.b #$80
	sta.b Mosaic
event1Noshake:
;	jsr ObjectProcessor

	lda.b Mosaic
	and.b #$f0
	beq Event1MosDec
	
	sec
	sbc.b #$10
Event1MosDec:
	ora.b #%10
	sta.b Mosaic
	plp
	rts

;wait some frames
EventRoutine3:
	sep #$20
	dec.w FrameWait
	bne EventRoutine3Wait

	lda.w EventBuffer
	sta.b CurrentEvent

EventRoutine3Wait:
	rts

;fade out, dont accept any new commands
EventRoutine4:
	jsr VwfHandler	
	jsr ProcessHdmaList
	jsr PalEffectHandler
	rts

WaitFrameEvent:
	pha
	php
	sep #$20
	sta.w FrameWait
	lda.b CurrentEvent
	sta.w EventBuffer
	lda.b #3
	sta.b CurrentEvent
	plp
	pla
	rts

.ends
		