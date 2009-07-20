/*
tiny menu system for snes by d4s in 2006
depends on:
printstring.asm, function: LoadTextString

execute with:
ldx.w #number_of_menu_file_to_load
jsr LoadMenuFile



features:
-1 column, x rows
-seperate bytes and bits manipulation
-print text and menu options with palette highlighting
-subroutine jump upon option change
-load complete menu from table, no hardcoded stuff
-menus tables should be compressable, relative pointers only!

menu table format:
byte		function
2		starting position of first option text on bg3 tilemap
1		relative starting position of first option variable(*2+starting position of first option text on bg3 tilemap)
1		number of seperating rows between options
1		number of rows/options
2xrow quantity	relative pointer to data for each row


row table format:
1		option data type (maximum number:7)
		0x0=no options, only exec subroutine
		0x1=1 byte
		0x2=8 bits
		0x3=2 interchangeable options with description text (eg: "sound: mono/stereo)
1		bitmask for changeable bits if data type=0x2; bitmask(usually only one bit) to determine the string to choose if data type=0x3(if zero=string 1, if not zero=string 2)
1		minimum value for option, wrap to maximum value if changed value equals this
1		maximum value for option, wrap to minimum value if changed value equals this
3		24bit adress of option byte to change
1		number of subroutine to execute when option is changed(push status and registers before executing)
1		palette number when unselected
1		palette number when selected
1		number of option text strings(max=3, AND with 0x3)
2xstring quant.	relative pointer to text string
x		option text string(s), #$00 terminated

processing flow:
1. setup pointer to menu file
2. load all general menu options and save to variables
3. process and draw each option string onto screen once
 -setup pointer to row table
 -draw each option with corresponding option value once
4. go to first option
 -if up pressed, redraw current line in unselected, decrease current line, redraw new current line in selected
 -if down pressed, redraw current line in unselected, increase current line, redraw new current line in selected
 
 -if right pressed, increase options variable pointer
 -if left pressed, decrease options variable pointer
 

data files:
MenuFiles


Pointer Tables
MenuFilesPTable

variables:
MenuFileThreeBytePointerLo
MenuFileThreeBytePointerHi
MenuFileThreeBytePointerBank
MenuRowsThreeBytePointerLo
MenuRowsThreeBytePointerHi
MenuRowsThreeBytePointerBank
MenuRowsThreeByteOptionPointerLo
MenuRowsThreeByteOptionPointerHi
MenuRowsThreeByteOptionPointerBank
MenuRowsThreeByteCodePointerLo
MenuRowsThreeByteCodePointerHi
MenuRowsThreeByteCodePointerBank
LoadMenuInitialOffset
LoadMenuInitialOptionOffset
LoadMenuVerticalSpacing
LoadMenuNumberOfRows
LoadMenuCurrentRow
LoadMenuStringPosLo


ram areas:
LoadMenuStringBuffer(16bytes)

string printing area, 16 bytes:
0 - LoadMenuStringBuffer
1 - LoadMenuStringSetOff		;always #$01, set new position
2 - LoadMenuStringPosLo		;position of string
3 - LoadMenuStringPosHi
4 - LoadMenuStringSetPal		;always #$07, set palette
5 - LoadMenuStringPal		;palette number
6 - LoadMenuStringNewAddr		;draw variable length string, always #$04
7 - LoadMenuStringVectLo
8 - LoadMenuStringVectHi
9 - LoadMenuStringVectBa
a - LoadMenuStringSetOff2
b - LoadMenuStringPosLo2		;position of string
c - LoadMenuStringPosHi2
d - LoadMenuStringByte/binary/string




;this is the textstring for the text buffer of the menu system
TextString20:
	.dw $0000				;offset on bg1 tilemap
	.db $04					;terminator
	.dw LoadMenuStringBuffer
	.db $7e


*/




LoadMenuFile:
	php
	sep #$20
	phb					;set data bank = wram
	lda.b #$7e
	pha
	plb
	lda.b LoadMenuDoInit			;check if menu was already initialized
	bne LoadMenuFileAlreadyInitialized

	lda #(:MenuFiles+BaseAdress>>16)			;get source bank of strings
	sta MenuFileThreeBytePointerBank		;
	rep #$30				;accu 16bit
	txa					;get number of string
	asl					;multiply by 2 to get pointertable entry
	tax
	lda.l (MenuFilesPTable+BaseAdress),x			;get source offset of string
	sta MenuFileThreeBytePointerLo
	ldy.w #$0000

	lda [MenuFileThreeBytePointerLo],y		;get target buffer in wram
	iny					;increment source position, word
	iny			
	and.w #$fffe				;mask off bit 0 to ensure text is word-formatted properly
	sta.b LoadMenuInitialOffset


	lda.b [MenuFileThreeBytePointerLo],y	;get option position
	iny
	clc		
	sep #$20
	asl a
	sta.b LoadMenuInitialOptionOffset	;store in variable

	lda.b [MenuFileThreeBytePointerLo],y	;get vertical spacing position
	iny
	and.b #$0f				;maximum range:16
	inc a
	tax
	lda.b #$00

LoadMenuCalcVerticalSpacingLoop:
	clc
	adc.b #$40
	dex
	bne LoadMenuCalcVerticalSpacingLoop	;add one tileline for every loop, but do it at least once
	sta.b LoadMenuVerticalSpacing

	lda.b [MenuFileThreeBytePointerLo],y	;get number of rows/options
	iny
	and.b #$1f				;maximum number:32
	bne LoadMenuNumberOfRowsNotZero
	
	plb					;exit if number of rows=0
	plp
	rts
	
LoadMenuNumberOfRowsNotZero:	
	sta.b LoadMenuNumberOfRows
	stz.b LoadMenuCurrentRow		;start processing at first row
	stz.b LoadMenuPalUnselSel		;draw unselected palette
	
LoadMenuInitialOptionDrawLoop:	
	jsr LoadMenuSetupRowPointer
	jsr LoadMenuDrawDescString
	jsr LoadMenuDrawOptionValue

	sep #$20
	lda.b LoadMenuCurrentRow
	inc a
	sta.b LoadMenuCurrentRow
	cmp.b LoadMenuNumberOfRows
	bne LoadMenuInitialOptionDrawLoop

	stz.b LoadMenuCurrentRow		;start at row 1
	inc.b LoadMenuDoInit			;store "menu initialized"

	lda.b #$01
	sta.b LoadMenuPalUnselSel		;draw first row highlighted
	jsr LoadMenuSetupRowPointer
	jsr LoadMenuDrawDescString
	jsr LoadMenuDrawOptionValue


LoadMenuFileAlreadyInitialized:
	sep #$20
	jsr LoadMenuSetupRowPointer
	jsr LoadMenuDrawOptionValue
	rep #$31
	lda.w JoyPortBufferTrigger&$ffff
	ldx.w #$ffff				;clear number of loaded button-1

LoadMenuFileCheckNextButton:	
	inx
	cpx.w #$0010				;check if all buttons have been checked
	beq LoadMenuFileCheckNextButtonDone
	lsr a					;start at button bit 0
	bcc LoadMenuFileCheckNextButton
	
	pha
	phx
	txa
	clc
	asl a
	tax
	jsr (LoadMenuFileCheckNextButtonJumpTbl,x)
	
	plx
	pla
	bra LoadMenuFileCheckNextButton



LoadMenuFileCheckNextButtonDone:	



	plb					;exit if number of rows=0
	plp
	rts


LoadMenuFileCheckNextButtonJumpTbl:
	.dw LoadMenuButtonVoid
	.dw LoadMenuButtonVoid
	.dw LoadMenuButtonVoid
	.dw LoadMenuButtonVoid
	.dw LoadMenuButtonVoid			;r
	.dw LoadMenuButtonVoid			;l
	.dw LoadMenuButtonVoid			;x
	.dw LoadMenuExecuteSubroutine		;a
	.dw LoadMenuIncreaseOption		;right
	.dw LoadMenuDecreaseOption		;left
	.dw LoadMenuIncreaseRow			;down
	.dw LoadMenuDecreaseRow			;up
	.dw LoadMenuButtonVoid			;start
	.dw LoadMenuButtonVoid			;select
	.dw LoadMenuButtonVoid			;a
	.dw LoadMenuButtonVoid			;b

	.dw LoadMenuButtonVoid

LoadMenuButtonVoid:
	rts

LoadMenuIncreaseRow:
	php
	rep #$31
	sep #$20

	stz.b LoadMenuPalUnselSel		;draw current row unselected
	jsr LoadMenuSetupRowPointer
	jsr LoadMenuDrawDescString
	jsr LoadMenuDrawOptionValue

	lda.b LoadMenuCurrentRow
	inc a
	cmp.b LoadMenuNumberOfRows
	bcc LoadMenuIncreaseRowNoOverflow

	lda.b #$00				;wrap around to zero if maximum number of options is reached
LoadMenuIncreaseRowNoOverflow:
	sta.b LoadMenuCurrentRow
	lda.b #$01
	sta.b LoadMenuPalUnselSel		;draw current row
	jsr LoadMenuSetupRowPointer
	jsr LoadMenuDrawDescString
	jsr LoadMenuDrawOptionValue

	plp
	rts

LoadMenuDecreaseRow:
	php
	rep #$31
	sep #$20

	stz.b LoadMenuPalUnselSel		;draw current row unselected
	jsr LoadMenuSetupRowPointer
	jsr LoadMenuDrawDescString
	jsr LoadMenuDrawOptionValue

	lda.b LoadMenuCurrentRow
	dec a
	bpl LoadMenuIncreaseRowNoOverflow

	lda.b LoadMenuNumberOfRows				;wrap around to highest row if minimum number of options is reached
	dec a
LoadMenuDecreaseRowNoOverflow:
	sta.b LoadMenuCurrentRow
	lda.b #$01
	sta.b LoadMenuPalUnselSel		;draw current row
	jsr LoadMenuSetupRowPointer
	jsr LoadMenuDrawDescString
	jsr LoadMenuDrawOptionValue

	plp
	rts

LoadMenuIncreaseOption:
	phy
	php
	rep #$31
	sep #$20
	jsr LoadMenuSetupRowPointer
	ldy.w #$0000
	lda.b [MenuRowsThreeBytePointerLo],y	;do nothing if were processing a subroutine-only option
	bne LoadMenuIncreaseOptionNoEnd
	
	plp
	ply
	rts
	
LoadMenuIncreaseOptionNoEnd:
	ldy.w #$0000
	lda.b [MenuRowsThreeByteOptionPointerLo],y	;get byte, increase it and compare if its greater than the maximum value
	ldy.w #$0003
	cmp.b [MenuRowsThreeBytePointerLo],y
	bcc LoadMenuIncreaseOptionNoOverflow
	
	dey
	lda.b [MenuRowsThreeBytePointerLo],y	;get minimum value if wrap around
	dec a
			
LoadMenuIncreaseOptionNoOverflow:
	ldy.w #$0000
	inc a
	sta.b [MenuRowsThreeByteOptionPointerLo],y	;save back changed value

;	ldy.w #0
	lda.b [MenuRowsThreeBytePointerLo],y
	cmp.b #1
	bne LoadMenuIncNoByte				;only check for b if we're processing a byte-option

	lda.w JoyPortBufferTrigger&$ffff+1
	bpl LoadMenuIncNoByte

	clc
	lda.b [MenuRowsThreeByteOptionPointerLo],y	;save back changed value
	adc.b #$f					;add a whole nibble if b is pressed
	sta.b [MenuRowsThreeByteOptionPointerLo],y

LoadMenuIncNoByte:	
	lda.b #$01
	sta.b LoadMenuPalUnselSel		;redraw current option
	jsr LoadMenuDrawDescString
	jsr LoadMenuDrawOptionValue
	plp
	ply
	rts

LoadMenuDecreaseOption:
	phy
	php
	rep #$31
	sep #$20
	jsr LoadMenuSetupRowPointer
	ldy.w #$0000
	lda.b [MenuRowsThreeBytePointerLo],y	;do nothing if were processing a subroutine-only option
	bne LoadMenuDecreaseOptionNoEnd
	
	plp
	ply
	rts
	
LoadMenuDecreaseOptionNoEnd:

	
	ldy.w #$0000
;	lda.b [MenuRowsThreeByteOptionPointerLo],y	;get byte, increase it and compare if its greater than the maximum value

	lda.b [MenuRowsThreeBytePointerLo],y
	cmp.b #1
	bne LoadMenuDecNoByte				;only check for b if we're processing a byte-option

	lda.w JoyPortBufferTrigger&$ffff+1
	bpl LoadMenuDecNoByte

	sec
	lda.b [MenuRowsThreeByteOptionPointerLo],y	;save back changed value
	sbc.b #$f					;add a whole nibble if b is pressed
	sta.b [MenuRowsThreeByteOptionPointerLo],y

LoadMenuDecNoByte:	

	lda.b [MenuRowsThreeByteOptionPointerLo],y	;get byte, increase it and compare if its greater than the maximum value
	dec a
	
	cmp.b #$ff
	beq LoadMenuDecreaseOptionOverflow
	ldy.w #$0002
	cmp.b [MenuRowsThreeBytePointerLo],y
	bcs LoadMenuDecreaseOptionNoOverflow
LoadMenuDecreaseOptionOverflow:	
	ldy.w #$0003
	lda.b [MenuRowsThreeBytePointerLo],y	;get minimum value if wrap around
				
LoadMenuDecreaseOptionNoOverflow:
	ldy.w #$0000
	sta.b [MenuRowsThreeByteOptionPointerLo],y	;save back changed value
	lda.b #$01
	sta.b LoadMenuPalUnselSel		;redraw current option
	jsr LoadMenuDrawDescString
	jsr LoadMenuDrawOptionValue
	plp
	ply
	rts


LoadMenuExecuteSubroutine:
	pha
	phy
	phx
	php
	phd
	phb
	rep #$31
	sep #$20
	lda.b #$7e
	pha
	plb
	
	ldy.w #0007
	lda.b [MenuRowsThreeBytePointerLo],y	;get number of subroutine to execute
	clc
	asl a
	tax
	jsr (MenuSubroutineLUT,x)
	
	
	plb
	pld
	plp
	plx
	ply
	pla
	rts
	
	

	
	
MenuDataTypeVoid:
MenuDataType0:
	rts					;no action if theres no string to draw


MenuDataType1:
	phx
	phy
	php
	rep #$31
	lda.b LoadMenuInitialOptionOffset	;get relative offset of option
	and.w #$00ff
	adc.b LoadMenuStringPosLo		;add offset of current row
	sta.w LoadMenuStringBuffer&$ffff+1		;store in string position
	ldy.w #$0004
	lda.b [MenuRowsThreeBytePointerLo],y	;get pointer to byte
	sta.w LoadMenuStringBuffer&$ffff+6
	sep #$20
	iny
	iny
	lda.b [MenuRowsThreeBytePointerLo],y	;get bank pointer to byte
	sta.w LoadMenuStringBuffer&$ffff+8
	lda.b #$05				;write "draw byte" command"
	sta.w LoadMenuStringBuffer&$ffff+5
	ldx.w #2			;print to screen
	jsr LoadTextString
	plp
	ply
	plx
	rts

MenuDataType2:
	phx
	phy
	php
	rep #$31
	lda.b LoadMenuInitialOptionOffset	;get relative offset of option
	and.w #$00ff
	adc.b LoadMenuStringPosLo		;add offset of current row
	sta.w LoadMenuStringBuffer&$ffff+1		;store in string position
	ldy.w #$0004
	lda.b [MenuRowsThreeBytePointerLo],y	;get pointer to byte
	sta.w LoadMenuStringBuffer&$ffff+6
	sep #$20
	iny
	iny
	lda.b [MenuRowsThreeBytePointerLo],y	;get bank pointer to byte
	sta.w LoadMenuStringBuffer&$ffff+8
	lda.b #$06				;write "draw byte" command"
	sta.w LoadMenuStringBuffer&$ffff+5
	ldx.w #2			;print to screen
	jsr LoadTextString
	plp
	ply
	plx
	rts

MenuDataType3:
	phx
	phy
	php
	rep #$31
	lda.b LoadMenuInitialOptionOffset	;get relative offset of option
	and.w #$00ff
	adc.b LoadMenuStringPosLo		;add offset of current row
	sta.w LoadMenuStringBuffer&$ffff+1		;store in string position

	lda.w #$0000				;clear a
	sep #$20
	ldy.w #$0001
	lda.b [MenuRowsThreeBytePointerLo],y	;get bitmask
	sta.w LoadMenuStringBuffer&$ffff+6		;just a temporary buffer

	ldy.w #$0000
	lda.b [MenuRowsThreeByteOptionPointerLo],y
	and.w LoadMenuStringBuffer&$ffff+6		;check which string to choose
	rep #$31					;if string1, a=0
	and.w #$ff
;	beq MenuDataType3String1
	
;	lda.b #$02				;load textstring 2, two byte pointer
;MenuDataType3String1:
	asl a					;2byte pointer

	adc.w #0012				;add until we're at pointer for string 1 or 2
	tay
	lda.b [MenuRowsThreeBytePointerLo],y	;get relative string pointer
	clc
	adc.b MenuFileThreeBytePointerLo		;add file offset to get real pointer
	sta.w LoadMenuStringBuffer&$ffff+6
	sep #$20
	lda.b MenuFileThreeBytePointerBank
	sta.w LoadMenuStringBuffer&$ffff+8

	lda.b #$04				;write "draw string from adress" command"
	sta.w LoadMenuStringBuffer&$ffff+5
	ldx.w #2			;print to screen
	jsr LoadTextString
	plp
	ply
	plx
	rts



LoadMenuInitialOptionDrawDataTypeJumpTable:
	.dw MenuDataType0
	.dw MenuDataType1
	.dw MenuDataType2
	.dw MenuDataType3
	.dw MenuDataTypeVoid
	.dw MenuDataTypeVoid
	.dw MenuDataTypeVoid
	.dw MenuDataTypeVoid


LoadMenuDrawDescString:
	phy
	phx
	php
	rep #$31				;16bit and carry clear
	lda.w #$0000
	tax

LoadMenuDrawDescStringClearBufferLoop:	
	sta.w LoadMenuStringBuffer&$ffff,x
	inx
	inx
	cpx.w #$0008				;clear 16 bytes of string buffer
	bcc LoadMenuDrawDescStringClearBufferLoop
	
	
	lda.b LoadMenuCurrentRow
	and.w #$001f
	tax
	lda.w #$0000
	
LoadMenuDrawDescStringVertPosLoop:	
	cpx.w #$0000
	beq LoadMenuDrawDescStringVertPosLoopExit

	clc
	adc.b LoadMenuVerticalSpacing
	dex
	bra LoadMenuDrawDescStringVertPosLoop
LoadMenuDrawDescStringVertPosLoopExit:
	clc
	adc.b LoadMenuInitialOffset	;add offset of whole menu
	sta.b LoadMenuStringPosLo	;store in initial offset of current row
	sta.w LoadMenuStringBuffer&$ffff+1

	lda.b LoadMenuPalUnselSel
	and.b #$0001			;only get lowest bit to check if we should draw highlighted or not
	clc
	adc.w #0008			;get palette number 10=unselected, 11=selected
	tay
	sep #$20
	lda.b [MenuRowsThreeBytePointerLo],y
	sta.w LoadMenuStringBuffer&$ffff+4
	lda.b #$04			;script command "set string adress"
	sta.w LoadMenuStringBuffer&$ffff+5

	inc.w LoadMenuStringBuffer&$ffff	;script command "set position" for description text
	lda.b #$07			;script command "set palette"
	sta.w LoadMenuStringBuffer&$ffff+3
	
	rep #$31
	ldy.w #0010			;get string offset
	lda.b [MenuRowsThreeBytePointerLo],y
	adc.b MenuFileThreeBytePointerLo	;add file offset to get real pointer
	sta.w LoadMenuStringBuffer&$ffff+6
	sep #$20
	lda.b MenuFileThreeBytePointerBank
	sta.w LoadMenuStringBuffer&$ffff+8

	ldx.w #2			;print to screen
	jsr LoadTextString

	plp
	plx
	ply
	rts

LoadMenuSetupRowPointer:
	phy
	php
	rep #$31				;16bit and carry clear
	lda.b LoadMenuCurrentRow
	and.w #$001f
	asl a
	adc.w #$0005				;add offset of relative pointertable in menu file
	tay
	lda.b [MenuFileThreeBytePointerLo],y
	adc.b MenuFileThreeBytePointerLo		;add file offset to get real pointer
	sta.b MenuRowsThreeBytePointerLo
	sep #$20
	lda.b MenuFileThreeBytePointerBank
	sta.b MenuRowsThreeBytePointerBank

	ldy.w #$0006				;get option byte bank pointer
	lda.b [MenuRowsThreeBytePointerLo],y
	sta.b MenuRowsThreeByteOptionPointerBank

	rep #$30	
	ldy.w #$0004				;get option byte pointer
	lda.b [MenuRowsThreeBytePointerLo],y
	sta.b MenuRowsThreeByteOptionPointerLo
	sep #$20
/*
	ldy.w #$0009				;get options subroutine byte bank pointer
	lda.b [MenuRowsThreeBytePointerLo],y
	sta.b MenuRowsThreeByteCodePointerBank

	rep #$30	
	ldy.w #$0007				;get options subroutine byte pointer
	lda.b [MenuRowsThreeBytePointerLo],y
	sta.b MenuRowsThreeByteCodePointerLo
	sep #$20
*/
	
	plp
	ply
	rts
	
LoadMenuDrawOptionValue:
	phy
	phx
	php
	rep #$31
	ldy.w #0000			;get option data type
	tya
	sep #$20
	lda.b [MenuRowsThreeBytePointerLo],y
	and.b #$07			;maximum number of types=7
	asl
	tax
	jsr (LoadMenuInitialOptionDrawDataTypeJumpTable,x)
	plp
	plx
	ply
	rts
	
