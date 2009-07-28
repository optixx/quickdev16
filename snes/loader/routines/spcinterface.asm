

SpcHandlerMain:
	php
	sep #$20
;	stz.w $4200			;disable irqs in case we are processing a lengthy spc transfer

;check if spc sends report data
	lda.w $2141
	and.b #$e0
	cmp.b #$e0
	bne SpcHandlerNoSpcReport

	rep #$31
	lda.w $2141			;get report type and select corresponding buffer accordingly
	and.w #7
	asl a
	tax
	lda.w $2142			;get report word and store in buffer
	sta.l SpcReportBuffer,x


SpcHandlerNoSpcReport:
	rep #$31
;	sep #$20
	lda.b SpcHandlerState
	and.w #$1f			;have 32 states maximum
	asl a
	tax
	jsr (SpcHandlerSubroutineJumpLUT,x)
	
	
	
	
	sep #$20
;	lda.b InterruptEnableFlags	;reenable irqs
;	sta.w $4200	
	
	plp
	rts

;stops currently playing stream, song, then waits for driver to return to idle state(wait for all scheduled sound effects to play)
SpcWaitAllCommandsProcessed:
	php
	sep #$20
	stz.w SpcStreamVolume
	jsr SpcStopSong
SpcWaitLoop:
	lda.b SpcCmdFifoStart
	cmp.b SpcCmdFifoEnd				;check if fifo is empty
	bne SpcWaitLoop
	
	lda.b SpcHandlerState
	cmp.b #1
	bne SpcWaitLoop
	
	plp
	rts
	
SpcHandlerSubroutineJumpLUT:
	.dw SpcUploadDriver		;0
	.dw SpcIdle
	.dw SpcUploadSong
	.dw SpcUploadSongWait
	.dw SpcUploadSampleset
	.dw SpcUploadSamplesetWait	;5
	.dw SpcStreamData
	.dw SpcStreamDataWait
	.dw SpcPlaySoundeffectUpload	
	.dw SpcPlaySoundeffectWait
	.dw SpcStopSongInit		;10
	.dw SpcStopSongWait 
	.dw SpcSetSpeedInit
        .dw SpcSetSpeedWait
	.dw SpcSetChMaskInit
        .dw SpcSetChMaskWait		;15
        .dw SpcSetReportTypeInit
	.dw SpcSetReportTypeWait



SpcSetReportTypeInit:
	rep #$31
	lda.b SpcHandlerArgument0		;store type and sub-arg
	sta.w $2141
	sep #$20

	lda.b #SpcCmdReportType		;exec command
	sta.w $2140
	
	
	inc.b SpcHandlerState					;goto "wait SE ack"-state
	rts

SpcSetReportTypeWait:
	sep #$20
	lda.w $2140
	cmp.b #SpcCmdReportType
	bne SpcSetReportTypeWaitNoIdle			;wait until spc has ack'd the speed change before returning

	lda.b #1
	sta.b SpcHandlerState					;return to idle once spc has answered

SpcSetReportTypeWaitNoIdle:
	rts

SpcSetChMaskInit:
	sep #$20
	lda.b SpcHandlerArgument0		;store mask
	sta.w $2141


	lda.b #SpcCmdSetSongChMask		;exec command
	sta.w $2140
	
	
	inc.b SpcHandlerState					;goto "wait SE ack"-state
	rts

SpcSetChMaskWait:
	sep #$20
	lda.w $2140
	cmp.b #SpcCmdSetSongChMask
	bne SpcSetChMaskWaitNoIdle			;wait until spc has ack'd the speed change before returing

	lda.b #1
	sta.b SpcHandlerState					;return to idle once spc has answered

SpcSetChMaskWaitNoIdle:
	rts

SpcSetSpeedInit:
	sep #$20
	lda.b SpcHandlerArgument0		;store speed
	sta.w $2141


	lda.b #SpcCmdSetSongSpeed		;exec command
	sta.w $2140
	
	
	inc.b SpcHandlerState					;goto "wait SE ack"-state
	rts

SpcSetSpeedWait:
	sep #$20
	lda.w $2140
	cmp.b #SpcCmdSetSongSpeed
	bne SpcSetSpeedWaitNoIdle			;wait until spc has ack'd the speed change before returing

	lda.b #1
	sta.b SpcHandlerState					;return to idle once spc has answered
	

SpcSetSpeedWaitNoIdle:
	rts	
	
	
SpcUploadSampleset:
	sep #$20
	lda.b #SpcCmdUploadSamplePack					;send "upload song" command
	sta.w $2140
	lda.b #5
	sta.b SpcHandlerState					;goto "wait for spc to ack song upload" state
	rts
	
SpcUploadSamplesetWait:
	sep #$20
	lda.w $2140
	cmp.b #SpcCmdUploadSamplePack					;wait until spc has ack'd upload song command
	bne SpcUploadSamplePackWaitExit				;else try again next frame
	
;	stz.w $4200						;disable irqs
;upload SamplePack here
	lda.b SpcHandlerArgument0				;get song number to upload
	sta.b PtPlayerCurrentSamplePack

	
	rep #$31				;multiply song number by 3 and use as index into song pointertable
	and.w #$00ff
	sta.b PtPlayerDataPointerLo
	asl a
	clc
	adc.b PtPlayerDataPointerLo
	tax

	lda.l (PtPlayerSamplePackPointertable+BaseAdress),x	;store pointer to song
	sta.b PtPlayerDataPointerLo
		
	lda.l (PtPlayerSamplePackPointertable+$c00001),x
	
	sta.b PtPlayerDataPointerHi	
;	sep #$20
;	sta.b PtPlayerDataPointerBa
	
;	rep #$31
	ldy.w #$0000
	lda.b [PtPlayerDataPointerLo],y		;get song length
	dec a					;substract length word
	dec a
	sta.w PtPlayerSmplBufferPosLo
	iny					;increment source pointer to actual song offset
	iny
	sep #$20

SpcUploadSamplePackTransfer1:		
		lda.b [PtPlayerDataPointerLo],y		;write 3 bytes to ports
		sta.w $2141
		iny
		lda.b [PtPlayerDataPointerLo],y
		sta.w $2142
		iny
		lda.b [PtPlayerDataPointerLo],y
		sta.w $2143
		iny
		
		lda.b #SpcCmdUploadSongT1		;write ack transfer 1 to port0
		sta.w $2140

SpcUploadSamplePackTransfer1WaitLoop:				;wait for spc to write transfer 1 ack back
		cmp.w $2140
		bne SpcUploadSamplePackTransfer1WaitLoop
	
SpcUploadSamplePackTransfer2:		
		lda.b [PtPlayerDataPointerLo],y		;write 3 bytes to ports
		sta.w $2141
		iny
		lda.b [PtPlayerDataPointerLo],y
		sta.w $2142
		iny
		lda.b [PtPlayerDataPointerLo],y
		sta.w $2143
		iny
		
		lda.b #SpcCmdUploadSongT2		;write ack transfer 1 to port0
		sta.w $2140

SpcUploadSamplePackTransfer2WaitLoop:				;wait for spc to write transfer 1 ack back
		cmp.w $2140
		bne SpcUploadSamplePackTransfer2WaitLoop
	
		cpy.w PtPlayerSmplBufferPosLo		;check if transfer length is exceeded
		bcc SpcUploadSamplePackTransfer1
		
		lda.b #SpcCmdUploadSamplePackDone		;send "upload song complete" commadn if transfer is done
		sta.w $2140
	
		lda.b #1
		sta.b SpcHandlerState					;return to idle

;		lda.b InterruptEnableFlags	;reenable irqs
;		sta.w $4200

SpcUploadSamplePackWaitExit:
	lda.b SpcUploadedFlag
	ora.b #$40						;set sample pack uploaded flag.
	sta.b SpcUploadedFlag

	rts	


SpcStopSongInit:
	rep #$31
	stz.w $2141
	stz.w $2142
	sep #$20
	lda.b #SpcCmdStopSong		;exec command
	sta.w $2140	
	inc.b SpcHandlerState		;goto "wait SE ack"-state
	rts

SpcStopSongWait:	
	sep #$20
	lda.w $2140
	cmp.b #SpcCmdStopSong
	bne SpcStopSongWaitNoIdle			;wait until spc has ack'd the song/stream stop before returing

	lda.b #1
	sta.b SpcHandlerState					;return to idle once spc has answered
	

SpcStopSongWaitNoIdle:
	rts



SpcPlaySoundeffectUpload:
	rep #$31
	lda.b SpcHandlerArgument0		;store arguments
	sta.w $2141
	lda.b SpcHandlerArgument1
	and.w #$7fff				;mask off msb and use as wurst
	ora.w SpcSoundEffectFlipFlag
	sta.w $2142
	lda.w SpcSoundEffectFlipFlag
	eor.w #$8000
	sta.w SpcSoundEffectFlipFlag
	sep #$20
	lda.b #SpcCmdPlaySoundEffect		;exec command
	sta.w $2140
	
	
;	lda.b #1
;	sta.b SpcHandlerState					;don't wait
	inc.b SpcHandlerState					;goto "wait SE ack"-state
	rts

;dont use this cause it sometimes plays a sound effect twice
SpcPlaySoundeffectWait:	
	sep #$20
	lda.w $2140
	cmp.b #SpcCmdPlaySoundEffect
	bne SpcPlaySoundeffectWaitNoIdle			;wait until spc has ack'd the soundeffect before returing

	lda.b #1
	sta.b SpcHandlerState					;return to idle once spc has answered
	

SpcPlaySoundeffectWaitNoIdle:
	rts
	
		
SpcUploadDriver:
		sep #$20
;		stz.w $4200
		lda.b #(:PtplayerSpcCode+BaseAdress>>16)
		ldx.w #PtplayerSpcCode
		sta.b PtPlayerDataPointerBa
		stx.b PtPlayerDataPointerLo
;		jsr PtPlayerInit
;		php
;		sep #$20
;		stz.w $4200
		REP #$31
		LDY.w #$0000				;clear data pointer
		LDA.w #$BBAA

PtPlayerInitSpcWaitLoop1:		
		CMP.w $2140				;wait for spc to respond
		BNE PtPlayerInitSpcWaitLoop1

		SEP #$20
		LDA.b #$CC				;send "start transfer"
		BRA PtPlayerInitDoTransfer

PtPlayerInitGetByte:
		LDA.b [PtPlayerDataPointerLo],y
		INY
		XBA
		LDA.b #$00
		BRA PtPlayerInitClearSpcPort0

PtPlayerInitGetNextByte:
		XBA
		LDA.b [PtPlayerDataPointerLo],y
		INY
		XBA

PtPlayerInitSpcWaitLoop2:
		CMP.w $2140
		BNE PtPlayerInitSpcWaitLoop2
		INC A
PtPlayerInitClearSpcPort0:
		REP #$20
		STA.w $2140
		SEP #$20
		DEX
		BNE PtPlayerInitGetNextByte

PtPlayerInitSpcWaitLoop3:
		CMP.w $2140
		BNE PtPlayerInitSpcWaitLoop3

PtPlayerInitAddLoop:
		ADC.b #$03
		BEQ PtPlayerInitAddLoop


PtPlayerInitDoTransfer:
		PHA
		REP #$20
		LDA.b [PtPlayerDataPointerLo],y
		INY
		INY
		TAX
		LDA.b [PtPlayerDataPointerLo],y
		INY
		INY
		STA.w $2142
		SEP #$20
		CPX.w #$0001				;whats this?
		LDA.b #$00
		ROL A
		STA.w $2141
		ADC.b #$7F
		PLA
		STA.w $2140
		cpx.w #$0001				;fix to be able to upload apucode during active nmi
		bcc PtPlayerInitDone

PtPlayerInitSpcWaitLoop4:
		CMP.w $2140
		BNE PtPlayerInitSpcWaitLoop4


		BVS PtPlayerInitGetByte

PtPlayerInitDone:
		SEP #$20
;		lda.b #%10000001
;		sta.w $4200

/*
		lda.b #$7f				;init some variables
		sta.b PtPlayerMainVolL
		sta.b PtPlayerMainVolR
		lda.b #%00001111
		sta.b PtPlayerChannelEnable
		lda.b #$00
		sta.b PtPlayerEchoVolL
		sta.b PtPlayerEchoVolR
		sta.b PtPlayerChannelEchoEnable
		lda.b #$01
		sta.b CGIntroPlaySongNumber
*/		
;		PLP
		lda.b InterruptEnableFlags	;reenable irqs
;		sta.w $4200
		inc.b SpcHandlerState			;go to idle state

;init some variables
	lda.b #$a0
	sta.w SpcSongSpeed			;set speed to default
	lda.b #$0f
	sta.w SpcSongChMask			;set song channel mask to default
		jsr ClearSpcReportBuffer
		
		RTS	
	
ClearSpcReportBuffer:
	php
	rep #$31
	lda.w #$0000			;clear with y-position at line 255
	ldx.w #16
ClearSpcReportBufferLoop:
	sta.l (SpcReportBuffer &$ffff + $7e0000-2),x
	dex
	dex
	bne ClearSpcReportBufferLoop
	plp
	rts	
		
;check if there's a new command in the fifo, else return
;fifo buffer organization is:
;each entry: 1 command byte, 3 argument bytes
;fifo has 16 entries/64bytes total
SpcIdle:
	sep #$20
	stz.w $2140					;clear port0
	
	lda.b SpcCmdFifoStart
	cmp.b SpcCmdFifoEnd				;check if fifo is empty
	beq SpcIdleFifoEmpty
	
;theres a spc command present:

	rep #$31
	and.w #$3f					;limit fifo pointer to 64 bytes
	tax						
	lda.w SpcCmdFifo,x				;get command
	
	sta.b SpcHandlerState					;store command/state and argument 1
	
	lda.w SpcCmdFifo+2,x				;get command
	
	sta.b SpcHandlerArgument1				;store arguments 1 and 2

	lda.b SpcHandlerState				;directly execute fetched command
	and.w #$1f					;except when its an idle command
	cmp.w #1					;because this would allow for unlimited nesting and possibly stack overflow
	beq SpcIdleFifoEmpty
	
	asl a
	tax
	jsr (SpcHandlerSubroutineJumpLUT,x)
	
	sep #$20
	lda.b SpcCmdFifoStart				;goto next fifo entry next frame
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoStart	
	
SpcIdleFifoEmpty:
	rts
	
SpcUploadSong:
	sep #$20

	lda.b #SpcCmdUploadSong					;send "upload song" command
	sta.w $2140
	lda.b #3
	sta.b SpcHandlerState					;goto "wait for spc to ack song upload" state
	rts
	
SpcUploadSongWait:
	sep #$20
	lda.w $2140
	cmp.b #SpcCmdUploadSong					;wait until spc has ack'd upload song command
	bne SpcUploadSongWaitExit				;else try again next frame
	
	
;upload song here
;	stz.w $4200
	lda.b SpcHandlerArgument0				;get song number to upload
	sta.b PtPlayerCurrentSong

	
	rep #$31				;multiply song number by 3 and use as index into song pointertable
	and.w #$00ff
	sta.b PtPlayerDataPointerLo
	asl a
	clc
	adc.b PtPlayerDataPointerLo
	tax

	lda.l (PtPlayerSongPointertable+BaseAdress),x	;store pointer to song
	sta.b PtPlayerDataPointerLo
		
	lda.l (PtPlayerSongPointertable+$c00001),x
	
	sta.b PtPlayerDataPointerHi	
;	sep #$20
;	sta.b PtPlayerDataPointerBa
	
;	rep #$31
	ldy.w #$0000
	lda.b [PtPlayerDataPointerLo],y		;get song length
	dec a					;substract length word
	dec a
	sta.w PtPlayerSmplBufferPosLo
	iny					;increment source pointer to actual song offset
	iny
	sep #$20

SpcUploadSongTransfer1:		
		lda.b [PtPlayerDataPointerLo],y		;write 3 bytes to ports
		sta.w $2141
		iny
		lda.b [PtPlayerDataPointerLo],y
		sta.w $2142
		iny
		lda.b [PtPlayerDataPointerLo],y
		sta.w $2143
		iny
		
		lda.b #SpcCmdUploadSongT1		;write ack transfer 1 to port0
		sta.w $2140

SpcUploadSongTransfer1WaitLoop:				;wait for spc to write transfer 1 ack back
		cmp.w $2140
		bne SpcUploadSongTransfer1WaitLoop
	
SpcUploadSongTransfer2:		
		lda.b [PtPlayerDataPointerLo],y		;write 3 bytes to ports
		sta.w $2141
		iny
		lda.b [PtPlayerDataPointerLo],y
		sta.w $2142
		iny
		lda.b [PtPlayerDataPointerLo],y
		sta.w $2143
		iny
		
		lda.b #SpcCmdUploadSongT2		;write ack transfer 1 to port0
		sta.w $2140

SpcUploadSongTransfer2WaitLoop:				;wait for spc to write transfer 1 ack back
		cmp.w $2140
		bne SpcUploadSongTransfer2WaitLoop
	
		cpy.w PtPlayerSmplBufferPosLo		;check if transfer length is exceeded
		bcc SpcUploadSongTransfer1
		
		lda.b #SpcCmdUploadSongDone		;send "upload song complete" commadn if transfer is done
		sta.w $2140
	
		lda.b #1
		sta.b SpcHandlerState					;return to idle

		lda.b InterruptEnableFlags	;reenable irqs
;		sta.w $4200

SpcUploadSongWaitExit:
	lda.b SpcUploadedFlag
	ora.b #$80						;set song uploaded flag.
	sta.b SpcUploadedFlag

	rts	

;stops currently playing song/stream
SpcStopSong:
	php
	rep #$31
	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax	
	sep #$20
;	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdStopSong
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd
	plp
	rts

;in:	a,8bit: SE number to play
;	x,16bit: volume & pitch low byte: pitch(use default if zero, multiplied by 16). high byte: volume(use default if zero) bit7:sign, bits6-4:panning, bits 3-0:volume(multiplied with $10)	
SpcPlaySoundEffect:
	php
	rep #$31
	pha					;push SE number
	phx					;push arguments
	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax
	pla					;get arguments
	sta.w (SpcCmdFifo+2),x			;store in fifo arguments 2,3
	
	pla					;fetch song number again
	sep #$20
	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdPlaySE
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd
	plp
	rts

;same as above, but doesn't use x as input
SpcPlaySoundEffectSimple:
	php
	rep #$31
	phx					;push arguments
	pha					;push SE number

	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax
	lda.w #0
	sta.w (SpcCmdFifo+2),x			;store in fifo arguments 2,3
	
	pla					;fetch song number again
	sep #$20
	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdPlaySE
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd
	plx
	plp
	rts


;in: a,8bit: song number to play
SpcPlaySong:
	php
	rep #$31
	pha					;push song number
	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax
	
	pla					;fetch song number again
	sep #$20
	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdUploadSong
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd
	
	stz.w SpcStreamVolume			;mute eventually playing stream, stop stream
	
	lda.b SpcUploadedFlag
	and.b #$7f						;clear song uploaded flag. will be set once song upload has been completed later on
	sta.b SpcUploadedFlag
;	lda.b SpcCurrentStreamSet		;useful, but messes up if sampleset + song dont fit into spc ram
;	jsr SpcIssueSamplePackUpload
	plp
	rts

;in: a,8bit: streamset number to play
SpcPlayStream:
	php
	sep #$20
	pha
	rep #$31
	stz.w SpcStreamFrame
	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax
	
	sep #$20
	pla					;store frameset number
	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdStreamBrr
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd

	sep #$20
	stz.w SpcStreamVolume			;mute eventually playing stream, stop stream
	plp
	rts
	
	
;in: a,8bit: sample pack number to upload
SpcIssueSamplePackUpload:
	php
	rep #$31
	pha					;push song number
	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax
	
	pla					;fetch song number again
	sep #$20
	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdUploadSamplePack
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd

	lda.b SpcUploadedFlag
	and.b #$bf						;clear sample pack uploaded flag. will be set once song upload has been completed later on
	sta.b SpcUploadedFlag
	
	plp
	rts

SpcStreamData:
	sep #$20
	lda.b #$7f				;turn on streaming volume
	sta.w SpcStreamVolume
	
	lda.b SpcHandlerArgument0		;get frameset number to stream
	sta.b SpcCurrentStreamSet		;multiply by 5 to get pointer into frameset table			
	rep #$31
	and.w #$ff
	sta.b SpcTempBuffer
	asl a
	asl a
	clc
	adc.b SpcTempBuffer
	
	tax
	lda.l (StreamSetLut+BaseAdress+3),x	;get number of frames for this set
	sta.w $2141				;write number of frames to stream to spc
	
				
	sep #$20	
	lda.b #SpcCmdReceiveStream				;send "upload song" command
	sta.w $2140

	lda.l $2140
	cmp.b #SpcCmdReceiveStreamComplete			;don't switch to streamer yet if spc is still finishing the last transfer.(only applicable if last command was streaming)
	beq SpcStreamDataWaitSpcLastStream
	
	lda.b #7
	sta.b SpcHandlerState					;goto "wait for spc to ack song upload" state
	
SpcStreamDataWaitSpcLastStream:	
	rts


SpcStreamDataReturnIdle:
	sep #$20
	lda.b HdmaFlags
	and.b #$7f					;disable spc stream on hdma channel 7
	sta.b HdmaFlags	
	lda.b #1						;return to idle if spc signalizes that transfer is complete
	sta.b SpcHandlerState
SpcStreamDataExit:
	rts

	
SpcStreamDataWait:
	sep #$20
	lda.w $2140
	cmp.b #SpcCmdReceiveStreamComplete			;check if transfer complete
	beq SpcStreamDataReturnIdle
	cmp.b #SpcCmdReceiveStream				;wait until spc has ack'd upload song command
	bne SpcStreamDataExit					;else try again next frame	

	
	
	lda.b SpcCurrentStreamSet				;get current frameset number
	rep #$31
	and.w #$ff
	sta.b SpcTempBuffer					;multiply by 5
	asl a
	asl a
	clc
	adc.b SpcTempBuffer
	
	tax
	lda.l (StreamSetLut+BaseAdress),x				;store offset of first frame in set
	sta.b SpcTempBuffer+2
	lda.l (StreamSetLut+BaseAdress+1),x
	sta.b SpcTempBuffer+3
	
	lda.w $2141						;get frame request number
									;multiply with 144, get pointer to requested frame. warning: one bank holds $1c7 frames, or $fff0 bytes of data. must wrap to next bank if bigger.
	sta.b SpcTempBuffer+5					;store frame request number
	sta.w SpcStreamFrame
	pha
	lda.w SpcStreamVolume			;only write current frame if stream hasnt been stopped. otherwise external routines might be confused when transitioning between streams
	and.w #$ff
	bne DontClearSpcStreamFrame
	
	stz.w SpcStreamFrame

	
DontClearSpcStreamFrame:
	pla

SpcStreamCalcFrameOffsetLoop:
	cmp.w #SpcFramesPerBank					;check if number of frames exceeds bank limit
	bcc SpcStreamCalcFrameOffsetNoBankWrap

	sep #$20						;if it does, increase bank
	inc.b SpcTempBuffer+4
	rep #$31						;and substract one bank full of samples from total amount
	sec
	sbc.w #SpcFramesPerBank
	bra SpcStreamCalcFrameOffsetLoop			;do this until we're in the bank the frame actually is in

SpcStreamCalcFrameOffsetNoBankWrap:
	
	asl a							;multiply by 144
	asl a
	asl a
	asl a							;first multiply by 16
	sta.b SpcTempBuffer+2
	
	asl a							;then by 128
	asl a
	asl a
	
	clc
	adc.b SpcTempBuffer+2					;this is x*144
	sta.b SpcTempBuffer+2					;and save in pointer

;line 1, command
	lda.w #36/2
	sta.b SpcTempBuffer					;set scanline counter
	lda.w #SpcScanlineWaitCount						;store 1 in frame count
	ldx.w #0						;clear hdma table pointer
	txy							;clear brr source counter
	
	sta.l HdmaSpcBuffer,x

	lda.w #SpcCmdSubmitStreamNumber				;send "submit frame #"-cmd
	sta.l HdmaSpcBuffer+1,x
	
	lda.b SpcTempBuffer+5					;get frame submit number
	sta.l HdmaSpcBuffer+2,x

	lda.w SpcStreamVolume
	sta.l HdmaSpcBuffer+4,x					;store volume
	
	inx
	inx
	inx
	inx
	inx
;line 2, waitloop &  frame transmit
SpcStreamSetupHdmaTableLoop:
	lda.w #1
	sta.l HdmaSpcBuffer,x				;one line
	
	lda.b [SpcTempBuffer+2],y				;brr data to ports $2140-$2143
	sta.l HdmaSpcBuffer+1,x

	iny
	iny
	
	lda.b [SpcTempBuffer+2],y
	sta.l HdmaSpcBuffer+3,x

	inx
	inx
	inx
	inx
	inx
	
;	iny
;	iny
	iny
	iny

	lda.w #1
	sta.l HdmaSpcBuffer,x				;one line
	
	lda.b [SpcTempBuffer+2],y				;brr data to ports $2140-$2143
	sta.l HdmaSpcBuffer+1,x

	iny
	iny
	
	lda.b [SpcTempBuffer+2],y
	sta.l HdmaSpcBuffer+3,x

	inx
	inx
	inx
	inx
	inx
	
;	iny
;	iny
	iny
	iny	
	dec.b SpcTempBuffer
	bne SpcStreamSetupHdmaTableLoop

;this doesnt work right. reason is that the the stream data sometimes contains "false" commands.
;write spc command here: (only play soundeffect, stop song/stream is allowed here)

	stz.b SpcTempBuffer				;clear spc command field
	stz.b SpcTempBuffer+2
/*
	
	lda.b SpcCmdFifoStart
	cmp.b SpcCmdFifoEnd				;check if fifo is empty
	beq SpcStreamFifoEmpty
	
;theres a spc command present:
	rep #$31
	and.w #$3f					;limit fifo pointer to 64 bytes
	tax						
	lda.w SpcCmdFifo,x				;get command
	and.w #$ff
	cmp.w #SpcCmdPlaySoundEffect
	beq SpcStreamFifoExecCmd
	cmp.w #SpcCmdStopSong
	beq SpcStreamFifoExecCmdExit
	
	bra SpcStreamFifoEmpty

SpcStreamFifoExecCmdExit:
	sep #$20
	lda.b #1
	sta.b SpcHandlerState				;if stop command is present in fifo, immediately halt streamer and return to idle state. next frame, fifo command handler will process the stop command normally.
	rts

SpcStreamFifoExecCmd:	

;state
	lda.w SpcCmdFifo,x				;get command
	sta.b TempBuffer				;store command/state and argument 1
	
	lda.w SpcCmdFifo+2,x				;get command
	
	sta.b TempBuffer+2				;store arguments 1 and 2

	
	sep #$20
	lda.b SpcCmdFifoStart				;goto next fifo entry next frame
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoStart	
	
SpcStreamFifoEmpty:
	rep #$31
*/
	lda.w #1
	sta.l HdmaSpcBuffer,x				;one line
	

	lda.b SpcTempBuffer
	sta.l HdmaSpcBuffer+1,x				

	
	lda.b SpcTempBuffer+2
	sta.l HdmaSpcBuffer+3,x

	inx
	inx
	inx
	inx
	inx

;terminate hdma table:	
	lda.w #0
	sta.l HdmaSpcBuffer,x
	
	lda.w #HdmaSpcBuffer & $ffff
	sta.l $4372					;hdma source offset
	
	sep #$20
	lda.b HdmaFlags
	ora.b #$80					;enable spc stream on hdma channel 7
	sta.b HdmaFlags
	
	lda.b #$7e					;hdma source bank
	sta.l $4374
	
	lda.b #%00000100				;hdma config, direct, 4 regs write once
	sta.l $4370
	
	lda.b #$40					;bbus target, apu port $2140-$2143
	sta.l $4371
	
	rts
	
;in: SpcSongSpeed,8bit: song speed(timer duration. default is #$a0. lower=faster, higher=slower
SpcSetSongSpeed:
	php
	rep #$31
;	pha					;push speed
	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax
	
;	pla					;fetch speed again
	sep #$20
	lda.w SpcSongSpeed
	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdSetSpeed
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd
	
;	stz.w SpcStreamVolume			;mute eventually playing stream, stop stream
	
;	lda.b SpcCurrentStreamSet		;useful, but messes up if sampleset + song dont fit into spc ram
;	jsr SpcIssueSamplePackUpload
	plp
	rts

;in: SpcSongSpeed,8bit: song speed(timer duration. default is #$a0. lower=faster, higher=slower
SpcSetSongChannelMask:
	php
	rep #$31
	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax
	
	sep #$20
	lda.w SpcSongChMask
	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdSetChMask
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd
	
	plp
	rts

;input: SpcReportType,8bit: 0=none 1=timecode 2=channel-levels(vol out) 3=special mod command (mod command $e0. this is "set filter", unused in almost any player
SpcSetReportType:
	php
	rep #$31
	lda.b SpcCmdFifoEnd			;get current position in spc command fifo buffer		
	and.w #$ff
	tax
	
	sep #$20
	lda.w SpcReportType
	and.b #$07				;8 types max
	sta.w (SpcCmdFifo+1),x
	lda.b #SpcFifoCmdSetReportType
	sta.w SpcCmdFifo,x
	
	lda.b SpcCmdFifoEnd
	clc
	adc.b #4
	and.b #$3f				;16x4byte entries maximum 
	sta.b SpcCmdFifoEnd
	
	plp
	rts
	
;legacy code:
PtPlayerInit:
PtPlayerMainHandler:
PtPlayerUploadVolEcho:
	rts
	
;plays sound effect with panning set to current objects x-position
SpcPlaySoundEffectObjectXPos:
	php

	rep #$31
	pha
	ldx.b ObjectListPointerCurrent
	lda.w ObjEntryXPos,x				;get upper 4 nibbles of x-pos
	lsr a												;remove subpixel prec
	lsr a
	lsr a
	lsr a
	
	lsr a						;use bits 6-4 for panning
	and.w #$7f					;clear sign
	ora.w #$0F					;set max volume
	xba
	tax
	pla
	sep #$20

	jsr SpcPlaySoundEffect	
	
	
	
	plp
	rts
		

