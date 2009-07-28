;----------------------------------------------------------------------------------------
;pro tracker (mod) player for the spc700 (c)d4s in 2005
;----------------------------------------------------------------------------------------

.MEMORYMAP					; Tell WLA that the SPC has RAM at locations ;$0000-$ffff in every bank
SLOTSIZE $10000			; and that this area is $10000 bytes in size.
DEFAULTSLOT 0				; There is only a single slot in SNES, other consoles
SLOT 0 $0000				; may have more slots per bank.
.ENDME

.ROMBANKSIZE $10000		; Every ROM bank is 64 KBytes in size
.ROMBANKS 1
;----------------------------------------------------------------------------------------
.BANK 0 SLOT 0



.define SpcCmdUploadSong	$f1		;indicates that a song is to be uploaded
.define SpcCmdUploadSongT1	$f2		;indicates that data for transfer1 is on apu ports
.define SpcCmdUploadSongT2	$f3		;indicates that data for transfer2 is on apu ports
.define SpcCmdUploadSongDone	$f4		;indicates that song upload is complete
.define SpcCmdReceiveStream	$f5		;indicates that 65816 wants to stream brr data to spc
.define SpcCmdSubmitStreamNumber	$f7		;indicates that 65816 wants to stream brr data to spc
.define SpcCmdReceiveStreamComplete	$f6	;spc wants to end stream transmission
.define SpcCmdUploadSamplePack	$f8		;indicates that a sample pack is to be uploaded. the rest of the commands are taken from normal song upload
.define SpcCmdUploadSamplePackDone	$f9		;indicates that sample pack upload is complete
.define SpcCmdPlaySoundEffect	$fa		;play a sound effect
.define SpcCmdStopSong		$fb
.define SpcCmdSetSongSpeed	$fc		;set timer speed of mod playback routine
.define SpcCmdSetSongChMask	$fd		;song channel mask
.define SpcCmdReportType	$fe		;type of data spc should respond with
.define SpcCmdSetSongVolume	$ff

.define SpcStreamLatency	7		;number of frames to load before actually playing sample
.define SpcStreamSampleNumber	33		;number of dsp sample to use as streamer
.define StreamerTimer0		$00		;how long to wait before downloading next frame

.define ChannelArray		$0
.define ExtSample		$20		;this sample is used for set sample offset effect

.define Stack			$ff
.define ApuCodeBaseAdress	$300
.define SampleSource		$0200		;$ff bytes long
.define StreamBuffer		$fb80		;480 bytes long
.define HighestPortaPeriod	$0071
.define LowestPortaPeriod	$0358
.define EchoStartAdress		$f8
.define EchoDelay		$01

.define SongEffectsUploaded	$ef			;flags that indicate whether a song or sample pack have already been uploaded.bit0=song bit1=sample pack. wla is too stupid to recognize a variable here, so we have to use a static one.
;variables:
;.........................................................
.enum $60
WaitCounter		dw
CurrentPatternPointerLo	db
CurrentPatternPointerHi	db
InitPatternPointerLo	db
InitPatternPointerHi	db
PortamentoDownLimitLo	db
PortamentoDownLimitHi	db
PortamentoUpLimitLo	db
PortamentoUpLimitHi	db
CurrentChDataLo		db
CurrentChDataHi		db
TempBuffer1		db
TempBuffer2		db
TempBuffer3		db
TempBuffer4		db

TempBuffer5		db
TempBuffer6		db
TempBuffer7		db
TempBuffer8		db
TempBuffer9		db
TempBuffer10		db
TempVol			db
KeyOn			db
CurrentChannel		db
CurrentVolume		db
PitchBufferLo		db
PitchBufferHi		db
CurrentPatternNumber	db
PortamentoSpeedMask	db
NotUsedVariable		db
CurrentPeriodPointer	db
CurrentPeriodPointerHi	db

PeriodTablePointerLo	db
PeriodTablePointerHi	db
SpcDataPointerLo	db
SpcDataPointerHi	db
ModFilePointerLo	db
ModFilePointerHi	db
ModFilePointer2Lo	db
ModFilePointer2Hi	db

;added variables

ScratchPad1		db
ScratchPad2		db
ScratchPad3		db

PeriodBufferLo		db
PeriodBufferHi		db

NumberOfPatterns	db
PatternSpeed		db
NextRowChFadeOuts	db		;lower nibble bits0-4:channels to fade out for next row
NextRowChFadeOutTime	db		;patternspeed-2
PortamentoLUTLo		db	;table for portamento up/down
PortamentoLUTHi		db	;table for portamento up/down
TonePortamentoLUTLo	db	;table for portamento up/down
TonePortamentoLUTHi	db	;table for portamento up/down
CurrentRowPointerLo	db
CurrentRowPointerHi	db
CurrentLoopPointerLo	db
CurrentLoopPointerHi	db
LoopCounter		db
PatternBreakFlag	db	;if bit0 is set, break to next pattern

;main volume section:
MainVolL		db
MainVolR		db
EchoVolL		db
EchoVolR		db
ChannelEnable		db
ChannelEchoEnable	db


PitchTablePointerLo	db
PitchTablePointerHi	db

Channel1Array		dw
Channel2Array		dw
Channel3Array		dw
Channel4Array		dw

;apu streaming stuff
StreamState		db			;variable for streaming state machine
StreamCurrentBuffer	db			;indicates currently active stream buffer out of 8
StreamVolume		db

ActiveSoundEffectCh	db			;0-3
ModPlayerTimer0		db
SongChannelMask		db			;mask variable for 4 song channels. default $0f
SpcReportType		db
ModTimecode		dw			;timecode for mod currently playing. reset after song upload
SpecialReportCmd	db
SoundEffectFlipFlag	db
TickSampleStart		db		;channel mask	to start samples on the next tick. used for delay and retrigger
MusicVol1			db		;mod channel 0,2 volume
MusicVol2			db		;mod channel 1,3 volume
.ende


;channel variables
.enum			$00
ch_instrument			db		;00
ch_note				db		;01
ch_effect			db		;02
ch_effect_data			db		;03
ch_finetune			db		;04
ch_current_volume		db		;05
ch_current_pitch_lo		db		;06 this gets updated on portamento etc
ch_current_pitch_hi		db		;07 (not directly used in sourcecode)
ch_target_pitch_lo		db		;08 (not used at all in sourcecode) target pitch for tone portamento
ch_target_pitch_hi		db		;09 (not used at all in sourcecode)
ch_tone_portamento_speed	db		;0a
ch_portamento_slide_dir		db		;0b bit0 sets up/down
ch_vibrato_command_buffer	db		;0c this is a buffer for depth and speed when using vibrato continue
ch_vibrato_position		db		;0d
ch_vibrato_tremolo_waveform	db		;0e 0=sine, 1=ramp down, 2=square
ch_tremolo_position		db		;0f
ch_tremolo_command_buffer	db		;10
ch_arpeggio_note_buffer		db		;11
ch_tick_delay			db		;12
ch_amiga_period_lo		db		;13 this is the current amiga period
ch_amiga_period_hi		db		;14 (not directly used in sourcecode)
ch_current_amiga_period_lo	db		;15 this is the tone portamento target amiga period
ch_current_amiga_period_hi	db		;16 (not directly used in sourcecode)
;next must be the last entry in this array:
ch_current_channelmask		db		;last byte in array and mask for channel



.ende


;.........................................................



.ORGA (ApuCodeBaseAdress-2)		;$2FC
.DW ApuCodeBaseAdress			;spcram copy offset




BootInit:
	clrp
	mov	$f1,#$f0
	mov	a,#$ff
	mov	!$fff0,a
	mov	x,#Stack			;set stack pointer (old: $cf)
	mov	sp,x
	mov	a,#$00
	mov	x,a
	jmp	!ClearVariables

CopyRight:
	.db "matt@dforce3000.de"
CopyRightEnd:

ClearVariables:
	mov	(x)+,a
	cmp	x,#$f0
	bne	ClearVariables

	mov MainVolL,#$7f
	mov MainVolR,#$7f
	mov EchoVolL,#$00
	mov EchoVolR,#$00
	mov ChannelEnable,#$0f
	mov ChannelEchoEnable,#$0f
	mov SpcReportType,#0		;reset report type to void
	mov	MusicVol1,#$7f			;init volume. narrow stereo, a little bit stereo separation
	mov	MusicVol2,#$7f

	
	mov	SpcDataPointerLo,#(ModFile & $00ff)		;setup mod pointer
	mov	SpcDataPointerHi,#(ModFile >> 8)

	call	!ClearAllChannelArray
	call	!SetupDspRegsAndTimers		;reset dsp regs first.
	call	!InitVariables		;set up pointers to first pattern
	call	!UpdateVolEchoVariablesOnly
	

MainLoop:
	mov	a,$fd		;clear counter0
MainLoop1:
	mov	a,$fd		;wait for counter 0 to become zero
	beq	MainLoop1		;takes roughly 44 scanlines total

	call	!MainWaitLoop		
	call	!SendReportData

	mov	a,$f4		;check if command present on port0
	and	a,#$f0
	cmp	a,#$f0
	bne	MainLoop	;return to mainloop if there isn't

	mov	a,$f4		;get command
	and	a,#$0f	
	asl 	a
	mov	x,a			;pointer into jump table
	jmp	[!CommandJumpTable+x]
	
CommandJumpTable:
	.dw CommandVoid			;0
	.dw ReceiveSong
	.dw CommandVoid
	.dw CommandVoid
	.dw CommandVoid
	.dw ReceiveStream		;5
	.dw CommandVoid
	.dw CommandVoid
	.dw ReceiveSamplePack
	.dw CommandVoid
	.dw PlaySoundEffect		;10
	.dw StopSongStream
	.dw SetSongSpeed
	.dw SetSongChannelMask
	.dw SetReportType
	.dw SetSongVolume


ReceiveSong:
	 ;timers must be reset before receiving a song, else we can't wait properly for the echo buffer to initialize
	call	!ReceiveSongHandler
	call	!ClearAllChannelArray
	call	!SetupSamples		;set up sample offsets
	call	!SetupDspRegsAndTimers		;reset dsp regs first.
	call	!InitVariables		;set up pointers to first pattern
	call	!UpdateVolEchoVariablesOnly
;return to mainloop:
	bra MainLoop

UpdateVolEcho:
	call	!UpdateVolEchoHandler
	bra	MainLoop

ResetHandler:
;clear apu ports:
	mov	y,#$00
	mov	$f4,y
	mov	$f5,y
	mov	$f6,y
	mov	$f7,y
;clear dsp regs:
	mov	a,#$6c
	mov	y,#$e0		;reset dsp, mute on, echo off
	movw	$f2,ya
	mov	a,#$0c
	movw	$f2,ya
	mov	a,#$1c
	movw	$f2,ya
	mov	a,#$2c
	movw	$f2,ya
	mov	a,#$3c
	movw	$f2,ya
	mov	a,#$4c
	movw	$f2,ya
	mov	y,#$ff
	mov	a,#$5c
	movw	$f2,ya
	mov	y,#$00
	mov	a,#$0d
	movw	$f2,ya
	mov	a,#$2d
	movw	$f2,ya
	mov	a,#$3d
	movw	$f2,ya
	mov	a,#$4d
	movw	$f2,ya
	mov	a,#$5d
	movw	$f2,ya
	mov	a,#$6d
	movw	$f2,ya
	mov	a,#$7d
	movw	$f2,ya
	mov	$f1,#$f0
;jump to ipl rom:
	jmp	!$ffc0
	jmp	!$ffc0

MainWaitLoop:
	bbs	$ef.0,MainWaitLoopSongPresent		;only process song if song has been uploaded before
	ret
	
MainWaitLoopSongPresent:
	inc	WaitCounter

InnerWaitLoop:
	cmp	WaitCounter,PatternSpeed			;old #$04 incremet WaitCounter until 4, selfmodying code changes the default speed(command setspeed0x0f) by writing to the second operand
	bne	UpdateTickBasedEffectsOnly


	mov	WaitCounter,#$00
	mov	y,NextRowChFadeOuts
	mov	a,#$5c
	movw	$f2,ya
	mov	a,$fe	;clear 4bit counter
	mov	y,#2
KofWaitLoop:
	mov	a,$fe		;wait for 2mS
	beq	KofWaitLoop
;**************************************************
	dbnz	y,KofWaitLoop

	call	 !ProcessNextRow

	call	!PreTestRow
	ret



UpdateTickBasedEffectsOnly:	


	movw	ya,Channel1Array
	movw	CurrentChDataLo,ya
	mov	CurrentChannel,#$00
	call	!CheckForTickBasedEffects
	movw	ya,Channel2Array
	movw	CurrentChDataLo,ya
	mov	CurrentChannel,#$10
	call	!CheckForTickBasedEffects
	movw	ya,Channel3Array
	movw	CurrentChDataLo,ya
	mov	CurrentChannel,#$20
	call	!CheckForTickBasedEffects

	movw	ya,Channel4Array
	movw	CurrentChDataLo,ya	
	mov	CurrentChannel,#$30
	call	!CheckForTickBasedEffects

	mov	a,TickSampleStart
	beq UpdateTickBasedEffectsOnlyNoRetrigger

	mov	$f2,#$4c			
	mov	$f3,a							;retrigger recently started samples
	
	mov	TickSampleStart,#0
UpdateTickBasedEffectsOnlyNoRetrigger:

	ret



ProcessNextRow:
	movw	ya,CurrentPatternPointerLo	;mark beginning of current row
	movw	CurrentRowPointerLo,ya

	incw	ModTimecode
	mov	KeyOn,#$00			;clear key on
	movw	ya,Channel1Array
	movw	CurrentChDataLo,ya	
	mov	CurrentChannel,#$00
	call	!GetDataFromPattern
	call	!SetVolume
	movw	ya,Channel2Array
	movw	CurrentChDataLo,ya	
	mov	CurrentChannel,#$10
	call	!GetDataFromPattern
	call	!SetVolume
	movw	ya,Channel3Array
	movw	CurrentChDataLo,ya	
	mov	CurrentChannel,#$20
	call	!GetDataFromPattern
	call	!SetVolume
	movw	ya,Channel4Array
	movw	CurrentChDataLo,ya	
	mov	CurrentChannel,#$30
	call	!GetDataFromPattern
	call	!SetVolume
	
	call	!KeyOnKeyOff
	call	!PatternLoopJumper
	jmp	!GetDataFromPatternCheckPatternPosition


GetDataFromPatternEntryIsVoid:
	incw	CurrentPatternPointerLo
	mov	y,#ch_instrument
	mov	a,#$ff					;store $ff in all 4 channel input buffers
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	[CurrentChDataLo]+y,a
	ret



GetDataFromPattern:
	mov	y,#ch_instrument
	mov	a,[CurrentChDataLo]+y			;get last rows instrument, pitch and effects
	mov	TempBuffer1,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer5,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer6,a
	or	 TempBuffer1,TempBuffer2
	or	 TempBuffer1,TempBuffer5
	or	 TempBuffer1,TempBuffer6			;check if a note was present
	mov	a,TempBuffer1
	bne	GetDataFromPatternEntryContainsNewData
	call	!GetPitchFromBufferAndSet

GetDataFromPatternEntryContainsNewData:
	mov	y,#$00
	mov	a,[CurrentPatternPointerLo]+y
	cmp	a,#$ff			;compare if channel entry is $ff(skip)
	beq	GetDataFromPatternEntryIsVoid

;there is a new note to trigger, copy data from pattern to current channel data buffer:
	
	mov	[CurrentChDataLo]+y,a	;copy instrument
	inc	y
	mov	a,[CurrentPatternPointerLo]+y			;copy note
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,[CurrentPatternPointerLo]+y			;copy effect
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,[CurrentPatternPointerLo]+y			;copy effect data
	mov	[CurrentChDataLo]+y,a
	incw	CurrentPatternPointerLo
	incw	CurrentPatternPointerLo
	incw	CurrentPatternPointerLo
	incw	CurrentPatternPointerLo


	mov	y,#ch_effect
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	mov	TempBuffer1,a
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	and	a,#$f0
	or	a,TempBuffer1
	cmp	a,#$de					;delay sample effect present? if yes, don't trigger sample
	beq DontSetNewSample
	
	call	!SetNewSample
	
DontSetNewSample:
	ret
	
SetNewSample:		
	mov	y,#ch_instrument						;a non-present instrument is indicated by $00
	mov	a,[CurrentChDataLo]+y
	beq	GetDataFromPatternInstrumentIsVoid

;trigger new instrument, fetch sample data for current instrument
	dec	a							;multiply instrument number with 8 to get pointer into sample array
	mov	y,#$08
	mul	ya
	addw	ya,ModFilePointer2Lo				
	movw	TempBuffer1,ya					;tempbuffer1 is now pointer to current sample entry
	mov	y,#$02						;get sample finetune
	mov	a,[TempBuffer1]+y
	mov	y,#ch_finetune						;save to current channel finetune
	mov	[CurrentChDataLo]+y,a				
	mov	y,#$03						;get sample volume
	mov	a,[TempBuffer1]+y
	mov	CurrentVolume,a					;save to current volume and current channel volume
	mov	y,#ch_current_volume
	mov	[CurrentChDataLo]+y,a

GetDataFromPatternInstrumentIsVoid:
	mov	y,#ch_note
	mov	a,[CurrentChDataLo]+y
	cmp	a,#$ff					;a non-present note is indicated by $ff
	beq	GetDataFromPatternPeriodIsVoid

	mov	y,#ch_effect					;check if effect command is $ff
	mov	a,[CurrentChDataLo]+y
	cmp	a,#$ff
	beq	GetDataFromPatternCalculateNewPeriod	;if it is, directly calculate new period (why?)

	and	a,#$0f					;merge effect command and data to test for finetune command
	mov	TempBuffer1,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	and	a,#$f0
	or	 a,TempBuffer1
	xcn	a

	cmp	a,#$e5			;finetune
	beq	FinetuneStep

	mov	y,#ch_effect
	mov	a,[CurrentChDataLo]+y			;get current effect
	and	a,#$0f
	cmp	a,#$03					;check if effect is tone portamento
	beq	GetDataFromPatternPortamento

	cmp	a,#$05					;check if effect is tone portamento and volslide
	beq	GetDataFromPatternPortamento
	bra	GetDataFromPatternCalculateNewPeriod

FinetuneStep:
	call	!Finetune
	bra	GetDataFromPatternCalculateNewPeriod

GetDataFromPatternPortamento:
	call	!TonePortamento
	
	jmp	!GetDataFromPatternCheckEffects


GetDataFromPatternPeriodIsVoid:
	jmp	!GetDataFromPatternCheckEffects

GetDataFromPatternCalculateNewPeriod:
	mov	y,#ch_note
	mov	a,[CurrentChDataLo]+y
	mov	y,#ch_arpeggio_note_buffer
	mov	[CurrentChDataLo]+y,a
	mov	y,a				;put period in y
	push	y
	mov	y,#ch_finetune				
	mov	a,[CurrentChDataLo]+y		;calculate finetune
	mov	y,#$48
	mul	ya
	addw	ya,PeriodTablePointerLo
	movw	CurrentPeriodPointer,ya
	
	pop	y					;get corresponding value in 14bit period table
	mov	a,[CurrentPeriodPointer]+y				;get current period
	mov	TempBuffer1,a
	mov	PeriodBufferLo,a
	inc	y
	mov	a,[CurrentPeriodPointer]+y
	mov	TempBuffer2,a
	mov	PeriodBufferHi,a

;fetch pitch from pitch table. input: period	
	mov	y,TempBuffer2		
	mov	a,TempBuffer1
	addw	ya,TempBuffer1		;multiply by 2
	addw	ya,PitchTablePointerLo	;add table offset	
	movw	TempBuffer1,ya				;move adress to pointer
	mov	y,#0

	mov	a,[TempBuffer1]+y				;get lo pitch byte	
	mov	PitchBufferLo,a			;save new pitch
	inc	y
	mov	a,[TempBuffer1]+y
	mov	PitchBufferHi,a
;fetch pitch end	
	
	mov	y,#ch_current_pitch_lo			;save new pitch to array
	mov	a,PitchBufferLo
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,PitchBufferHi
	mov	[CurrentChDataLo]+y,a
	
	mov	y,#ch_amiga_period_lo			;save new period to array
	mov	a,PeriodBufferLo
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,PeriodBufferHi
	mov	[CurrentChDataLo]+y,a
	mov	y,#ch_vibrato_tremolo_waveform			;check vibrato waveform
	mov	a,[CurrentChDataLo]+y
	cmp	a,#$04
	bne	VibratoRetrigger

	mov	y,#ch_vibrato_position			;reset vibrato position
	mov	a,#$00
	mov	[CurrentChDataLo]+y,a

VibratoRetrigger:
	mov	y,#ch_vibrato_tremolo_waveform
	mov	a,[CurrentChDataLo]+y
	cmp	a,#$40
	bne	VibratoNoWaveformReset

	mov	y,#ch_tremolo_position
	mov	a,#$00
	mov	[CurrentChDataLo]+y,a

VibratoNoWaveformReset:
	mov	a,CurrentChannel
	or	 a,#$02
	mov	x,a
	mov	y,#ch_current_pitch_lo
	mov	a,[CurrentChDataLo]+y
	mov	$f2,x			;set pitch height(hi), dsp reg $x2
	mov	$f3,a
	inc	y
	inc	x
	mov	a,[CurrentChDataLo]+y
	
	mov	$f2,x			;set pitch height(lo), dsp reg $x3
	mov	$f3,a
	inc	x
	
	mov	y,#ch_instrument			;set source sample number
	mov	a,[CurrentChDataLo]+y
	beq	DontUpdateSourceSampleNumber
	mov	$f2,x
	mov	$f3,a
	inc	x
	mov	$f2,x			;disable adsr, enable gain
	mov	$f3,#$00			;old #$0c
	inc	x

	mov	$f2,x			;clear adsr2
	mov	$f3,#$00			
	inc	x
	inc	x
	mov	y,#ch_current_channelmask
	mov	a,[CurrentChDataLo]+y			;trigger note
	or	 a,KeyOn			;set note in key on var
	mov	KeyOn,a
DontUpdateSourceSampleNumber:
	jmp	!GetDataFromPatternCheckEffects

GetPitchFromBufferAndSet:
	mov	a,CurrentChannel
	or	 a,#$02
	mov	x,a
	mov	y,#ch_current_pitch_lo
	mov	a,[CurrentChDataLo]+y
	mov	$f2,x
	mov	$f3,a
	inc	x
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	$f2,x
	mov	$f3,a
	ret


CheckForTickBasedEffects:
;check if note cutoff is due

	mov	y,#ch_effect
	mov	a,[CurrentChDataLo]+y
	cmp	a,#$ff
	beq	GetPitchFromBufferAndSet
;check for special effects:
	and	a,#$0f
	asl	a
	mov	x,a			;use effect command as pointer into jump table
	jmp	[!TickBasedEffectJumpTable+x]
	
TickBasedEffectJumpTable:	
	.dw TickBasedArpeggio
	.dw TickBasedPortamentoUp
	.dw TickBasedPortamentoDown
	.dw TickBasedTonePortamento
	.dw TickBasedVibrato
	.dw TickBasedVolslidePorta
	.dw VolslideVibrato
	.dw TickBasedTremolo
	.dw TickBasedVoid
	.dw TickBasedVoid
	.dw TickbasedVolslide
	.dw TickBasedVoid
	.dw TickBasedVoid
	.dw TickBasedVoid
	.dw TickBasedExEffects
	.dw TickBasedVoid


TickBasedVoid:
	ret
	
TickBasedExEffects:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;get ex-effect subtype
	and	a,#$f0
	lsr a
	lsr a
	lsr a
	mov	x,a			;use effect command as pointer into jump table
	jmp	[!TickBasedExEffectJumpTable+x]

TickBasedExEffectJumpTable:
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExEffectNoteRetrigger
	.dw TickBasedExVoid
	.dw TickBasedExVoid
	.dw TickBasedExEffectNoteCutOff
	.dw TickBasedExEffectNoteDelay
	.dw TickBasedExVoid
	.dw TickBasedExVoid

TickBasedExVoid:
	ret

;check for special effects commands
GetDataFromPatternCheckEffects:
	mov	y,#ch_effect
	mov	a,[CurrentChDataLo]+y
	cmp	a,#$ff			;no effect present?
	beq	GetPitchFromBufferAndSetLong

	and	a,#$0f

	asl	a
	mov	x,a			;use effect command as pointer into jump table
	jmp	[!NormalEffectJumpTable+x]
	
NormalEffectJumpTable:
	.dw GetPitchFromBufferAndSet
	.dw GetPitchFromBufferAndSet
	.dw GetPitchFromBufferAndSet
	.dw GetPitchFromBufferAndSet
	.dw GetPitchFromBufferAndSet
	.dw GetPitchFromBufferAndSet
	.dw GetPitchFromBufferAndSet
	.dw GetPitchFromBufferAndSet
	.dw GetPitchFromBufferAndSet
	.dw SetSampleOffset
	.dw GetPitchFromBufferAndSet
	.dw PositionJump
	.dw EffectSetVolume
	.dw PatternBreak
	.dw GetDataFromPatternCheckExEffects
	.dw SetSpeed0x0F

GetPitchFromBufferAndSetLong:
	jmp	!GetPitchFromBufferAndSet
	
;these are the special $Ex effects
GetDataFromPatternCheckExEffects:

	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	and	a,#$f0
	mov	y,a			;save a
	xcn	a
	asl	a
	mov	x,a			;pointer into jump table

	jmp	[!ExEffectJumpTable+x]
	
ExEffectJumpTable:
	.dw ExEffectSpecialReportCmd
	.dw FinePortamentoUp
	.dw FinePortamentoDown
	.dw ExEffectVoid	
	.dw SetVibratoWaveform
	.dw Finetune
	.dw PatternLoop
	.dw ExEffectVoid
	.dw ExEffectVoid
	.dw EffectNoteRetrigger
	.dw FineVolSlideUp
	.dw FineVolSlideDown
	.dw EffectNoteCutOff
	.dw EffectNoteDelay
	.dw ExEffectVoid
	.dw ExEffectVoid
	

ExEffectVoid:
	ret
	

TickBasedArpeggio:
	mov	y,#ch_finetune				;calculate finetune
	mov	a,[CurrentChDataLo]+y
	mov	y,#$48
	mul	ya
	addw	ya,PeriodTablePointerLo
	movw	CurrentPeriodPointer,ya
	
	mov	y,#ch_current_pitch_lo
	mov	a,[CurrentChDataLo]+y		;save current pitch to buffer
	mov	PitchBufferLo,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	PitchBufferHi,a
	jmp	!ArpeggioToneSelector

TickBasedVolslidePorta:
	call	!TonePortamentoContinue
	jmp	!TickbasedVolslide
VolslideVibrato:
	call	!ContinueVibrato
	jmp	!TickbasedVolslide




ArpeggioToneSelector:
	mov	a,WaitCounter		;get waitcounter
	mov	y,#0			;clear y
	mov	x,#3			;put 3 into x
	div	ya,x			;divide waitcounter by three
	
	mov	TempBuffer1,y		;put remainder(can take values 0-2) into TempBuffer1
	
	cmp	TempBuffer1,#$01
	beq	ArpeggioTone1

	cmp	TempBuffer1,#$02
	beq	ArpeggioTone2

	cmp	TempBuffer1,#$00
	beq	ArpeggioTone3
	
		
	ret

ArpeggioTone1:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	xcn	a
	and	a,#$0f
	bra	ArpeggioCommit

ArpeggioTone2:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	bra	ArpeggioCommit

ArpeggioTone3:
	mov	y,#ch_current_pitch_lo				;use current tone as pitch, unchanged
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer1,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a
	bra	ArpeggioSetPitch

ArpeggioCommit:
	asl	a
	mov	TempBuffer1,a
	mov	y,#ch_arpeggio_note_buffer
	mov	a,[CurrentChDataLo]+y
	clrc
	adc	a,TempBuffer1
	mov	y,a

	mov	a,[CurrentPeriodPointer]+y				;get current period
	mov	ScratchPad1,a
	inc	y
	mov	a,[CurrentPeriodPointer]+y
	mov	ScratchPad2,a
	
	mov	y,ScratchPad2		
	mov	a,ScratchPad1
	addw	ya,ScratchPad1		;multiply by 2
	addw	ya,PitchTablePointerLo	;add table offset	
	movw	ScratchPad1,ya				;move adress to pointer
	mov	y,#0

	mov	a,[ScratchPad1]+y				;get lo pitch byte	
	mov	TempBuffer1,a			;save new pitch
	inc	y
	mov	a,[ScratchPad1]+y
	mov	TempBuffer2,a


ArpeggioSetPitch:
	mov	a,CurrentChannel
	or	 a,#$02
	mov	$f2,a
	mov	$f3,TempBuffer1
	inc	a
	mov	$f2,a
	mov	$f3,TempBuffer2
	ret

FinePortamentoUp:
	cmp	WaitCounter,#$00
	bne	PortamentoExit				

	mov	PortamentoSpeedMask,#$0f				;portamento up, but only with lower nibble of effect value

TickBasedPortamentoUp:
	mov	a, #(HighestPortaPeriod & $00ff)
	mov	y, #(HighestPortaPeriod	>> 8)
	movw	TempBuffer5,ya			;put highest slideable period into buffer
	
	mov	y,#ch_amiga_period_lo				;get current period
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer1,a			;and save it to temp buffer
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a

	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;get current effect value
	and	a,PortamentoSpeedMask		;and with mask depeding on normal or finetune portamento
	mov	y,#$00				;inverse clear y
	movw	TempBuffer3,ya			;put ya into tempbuffer3
	movw	ya,TempBuffer1			;get current period
	subw	ya,TempBuffer3			;substract portamento speed

	cmpw	ya,TempBuffer5			;if negative, we've slided too high
	bpl	PortaUpNoLimit

	movw	ya,TempBuffer5		;use limit value if we've slided too high

PortaUpNoLimit:	
	movw	TempBuffer1,ya		;put new pitch to buffer

	mov	y,#ch_amiga_period_lo				;save back current period
	mov	a,TempBuffer1
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,TempBuffer2
	mov	[CurrentChDataLo]+y,a
	
	mov	y,TempBuffer2		;get current period	
	mov	a,TempBuffer1
	addw	ya,TempBuffer1		;multiply by 2
	addw	ya,PitchTablePointerLo	;add table offset	
	movw		TempBuffer3,ya				;move adress to pointer
	mov	y,#0

	mov	a,[TempBuffer3]+y				;get lo pitch byte	
	mov	TempBuffer1,a			;save new pitch
	inc	y
	mov	a,[TempBuffer3]+y
	mov	TempBuffer2,a	
	
	
	mov	y,#ch_current_pitch_lo		;save new pitch to current pitch and update pitch register
	mov	a,TempBuffer1
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,TempBuffer2
	mov	[CurrentChDataLo]+y,a
	mov	a,CurrentChannel
	or	 a,#$02
	mov	$f2,a
	mov	$f3,TempBuffer1
	inc	a
	mov	$f2,a
	mov	$f3,TempBuffer2

PortamentoExit:
	mov	PortamentoSpeedMask,#$ff
	ret

FinePortamentoDown:
	cmp	WaitCounter,#$00
	bne	PortamentoExit

	mov	PortamentoSpeedMask,#$0f

TickBasedPortamentoDown:

	mov	a, #(LowestPortaPeriod & $00ff)
	mov	y, #(LowestPortaPeriod	>> 8)
	movw	TempBuffer5,ya			;put lowest slideable period into buffer
	
	mov	y,#ch_amiga_period_lo				;get current period
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer1,a			;and save it to temp buffer
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a

	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;get current effect value
	and	a,PortamentoSpeedMask		;and with mask depeding on normal or finetune portamento
	mov	y,#$00				;inverse clear y
	addw	ya,TempBuffer1			;add portamento speed

	cmpw	ya,TempBuffer5			;if positive, we've slided too high
	bmi	PortaDownNoLimit

	movw	ya,TempBuffer5		;use limit value if we've slided too high

PortaDownNoLimit:	
	movw	TempBuffer1,ya		;put new pitch to buffer

	mov	y,#ch_amiga_period_lo				;save back current period
	mov	a,TempBuffer1
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,TempBuffer2
	mov	[CurrentChDataLo]+y,a
	
	mov	y,TempBuffer2		;get current period	
	mov	a,TempBuffer1
	addw	ya,TempBuffer1		;multiply by 2
	addw	ya,PitchTablePointerLo	;add table offset	
	movw		TempBuffer3,ya				;move adress to pointer
	mov	y,#0

	mov	a,[TempBuffer3]+y				;get lo pitch byte	
	mov	TempBuffer1,a			;save new pitch
	inc	y
	mov	a,[TempBuffer3]+y
	mov	TempBuffer2,a
	
		
		
	mov	y,#ch_current_pitch_lo
	mov	a,TempBuffer1
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,TempBuffer2
	mov	[CurrentChDataLo]+y,a
	mov	a,CurrentChannel
	or	 a,#$02
	mov	$f2,a
	mov	$f3,TempBuffer1
	inc	a
	mov	$f2,a
	mov	$f3,TempBuffer2
	ret

TonePortamento:
	mov	y,#ch_note							;get note
	mov	a,[CurrentChDataLo]+y
	mov	y,a				;put period in y
	push	y
	mov	y,#ch_finetune				
	mov	a,[CurrentChDataLo]+y		;calculate finetune
	mov	y,#$48
	mul	ya
	addw	ya,PeriodTablePointerLo
	movw	CurrentPeriodPointer,ya
	
	pop	y					;get corresponding value in 14bit period table
	mov	a,[CurrentPeriodPointer]+y				;get current period
	mov	PeriodBufferLo,a
	inc	y
	mov	a,[CurrentPeriodPointer]+y
	mov	PeriodBufferHi,a

	mov	y,#ch_current_amiga_period_lo							;this is our target period to slide to				
	mov	a,PeriodBufferLo
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,PeriodBufferHi
	mov	[CurrentChDataLo]+y,a

	mov	y,#ch_amiga_period_lo							;get current actual period
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer1,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a	
	
	
	mov	y,#ch_portamento_slide_dir
	mov	a,#$00
	mov	[CurrentChDataLo]+y,a
	movw	ya,PeriodBufferLo
	cmpw	ya,TempBuffer1
	beq	TonePortamentoEqual					;check if already at target period

	bmi	TonePortamentoExit					;if plus, the target period is higher(tone is lower) than the current period. slide value must be added in that case

	mov	y,#ch_portamento_slide_dir							;invert sliding direction
	mov	a,#$01
	mov	[CurrentChDataLo]+y,a

TonePortamentoExit:
	ret

TonePortamentoEqual:
	mov	y,#ch_current_amiga_period_lo
	mov	a,#$00
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	[CurrentChDataLo]+y,a
	ret

TickBasedTonePortamento:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;get effect data (tone portamento speed value)
	beq	TonePortamentoContinue		;if zero, continue and dont alter period

	mov	y,#ch_tone_portamento_speed				;save speed
	mov	[CurrentChDataLo]+y,a
	mov	y,#ch_effect_data				;clear effect data
	mov	a,#$00
	mov	[CurrentChDataLo]+y,a

TonePortamentoContinue:
	mov	y,#ch_current_amiga_period_lo				;get target period, put into tempbuffer1
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer1,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a
	
	
	movw	ya,TempBuffer1			;exit if target period is zero (set before if target period has already been reached
	beq	TonePortamentoExit
	mov	y,#ch_tone_portamento_speed
	mov	a,[CurrentChDataLo]+y		;get current speed value (tone portamento speed)
	mov	y,#$00				;clear y
	movw	TempBuffer3,ya			;put slide amount into tempbuffer3
	
	mov	y,#ch_portamento_slide_dir
	mov	a,[CurrentChDataLo]+y
	bne	TonePortaSlideDown		;check whether to slide up or down

	mov	y,#ch_amiga_period_lo				;get current period, put into TempBuffer5
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer5,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer6,a
	
	movw	ya,TempBuffer5			;get current period
	subw	ya,TempBuffer3			;substract slidespeed
	cmpw	ya,TempBuffer1			;check for overflow
	bpl	TonePortaSlideDone

	mov	y,#ch_current_amiga_period_lo				;if overflow occured, clear tone porta target period
	mov	a,#$00
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	[CurrentChDataLo]+y,a
	movw	ya,TempBuffer1			;and load target period instead
	bra	TonePortaSlideDone

TonePortaSlideDown:
	mov	y,#ch_amiga_period_lo				;get current period, put into TempBuffer5
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer5,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer6,a
	
	movw	ya,TempBuffer5			;get current period
	addw	ya,TempBuffer3			;substract slidespeed
	cmpw	ya,TempBuffer1			;check for overflow
	bmi	TonePortaSlideDone

	mov	y,#ch_current_amiga_period_lo				;if overflow occured, clear tone porta target period
	mov	a,#$00
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	[CurrentChDataLo]+y,a
	movw	ya,TempBuffer1			;and load target period instead
;	bra	TonePortaSlideDone

TonePortaSlideDone:
	movw	TempBuffer1,ya			;save newly calculated period to current period buffer

	mov	y,#ch_amiga_period_lo				;save to current period buffer
	mov	a,TempBuffer1
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,TempBuffer2
	mov	[CurrentChDataLo]+y,a

;fetch pitch from pitch table. input: period	
	movw	ya,TempBuffer1		
	addw	ya,TempBuffer1		;multiply by 2
	addw	ya,PitchTablePointerLo	;add table offset	
	movw	TempBuffer1,ya				;move adress to pointer
	mov	y,#0

	mov	a,[TempBuffer1]+y				;get lo pitch byte	
	mov	TempBuffer5,a			;save new pitch
	inc	y
	mov	a,[TempBuffer1]+y
	mov	TempBuffer6,a


	mov	y,#ch_current_pitch_lo				;save to current pitch buffer
	mov	a,TempBuffer5
	mov	[CurrentChDataLo]+y,a
	inc	y
	mov	a,TempBuffer6
	mov	[CurrentChDataLo]+y,a
	mov	a,CurrentChannel
	or	 a,#$02
	mov	$f2,a				;write to dsp
	mov	$f3,TempBuffer5
	inc	a
	mov	$f2,a
	mov	$f3,TempBuffer6
	ret

TickBasedVibrato:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;if speed & depth are zero, continue vibrato without updating em.
	beq	ContinueVibrato

	mov	TempBuffer1,a
	mov	y,#ch_vibrato_command_buffer				;get vibrato speed & depth buffer
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a
	and	TempBuffer1,#$0f
	beq	VibratoSkipDepthUpdate		;dont update depth if its zero

	and	TempBuffer2,#$f0			;update buffer
	or	 TempBuffer2,TempBuffer1

VibratoSkipDepthUpdate:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;get vibrato speed
	mov	TempBuffer1,a
	and	TempBuffer1,#$f0
	beq	VibratoSkipSpeedUpdate		;dont upgrade speed if its zero

	and	TempBuffer2,#$0f			;update buffer
	or	 TempBuffer2,TempBuffer1

VibratoSkipSpeedUpdate:
	mov	y,#ch_vibrato_command_buffer
	mov	a,TempBuffer2			;save back updated vibrato speed & depth
	mov	[CurrentChDataLo]+y,a

ContinueVibrato:
	mov	y,#ch_vibrato_position
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a			;get current vibrato position, this is our pointer into the vibrato table
	lsr	TempBuffer2
	lsr	TempBuffer2
	and	TempBuffer2,#$1f			;divide by 4 and limit
	mov	y,#ch_vibrato_tremolo_waveform			
	mov	a,[CurrentChDataLo]+y
	and	a,#$03				;select current waveform
	mov	TempBuffer3,a
	beq	VibratoSelectSine

	asl	TempBuffer2			;multiply table pointer with 8
	asl	TempBuffer2
	asl	TempBuffer2
	cmp	a,#$01
	beq	VibratoSelectRampDown

	mov	TempBuffer3,#$ff			;this is square vibrato
	bra	VibratoSelectSet

VibratoSelectRampDown:
	mov	y,#ch_vibrato_position
	mov	a,[CurrentChDataLo]+y		;get current vibrato position
	bpl	VibratoSet

	mov	TempBuffer3,#$ff
	setc
	sbc	TempBuffer3,TempBuffer2
	bra	VibratoSelectSet

VibratoSet:
	mov	TempBuffer3,TempBuffer2
	bra	VibratoSelectSet

VibratoSelectSine:
	mov	x,TempBuffer2
	mov	a,!VibratoTable+x			;access vibrato table
	mov	TempBuffer3,a

VibratoSelectSet:
	mov	y,#ch_vibrato_command_buffer					
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f				;get current vibrato depth		
	mov	y,TempBuffer3
	mul	ya				;multiply with selected vibrato position
	movw	TempBuffer5,ya
	mov	y,#$07

VibratoDivLoop:
	lsr	TempBuffer6			;divide by 128
	ror	TempBuffer5
	dbnz	y,VibratoDivLoop
	
	mov	y,#ch_amiga_period_lo				;get current period
	mov	a,[CurrentChDataLo]+y
	mov	PitchBufferLo,a
	inc	y
	mov	a,[CurrentChDataLo]+y
	mov	PitchBufferHi,a
	mov	y,#ch_vibrato_position
	mov	a,[CurrentChDataLo]+y		;get vibrato position
	bmi	VibratoPosNegative


	movw	ya,PitchBufferLo
	subw	ya,TempBuffer5
	bra	VibratoUpdatePitch

VibratoPosNegative:
	movw	ya,PitchBufferLo
	addw	ya,TempBuffer5

VibratoUpdatePitch:
	movw	PitchBufferLo,ya			;this is our new period
	addw	ya,PitchBufferLo		;multiply by 2
	addw	ya,PitchTablePointerLo		;add table offset	
	movw	ScratchPad1,ya			;move adress to pointer
	mov	y,#0

	mov	a,[ScratchPad1]+y				;get lo pitch byte	
	mov	PitchBufferLo,a			;save new pitch
	inc	y
	mov	a,[ScratchPad1]+y
	mov	PitchBufferHi,a

	
	mov	a,CurrentChannel			;write pitch to dsp
	or	 a,#$02
	mov	$f2,a
	mov	$f3,PitchBufferLo
	inc	a
	mov	$f2,a
	mov	$f3,PitchBufferHi


	mov	y,#ch_vibrato_command_buffer
	mov	a,[CurrentChDataLo]+y
	lsr	a
	lsr	a
	and	a,#$3c
	mov	TempBuffer1,a
	mov	y,#ch_vibrato_position
	mov	a,[CurrentChDataLo]+y
	clrc
	adc	a,TempBuffer1
	mov	[CurrentChDataLo]+y,a
	ret

TickBasedTremolo:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	beq	TremoloContinue


	mov	TempBuffer1,a
	mov	y,#ch_tremolo_command_buffer
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a
	and	TempBuffer1,#$0f
	beq	TremoloNoNewSpeed

	and	TempBuffer2,#$f0
	or	 TempBuffer2,TempBuffer1

TremoloNoNewSpeed:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer1,a
	and	TempBuffer1,#$f0
	beq	TremoloNoNewDepth

	and	TempBuffer2,#$0f
	or	 TempBuffer2,TempBuffer1

TremoloNoNewDepth:
	mov	y,#ch_tremolo_command_buffer
	mov	a,TempBuffer2
	mov	[CurrentChDataLo]+y,a

TremoloContinue:
	mov	y,#ch_tremolo_position
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer2,a
	lsr	TempBuffer2
	lsr	TempBuffer2
	and	TempBuffer2,#$1f
	mov	y,#ch_vibrato_tremolo_waveform
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer3,a
	lsr	TempBuffer3
	lsr	TempBuffer3
	lsr	TempBuffer3
	lsr	TempBuffer3
	and	TempBuffer3,#$03
	beq	TremolWavSel


	asl	TempBuffer2
	asl	TempBuffer2
	asl	TempBuffer2
	asl	TempBuffer2
	cmp	TempBuffer3,#$01
	beq	TremolWavSel2

	mov	TempBuffer3,#$ff
	bra	TremoloCommit

TremolWavSel2:
	mov	y,#ch_vibrato_position
	mov	a,[CurrentChDataLo]+y
	bpl	TremoloCommit2

	mov	TempBuffer3,#$ff
	setc
	sbc	TempBuffer3,TempBuffer2
	bra	TremoloCommit

TremoloCommit2:
	mov	TempBuffer3,TempBuffer2
	bra	TremoloCommit

TremolWavSel:
	mov	x,TempBuffer2
	mov	a,!VibratoTable+x			;access table #2
	mov	TempBuffer3,a

TremoloCommit:
	mov	y,#ch_tremolo_command_buffer
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	mov	y,TempBuffer3
	mul	ya
	movw	TempBuffer5,ya
	mov	y,#$06

TremolDivLoop:
	lsr	TempBuffer6
	ror	TempBuffer5
	dbnz	y,TremolDivLoop
	
	mov	y,#ch_current_volume
	mov	a,[CurrentChDataLo]+y
	mov	TempBuffer1,a
	mov	y,#ch_tremolo_position
	mov	a,[CurrentChDataLo]+y
	bmi	TremolPosNeg

	clrc
	adc	TempBuffer1,TempBuffer5
	bra	TremolPosPos

TremolPosNeg:
	setc
	sbc	TempBuffer1,TempBuffer5

TremolPosPos:
	bpl	TremolPosRes

	mov	TempBuffer1,#$00

TremolPosRes:
	cmp	TempBuffer1,#$40
	bmi	TremolPosRes2

	mov	TempBuffer1,#$40

TremolPosRes2:
	mov	a,CurrentChannel
	or	 a,#$00
	mov	$f2,a
	mov	$f3,TempBuffer1
	inc	a
	mov	$f2,a
	mov	$f3,TempBuffer1
	mov	y,#ch_tremolo_command_buffer
	mov	a,[CurrentChDataLo]+y
	lsr	a
	lsr	a
	and	a,#$3c
	mov	TempBuffer1,a
	mov	y,#ch_tremolo_position
	mov	a,[CurrentChDataLo]+y
	clrc
	adc	a,TempBuffer1
	mov	[CurrentChDataLo]+y,a
	ret

TickbasedVolslide:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	xcn	a
	and	a,#$0f
	beq	TickbasedVolSlideDown

	mov	TempBuffer1,a

VolSlideUp:
	mov	y,#ch_current_volume
	mov	a,[CurrentChDataLo]+y
	adc	a,TempBuffer1
	cmp	a,#$40
	bmi	VolSlideUpNoWaveformReset

	mov	a,#$40

VolSlideUpNoWaveformReset:
	mov	[CurrentChDataLo]+y,a
	mov	CurrentVolume,a

	call	!SetVolumeVibrato
	
	ret

TickbasedVolSlideDown:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	mov	TempBuffer1,a

VolSlideDown:
	mov	y,#ch_current_volume
	mov	a,[CurrentChDataLo]+y
	sbc	a,TempBuffer1
	bpl	VolSlideDownNoReset

	mov	a,#$00

VolSlideDownNoReset:
	mov	[CurrentChDataLo]+y,a
	mov	CurrentVolume,a
	call	!SetVolumeVibrato


	
	ret

SetSpeed0x0F:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	mov	PatternSpeed,a			;self-modifying code
	dec	a
	mov	NextRowChFadeOutTime,a
	ret

SetSampleOffset:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	mov	x,a
	mov	a,#0
	mov	y,#0
	mov	TempBuffer3,#$90			;512/16*9=288, $120
	mov	TempBuffer4,#$00

SetSampleOffsetCalcLoop:	
	cmp	x,#0
	beq SetSampleOffsetCalcLoopDone

	addw	ya,TempBuffer3
	dec	x
	bra SetSampleOffsetCalcLoop

SetSampleOffsetCalcLoopDone:
	movw	TempBuffer1,ya
			
	mov	y,#ch_instrument
	mov	a,[CurrentChDataLo]+y
	asl	a
	asl	a
	mov	x,a
	mov	a,!SampleSource+x			;create a new sample (#3f) that plays with the modified start position
	mov	TempBuffer5,a
	mov	!(SampleSource+(ExtSample*4)),a
	mov	a,!(SampleSource+1)+x
	mov	TempBuffer6,a
	mov	!(SampleSource+1+(ExtSample*4)),a
	mov	a,!(SampleSource+2)+x
	mov	!(SampleSource+2+(ExtSample*4)),a
	mov	a,!(SampleSource+3)+x
	mov	!(SampleSource+3+(ExtSample*4)),a
	movw	ya,TempBuffer5
	addw	ya,TempBuffer1
	movw	TempBuffer5,ya
	mov	a,TempBuffer5
	mov	!(SampleSource+(ExtSample*4)),a
	mov	a,TempBuffer6
	mov	!(SampleSource+1+(ExtSample*4)),a
	mov	a,CurrentChannel
	or	 a,#$04
	mov	$f2,a
	mov	$f3,#ExtSample
	ret

PositionJump:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	mov	CurrentPatternNumber,a
	mov	x,a
	mov	a,!(ModFile+$fa)+x
	asl	a
	mov	x,a
	mov	a,!(ModFile+$17a)+x
	mov	y,a
	mov	a,!(ModFile+$17b)+x
	addw	ya,InitPatternPointerLo
	movw	CurrentPatternPointerLo,ya
	pop	a							;need to purge the stack a bit first
	pop	a							;else, it would overflow slowly when using the pattern break command constantly
	pop	a
	pop	a
	pop	a
	pop	a

	jmp	!MainLoop


;set channel volume:
EffectSetVolume:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y


	mov	y,#ch_current_volume
	mov	[CurrentChDataLo]+y,a
	
	mov	CurrentVolume,a
	call	!SetVolume
	ret

PatternBreak:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y					;get break row.
	bne	PatternBreakZero						;only breaks to row #0 of next pattern are supported. else dont do anything

	mov	PatternBreakFlag,#1					;set break flag. the actual break gets executed after the current row has been completely processed
PatternBreakZero:
	ret


ExEffectSpecialReportCmd:
	inc SpecialReportCmd		;increase special cmd counter.
	ret
	
SetVibratoWaveform:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	mov	TempBuffer1,a
	mov	y,#ch_vibrato_tremolo_waveform							;update vibrato waveform
	mov	a,[CurrentChDataLo]+y
	and	a,#$f0
	or	 a,TempBuffer1
	mov	[CurrentChDataLo]+y,a
	ret

Finetune:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	mov	y,#ch_finetune						;whats written to chdata $04 here?
	mov	[CurrentChDataLo]+y,a
	ret

FineVolSlideUp:
	mov	a,WaitCounter
	bne	FineVolSlideUpCancel

	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	mov	TempBuffer1,a
	jmp	!VolSlideUp

FineVolSlideUpCancel:
	ret

FineVolSlideDown:
	mov	a,WaitCounter
	bne	FineVolSlideDownCancel

	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	mov	TempBuffer1,a
	jmp	!VolSlideDown

FineVolSlideDownCancel:
	ret

InitVariables:
	mov	WaitCounter,#$00
	mov	a,#$06
	mov	PatternSpeed,a		;self-modifying code
	dec	a
	mov	NextRowChFadeOutTime,a
	mov	LoopCounter,#0
	mov	CurrentPatternNumber,#$00
	mov	PortamentoSpeedMask,#$ff
	mov	a,!(ModFile+$f8)			;$13d3
	mov	NumberOfPatterns,a
	mov	TempBuffer1,#$fc			;initial relative adress of pattern tables
	mov	TempBuffer2,#$01
	movw	ya,ModFilePointerLo
	movw	ModFilePointer2Lo,ya		
	addw	ya,TempBuffer1			
	movw	CurrentPatternPointerLo,ya
	movw	InitPatternPointerLo,ya
	movw	CurrentLoopPointerLo,ya
	mov	a,!(ModFile+$fa)			;load initial pattern.
	asl	a
	mov	x,a
	mov	a,!(ModFile+$17a)+x			;get pointer to pattern
	mov	y,a
	mov	a,!(ModFile+$17b)+x
	addw	ya,InitPatternPointerLo
	movw	CurrentPatternPointerLo,ya

	mov	a,#(PeriodTable & $00ff)			;set pointer to amiga period table
	mov	PeriodTablePointerLo,a
	mov	a,#(PeriodTable >> 8)
	mov	PeriodTablePointerHi,a

	mov	a,#(PitchTable & $00ff)			;set pointer to snes pitch table
	mov	PitchTablePointerLo,a
	mov	a,#(PitchTable >> 8)
	mov	PitchTablePointerHi,a
	mov	PatternBreakFlag,#0

	mov	SpecialReportCmd,#0

	ret

PortamentoDownLimit:
	.dw $01e3

;set reset some dsp registers:
SetupDspRegsAndTimers:
	mov	$f1,#$00							;stop timers
	mov		a,$fd									;clear 4bit counters
	mov		a,$fe									;clear 4bit counters
	mov		a,$ff									;clear 4bit counters
	mov	SongChannelMask,#$0f		;enable all 4 mod channels
	mov	ModPlayerTimer0,#$a0			
	mov	$fa,ModPlayerTimer0			;20 milliSeconds on timer 0
	mov	$fb,#$10			;2 milliSeconds on timer 1

	nop
	nop
	mov	$f1,#$03			;start timer(s)

	mov	a,#$00			;clear all dsp regs
	mov	x,a

DspRegClearLoop:
	cmp	x,#$6c
	bne	DspRegClearLoopEchoSkip

	mov	$f2,x			;disable echo
	mov	$f3,#$20
	inc	x
	
DspRegClearLoopEchoSkip:	
	mov	$f2,x
	mov	$f3,a
	inc	x
	bpl	DspRegClearLoop

	mov	$f2,#$5d			;setup dsp regs
	mov	$f3,#(SampleSource >> 8)			;this is the sample info loaction (dir)
	mov	$f2,#$6c
	mov	$f3,#$20
	mov	$f2,#$0c			;set master volume
	mov	$f3,MainVolL
	mov	$f2,#$1c
	mov	$f3,MainVolR
	mov	$f2,#$3d
	mov	$f3,#$00
	mov	$f2,#$7c
	mov	$f3,a
;setup echo stuff:

;set filter:
	mov	$f2,#$0f			;fir
	mov	$f3,#$30
	mov	$f2,#$1f			;fir
	mov	$f3,#$00
	mov	$f2,#$2f			;fir
	mov	$f3,#$00
	mov	$f2,#$3f			;fir
	mov	$f3,#$00
	mov	$f2,#$4f			;fir
	mov	$f3,#$00
	mov	$f2,#$5f			;fir
	mov	$f3,#$00
	mov	$f2,#$6f			;fir
	mov	$f3,#$20
	mov	$f2,#$7f			;fir
	mov	$f3,#$30


;setup adsr on channels 1-4:	
	mov	a,CurrentChannel
	push	a
	mov	CurrentChannel,#$00		;channel 1
	call	!SetupAdsr
	mov	CurrentChannel,#$10		;channel 2
	call	!SetupAdsr
	mov	CurrentChannel,#$20		;channel 3
	call	!SetupAdsr
	mov	CurrentChannel,#$30		;channel 4
	call	!SetupAdsr

	pop	a


	ret
	
SetupAdsr:
	mov	a,#$05			;dsp reg $x5 (adsr1)
	or	a,CurrentChannel			;or with channel number
	mov	x,a
	
	mov	$f2,x			;set adsr 1 (winter gold example: $dc) new value: $cb (slower attack an decay)
	mov	$f3,#$0e			;old #$0c
	inc	x
	mov	$f2,x			;set adsr 2 (winter gold example: $2d) new value: $36 (longer release)
	mov	$f3,#$2d			;old #$2d
	inc	x

	mov	a,x
	and	a,#$f0			;only get channel, not register
	mov	x,a
	mov	a,#$40
	ret	


SetupSamples:
	mov	x,#$00
	mov	y,#$00

SetupSamplesLoop:
	inc	y
	mov	a,[ModFilePointerLo]+y		;get sample relative adress low
	dec	y
	clrc
	adc	a,ModFilePointerLo		;add modfile position
	mov		!(SampleSource+4)+x,a		;store in dsp sample area
	mov	a,[ModFilePointerLo]+y		;get sample relative adress hi
	adc	a,ModFilePointerHi		;add modfile position, with carry from previous add
	mov	!(SampleSource+5)+x,a		;store in dsp sample area
	inc	y
	inc	y
	inc	y
	inc	y
;repeat the same with loop adress:	
	inc	y
	mov	a,[ModFilePointerLo]+y		;get sample relative adress low
	dec	y
	clrc
	adc	a,ModFilePointerLo		;add modfile position
	mov		!(SampleSource+6)+x,a		;store in dsp sample area
	mov	a,[ModFilePointerLo]+y		;get sample relative adress hi
	adc	a,ModFilePointerHi		;add modfile position, with carry from previous add
	mov	!(SampleSource+7)+x,a		;store in dsp sample area
	inc	y
	inc	y
	inc	y
	inc	y
	inc	x
	inc	x
	inc	x
	inc	x
	cmp	x,#$7c
	bne	SetupSamplesLoop
	
	ret
	
;old:
	clrc
	mov	a,!(ModFile+1)+x		;get sample location $12dc
	adc	a,#(ModFile & $00ff)		;add #$12db
	mov	!(SampleSource+4)+y,a		;store in dsp sample area
	mov	a,!ModFile+x
	adc	a,#(ModFile >> 8)
	mov	!(SampleSource+5)+y,a
	inc	x
	inc	x
	inc	x
	inc	x
	clrc
	mov	a,!(ModFile+1)+x
	adc	a,#(ModFile & $00ff)
	mov	!(SampleSource+6)+y,a
	mov	a,!ModFile+x
	adc	a,#(ModFile >> 8)
	mov	!(SampleSource+7)+y,a
	inc	x
	inc	x
	inc	x
	inc	x
	inc	y
	inc	y
	inc	y
	inc	y
	cmp	y,#$7c
	bne	SetupSamplesLoop



	ret

;setup stream sample adress, sample #33

SetupStreamSampleAdress:
	mov	a,#SpcStreamSampleNumber	;get number of sample to use
	asl	a				;multiply by 4
	asl	a
	mov	y,a				;pointer into sample source table
	
	mov	a,#(StreamBuffer & $00ff)		;get 1st stream buffer adress
	mov	!(SampleSource)+y,a		;store in dsp sample area
	mov	!(SampleSource+2)+y,a
	mov	a,#(StreamBuffer >> 8)
	mov	!(SampleSource+1)+y,a
	mov	!(SampleSource+3)+y,a
	ret
	
KeyOnKeyOff:
	mov	y,#0
	mov	a,#$5c
	movw	$f2,ya
	mov	$f2,#$4c			
	mov	a,KeyOn
	and	a,SongChannelMask
	mov	$f3,a
	ret

CutOffChannelGain:
	mov	a,#$07				;select GAIN dsp reg
	or	a,CurrentChannel		;
	mov	$f2,a			;set gain	(winter gold example: $3d)
	mov	$f3,#%10011111		;decrease volume, 8mS	;old #%10111111
	mov	y,#ch_current_volume
	mov	a,#0						;clear volume
	mov	[CurrentChDataLo]+y,a
	ret					;return





SetVolumeVibrato:
;set channel mod volume
	mov	a,#$07				;select GAIN dsp reg
	or	a,CurrentChannel		;
	mov	$f2,a				;set gain
	mov	a,CurrentVolume
	and	a,#$7f				;set direct gain mode
	mov	$f3,a	

;set channel global volume
	mov	a,CurrentChannel					;check active channel
	cmp a,#0
	beq SetVolumeLeftCh
	cmp a,#$20
	beq SetVolumeLeftCh

	mov	y,MusicVol1
	movw	$f2,ya
	inc	a
	mov	y,MusicVol2
	movw	$f2,ya	
	ret


SetVolume:
;set channel mod volume
	mov	a,#$07				;select GAIN dsp reg
	or	a,CurrentChannel		;
	mov	$f2,a				;set gain
	mov	y,#ch_current_volume
	mov	a,[CurrentChDataLo]+y	
	and	a,#$7f				;set direct gain mode
	mov	$f3,a	

;set channel global volume
	mov	a,CurrentChannel					;check active channel
	cmp a,#0
	beq SetVolumeLeftCh
	cmp a,#$20
	beq SetVolumeLeftCh

	mov	y,MusicVol1
	movw	$f2,ya
	inc	a
	mov	y,MusicVol2
	movw	$f2,ya	
	ret


SetVolumeLeftCh:
	mov	y,MusicVol2
	movw	$f2,ya
	inc	a
	mov	y,MusicVol1
	movw	$f2,ya	
	ret
		
	mov	a,CurrentChannel					;save current volume
	mov	$f2,a
	mov	a,$f3
	mov	TempVol,a
	
SetVolumeLoop:
	cmp	CurrentVolume,TempVol						;check if bigger or smaller
	beq	SetVolumeExit
	bmi	SetVolumeDec							;branch if old value is bigger than new

	inc	TempVol
	mov	a,CurrentChannel					;check active channel
	mov	y,TempVol
	movw	$f2,ya
	inc	a
	movw	$f2,ya
	
	bra	SetVolumeLoop

SetVolumeDec:
	dec	TempVol	
	mov	a,CurrentChannel					;check active channel
	mov	y,TempVol
	movw	$f2,ya
	inc	a
	movw	$f2,ya	
	bra	SetVolumeLoop


SetVolumeExit:

	ret




	

PreTestRow:
	mov	y,#$00
	mov	NextRowChFadeOuts,#$00
	mov	CurrentChannel,#$01
	call	!PreTestChannel
	mov	CurrentChannel,#$02
	call	!PreTestChannel
	mov	CurrentChannel,#$04
	call	!PreTestChannel
	mov	CurrentChannel,#$08
	jmp	!PreTestChannel
	
	
PreTestChannel:
	mov	a,[CurrentPatternPointerLo]+y
	cmp	a,#$00
	beq	PreTestChannelIsEffectOnly
	cmp	a,#$ff			;compare if channel entry is $ff(skip)
	beq	PreTestChannelIsVoid
	
	inc	y				;check if a period is present
	mov	a,[CurrentPatternPointerLo]+y
	cmp	a,#$ff
	beq	PreTestChannelIsEffectOnly2
	
	inc	y
	mov	a,[CurrentPatternPointerLo]+y
	cmp	a,#3				;check if tone portamento (which doesnt actually play a new note, so the old one must not be muted)
	beq	PreTestChannelIsEffectOnly3

	and	a,#$0f
	mov	TempBuffer1,a
	inc	y
	mov	a,[CurrentPatternPointerLo]+y
	dec y
	and	a,#$f0
	or	a,TempBuffer1
	cmp	a,#$de					;delay sample effect present? if yes, don't trigger sample
	beq PreTestChannelIsEffectOnly3
	
	mov	a,NextRowChFadeOuts
	or	a,CurrentChannel	;if theres a note in the next row on this channel, set flag to fade it out before the next row gets processed
	mov	NextRowChFadeOuts,a
	bra	PreTestChannelIsEffectOnly3

PreTestChannelIsEffectOnly:	
	inc	y				;if theres a note, the pattern data entry is 4 bytes long
PreTestChannelIsEffectOnly2:
	inc	y
PreTestChannelIsEffectOnly3:	
	inc	y

PreTestChannelIsVoid:
	inc	y
	ret

	
PatternLoop:
;get effect value
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;get current effect value
	and	a,#$0f				;and $0f
	bne	PatternLoopDontSetLoopPoint

;effect value is 0, set loop point:
	movw	ya,CurrentRowPointerLo	
	movw	CurrentLoopPointerLo,ya
	ret

;effect value is not 0, check if we are in an active loop
PatternLoopDontSetLoopPoint:
	mov	a,LoopCounter			;check if loop is in progress, must be reset by loop jumper after loop repeat has been finished.
	and	a,#$20
	bne	PatternLoopDontSetLoopPointButLoop
;we arent in a loop, set loop count:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;get current effect value
	and	a,#$0f				;and $0f
	or	a,#$30				;set "in loop"-flag and "do jump"-flag
	mov	LoopCounter,a
	ret

PatternLoopDontSetLoopPointButLoop:
;we are in a loop, just jump:
	mov	a,LoopCounter
	and	a,#$0f
	dec	a
	mov	LoopCounter,a
	beq	PatternLoopFinished

;do another loop jump
	or	LoopCounter,#$30
	
PatternLoopFinished:
	ret


PatternLoopJumper:
	mov	a,LoopCounter
	and	a,#%00010000
	beq	PatternLoopJumperNoJump

	movw	ya,CurrentLoopPointerLo
	movw	CurrentPatternPointerLo,ya
	mov	a,LoopCounter		
	and	a,#%11101111
	mov	LoopCounter,a

PatternLoopJumperNoJump:
	ret

EffectNoteRetrigger:
EffectNoteDelay:
EffectNoteCutOff:
	mov	y,#ch_effect_data
	mov	a,[CurrentChDataLo]+y		;get current effect value
	and	a,#$0f				;and $0f
	beq EffectNoteCutOffImmediately

	dec a

EffectNoteCutOffImmediately:
	mov	y,#ch_tick_delay
	mov	[CurrentChDataLo]+y,a		;write to buffer
	ret

TickBasedExEffectNoteCutOff:
	mov	y,#ch_tick_delay	
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	bne	CheckForTickBasedEffectsNoteCutOffDec
	

	mov	[CurrentChDataLo]+y,a
	call	!CutOffChannelGain
	
	mov	y,#ch_current_channelmask
	mov	a,[CurrentChDataLo]+y			;disable channel in keyon reg
	eor	a,#$ff
	mov	ScratchPad1,a
	and	KeyOn,ScratchPad1

	mov	CurrentVolume,#$00
	call	!SetVolume

	mov	y,#ch_effect_data					;clear effect data so this routine isnt triggered on successive ticks of this frame
	mov	a,#0
	mov	[CurrentChDataLo]+y,a
	
	ret


CheckForTickBasedEffectsNoteCutOffDec:
	dec a
	mov	[CurrentChDataLo]+y,a
	
;CheckForTickBasedEffectsNoNoteCutOff:
	ret	

TickBasedExEffectNoteRetrigger:
TickBasedExEffectNoteDelay:
	mov	y,#ch_tick_delay
	mov	a,[CurrentChDataLo]+y
	and	a,#$0f
	bne	CheckForTickBasedEffectsNoteDelayDec
	
	call	!SetNewSample

	mov	y,#ch_current_channelmask
	mov	a,[CurrentChDataLo]+y			;enable channel in keyon reg
	or	a,TickSampleStart
	mov	TickSampleStart,a

	mov	y,#ch_effect_data					;clear effect data so this routine isnt triggered on successive ticks of this frame
	mov	a,#0
	mov	[CurrentChDataLo]+y,a
	ret

CheckForTickBasedEffectsNoteDelayDec:
	dec a
	mov	[CurrentChDataLo]+y,a
	ret		

SetEchoAndVolume:
	mov	$f2,#$2c			;echo volume l
	mov	$f3,#$00			;0d
	mov	$f2,#$3c			;echo volume r
	mov	$f3,#$00			;0d
	mov	$f2,#$4d			;echo channel enable
	mov	$f3,#$00			;ff
	ret

ReceiveSongHandler:
	mov	$f2,#$0c
	mov	$f3,#$00
	mov	$f2,#$1c
	mov	$f3,#$00
	mov	$f2,#$2c
	mov	$f3,#$00
	mov	$f2,#$3c
	mov	$f3,#$00
	mov	$f2,#$6c			;disable echo
	mov	$f3,#$20			

	
	
	mov	a,#SpcCmdUploadSong			;set "upload song command received"-flag	so cpu knows we're ready
	mov	$f4,a
	mov	a,#$00			;clear remaining ports
	mov	$f5,a
	mov	$f6,a
	mov	$f7,a	

	mov	SpcDataPointerLo,#(ModFile & $00ff)		;setup mod pointer
	mov	SpcDataPointerHi,#(ModFile >> 8)
	mov	ModFilePointerLo,#(ModFile & $00ff)		;setup mod pointer
	mov	ModFilePointerHi,#(ModFile >> 8)

	mov	y,#$00				;clear mod pointer counter(actually, this counter always stays 0,just needed for indirect write)

ReceiveSongWaitLoop2:				;wait for first transfer
	cmp	$f4,#SpcCmdUploadSongDone
	beq	ReceiveSongComplete		;check if upload complete
	
	cmp	$f4,#SpcCmdUploadSongT1			;check if data for transfer 1 is stable and on ports
	bne	ReceiveSongWaitLoop2

	mov	a,$f5				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word
	mov	a,$f6				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word
	mov	a,$f7				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word

	mov	$f4,#SpcCmdUploadSongT1
	
ReceiveSongWaitLoop3:				;wait for second transfer
	cmp	$f4,#SpcCmdUploadSongT2
	bne	ReceiveSongWaitLoop3

	mov	a,$f5				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word
	mov	a,$f6				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word
	mov	a,$f7				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word

	mov	$f4,#SpcCmdUploadSongT2
	bra	ReceiveSongWaitLoop2

ReceiveSongComplete:
	mov	a,#$00			;clear apu ports so that cpu knows were ready
	mov	$f4,a
	mov	$f5,a
	mov	$f6,a
	mov	$f7,a

	mov	ModTimecode,#0					;clear Timecode
	mov	ModTimecode+1,#0

	mov	SongEffectsUploaded,#%00000001			;set "song uploaded" flag, clear "sample pack uploaded" flag
	ret

ReceiveSamplePack:
	mov	a,#SpcCmdUploadSamplePack			;set "upload song command received"-flag	so cpu knows we're ready
	mov	$f4,a
	mov	a,#$00			;clear remaining ports
	mov	$f5,a
	mov	$f6,a
	mov	$f7,a
	
	mov	$f2,#$5c			;key off all channels
	mov	$f3,#%11111111
	
	movw	ya,SpcDataPointerLo		;save offset
	movw	TempBuffer1,ya

	mov	y,#$00				;clear mod pointer counter(actually, this counter always stays 0,just needed for indirect write)

ReceiveSamplePackWaitLoop2:				;wait for first transfer
	cmp	$f4,#SpcCmdUploadSamplePackDone
	beq	ReceiveSamplePackComplete		;check if upload complete
	
	cmp	$f4,#SpcCmdUploadSongT1			;check if data for transfer 1 is stable and on ports
	bne	ReceiveSamplePackWaitLoop2

	mov	a,$f5				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word
	mov	a,$f6				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word
	mov	a,$f7				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word

	mov	$f4,#SpcCmdUploadSongT1
	
ReceiveSamplePackWaitLoop3:				;wait for second transfer
	cmp	$f4,#SpcCmdUploadSongT2
	bne	ReceiveSamplePackWaitLoop3

	mov	a,$f5				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word
	mov	a,$f6				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word
	mov	a,$f7				;get byte
	mov	[SpcDataPointerLo]+y,a
	incw	SpcDataPointerLo				;increment counter, word

	mov	$f4,#SpcCmdUploadSongT2
	bra	ReceiveSamplePackWaitLoop2

ReceiveSamplePackComplete:
	mov	a,#$00			;clear apu ports so that cpu knows were ready
	mov	$f4,a
	mov	$f5,a
	mov	$f6,a
	mov	$f7,a
	movw	ya,TempBuffer1					;restore saved pointer
	movw	SpcDataPointerLo,ya
	
	or	SongEffectsUploaded,#%00000010			;set "sample pack uploaded" flag
	call	!SetupEffectSamples
;return to mainloop:
	jmp	!MainLoop


SetupEffectSamples:
	mov	x,#34*4				;start copying to 34th dsp sample
	mov	y,#$00
	mov	a,[SpcDataPointerLo]+y		;get number of sample in pack
	and	a,#$1f				;maximum number of samples 31
	mov	TempBuffer1,a			;this is our number of samples to process
	inc	y				;advance to first byte pointer

SetupEffectSamplesLoop:

;sample adress:
	mov	a,[SpcDataPointerLo]+y		;get sample relative adress low
	inc	y
	clrc
	adc	a,SpcDataPointerLo		;add modfile position
	mov		!(SampleSource+0)+x,a		;store in dsp sample area
	mov	a,[SpcDataPointerLo]+y		;get sample relative adress hi
	adc	a,SpcDataPointerHi		;add modfile position, with carry from previous add
	dec	y
	mov	!(SampleSource+1)+x,a		;store in dsp sample area
	inc	y
	inc	y
;sample loop adress:	
	mov	a,[SpcDataPointerLo]+y		;get sample relative adress low
	inc	y
	clrc
	adc	a,SpcDataPointerLo		;add modfile position
	mov		!(SampleSource+2)+x,a		;store in dsp sample area
	
	
	mov	a,[SpcDataPointerLo]+y		;get sample relative adress hi
	adc	a,SpcDataPointerHi		;add modfile position, with carry from previous add
	dec	y
	mov	!(SampleSource+3)+x,a		;store in dsp sample area

	mov	a,y				;increment source pointer by 15 (entry is 16 bytes long)
	clrc
	adc	a,#14
	mov	y,a
	
	inc	x
	inc	x
	inc	x
	inc	x
	dbnz	TempBuffer1,SetupEffectSamplesLoop
	
	ret

PlaySoundEffectAbort:
	jmp	!MainLoop

PlaySoundEffect:
	movw	ya,$f4				;save all 4 ports to variables
	movw	TempBuffer1,ya			;tempbuffer1:command,tb2:se number,tb3-4:volume/pitch
	movw	ya,$f6
	movw	TempBuffer3,ya
	
	mov	$f4,#SpcCmdPlaySoundEffect	;echo command received


	

	bbc	$ef.1,PlaySoundEffectAbort		;only process song if song has been uploaded before

	mov	a,TempBuffer4
	and	a,#$80				;get flip flag
	eor	a,SoundEffectFlipFlag
	bne	PlaySoundEffectAbort		;only play effect if this flag has changed as expected
	
	eor	SoundEffectFlipFlag,#$80	;toggle flag for next soundeffect

	mov	y,#0
	mov	a,[SpcDataPointerLo]+y		;get number of samples in pack
	mov	TempBuffer5,a
	mov	a,TempBuffer2
	
	cmp	a,TempBuffer5
	bcs	PlaySoundEffectAbort		;return if sound effect to play is bigger than number of samples in pack


	mov	a,ActiveSoundEffectCh
	inc	a				;goto next channel
	and	a,#3				;maximum number of channels is 3
	mov	ActiveSoundEffectCh,a		;save back
	mov	x,a
	mov	a,!SoundEffectChannelDspRegLUT+x	;get channel value for dsp access
	mov	TempBuffer5,a

	mov	a,!SoundEffectChannelKofLUT+x	;get channel key off bit
	mov	TempBuffer6,a
	mov	$f2,#$5c				;key off current channel
	mov	$f3,a

	
;wait 8ms for key off to finish
	mov	a,$fe	;clear 4bit counter
	mov	y,#4
KofWaitLoop2:
	mov	a,$fe				;wait for 2mS
	beq	KofWaitLoop2
	dbnz	y,KofWaitLoop2

	mov	x,TempBuffer5			;get pointer to dsp channel register set
	
	mov	a,TempBuffer2			;get sample number
	asl	a				;multiply by 16
	asl	a
	asl	a
	asl	a
	inc	a				;increment because first byte in sample pack is number of samples
	mov	y,a
	
	inc	y
	inc	y
	inc	y
	inc	y
	
	mov	a,TempBuffer4			;get 65816 command volume
	and	a,#$7f				;mask off flip flag
	beq	PlaySoundEffectDefaultVol	

;play SE with custom volume
	and	a,#%0001111			;get volume
	mov	TempBuffer7,a
	clrc
	asl	a
	asl	a
	asl	a
	and	TempBuffer7,#%00001111		;add lower bits to get maximum range
	or	a,TempBuffer7
	mov	TempBuffer7,a

	call	!SoundEffectCalcPanL

	mov	$f2,x
	mov	$f3,a
	inc	y
	inc	x
	call	!SoundEffectCalcPanR
	mov	$f2,x
	mov	$f3,a
	bra 	PlaySoundEffectCustomVol

PlaySoundEffectDefaultVol:	
	mov	a,[SpcDataPointerLo]+y		;get volume l
	
	mov	$f2,x
	mov	$f3,a
	inc	y
	inc	x
	mov	a,[SpcDataPointerLo]+y		;get volume r
	
	mov	$f2,x
	mov	$f3,a

PlaySoundEffectCustomVol:
	inc	y
	inc	x

	mov	a,TempBuffer3			;get 65816 command pitch
	beq	PlaySoundEffectDefaultPitch

;play custom pitch. same as mod pitch numbers. valid pitch numbers: 0-59
	push	y
;**************************************************
	asl	a			;asl because each period is 2 bytes long
	mov	y,a				;get period number
	mov	a,[PeriodTablePointerLo]+y				;get current period
	mov	TempBuffer7,a
	inc	y
	mov	a,[PeriodTablePointerLo]+y
	mov	TempBuffer8,a


;fetch pitch from pitch table. input: period	
	mov	y,TempBuffer8		
	mov	a,TempBuffer7
	addw	ya,TempBuffer7		;multiply by 2
	addw	ya,PitchTablePointerLo	;add table offset	
	movw	TempBuffer7,ya				;move adress to pointer
	mov	y,#0

	mov	a,[TempBuffer7]+y				;get lo pitch byte	
	mov	PitchBufferLo,a			;save new pitch
	inc	y
	mov	a,[TempBuffer7]+y
	mov	PitchBufferHi,a
	pop	y
	mov	a,PitchBufferLo		;get pitch lo
	mov	$f2,x
	mov	$f3,a
	inc	y
	inc	x
	mov	a,PitchBufferHi		;get pitch hi
	mov	$f2,x
	mov	$f3,a
	bra	PlaySoundEffectCustomPitch
	
;**********************************************+


	
PlaySoundEffectDefaultPitch:		
	mov	a,[SpcDataPointerLo]+y		;get pitch lo
	
	mov	$f2,x
	mov	$f3,a
	inc	y
	inc	x
	mov	a,[SpcDataPointerLo]+y		;get pitch hi
	
	mov	$f2,x
	mov	$f3,a

PlaySoundEffectCustomPitch:
	inc	x

	mov	a,TempBuffer2			;get sound effect to play
	clrc
	adc	a,#34				;add 34 (sound effects start at dsp sample 34)
	
	mov	$f2,x
	mov	$f3,a

	inc	y
	inc	x
	mov	a,[SpcDataPointerLo]+y		;get adsr lo
	
	mov	$f2,x
	mov	$f3,a

	inc	y
	inc	x
	mov	a,[SpcDataPointerLo]+y		;get adsr hi
	
	mov	$f2,x
	mov	$f3,a

	inc	y
	inc	x
	mov	a,[SpcDataPointerLo]+y		;get gain
	
	mov	$f2,x
	mov	$f3,a

	mov	$f2,#$5c			;reset key off in case it's still active.(shouldn't, though)
	mov	$f3,#%00000000

	mov	$f2,#$4c			;key on channel
	mov	$f3,TempBuffer6
	
	jmp	!MainLoop

;in: 	TempBuffer4:volume&pan command
;	TempBuffer7:total channel volume
;out:	a,8bit: panned channel volume
;uses:	TempBuffer8

SoundEffectCalcPanL:
	mov	a,TempBuffer4			;get pan bits
	xcn	a				;swap nibbles
						;put pan bits into lower 4 bits. we're skipping lsr a, asl a here because the pointers in the LUT are words.
	and	a,#%00000111			;mask off rest
	push	x
	asl	a
	mov	x,a				;use as pointer
	mov	a,TempBuffer7
	jmp	[!PlaySoundEffectLVolLUT+x]


PlaySoundEffectLVolLUT:
	.dw PlaySoundEffectVolTBL
	.dw PlaySoundEffectVolTBL+1
	.dw PlaySoundEffectVolTBL+2
	.dw PlaySoundEffectVolTBL+3
	.dw PlaySoundEffectVolTBL+4
	.dw PlaySoundEffectVolTBL+5
	.dw PlaySoundEffectVolTBL+6
	.dw PlaySoundEffectVolTBL+7


SoundEffectCalcPanR:
	mov	a,TempBuffer4			;get pan bits
	xcn	a				;swap nibbles
						;put pan bits into lower 4 bits. we're skipping lsr a, asl a here because the pointers in the LUT are words.
	and	a,#%00000111			;mask off rest
	push	x
	asl	a
	mov	x,a				;use as pointer
	mov	a,TempBuffer7
	jmp	[!PlaySoundEffectRVolLUT+x]


PlaySoundEffectRVolLUT:
	.dw PlaySoundEffectVolTBL+7
	.dw PlaySoundEffectVolTBL+6
	.dw PlaySoundEffectVolTBL+5
	.dw PlaySoundEffectVolTBL+4
	.dw PlaySoundEffectVolTBL+3
	.dw PlaySoundEffectVolTBL+2
	.dw PlaySoundEffectVolTBL+1
	.dw PlaySoundEffectVolTBL
			
PlaySoundEffectVolTBL:
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	nop
	mov	TempBuffer8,a			;save
	mov	a,TempBuffer7
	setc
	sbc	a,TempBuffer8			;substract calculated pan value
	pop	x
	ret

ReceiveStream:
	mov	$f2,#$5c			;key off channels 0-3
	mov	$f3,#%00001111
	
	mov	StreamState,#0			;reset transfer state machine
	mov	StreamCurrentBuffer,#0		;reset to first buffer
	
	call	!SetupStreamSampleAdress
	movw	ya,$f5
	movw	TempBuffer1,ya			;get total number of samples to transfer
	mov	a,#0
	mov	y,a
	movw	TempBuffer3,ya			;clear current frame counter
	movw	$f5,ya				;tell 65816 current frame to transfer
	mov	$f4,#SpcCmdReceiveStream

ReceiveStreamWaitLoop1:
	cmp	$f4,#SpcCmdSubmitStreamNumber
	bne	ReceiveStreamWaitLoop1


;wait for desired frame
	movw	ya,$f5
	cmpw	ya,TempBuffer3
	bne	ReceiveStreamRequestFrame

	mov	a,$f7
	mov	StreamVolume,a
	beq	ReceiveStreamExit		;exit streaming if volume is 0
;enter receive state machine
	mov	a,StreamState
	asl 	a
	mov	x,a			;pointer into jump table
	jmp	[!StreamJumpTable+x]
	
ReceiveStreamWaitLoop2:
	cmp	$f4,#SpcCmdSubmitStreamNumber	;wait for port0 data to change. Don't know if this works out as desired, but 
	beq	ReceiveStreamWaitLoop2	
	
		
ReceiveStreamTransferLoop:			;66 cycles per scanline. this loop must be exactly 66 bytes long while looping.
;26 cycles					;59 cycles per scanline left for actual transfer

	movw	ya,$f4				
	push a					;save data on stack cause that's the fastest way possible
	push y
	movw	ya,$f6
	push a
	push y
	
;waste 33 cyles:
;******************************
	mov	a,TempBuffer1			;real snes/bsnes need a total of 33 cycles here
	mov	a,TempBuffer1
	mov	a,TempBuffer1
	mov	a,TempBuffer1
	mov	a,TempBuffer1
	mov	a,TempBuffer1
	mov	a,TempBuffer1
	mov	a,TempBuffer1
	mov	a,TempBuffer1
	mov	a,TempBuffer1
	mov	a,TempBuffer1




;7 cycles when looping back
	dbnz	TempBuffer5,ReceiveStreamTransferLoop

	mov	a,#36
	mov	TempBuffer5,a	

	mov	y,#144-1				;set counter to last byte in buffer(reverse copy because of stack)
	
ReceiveStreamSaveLoop:
	pop a					;dummy loop to fix stack
	mov	[TempBuffer7]+y,a		;7	-move to buffer
	dec	y
	pop a					;dummy loop to fix stack
	mov	[TempBuffer7]+y,a		;7	-move to buffer
	dec	y
	pop a					;dummy loop to fix stack
	mov	[TempBuffer7]+y,a		;7	-move to buffer
	dec	y
	pop a					;dummy loop to fix stack
	mov	[TempBuffer7]+y,a		;7	-move to buffer
	dec	y				
	dbnz	TempBuffer5,ReceiveStreamSaveLoop

	cmp	StreamCurrentBuffer,#SpcStreamLatency
	bne	SpcStreamDontSetEndBit
	

	mov	y,#144-9				;set pointer to last samples header
	mov	a,[TempBuffer7]+y			;get header byte
	or	a,#%00000011				;set loop	& end bits

	mov	[TempBuffer7]+y,a			;save back header byte

SpcStreamDontSetEndBit:	
	incw	TempBuffer3			;request next frame
	inc	StreamCurrentBuffer
	and	StreamCurrentBuffer,#7				;maximum number of buffers: 8
StreamTimerWait:				;wait for 16 brr samples to play / ca. 2 frames to pass, then continue downloading stream
	mov	a,$fd
	beq	StreamTimerWait

ReceiveStreamRequestFrame:
	movw	ya,TempBuffer3
	movw	$f5,ya				;tell 65816 which frame to transfer
	
	movw	ya,TempBuffer1			;check if done transfering
	cmpw	ya,TempBuffer3
	bne	ReceiveStreamWaitLoop1

ReceiveStreamExit:
	mov	$f2,#$5c			;key off channels 0-3
	mov	$f3,#%00000001
	mov	$f4,#SpcCmdReceiveStreamComplete	;tell 65816 we're done

;StreamWaitSampleEnd:
	mov	$f1,#$00			;stop timer
	nop
	nop
	mov	$fa,ModPlayerTimer0			;reset timer, return to modplayer
	
	nop
	nop
	mov	$f1,#$03			;start timer
	
	jmp	!MainLoop				;return


StreamJumpTable:
	.dw	InitStream		;fill buffer, then start playing sample
	.dw	StreamContinue		;fill buffer according to timer

StreamContinue:
;setup current stream buffer
	mov	TempBuffer7,#(StreamBuffer & $00ff)		;pointer to first stream buffer
	mov	TempBuffer8,#(StreamBuffer >> 8)
	mov	a,StreamCurrentBuffer

	mov	y,#144
	mul	ya
	addw	ya,TempBuffer7
	movw	TempBuffer7,ya			;pointer to current stream buffer in TempBuffer7,8

	mov	a,#$00
	mov	y,StreamVolume			;update volume
	movw	$f2,ya
	inc	a
	movw	$f2,ya


	mov	y,#0				;clear target copy counter
	
	mov	a,#36				;amount of scanlines to copy
	mov	TempBuffer5,a
	
	jmp	!ReceiveStreamWaitLoop2

InitStream:
;receive stream here:

;setup current stream buffer
	mov	TempBuffer7,#(StreamBuffer & $00ff)		;pointer to first stream buffer
	mov	TempBuffer8,#(StreamBuffer >> 8)
	mov	a,StreamCurrentBuffer
	and	a,#7				;maximum number of buffers: 8
	mov	y,#144
	mul	ya
	addw	ya,TempBuffer7
	movw	TempBuffer7,ya			;pointer to current stream buffer in TempBuffer7,8


	mov	y,#0				;clear target copy counter
	
	mov	a,#36				;amount of scanlines to copy
	mov	TempBuffer5,a

		
	cmp	StreamCurrentBuffer,#SpcStreamLatency
	bne	SpcStreamDontStartSample




;setup sample dsp regs on channel 1:
;wide stereo effect
	mov	a,#$00
	mov	y,StreamVolume			;volume
	movw	$f2,ya
	inc	a
	movw	$f2,ya
	inc	a
	mov	y,#$00				;pitch lo
	movw	$f2,ya
	inc	a
	mov	y,#$04				;pitch hi
	movw	$f2,ya
	inc	a
	mov	y,#SpcStreamSampleNumber	;sample number
	movw	$f2,ya
	inc	a
	mov	y,#0				;adsr1 (use gain instead)
	movw	$f2,ya
	inc	a
	movw	$f2,ya				;adsr2
	inc	a
	mov	y,#$7f				;volume
	movw	$f2,ya

	mov	$f2,#$5c			;reset key off in case it's still active.(shouldn't, though)
	mov	$f3,#%00000000

	mov	$f2,#$4c			;key on channel 1
	mov	$f3,#%00000001

	mov	$f1,#$00			;stop timer
	nop
	nop
	mov	$fa,#StreamerTimer0			;32 milliSeconds on timer 0
	nop
	nop
	mov	$f1,#$03			;start timer
	inc	StreamState			;goto next state

SpcStreamDontStartSample:		
	jmp	!ReceiveStreamWaitLoop2



UpdateVolEchoHandler:
	mov	ModFilePointerLo,#(MainVolL & $00ff)		;setup mod pointer
	mov	ModFilePointerHi,#(MainVolL >> 8)

	mov	y,#$00				;clear mod pointer counter(actually, this counter always stays 0,just needed for indirect write)
	mov	$f4,#$01


UpdateVolEchoWaitLoop1:	
	cmp	$f4,#$00			;check if another data download is due
	bne	UpdateVolEchoWaitLoop1

	mov	a,$f5				;get byte
	mov	[ModFilePointerLo]+y,a
	incw	ModFilePointerLo				;increment counter, word
	mov	a,$f6				;get byte
	mov	[ModFilePointerLo]+y,a
	incw	ModFilePointerLo				;increment counter, word
	mov	a,$f7				;get byte
	mov	[ModFilePointerLo]+y,a
	incw	ModFilePointerLo				;increment counter, word

	mov	$f4,#$00

UpdateVolEchoWaitLoop2:	
	cmp	$f4,#$01			;check if another data download is due
	bne	UpdateVolEchoWaitLoop2

	mov	a,$f5				;get byte
	mov	[ModFilePointerLo]+y,a
	incw	ModFilePointerLo				;increment counter, word
	mov	a,$f6				;get byte
	mov	[ModFilePointerLo]+y,a
	incw	ModFilePointerLo				;increment counter, word
	mov	a,$f7				;get byte
	mov	[ModFilePointerLo]+y,a
	incw	ModFilePointerLo				;increment counter, word

	mov	a,#$00			;clear apu ports so that cpu knows were ready
	mov	$f4,a
	mov	$f5,a
	mov	$f6,a
	mov	$f7,a
UpdateVolEchoVariablesOnly:	

	mov	$f2,#$6d			;echo start adress
	mov	$f3,#EchoStartAdress			;80
	mov	$f2,#$7d			;echo delay
	mov	$f3,#EchoDelay			;06
	mov	$f2,#$0d			;echo feedback
	mov	$f3,#$04			;04


	mov	$f2,#$0c
	mov	$f3,MainVolL
	mov	$f2,#$1c
	mov	$f3,MainVolR
	mov	$f2,#$2c
	mov	$f3,EchoVolL
	mov	$f2,#$3c
	mov	$f3,EchoVolR
	mov	$f2,#$4d
	mov	$f3,ChannelEchoEnable

	
	ret



;set key on/key off here:
GetDataFromPatternCheckPatternPosition:
;***********************************************	
;check if pattern break is due
	mov	a, PatternBreakFlag					;check if pattern break flag is set
	beq	CheckPatternBreakExit

	inc	CurrentPatternNumber					;increment current pattern
	mov	x,CurrentPatternNumber					;check if we overflow
	cmp	x,NumberOfPatterns
	bne	PatternBreakNoOverflow

	mov	CurrentPatternNumber,#$00					;reset to pattern #0 if overflow occured
	mov	x,CurrentPatternNumber

PatternBreakNoOverflow:
	mov	a,!(ModFile+$fa)+x					;get id number of next pattern
	asl	a
	mov	x,a
	mov	a,!(ModFile+$17a)+x					;get word-pointer to that table
	mov	y,a
	mov	a,!(ModFile+$17b)+x
	addw	ya,InitPatternPointerLo				
	movw	CurrentPatternPointerLo,ya				;update current pattern pointer

	mov	PatternBreakFlag,#0					;clear pattern break flag and exit
	pop	a							;need to purge the stack a bit first
	pop	a							;else, it would overflow slowly when using the pattern break command constantly
	pop	a
	pop	a
	jmp	!MainLoop

CheckPatternBreakExit:

;check if at end of current pattern
	mov	x,CurrentPatternNumber
	mov	a,!(ModFile+$fa)+x		;load pattern number
	inc	a
	asl	a			;multiply, use as pointer into patterntable pointertable list
	mov	x,a
	mov	a,!(ModFile+$17a)+x				;$1455+x	;(ModFile+$17a) ;$17a
	mov	TempBuffer2,a
	mov	a,!(ModFile+$17b)+x				;$1456+x	;17b
	mov	TempBuffer1,a
	movw	ya,CurrentPatternPointerLo
	subw	ya,InitPatternPointerLo
	cmpw	ya,TempBuffer1
	bpl	CheckPatternPositionPatternEnd

	ret

CheckPatternPositionPatternEnd:
	inc	CurrentPatternNumber		;increment position in current pattern
	mov	a,CurrentPatternNumber
	cmp	a,NumberOfPatterns		;check if at end of pattern
	beq	CheckPatternPositionReset

	mov	x,a
	mov	a,!(ModFile+$fa)+x
	asl	a
	mov	x,a
	mov	a,!(ModFile+$17a)+x
	mov	y,a
	mov	a,!(ModFile+$17b)+x
	addw	ya,InitPatternPointerLo
	movw	CurrentPatternPointerLo,ya
	movw	CurrentLoopPointerLo,ya
	ret

CheckPatternPositionReset:				;wrap to pattern 0
	mov	$f6,#$50
	mov	CurrentPatternNumber,#$00
	mov	x,CurrentPatternNumber
	mov	a,!(ModFile+$fa)+x
	asl	a
	mov	x,a
	mov	a,!(ModFile+$17a)+x
	mov	y,a
	mov	a,!(ModFile+$17b)+x
	addw	ya,InitPatternPointerLo
	movw	CurrentPatternPointerLo,ya
	ret
	

ClearAllChannelArray:
	mov	a,#(ChannelArray & $ff)		;get pointer to first channel array
	mov	y,#(ChannelArray	>> 8)
	movw	TempBuffer1,ya
	movw	Channel1Array,ya
	mov	a,#1	
	call	!ClearChannelArray
	
	movw	Channel2Array,ya
	mov	a,#2
	call	!ClearChannelArray

	movw	Channel3Array,ya
	mov	a,#4
	call	!ClearChannelArray

	movw	Channel4Array,ya
	mov	a,#8
	call	!ClearChannelArray		
	ret
	
	
ClearChannelArray:
	mov	y,#ch_current_channelmask	;number of bytes to copy
	mov	[TempBuffer1]+y,a		;put channel mask into array pointer + channel mask value
	decw	TempBuffer1
	mov	a,#0
ClearChannelArrayLoop:
	mov	[TempBuffer1]+y,a		;clear array
	dbnz	y,ClearChannelArrayLoop
	
	incw	TempBuffer1
	mov	a,#ch_current_channelmask+1	;get amount of bytes per array and add to current adress
	mov	y,#0
	addw	ya,TempBuffer1
	movw	TempBuffer1,ya			;save adress of next array
	ret		

CommandVoid:
	jmp	!MainLoop

SetSongChannelMask:
	mov		SongChannelMask,$f5			;get new channel mask
	mov	$f2,#$5c
	mov	a,SongChannelMask
	eor	a,#$0f					;invert, switch off masked channels immediately
	mov	$f3,a					;key off 
	mov	$f4,#SpcCmdSetSongChMask		;ack
	jmp	!MainLoop				;return


SetSongSpeed:
	mov	ModPlayerTimer0,$f5			;get new speed
	
	mov	$f1,#$00
	nop
	nop
	mov	$fa,ModPlayerTimer0			;20 milliSeconds on timer 0
	mov	$fb,#$10			;2 milliSeconds on timer 1
	nop
	nop
	mov	$f1,#$03			;start timer(s)	
	mov	$f4,#SpcCmdSetSongSpeed			;ack
	jmp	!MainLoop				;return


StopSongStream:
	and	SongEffectsUploaded,#%11111110		;clear "song uploaded" flag
	mov	$f2,#$5c				;reset key off in case it's still active.(shouldn't, though)
	mov	$f3,#%00001111				;key off all mod/stream channels
	mov	$f4,#SpcCmdStopSong			;ack command
	jmp	!MainLoop				;return



SetReportType:
	mov		SpcReportType,$f5			;get new report type
	mov	$f4,#SpcCmdReportType			;ack
	jmp	!MainLoop

SetSongVolume:
	mov		MusicVol1,$f5			;get new volume, mod channel 0,1
	mov		MusicVol2,$f6			;get new volume, mod channel 0,1
	mov	$f4,#SpcCmdSetSongVolume			;ack
	jmp	!MainLoop

SendReportData:
	mov	a,SpcReportType		;get report type
	and	a,#$07	
	asl 	a
	mov	x,a			;pointer into jump table

	jmp	[!ReportJumpTable+x]
	
ReportJumpTable:
	.dw	ReportVoid
	.dw	ReportTimecode
	.dw	ReportChannelLevels
	.dw	ReportSpecialCmd
	.dw	ReportVoid
	.dw	ReportVoid
	.dw	ReportVoid
	.dw	ReportVoid
	
ReportVoid:
	mov	a,#0
	mov	$f5,a			;clear report data
	mov	$f6,a			
	mov	$f7,a	
	ret


ReportTimecode:
	mov	$f5,#0			;clear report byte first
	movw	ya,ModTimecode
	movw	$f6,ya
	mov	a,SpcReportType
	or	a,#$e0
	mov	$f5,a			;return report type 
	ret


;report current sample levels of all 4 mod channels as 4bit nibbles unsigned.
ReportChannelLevels:
	mov	$f2,#$09
	mov	a,$f3			;get current wave height ch0
	lsr	a
	lsr	a
	lsr	a
	and	a,#$0f
	mov	TempBuffer1,a	

	mov	$f2,#$19
	mov	a,$f3			;get current wave height ch1
	asl	a
	and	a,#$f0
	or	a,TempBuffer1
	mov	TempBuffer1,a

	mov	$f2,#$29
	mov	a,$f3			;get current wave height ch2
	lsr	a
	lsr	a
	lsr	a
	and	a,#$0f
	mov	TempBuffer1+1,a	

	mov	$f2,#$39
	mov	a,$f3			;get current wave height ch3
	asl	a
	and	a,#$f0
	or	a,TempBuffer1+1
	mov	TempBuffer1+1,a				
	
	movw	ya,TempBuffer1
	
	mov	$f5,#0			;clear report byte first
	movw	$f6,ya			;write data to ports
	mov	a,SpcReportType
	or	a,#$e0
	mov	$f5,a			;return report type 
	ret	

ReportSpecialCmd:
	mov	$f5,#0			;clear report byte first
	mov	a,SpecialReportCmd
	mov	$f6,a
	mov	a,SpcReportType
	or	a,#$e0
	mov	$f5,a			;return report type 
	ret
	
	
SoundEffectChannelDspRegLUT:
	.db	$40
	.db	$50
	.db	$60
	.db	$70

SoundEffectChannelKofLUT:
	.db	$10
	.db	$20
	.db	$40
	.db	$80
		
VibratoTable:
	.incbin "data/apu/vibratotable.tbl"

PeriodTable:
	.include "data/apu/amigaperiodtable.tbl"

PitchTable:
	.include "data/apu/pitchtable_new.tbl"
	.incbin "data/apu/pitchtable2.tbl"
ModFile:
