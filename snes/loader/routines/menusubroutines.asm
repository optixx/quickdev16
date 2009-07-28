MenuSubroutineLUT:
	.dw MenuSubroutineVoid			;0
	.dw MenuSubroutineJumpROM
;	.dw MenuSubroutineLoadBattle
	.dw MenuSubroutineJumpRAM
	.dw MenuSubroutineAudioMenu

	.dw MenuSubroutineWrite3000
	.dw MenuSubroutinePlaySong		;5
	.dw MenuSubroutineUploadSEPack
	.dw MenuSubroutinePlaySE
	.dw SpcStopSong				
	.dw SpcSetSongSpeed
	.dw SpcSetSongChannelMask		;10
	.dw SpcSetReportType
	.dw MenuSubroutineChsum
	.dw MenuSubroutineReturnMain
	.dw MenuSubroutineTablistRecorder
	.dw MenuSubroutineExecTablistRec	;15
	.dw MenuSubroutinePlayTablist
	.dw MenuSubroutineDelTablist
	.dw MenuSubroutineLoadDebugmap
	.dw MenuSubroutineLoadCredits

MenuSubroutineChsum:
	sep #$20
	phk
	pla
	sta.b TempBuffer+2			;operating bank
	
	rep #$31
	stz.b TempBuffer
	stz.w CartChecksum

	
	ldy.w #0
WramChecksumLoop:
	rep #$31
	lda.b [TempBuffer],y
	and.w #$ff
	adc.w CartChecksum
	sta.w CartChecksum
	iny
	bne WramChecksumLoop
	
	ldy.w #$ffde
	lda.b [TempBuffer],y
	cmp.w CartChecksum
	beq WramChecksumOK

	ldx.w #17
	jsr LoadTextString		;print $00:3000	
	rts

WramChecksumOK:
	ldx.w #16
	jsr LoadTextString		;print $00:3000
	rts

MenuSubroutineJumpROM:
	jml (MenuSubroutineJumpReturn+BaseAdress)
MenuSubroutineJumpRAM:
	jml (MenuSubroutineJumpReturn+$7f0000)

MenuSubroutineJumpReturn:
	rts

MenuSubroutineWrite3000:
	sep #$20
	lda.w Reg3000WriteVar
	sta.l $3000
	rts
	
MenuSubroutineExecTablistRec:
	sep #$20
	lda.b #2
	sta.b BattleMusicState
	rts
MenuSubroutinePlayTablist:
	sep #$20
	lda.b #4
	sta.b BattleMusicState
	rts
	
MenuSubroutineDelTablist:
	sep #$20
	stz.b BattleMusicState			;just init the whole tablist fsm 
	rts
	
MenuSubroutineStartStream:
	lda.b SpcCurrentStreamSet
	jsr SpcPlayStream
	rts
MenuSubroutinePlaySong:
	lda.b PtPlayerCurrentSong			;play song
	jsr SpcPlaySong
	rts
MenuSubroutineUploadSEPack:
	sep #$20
	lda.b PtPlayerCurrentSamplePack
	jsr SpcIssueSamplePackUpload
	rts
MenuSubroutinePlaySE:
	sep #$20
	lda.w SpcSEVolume
	xba
	lda.w SpcSEPitch
	rep #$31
	tax
	sep #$20
	lda.b PtPlayerCurrentSoundEffect
	jsr SpcPlaySoundEffect
	rts



MenuSubroutineVoid:
	rts

MenuSubroutineReturnMain:
	sep #$20
	lda.b #0
	sta.b CurrentEvent
	rts

MenuSubroutineTablistRecorder:
	sep #$20
	lda.b #10
	sta.b CurrentEvent
	rts
	
MenuSubroutineInputMenu:
	sep #$20
	lda.b #8
	sta.b CurrentEvent
	rts

MenuSubroutineAudioMenu:
	sep #$20
	lda.b #2
	sta.b CurrentEvent
	rts

MenuSubroutineLoadLevel:
	sep #$20
	lda.b #4
	sta.b CurrentEvent
	rts

MenuSubroutineLoadBattle:
	sep #$20
	lda.b #2
	sta.b CurrentEvent
	rts

MenuSubroutineLoadIntro:
	sep #$20
	lda.b #28
	sta.b CurrentEvent
	rts

MenuSubroutineLoadDebugmap:
	sep #$20
	lda.b #37
	sta.b CurrentEvent
	rts

MenuSubroutineLoadCredits:
	sep #$20
	lda.b #36
	sta.b CurrentEvent
;	stz.b ScreenBrightness
	rts	
	