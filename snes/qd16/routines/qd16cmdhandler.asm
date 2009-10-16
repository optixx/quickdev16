	.include "routines/main.h"
.section "qd16cmd" semifree
/*
processes commands from qd16 microcontroller.
communicates using physical rom adresses $00:0000(ACK), $00:0001(CMD), $00:0000(DAT) 
check for open bus before processing any commands
*/
QD16CommandPoll:
	php
	sep #$20
	lda.w Qd16Flags
	bit.b #%100
	bne QD16CommandPollCancel	;cancel manual command polling if irq-com mode is enabled

	jsr QD16CommandHandler
		
QD16CommandPollCancel:
	plp
	rts

QD16CommandHandler:
	php
	rep #$31
	lda.l QD16McCommand				;get data first(in case the mc is just about to steal the bus again)
	sta.b tmp
	lda.l QD16McHandshake				;has microcontroller disabled rom access for snes?(reading open bus returns differing values here)
	cmp.l QD16OpenBusMirror
	bne QD16OpenBus 
	
	sep #$20
	cmp.b #QD16McAck
	bne QD16OpenBus			;either open bus or the mc hasn't written any new command yet
	
	lda.b tmp
	bit.b #$f8					;valid command present?(max number of commands is 8)
	bne QD16OpenBus			;if any of the upper 5 bits are set, command is invalid
	
	lda.b #QD16SnesAck
	sta.l $c00000				;tell mc that data has been received

;handshake complete, process data
	rep #$31
	lda.b tmp
	pha
	and.w #$7					;8 commands max
	asl a
	tax
	pla
	jsr (QD16CmdLUT,x)	;execute command
	
QD16OpenBus:
	plp
	rts
	
QD16CmdLUT:
	.dw QD16GetBankCount
	.dw QD16EnableIrqCom
	.dw QD16UpdateLoadPercentage
	.dw QD16FadeOut
	.dw QD16DrawHeader
	.dw QD16CmdVoid
	.dw QD16CmdVoid
	.dw QD16CmdVoid
	
QD16CmdVoid:
	rts

;this can be triggered by an explicit mc-command. apart from that, it's also triggered when mc reports bank 0 has been uploaded.
QD16DrawHeader:
	rts

QD16EnableIrqCom:
	rep #$31
	lda.l QD16RamIrq		;did mc copy irq-vector to correct location in rom?
	cmp.l QD16RomIrq
;	beq QD16EnableIrqVectorOk	;debug
	
	lda.w #$adba				;clear mc-ack, signalize error, don't enter irq-mode
	sta.l QD16McHandshake
	rts
	
QD16EnableIrqVectorOk:
	sep #$20
	lda.b #1
	sta.w IrqRoutineNumber
	lda.b #%100
	ora.w Qd16Flags
	sta.w Qd16Flags			;set "enable irq-com"-flag
	rts

;input:x,header position in bank
QD16CheckHeaderValid:
	php
	sep #$20
	lda.w Qd16Flags
	bit.b #%10
	bne QD16CheckHeaderNoHirom	;skip header drawing if a valid header has been found before
	
	rep #$31
	lda.l RomVectorReset	;reset vector
	bpl QD16CheckHeaderNoLorom
	
	lda.l RomHeaderMap	;rom map, lorom?
	lsr a
	bcs QD16CheckHeaderNoLorom

	lda.l RomHeaderSize	;rom size
	and.w #$ff
	cmp.w #$7		;smaller than 4mbit?
	bcc QD16CheckHeaderNoLorom

	cmp.w #$e		;bigger than 64mbit?
	bcs QD16CheckHeaderNoLorom

	ldx.w #LoromOffset		;header found
	jsr QD16CheckTitle
	bcc QD16CheckHeaderNoLorom

	sep #$20
	lda.b #%10
	ora.w Qd16Flags
	sta.w Qd16Flags			;set "header found, title printed"-flag	

	plp
	sec
	rts	
	
QD16CheckHeaderNoLorom:
	rep #$31
	lda.l RomVectorReset+HiromOffset	;reset vector high byte
	bpl QD16CheckHeaderNoHirom
	
	lda.l RomHeaderMap+HiromOffset	;rom map, lorom?
	lsr a
	bcc QD16CheckHeaderNoHirom

	lda.l RomHeaderSize+HiromOffset	;rom size
	and.w #$ff
	cmp.w #$7		;smaller than 4mbit?
	bcc QD16CheckHeaderNoHirom

	cmp.w #$e		;bigger than 64mbit?
	bcs QD16CheckHeaderNoHirom

	ldx.w #HiromOffset		;header found
	jsr QD16CheckTitle
	bcc QD16CheckHeaderNoHirom

	sep #$20
	lda.b #%10
	ora.w Qd16Flags
	sta.w Qd16Flags			;set "header found, title printed"-flag	

	plp
	sec
	rts
	
QD16CheckHeaderNoHirom:
	plp
	clc
	rts	


;input:x,16bit:lo/hirom pointer.
;checks if bytes are within ranges $20-$7f and $a7-$df(kana)
QD16CheckTitle:
	php
	phx
	sep #$20
	ldy.w #0

QD16CheckTitleLoop:
;check if chars are in ASCII/kana range and return null if any invalid ones are found. 
	lda.l RomHeaderTitle,x
	cmp.b #$20		
	bcc QD16CheckTitleVoid		
								;is bigger than $20
	cmp.b #$df+1			
	bcs QD16CheckTitleVoid		
								;is between $20 and $df
	bpl QD16CheckTitleCharOk
								;is between $80 and $df
	cmp.b #$a7
	bcc QD16CheckTitleVoid
								;is between $a7 and $df
QD16CheckTitleCharOk:
	inx							;is between $a7 and $df or between $20 and $7f
	iny
	cpy.w #21
	bne QD16CheckTitleLoop

	plx							;title is ok, ready to print
	plp
	sec
	rts

QD16CheckTitleVoid:	
	plx							;title is not present/contains bad chars
	plp
	clc
	rts

QD16GetBankCount:
	sep #$20
	xba
	sta.w QD16BankCount
	lda.b #%1
	ora.w Qd16Flags
	sta.w Qd16Flags			;set "rom upload commenced"-flag
	rts

QD16UpdateLoadPercentage:
	sep #$20
	xba
	cmp.b #101
	bcc QD16UpdateLoadPercentageNoOver
	
	lda.b #100

QD16UpdateLoadPercentageNoOver:
	cmp.w PercentageBuff
	beq QD16UpdateLoadPercentageNoUpdate

	sta.w PercentageBuff
	rep #$31
	lda.w #2															;print percentage bar
	jsr VwfCreateTextbox	
		
QD16UpdateLoadPercentageNoUpdate:
	sep #$20
	lda.b #%1
	ora.w Qd16Flags
	sta.w Qd16Flags			;set "rom upload commenced"-flag
	rts
	
/*
?		= loadstat	
100 = bankcount
*/	
QD16UpdateLoadstatus:
	sep #$20
	xba
	sta.l $4202			;current banks*100
	lda.b #100
	sta.l $4203
	nop
	nop
	nop
	nop
	rep #$31
	lda.l $4216
	sta.l $4204
	sep #$20
	lda.w QD16BankCount	;divide by 100
	sta.l $4206
	nop									;wait some cycles for division to finish
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	rep #$31
	lda.l $4214
	sta.w Qd16LoadProgress


QD16DivErr:
	rts
	
QD16Cmd2:
	rts

QD16FadeOut:
	sep #$20
	jsr PalEffectInit
	lda.b #13
	jsr PalEffectCreate
	rts
	
.ends
