/*
this is the sprite/oam routine file.

direct sprite features:
-setup sprite variables
-create and upload spritelist-buffer to oam-ram
-upload sprite palettes

object manager features:
-create and process object list
-create objects, each with own memory space
-delete objects
-routines for:
	-cycling through animations
	-moving objects along preset paths
	
areas needed:
OAMBuffer		ds 200
oam memory format:
Byte 1    xxxxxxxx    x: X coordinate
Byte 2    yyyyyyyy    y: Y coordinate
Byte 3    cccccccc    c: starting character (tile) number    p: palette number
Byte 4    vhoopppc    v: vertical flip   h: horizontal flip  o: priority bits
          Note: the 'c' in byte 4 is the MOST significant bit in the 9-bit char #.

00110000

OAMPriorityBuffer	ds 20
oam priority format:
2bits per sprite, 4 sprites per byte
bit0 = size toggle
bit1 = x coordinate msb

ObjectList		ds $400

Maximum number of objects: 64
Object Format: 32bytes per object
	.db %11101010		;0 object type designation
				;bit0=X position sign of sprite(usually 0)
				;bit1=Object size flag
				;bit2=collidable
				;bit3=subroutine? if set, this object has its own subroutine that must be executed every frame
				;bit4=animate? if set, this object is animated(a special table for each object specifies the exact animation with commands for tileloading, waiting, animation loop etc)
				;bit5=bg-bound? if set, this object must move in accordance with background layer 0
				;bit6=object active? if set, this object is active and needs to be drawn. if clear, this sprite doesnt need to be drawn, but must be processed, anyway
				;bit7=object present? if set, this slot has an object. if clear, its considered empty and can be overwritten
	.db %00000000		;1 object type designation 2
				;bit7=pseudo 3d sprite that needs to be moved according to z-value when background moves
				;bit6=don't upload tiles for this sprite. useful for stuff like particles and such where lots of sprites share the same tiles.
	.db 6			;2 number of subroutine. executed if bit6 of object type is set
	.db 14			;3 tileset to use
	.db 9			;4 current "frame" of tileset to display
	.db $cc			;5 starting tile in vram
	.db %00111101		;6 palette and config. vhoopppN; oo=priority. ppp=palette. N=nametable vh=flip
	.dw $0			;7 x position
	.dw 201			;9 y position
	.db 0			;11 current frame in animation list
	.db 0			;12 object command list to use	
	.db 0			;13 object offset in object list.
	.db 9			;14 palette number to upload for this sprite
	.dw OamTypeTextbox	;15 object number

	.db 0			;17 x-displacement
	.db 0			;18 y-displacement
	.db 4			;19 z-displacement
	.db 0			;20 z-value for pseudo-3d scenes
	.db 0			;21 collision subroutine
	.db 0			;22 animation repeat counter for nop, must not be set up
;23-25 left void for future expansion

;custom variables differing between sprites:
	.db 0			;26 spare variable
	.db $80			;27 target x-pos /npc walking distance
	.db 32			;28 target y-pos
	.db 0			;29 current x-pos, must not be set up
	.db 8			;30 current y-pos, must not be set up
	.db 0			;void
	.db 0			;void
	.db 0			;void
	.db 0			;void
	.db 0			;void
	.db 0			;void
	.db 0			;void
object command list format:
an entry is always 2 bytes long, mostly consisting of a command byte and a parameter
normally, one command from the list is executed per frame.
".." means no parameter.
if bit7 of command byte is set, execute the next command immediately.
this is used to execute multiple commands per frame.

control code:	function:
-00NN		create another object in object list. NN is the number of the object to create. searches for the next free slot in object list
-01..		deletes object from list(can only delete itself. if an object wants to delete another one, it has to do this via the subroutine)
-02XY		adds signed xy-vector to objects position.
-03TF		increment current tileset frame. TF=number of frames to advance
-04TF		set tileset frame. TF=tileset frame to set
-05PC		set palette and config PC=palette and config ;IMPORTANT!! if this is executed, the tile positions of an object bigger than 8x8 must be swapped!!
-06AF		goto frame in animation list. can be used to create infinite looping objects. AF=frame in animation list
-07CL		goto command list. always starts at position 0 in that command list and discards current command list. CL=command list to go to.
-08.b0		set/unset object screen-bound (with bit0 of parameter)
-09.b0		set/unset object subroutine enable (with bit0 of parameter)
-0aSS		set object subroutine. SS=subroutine number
-0bTS		set tileset. TS:number of tileset to use
-0cSE		play soundeffect
-0dFN		nop and wait for next frame(s). used for delays. FN specifies number of frames to wait. 0=wait one frame, 5=wait 6 frames etc
-0e..		inifite loop. object stays active
-0f..		terminate. used to end object list processing. marks object as inactive.
-10VS		set target speed vector. bits0-5: speed (0-2subpixel precision) bits 6,7: accel type (0=direct, 1=linear slow, 2=linear fast, 3=smooth). speed 0 disables vector speed calculation
-11VD		set target vector direction/angle. bits0-5: target direction ($00=up $10=right $20=down $30=left) bits 6,7: direction change type (0=direct, 1=linear slow, 2=linear fast, 3=smooth). clockwise/counterclockwise direction is determined by distance between current and target direction(alsways uses fastest/smallest angle)
-12NN		create object and put it to the coordinates of the calling object. NN=number of object to create

sprite palettes start at cgram entry 128


oam registers:
$2101 - global sprite size select
sprite tile location select: 0,4000,8000,c000
$2102 - oam adress
$2103 - oam table and priority rotation select
$2104 - oam data. this is a doublewrite/read register.

free space for sprites:
$4000 bytes at $c000
$20 bytes per 8x8 sprite = 512 tiles for sprites
first sprite tile must always be blank


size:	amount possible in terms of vram space:
8x8	512
16x16	128
32x32	32

for sprites bigger than 8x8, the vram tile data is seperated into horizontal rows, each 16 tiles (=512 bytes) apart.
they are always 16 tiles apart, no matter if the sprites are 16,32 or 64 pixels in size
heres a table what to upload where for different sized sprites:

xsize:	ysize:	number of lines to upload:	number of bytes to transfer per line:
8	8	1				32
16	16	2				64
32	32	4				128
64	64	8				256
16	32	4				64
32	64	8				128




initial setting for register $2101 gets loaded with
graphics config file loader "SetBGMode" and put into variable "ObjSel", updated every vblank

variables:
ObjectListPointer	dw		;pointer to current object in object list, used to set direct register
CurrentObjectNumber	db		;number of currently selected object in list
OamBufferPointer	dw		;pointer to current sprite in oam buffer

fixed variables:
.define ObjectFileSize	16		;size of one object file

routines in detail:
InitOam:
	-clears oam table buffer
	-clears object memory

CreateObject:
	-upload object file to first free slot in object list
	-upload current frame of tileset to vram
		-calculate transfer size by object size
		-calculate transfer source by object size*frame number
		-calculate transfer target by starting tile in vram
	-upload palette to vram according to palette number config
	-set object offset in object list variable
ObjectProcessor:
	-check object list for active object
		-write sprite(s) to oambuffer according to object file
		-process animation list if enabled
		-execute subroutine if enabled

*/

ObjectProcessSubroutine:
	rep #$31
	phx
	ldx.b ObjectListPointerCurrent
	lda.w ObjEntrySubRout,x
	and.w #$ff				;max number of routines: 256
	asl a
	tax

	php
	phd
	jsr (AniSubroutineJumpLUT,x)
	pld
	plp
	plx
	rts





ObjectProcessAniList:
	php
ObjectProcessAniListYes:
	rep #$31
;load pointer to current animation list:	
	lda.w ObjEntryAniList,x	;get number of list to use
	and.w #$ff
	asl a
	phx
	tax
;	sta.b TempBuffer
	lda.l (ObjectAnimationLUT+BaseAdress),x	;get relative pointer to object
	plx

	clc
	adc.w #ObjectAnimationLUT		;calculate real pointer
	sta.b ThreeBytePointerLo
	
	sep #$20
	lda.b #(:ObjectAnimationLUT+BaseAdress>>16)
	sta.b ThreeBytePointerBank

;get pointer to currently active frame:

	rep #$31
	lda.w ObjEntryAniFrame,x
	and.w #$ff
	asl a
	tay

;get command
	phx
	lda.b [ThreeBytePointerLo],y		;get current command
	sta.b OamAniListStraightRepeatFlag	;store for immediate repeat checking
	pha
	and.w #$1f				;mask off repeat flag, maximum number of commands: 32
	asl a
	tax

	pla
	phy
	php
	phd

/*
	rep #$31
	pha
	lda.b ObjectListPointerCurrent
	clc
	adc.w #ObjectList & $ffff
	tcd
	
	pla
*/	
	
	sep #$20
	lda.b #0		;clear high byte
	xba			;and put parameter into a
;parameter is in a,8bit
;direct register points to current object
;bank is $7e
	jsr (AniListJumpLUT,x)
	

	pld
	plp
	ply	
	plx
	sep #$20
	lda.b OamAniListStraightRepeatFlag
	bmi ObjectProcessAniListYes			;if bit7 is set, next command is executed immediately
	
	plp
	rts

AniListJumpLUT:
	.dw AniListCreateObj			;0
	.dw AniListDelete
	.dw AniListMove
	.dw AniListTileFrameInc
	.dw AniListTileFrameSet
	.dw AniListPalConfSet
	.dw AniListGotoAniFrame
	.dw AniListGotoAniList
	.dw AniListMakeBGBound
	.dw AniListSubroutineEnable
	.dw AniListSetSubroutine		;10
	.dw AniListTileSetSet
	.dw AniListPlaySoundEffect
	.dw AniListVoid
	.dw AniListInfiniteLoop
	.dw AniListEnd							;15
	.dw AniListSetVectorSpeed
	.dw AniListSetVectorDir
	.dw AniListCreateObjPosition

;does nothing, doesn't proceed to next animation command	
AniListInfiniteLoop:
	rts
	
AniListVoid:
	ldx.b ObjectListPointerCurrent	


	sta.w TempBuffer
	lda.w TempBuffer
	beq AniListVoidProceed			;if argument is 0, proceed to next frame directly


	lda.w ObjEntryAniCmdRepeat,x		;if argument is not zero, but command list repeat is, we just encountered this void command. load new wait value.
	bne AniListVoidDecrease			;if argument is not zero, we're on a repeated loop here. don't load new value, just decrease old one.
	
	lda.w TempBuffer
	sta.w ObjEntryAniCmdRepeat,x
	rts
	
;argument was already loaded, decrease
AniListVoidDecrease:
	dec a
	sta.w ObjEntryAniCmdRepeat,x
	beq AniListVoidProceed			;if repeat value just turned 0, we're done here, go to next command

	rts

AniListVoidProceed:
;	ldx.b ObjectListPointerCurrent
;	stz.w ObjEntryAniCmdRepeat,x		;not needed, but do it anyway just to be safe
	inc.w ObjEntryAniFrame,x
	rts
	
AniListCreateObj:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x
	jsr CreateObject
	rts

AniListCreateObjPosition:
	rep #$31
	tay																;save number of object to load
	ldx.b ObjectListPointerCurrent		;get position of calling object
	lda.w ObjEntryXPos,x							;save to stack
	adc.w #8*16												;move one tile to the left
	pha
	lda.w ObjEntryYPos,x
	clc
	adc.w #8*16												;move one tile to the bottom
	pha
	inc.w ObjEntryAniFrame,x
	tya
	jsr CreateObject
	ldx.b ObjectListPointerCurrent		;get position of newly-created object
	pla
	sta.w ObjEntryYPos,x							;update position
	pla
	sta.w ObjEntryXPos,x
	
	rts


AniListGotoAniFrame:
	ldx.b ObjectListPointerCurrent
	sta.w ObjEntryAniFrame,x			;store new animation frame
	rts

AniListDelete:
	ldx.b ObjectListPointerCurrent
	sta.w ObjEntryType,x			;clear object type info and present flag
	rts

AniListMove:	
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x
	pha
	and.b #$f0			;get x-add
	clc
	lsr a
	lsr a
	lsr a
	lsr a
	bit.b #$08			;check if negative
	beq AniListMoveXNotNeg
	
	and.b #$07	
	eor.b #$ff			;xor to substract value
	inc a
AniListMoveXNotNeg:	
	rep #$31
	asl a						;subpixel precision
	asl a
	asl a
	asl a
	adc.w ObjEntryXPos,x
	sta.w ObjEntryXPos,x
	sep #$20
	pla
	and.b #$0f			;get x-add
	bit.b #$08			;check if negative
	beq AniListMoveYNotNeg
	
	and.b #$07
	eor.b #$ff			;xor to substract value
	inc a
AniListMoveYNotNeg:	
	rep #$31
	and.w #$ff			;mask off high byte from x-position calculation
	asl a						;subpixel precision
	asl a
	asl a
	asl a
	
	adc.w ObjEntryYPos,x
	sta.w ObjEntryYPos,x
	sep #$20
	rts

AniListTileFrameInc:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x		;increment animation frame
	clc	
	adc.w ObjEntryTilesetFrame,x		;add increment number to tileset frame
	sta.w ObjEntryTilesetFrame,x		;store new tileset frame
	lda.w ObjEntryType,x
	bit.b #%01000000			;check if active. if inactive/offscreen, dont waste time uploading a new frame
	beq AniListTileFrameIncOffscreen

	jsr CreateObjUploadSpriteFrame
AniListTileFrameIncOffscreen:
	rts

AniListTileSetSet:
	ldx.b ObjectListPointerCurrent
	sta.w ObjEntryTileset,x			;set tileset frame
	inc.w ObjEntryAniFrame,x		;increment animation frame
	
	jsr CreateObjUploadSpriteFrame
	rts

AniListTileFrameSet:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x		;increment animation frame
	sta.w ObjEntryTilesetFrame,x		;set tileset frame
	jsr CreateObjUploadSpriteFrame
	rts

AniListPalConfSet:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x		;increment animation frame
	sta.w ObjEntryPalConf,x		;set palette and priority config
	rts

AniListGotoAniList:
	ldx.b ObjectListPointerCurrent
	sta.w ObjEntryAniList,x		;set animation list	
	stz.w ObjEntryAniFrame,x		;reset animation frame
	stz.b TempBuffer+10			;clear animation-command repeat flag
	rts

AniListMakeBGBound:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x		;increment animation frame
	and.b #1				;get bit 0
	clc
	asl a					;move to correct location
	asl a
	asl a
	asl a
	asl a
	sta.b TempBuffer
	lda.w ObjEntryType,x		;get object type byte
	and.b #$DF				;mask off bg bound flag
	ora.b TempBuffer			;set flag according to command parameter
	sta.w ObjEntryType,x		;get object type byte
	rts

AniListSubroutineEnable:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x		;increment animation frame
	and.b #1				;get bit 0
	clc
	asl a					;move to correct location
	asl a
	asl a
	sta.b TempBuffer
	lda.w ObjEntryType,x		;get object type byte
	and.b #$F7				;mask off bg bound flag
	ora.b TempBuffer			;set flag according to command parameter
	sta.w ObjEntryType,x		;get object type byte
	rts

AniListSetSubroutine:
	ldx.b ObjectListPointerCurrent	
	inc.w ObjEntryAniFrame,x		;increment animation frame
	sta.w ObjEntrySubRout,x		;set new subroutine
	rts
	
AniListPlaySoundEffect:	
	rep #$31
	ldx.b ObjectListPointerCurrent	
	inc.w ObjEntryAniFrame,x		;increment animation frame	
	and.w #$ff
	jsr SpcPlaySoundEffectSimple
	rts


AniListEnd:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x		;increment animation frame
	lda.w ObjEntryType,x		;
	and.b #$ef				;mask off animate flag
	sta.w ObjEntryType,x		;
	rts

AniListSetVectorSpeed:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x		;increment animation frame
	sta.w ObjEntryVectorTarSpeed,x
	and.b #%111111
	cmp.b #$3f
	bne AniListSetVectorSpeedNoOver

	dec.w ObjEntryVectorTarSpeed,x

AniListSetVectorSpeedNoOver:	
	lda.w ObjEntryVectorSpeed,x
	and.b #%1111111			;clear "target met" flag
	sta.w ObjEntryVectorSpeed,x
	rts

AniListSetVectorDir:
	ldx.b ObjectListPointerCurrent
	inc.w ObjEntryAniFrame,x		;increment animation frame
	sta.w ObjEntryVectorTarDir,x
	and.b #%111111
	sta.b TempBuffer
	lda.w ObjEntryVectorDir,x
	and.b #%111111
	sta.b TempBuffer+1				;34
	cmp.b TempBuffer				;28
	beq AniListSetVectorDirEqual
	bcs AniListSetVectorDirCurrBigger

	lda.b TempBuffer
	sec
	sbc.b TempBuffer+1
	cmp.b #32						;check if that distance is bigger than 180 degrees
	bcs AniListSetVectorDirTargetCounterClock
	bra AniListSetVectorDirTargetClockwise

AniListSetVectorDirCurrBigger:
	sec								;substract current from target to get distance between them
	sbc.b TempBuffer				;c
	cmp.b #32						;check if that distance is bigger than 180 degrees
	bcc AniListSetVectorDirTargetCounterClock

AniListSetVectorDirTargetClockwise:
	lda.w ObjEntryVectorDir,x
	and.b #%00111111				;clear "target met"
	ora.b #%01000000				;set "clockwise"-flag
	sta.w ObjEntryVectorDir,x
	rts


AniListSetVectorDirTargetCounterClock:
	lda.w ObjEntryVectorDir,x
	and.b #%00111111				;clear "target met" and turn direction flag(counter clockwise)
	sta.w ObjEntryVectorDir,x
	rts


AniListSetVectorDirEqual:
	lda.w ObjEntryVectorDir,x
	ora.b #$80
	sta.w ObjEntryVectorDir,x
	rts

;in: x,16bit:relative object pointer
;out: x,16bit:relative pointer of next active object
;if carry is set on return, all objects have been processed
CheckActiveObject:
;	rep #$31
CheckActiveObjectLoop:
	lda.l (ObjEntryType + $7e0000-1),x	;get object type byte (get -1 so that the bmi check still works in 16bit mode)
;	xba
;	nop
	bmi CheckActiveObjectDone	;if a not-present object is found, end list processing


	txa
	clc
	adc.w #ObjectFileSize			;increment slot pointer
	tax
	cpx.w #ObjectFileSize*63		;check if maximum number of objects reached, then step out(maybe spit out an error if needed)
	bne CheckActiveObjectLoop

CheckActiveObjectListDone:
;	sep #$20
	sec
	rts


;	bra CheckActiveObjectListDone	;this is to prevent a lockup if the list is full
;	beq CheckActiveObjectListDone	;this is to prevent a lockup if the list is full
	
;	bra CheckActiveObjectLoop
	
CheckActiveObjectDone:
;	rep #$31
	stx.b ObjectListPointerCurrent		;store currently found object so that routines can check this back later
;	sep #$20
	clc
	rts



ObjectProcessor:
	php
	sep #$20
	phb
	lda.b #$7e
	pha
	plb
	lda.b NMIOamUploadFlag		;don't process list if last frame wasn't uploaded first. needed to prevent sprite flicker due to dma'ing incomplete oam list(irq firing during oam processor)
	bne ObjectProcessorExit
	rep #$31
	stz.w ColObjListPointer		;clear pointer in collision object list
;	sep #$20
;check for active objects
	jsr ClearOamBufferPart		;only clear the part of the buffer that was actually written to last frame
	jsr ClearOamPriorityBuffer
;	jsr ClearPriorityZBuffer
;	jsr ClearColObjList
	ldx.w #0			;start at object 0

ObjectProcessorLoop:
	jsr CheckActiveObject		;check for active object
	bcc ObjectProcessorNoExit	;if carry is set, all objects were processed
	
	jsr ZSortToOamBuffer
;	rep #$31
	stz.b ObjectListPointerCurrent
	inc.b NMIOamUploadFlag				;initiate oam buffer upload
ObjectProcessorExit:
	plb
	plp
	rts

ObjectProcessorNoExit:	

;check if sprite needs to be drawn on screen:
	lda.w ObjEntryType,x	;get object type byte
;	bit.b #%01000000
;	beq ObjProcInactive		;skip the oam write if bit isnt set
	
	bit.w #%00100000		;check if sprite is screenbound. if it is, dont check if the sprite is offscreen.
	bne ObjProcDontCheckOnscreen

ObjectProcessorCheckOnscreen:
;	rep #$31
	lda.w ObjEntryXPos,x					;remove precision
	lsr a
	lsr a
	lsr a
	lsr a
	sta.w TempBufferTest

	lda.w ObjEntryYPos,x					;remove precision
	lsr a
	lsr a
	lsr a
	lsr a
	sta.w TempBufferTest+2
	
	lda.w ObjEntryXDisplacement,x	
	and.w #$ff
	eor.w #$ffff

;	adc.w ObjEntryXPos,x		;check if sprite is left of screen
	clc
	adc.w TempBufferTest
	clc
	adc.w #32
	cmp.b ScreenPixelPositionX
	bcc ObjProcInactive
	
	lda.w ObjEntryXDisplacement,x	
	and.w #$ff
	clc	
	adc.b ScreenPixelPositionX
	
	clc
	adc.w #(TileBufferSizeX+1)*8	;check if sprite is right of screen
;	cmp.w ObjEntryXPos,x
	cmp.w TempBufferTest
	bcc ObjProcInactive

	lda.w ObjEntryYDisplacement,x
	and.w #$ff
	eor.w #$ffff
	clc
;	adc.w ObjEntryYPos,x		;check if sprite is on top of screen
	adc.w TempBufferTest+2
	clc
	adc.w #32
	cmp.b ScreenPixelPositionY
	bcc ObjProcInactive

	lda.w ObjEntryYDisplacement,x
	and.w #$ff
	clc
	adc.b ScreenPixelPositionY
	clc
	adc.w #TileBufferSizeX*8	;check if sprite is below screen
;	cmp.w ObjEntryYPos,x
	cmp.w TempBufferTest+2
	bcc ObjProcInactive


ObjProcDontCheckOnscreen:
	jsr ObjectSingleSpriteToOam

;	sep #$20
	lda.w ObjEntryType,x	;get object type byte
	ora.w #%01000000
	sta.w ObjEntryType,x	;set active flag


	bra ObjProcActive
ObjProcInactive:
;	sep #$20
	lda.w ObjEntryType,x	;get object type byte
	and.w #%10111111
	sta.w ObjEntryType,x	;clear active flag

ObjProcActive:
;	sep #$20
	lda.w ObjEntryType,x	;get object type byte
	phx
	bit.w #%00010000
	beq ObjProcNoAnim		;skip the oam write if bit isnt set
	
	jsr ObjectProcessAniList

ObjProcNoAnim:
	plx
	stx.b ObjectListPointerCurrent	;must be saved cause ani list routines that create new sprites might rewrite it
	phx
;	sep #$20
	lda.w ObjEntryType,x		;get object type byte
	bit.w #%00001000
	beq ObjProcNoSub		;skip the oam write if bit isnt set
	
	jsr ObjectProcessSubroutine

ObjProcNoSub:
;	sep #$20
	plx
	stx.b ObjectListPointerCurrent	;must be saved cause ani list routines that create new sprites might rewrite it
	phx
	lda.w ObjEntryType,x		;get object type byte
	bit.w #%00000100
	beq ObjProcNoColl		;skip the oam write if bit isnt set
	
	jsr CreateColListEntry

ObjProcNoColl:
	plx
	stx.b ObjectListPointerCurrent	;must be saved cause ani list routines that create new sprites might rewrite it
/*
	lda.w ObjEntryVectorTarSpeed,x		;get vector target speed
	and.w #%111111					;mask off target bit
	beq ObjProcNoVectorMove			;skip vector move if vector speed is 0

	jsr ObjVectorMoveHandler

ObjProcNoVectorMove:
*/
;check if object is of gravity decaying type
	lda.w ObjEntryType2,x	;get object type byte
	
	bit.w #%10000		;check if sprite is screenbound. if it is, dont check if the sprite is offscreen.
	beq ObjProcNoGravity

	jsr ObjProcUpdateGravity



ObjProcNoGravity:
;	rep #$31
	txa
	clc
	adc.w #ObjectFileSize
	tax
	stx.b ObjectListPointerCurrent			;relative pointer to current object in objectlist

	jmp ObjectProcessorLoop	
	
	plp
	rts

;upload collidable object information to a special list
CreateColListEntry:
	php
	txy
	ldx ColObjListPointer		;get pointer to currently active slot in col obj list
	sep #$20
;	lda.b #$80			;load col-obj present flag
	lda.w ObjEntryColliSub,y	;get subroutine number and
	ora.b #$80			;set enable flag
	xba
	lda.w ObjEntryListOffset,y	;load obj number in object list
	rep #$31
	sta.l ColObjList,x		;store in obj col list

	lda.w ObjEntryXPos,y		;get x pos of obj and divide by 16
	
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a
	
	inx
	inx
	sta.l ColObjList,x		;store in obj col list

	lda.w ObjEntryYPos,y		;get y pos of obj and divide by 16
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a	
	inx
	inx
	sta.l ColObjList,x		;store in obj col list
	inx
	inx
	stx ColObjListPointer		;update pointer to next entry in collision list

	tyx
	plp
	rts


;in: 	a, 8bit: number of object to load
;	x, 16bit: target low: x, high:y-position/8
;in	y, 8bit: target z-position
;uses TempBuffer0,1. maybe convert to use own temp area for better flexibility?
CreateObjectPosition:
	php
	
	rep #$31
	stx.b CollisionPixelX
	sty.b CollisionPixelY
;	sty.b CollisionPixelY
	and.w #$ff			;2byte pointer
	asl a
	tax				;put into x

	phd
	lda.w #0
	tcd

	sep #$20
	phb
	lda.b #$7e
	pha
	plb
	rep #$31

	lda.l (ObjectLUT+BaseAdress),x	;get relative pointer to object
	clc
	adc.w #ObjectLUT		;calculate real pointer
	sta.b ThreeBytePointerLo
	
	sep #$20
	lda.b #(:ObjectLUT+BaseAdress>>16)
	sta.b ThreeBytePointerBank

;check for free object space in object list:	
;	rep #$31
	ldx.w #0

CreateObjectPositionCheckFreeSpaceLoop:
	lda.w ObjEntryType,x	;get object type byte
	bpl CreateObjectPositionCheckFreeSpaceDone	;if free, step out

	rep #$31
	txa
	adc.w #ObjectFileSize			;increment slot pointer
	tax
	sep #$20
	cpx.w #ObjectFileSize*63		;check if maximum number of objects reached, then step out(maybe spit out an error if needed)
	beq CreateObjectPositionCheckFreeSpaceDone	;this is to prevent a lockup if the list is full
	
	bra CreateObjectPositionCheckFreeSpaceLoop
	
CreateObjectPositionCheckFreeSpaceDone:
	rep #$31
	stx.b ObjectListPointerCurrent			;relative pointer to current object in objectlist
	txa
	clc
	lsr a
	lsr a
	lsr a
	lsr a					;divide objectlist pointer by 16 to get current object number
	sep #$20
	sta.b CurrentObjectNumber

	rep #$31
	ldy.w #0
	
CreateObjectPositionUploadLoop:
	lda.b [ThreeBytePointerLo],y		;get first byte of object file in rom
	sta.w ObjEntryType,x
	
	inx
	inx
	iny
	iny
	cpy.w #(ObjectFileSize&$fffe)		;check if object file was uploaded, word-aligned
	bne CreateObjectPositionUploadLoop

	ldx.b ObjectListPointerCurrent

	lda.w ObjEntryType,x	;get object type byte
	bit.w #%100000		;check if screenbound
	beq CreateObjectPositionNotScreenBound

	lda.w ObjEntryType2-1,x	;pseudo-3d object?
	bpl CreateObjectPositionNot3D

	lda.b CollisionPixelX				;plot directly to loaded xy position if screenbound
	and.w #$ff
	clc
	asl a						;multiply with 16, 3d high-precision value
	asl a
	asl a
	asl a
;	asl a
;	asl a
	
	asl a
	asl a
	asl a
	sta.w ObjEntryXPos,x
	lda.b CollisionPixelX
	and.w #$ff00
	xba
	clc
	asl a						;multiply with 16, 3d high-precision value
	asl a
	asl a
	asl a
;	asl a
;	asl a	

	asl a
	asl a
	asl a	
	sta.w ObjEntryYPos,x

	lda.b CollisionPixelY				;store z-value
	sep #$20
	sta.w ObjEntryZDisplacement,x
	rep #$31
	bra CreateObjectPositionNotScreenBoundSkip



CreateObjectPositionNot3D:	
	lda.b CollisionPixelX				;plot directly to loaded xy position if screenbound
	and.w #$ff
	clc
	asl a						;multiply by 8
	asl a
	asl a
	
	asl a												;add subpixel precision
	asl a
	asl a
	asl a
	
	sta.w ObjEntryXPos,x
	lda.b CollisionPixelX
	xba
	and.w #$ff
	clc
	asl a						;multiply by 8
	asl a
	asl a

	asl a												;add subpixel precision
	asl a
	asl a
	asl a

	sta.w ObjEntryYPos,x


	bra CreateObjectPositionNotScreenBoundSkip

CreateObjectPositionNotScreenBound:
	clc
	lda.b CollisionPixelX
	and.w #$ff
	asl a					;multiply by 8
	asl a
	asl a

	asl a												;add subpixel precision
	asl a
	asl a
	asl a
		
	sta.w ObjEntryXPos,x
	clc
	lda.b CollisionPixelX
	and.w #$ff00
	xba 
	asl a					;multiply by 8
	asl a
	asl a

	asl a												;add subpixel precision
	asl a
	asl a
	asl a	
	sta.w ObjEntryYPos,x

CreateObjectPositionNotScreenBoundSkip:

;upload object number:
;	ldx.b ObjectListPointer
;	clc
;	adc.w #(ObjectList & $ffff)
;	clc
;	adc.w #13				
;	tax
	lda.w #0
	tay
;	tax
;	inx
	sep #$20
	lda.b CurrentObjectNumber
	sta.w ObjEntryListOffset,x	;store in the object number offset in an object file

	jsr CreateObjUploadSpriteFrame

	jsr CreateObjUploadPalette

;the animation list has to be executed at once in case objects create additional objects
	ldx.b ObjectListPointerCurrent
;	stx.b ObjectListPointerCurrent
	phx
	lda.w ObjEntryType,x	;get object type byte
	bit.b #%00010000
	beq ObjLoaderPositionNoAnim		;skip the oam write if bit isnt set
	
	jsr ObjectProcessAniList

ObjLoaderPositionNoAnim:

	lda.w ObjEntryType2,x		;increase max obj number if gravity particle present
	bit.b #%10000
	beq ObjLoaderPositionNoGravObj		;skip the oam write if bit isnt set
	
	jsr ObjectProcessGravObjInc

ObjLoaderPositionNoGravObj:



	plx
	stx.b ObjectListPointerCurrent	;must be saved cause ani list routines that create new sprites might rewrite it

	sep #$20

	lda.w ObjEntryType,x	;get object type byte
	bit.b #%00001000
	beq ObjLoaderPositionNoSub		;skip the oam write if bit isnt set
	
	jsr ObjectProcessSubroutine

ObjLoaderPositionNoSub:

	plb
	pld
	plp
	rts


;upload palette:
CreateObjUploadPalette:
	lda.w ObjEntryType2,x
	bit.b #%00100000					;don't upload palette if corresponding flag is set
	bne CreateObjUploadPaletteCancel
	
	rep #$31
	lda.w ObjEntryPalConf,x	;get palette config
	sta.b TempBuffer
	lda.w ObjEntryPalNumber,x	;get palette number
	and.w #$ff			;2byte pointer
	asl a				;2byte length
	asl a
	tax				;put into x
	lda.l (SpritePaletteLUT+BaseAdress),x	;get relative pointer to object
	clc
	adc.w #SpritePaletteLUT		;calculate real pointer
	sta.b ThreeBytePointerLo
	
	lda.l (SpritePaletteLUT+BaseAdress+2),x	;get length
	and.w #$fe														;mask off bit0. palettes always have word entries. prevents uploader from crashing if invalid palette is being loaded
	cmp.w #$20
	bcs UploadObjPalNoOverflow

	lda.w #$20								;never load palettes bigger than 32 bytes
UploadObjPalNoOverflow:


	sta.b TempBuffer+1
	
	lda.w #0
	sep #$20
	lda.b #(:SpritePaletteLUT+BaseAdress>>16)
	sta.b ThreeBytePointerBank

	rep #$31
	lda.b TempBuffer
	and.w #$E				;only get three palette bits
	clc
	lsr a					;rotate right to get palette number
	clc
	adc.w #8				;add sprite palette offset

	
	asl a					;multiply with 16 to get real start adress in palette buffer
	asl a
	asl a
	asl a
	asl a
	
	tax
	
	ldy.w #0
CreateObjectPositionUploadPaletteLoop:
	lda.b [ThreeBytePointerLo],y		;get first byte of object file in rom
	sta.w PaletteBuffer & $ffff,x
	
	inx
	inx
	iny
	iny
	cpy.b TempBuffer+1		;check if object file was uploaded, word-aligned
	bne CreateObjectPositionUploadPaletteLoop

	sep #$20
	inc.b NMIPaletteUploadFlag

CreateObjUploadPaletteCancel:	
	rts

;in: a, 8bit: number of object to load
CreateObject:
	php
	
	rep #$31
	and.w #$ff			;2byte pointer
	sta.b TempBuffer
	asl a
	tax				;put into x

	phd
	lda.w #0
	tcd
	



	sep #$20
	phb
	lda.b #$7e
	pha
	plb
	rep #$31

	lda.l (ObjectLUT+BaseAdress),x	;get relative pointer to object
	clc
	adc.w #ObjectLUT		;calculate real pointer
	sta.b ThreeBytePointerLo
	
	sep #$20
	lda.b #(:ObjectLUT+BaseAdress>>16)
	sta.b ThreeBytePointerBank

;check for free object space in object list:	
;	rep #$31
	ldx.w #0

CreateObjectCheckFreeSpaceLoop:
	lda.w ObjEntryType,x	;get object type byte
	bpl CreateObjectCheckFreeSpaceDone	;if free, step out

	rep #$31
	txa
	adc.w #ObjectFileSize			;increment slot pointer
	tax
	sep #$20
	cpx.w #ObjectFileSize*63		;check if maximum number of objects reached, then step out(maybe spit out an error if needed)
	beq CreateObjectCheckFreeSpaceDone	;this is to prevent a lockup if the list is full
	
	bra CreateObjectCheckFreeSpaceLoop
	
CreateObjectCheckFreeSpaceDone:
	rep #$31
	stx.b ObjectListPointerCurrent			;relative pointer to current object in objectlist
	txa
	clc
	lsr a
	lsr a
	lsr a
	lsr a					;divide objectlist pointer by 16 to get current object number
	sep #$20
	sta.b CurrentObjectNumber

	rep #$31
	ldy.w #0
CreateObjectUploadLoop:
	lda.b [ThreeBytePointerLo],y		;get first byte of object file in rom
	sta.w ObjEntryType,x
	
	inx
	inx
	iny
	iny
	cpy.w #(ObjectFileSize&$fffe)		;check if object file was uploaded, word-aligned
	bne CreateObjectUploadLoop
	
;upload object number:
	ldx.b ObjectListPointerCurrent
;	clc
;	adc.w #(ObjectList & $ffff)
;	clc
;	adc.w #13				
;	tax
	lda.w #0
	tay
;	tax
;	inx
;	lda.b TempBuffer			;store number of object so other routines can clearly identify it.
;	sta.w ObjEntryObjectNumber,x
	sep #$20
	lda.b CurrentObjectNumber
	sta.w ObjEntryListOffset,x	;store in the object number offset in an object file

	jsr CreateObjUploadSpriteFrame

/*
;upload palette:
	rep #$31
	lda.w ObjEntryPalConf,x	;get palette config
	sta.b TempBuffer
	lda.w ObjEntryPalNumber,x	;get palette number
	and.w #$ff			;2byte pointer
	asl a				;2byte length
	asl a
	tax				;put into x
	lda.l (SpritePaletteLUT+BaseAdress),x	;get relative pointer to object
	clc
	adc.w #SpritePaletteLUT		;calculate real pointer
	sta.b ThreeBytePointerLo
	
	lda.l (SpritePaletteLUT+BaseAdress+2),x	;get length
	and.w #$fe				;word-align, no longer than 256 bytes
	sta.b TempBuffer+1
	
	lda.w #0
	sep #$20
	lda.b #(:SpritePaletteLUT+BaseAdress>>16)
	sta.b ThreeBytePointerBank

	rep #$31
	lda.b TempBuffer
	and.w #$E				;only get three palette bits
	clc
	lsr a					;rotate right to get palette number
	clc
	adc.w #8				;add sprite palette offset

	
	asl a					;multiply with 16 to get real start adress in palette buffer
	asl a
	asl a
	asl a
	asl a
	
	tax
	
	ldy.w #0
CreateObjectUploadPaletteLoop:
	lda.b [ThreeBytePointerLo],y		;get first byte of object file in rom
	sta.w PaletteBuffer & $ffff,x
	
	inx
	inx
	iny
	iny
	cpy.b TempBuffer+1		;check if object file was uploaded, word-aligned
	bne CreateObjectUploadPaletteLoop

	sep #$20
	inc.b NMIPaletteUploadFlag
*/
	jsr CreateObjUploadPalette
	
	ldx.b ObjectListPointerCurrent
;	stx.b ObjectListPointerCurrent
;	sep #$20
	lda.w ObjEntryType,x	;get object type byte
	bit.b #%00010000
	beq ObjLoaderNoAnim		;skip the oam write if bit isnt set
	
	jsr ObjectProcessAniList

ObjLoaderNoAnim:
	lda.w ObjEntryType2,x		;increase max obj number if gravity particle present
	bit.b #%10000
	beq ObjLoaderPositionNoGravObj2		;skip the oam write if bit isnt set
	
	jsr ObjectProcessGravObjInc

ObjLoaderPositionNoGravObj2:

	sep #$20

	lda.w ObjEntryType,x	;get object type byte
	bit.b #%00001000
	beq ObjLoaderNoSub		;skip the oam write if bit isnt set
	
	jsr ObjectProcessSubroutine

ObjLoaderNoSub:
	rep #$30
;	stz.b ObjectListPointerCurrent

	plb
	pld
	plp
	rts

;upload current frame to vram
;get sprite config and lookup transfer type
CreateObjUploadSpriteFrame:
	php
	sep #$20
	lda.w ObjEntryType2,x
	bit.b #%01000000
	beq CreateObjUploadSpriteFrameDontCancel

;don't upload any sprite tiles if bit6 of object type 2 is set. this is used for the music notes and stuff like that where many sprites share the same tiles in order to minimize dma overhead.
	plp
	rts

CreateObjUploadSpriteFrameDontCancel:	
	rep #$31
	lda.w ObjEntryType,x	;get sprite size bit
	and.w #%10
	clc
	asl a				;put into bit3
	asl a
	sta.b TempBuffer
	
	
	lda.b ObjSel
	and.w #%11100000
	clc
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a
	ora.b TempBuffer		;add sprite size bit
	sta.b TempBuffer
	asl a				;multiply by 3 to get pointer
	clc
	adc.b TempBuffer
	phx

	tax				;put into x
	lda.l (ObjSizeLUT+BaseAdress),x	;get transfer length and number of bytes to transfer for this sprite
	sta.b TempBuffer+8		;+8 is length, +9 is transfer number
	lda.l (ObjSizeLUT+BaseAdress+2),x	;get transfer type
	sta.b TempBuffer+14		;+14 is type
	
	plx
;	rep #$31


;	lda.w #80
;	sta.b TempBuffer		;this is the number of tiles of one frame

;	stz.b TempBuffer+1
;	sep #$20
	clc
	lda.b TempBuffer+8		; length of one transfer and divide by 32 to get number of h-tiles
	and.w #$ff
	inc a				;increment once to get real length
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a
	sta.b TempBuffer

	lda.b TempBuffer+9		; transfer number 16bit
	and.w #$ff
	sta.b TempBuffer+2
	lda.w #0	
	tay

CreateObjCalcSizeLoop3:
	cpy.b TempBuffer+2
	beq CreateObjCalcSizeDone3

	iny
	adc.b TempBuffer
	bra CreateObjCalcSizeLoop3

CreateObjCalcSizeDone3:
	sta.b TempBuffer		;store total number of tiles

	lda.w ObjEntryTilesetFrame,x	;get current tileset frame
	and.w #$ff
	sta.b TempBuffer+2
	lda.w #0	
	tay

CreateObjCalcSizeLoop2:
	cpy.b TempBuffer+2
	beq CreateObjCalcSizeDone2

	iny
	adc.b TempBuffer
	bra CreateObjCalcSizeLoop2

CreateObjCalcSizeDone2:
	sta.b TempBuffer+2				;this is the first tile to be uploaded 

	lda.w ObjEntryTileset,x	;get tileset number
	and.w #$ff
	sta.b TempBuffer+4				;multiply with 3
	asl a
	clc
	adc.b TempBuffer+4
	
	txy
	tax
	lda.l (SpriteTilesetLUT+BaseAdress),x
	sta.b TempBuffer+4
	lda.l (SpriteTilesetLUT+1+BaseAdress),x
	tyx
;	and.w #$ff
	sta.b TempBuffer+5
	
	lda.b TempBuffer+2				;get first tile and multiply with 32 to get actual byte offset
	clc
	asl a
	asl a
	asl a
	asl a
	asl a
	clc
	adc.b TempBuffer+4
	bcc CreateObjCalcSourceNoWrap
	
	inc.b TempBuffer+6			;increment bank

CreateObjCalcSourceNoWrap:	
	sta.b TempBuffer+4			;store transfer source offset
	sep #$20
	lda.w ObjEntryPalConf,x	;get nametable bit(msb of sprites first tile)
	and.b #1
	xba
	lda.w ObjEntryVramTile,x	;get target tile number
;	and.w #$ff
	rep #$31
;	clc
	asl a						;multiply by 16(bytesize is 32, but vram is word-aligned)
	asl a
	asl a
	asl a
	clc
	adc.b SpriteTileOffsetVram			;add initial sprite offset
	sta.b TempBuffer+10				;this is the vram target
	phx						;push object list pointer
	ldx.b DmaFifoPointer

	lda.b TempBuffer+8				;get transfer length and increase by 1 to get real length 16bit
	and.w #$ff
	inc a			
	sta.b TempBuffer+12

CreateObjUploadTilesLoop:
	rep #$31
	lda.b TempBuffer+14					;transfer type
	sta.l DmaFifoEntryType,x

;store transfer target
	lda.b TempBuffer+10
	sta.l DmaFifoEntryTarget,x		;vram target 2116

;store transfer source
	lda.b TempBuffer+4
	sta.l DmaFifoEntrySrcLo,x		;source 4202
	lda.b TempBuffer+5
	sta.l DmaFifoEntrySrcHi,x		;source 4203

;store transfer length
	lda.b TempBuffer+12			;get transfer length. not used except for 8x8 sprites, where it is 32 bytes
	sta.l DmaFifoEntryCount,x		;length 4205
/*
;update vram target pointer:
	lda.b TempBuffer+10
	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBuffer+10
*/	
;update fifo buffer target:
	txa						;update fifo entry pointer
	clc
	adc.w #DmaFifoEntryLength

/*
	tax
	
;update source pointer
	lda.b TempBuffer+4
	clc
	adc.b TempBuffer+12
	sta.b TempBuffer+4

	sep #$20
	dec.b TempBuffer+9				;decrement number of transfers
	bne CreateObjUploadTilesLoop
	
;sprite tile transfer done
	rep #$31	
	txa						;update fifo entry pointer
;	clc
;	adc.w #DmaFifoEntryLength
*/
	sta.b DmaFifoPointer
	plx
	plp						;fetch object list pointer
	rts


/*
;old version which created multiple transfers in fifo buffer
CreateObjUploadSpriteFrame:
CreateObjLookupSpriteSize:
	php
	rep #$31
	lda.w ObjEntryType,x	;get sprite size bit
	and.w #%10
	clc
	asl a				;put into bit3
	asl a
	sta.b TempBuffer
	
	
	lda.b ObjSel
	and.w #%11100000
	clc
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a
	ora.b TempBuffer		;add sprite size bit
	asl a				;multiply by 2 to get pointer
	phx

	tax				;put into x
	lda.l (ObjSizeLUT+BaseAdress),x	;get transfer length and number of bytes to transfer for this sprite
	plx
	sta.b TempBuffer+8		;+8 is length, +9 is transfer number
;	rep #$31


;	lda.w #80
;	sta.b TempBuffer		;this is the number of tiles of one frame

;	stz.b TempBuffer+1
;	sep #$20
	clc
	lda.b TempBuffer+8		; length of one transfer and divide by 32 to get number of h-tiles
	and.w #$ff
	inc a				;increment once to get real length
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a
	sta.b TempBuffer

	lda.b TempBuffer+9		; transfer number 16bit
	and.w #$ff
	sta.b TempBuffer+2
	lda.w #0	
	tay

CreateObjCalcSizeLoop3:
	cpy.b TempBuffer+2
	beq CreateObjCalcSizeDone3

	iny
	adc.b TempBuffer
	bra CreateObjCalcSizeLoop3

CreateObjCalcSizeDone3:
	sta.b TempBuffer		;store total number of tiles

	lda.w ObjEntryTilesetFrame,x	;get current tileset frame
	and.w #$ff
	sta.b TempBuffer+2
	lda.w #0	
	tay

;this must be reprogrammed, too:
CreateObjCalcSizeLoop2:
	cpy.b TempBuffer+2
	beq CreateObjCalcSizeDone2

	iny
	adc.b TempBuffer
	bra CreateObjCalcSizeLoop2

CreateObjCalcSizeDone2:
	sta.b TempBuffer+2				;this is the first tile to be uploaded 

	lda.w ObjEntryTileset,x	;get tileset number
	and.w #$ff
	sta.b TempBuffer+4				;multiply with 3
	asl a
	clc
	adc.b TempBuffer+4
	
	txy
	tax
	lda.l (SpriteTilesetLUT+BaseAdress),x
	sta.b TempBuffer+4
	lda.l (SpriteTilesetLUT+1+BaseAdress),x
	tyx
;	and.w #$ff
	sta.b TempBuffer+5
	
	lda.b TempBuffer+2				;get first tile and multiply with 32 to get actual byte offset
	clc
	asl a
	asl a
	asl a
	asl a
	asl a
	clc
	adc.b TempBuffer+4
	bcc CreateObjCalcSourceNoWrap
	
	inc.b TempBuffer+6			;increment bank

CreateObjCalcSourceNoWrap:	
	sta.b TempBuffer+4			;store transfer source offset
	sep #$20
	lda.w ObjEntryPalConf,x	;get nametable bit(msb of sprites first tile)
	and.b #1
	xba
	lda.w ObjEntryVramTile,x	;get target tile number
;	and.w #$ff
	rep #$31
;	clc
	asl a						;multiply by 16(bytesize is 32, but vram is word-aligned)
	asl a
	asl a
	asl a
	clc
	adc.b SpriteTileOffsetVram			;add initial sprite offset
	sta.b TempBuffer+10				;this is the vram target
	phx						;push object list pointer
	ldx.b DmaFifoPointer

	lda.b TempBuffer+8				;get transfer length and increase by 1 to get real length 16bit
	and.w #$ff
	inc a			
	sta.b TempBuffer+12

CreateObjUploadTilesLoop:
	rep #$31
	lda #1					;transfer type normal dma
	sta.l DmaFifoEntryType,x

;store transfer target
	lda.b TempBuffer+10
	sta.l DmaFifoEntryTarget,x		;vram target 2116

;store transfer source
	lda.b TempBuffer+4
	sta.l DmaFifoEntrySrcLo,x		;source 4202
	lda.b TempBuffer+5
	sta.l DmaFifoEntrySrcHi,x		;source 4203

;store transfer length
	lda.b TempBuffer+12				;get transfer length
	sta.l DmaFifoEntryCount,x		;length 4205

;update vram target pointer:
	lda.b TempBuffer+10
	clc
	adc.w #32*16/2					;add one 16tile-line to vram target
	sta.b TempBuffer+10
;update fifo buffer target:
	txa						;update fifo entry pointer
	clc
	adc.w #DmaFifoEntryLength
	tax
	
;update source pointer
	lda.b TempBuffer+4
	clc
	adc.b TempBuffer+12
	sta.b TempBuffer+4

	sep #$20
	dec.b TempBuffer+9				;decrement number of transfers
	bne CreateObjUploadTilesLoop
	
;sprite tile transfer done
	rep #$31	
	txa						;update fifo entry pointer
;	clc
;	adc.w #DmaFifoEntryLength
	sta.b DmaFifoPointer
	plx
	plp						;fetch object list pointer
	rts
*/

;first writes a sorted list of object priorities, then transfers these to oam
ZSortToOamBuffer:
	php
	rep #$31
	sep #$20
	ldx.w #0				;reset pointer to priority list
	lda.b OamZsortSpriteNumber		;exit immediatly if there are no priority sprites on the current screen. else, it keeps on going on forever
	bne ZSortToOamBufferNoExit
	
	plp
	rts
	
ZSortToOamBufferNoExit:	
	sta.b TempBuffer+2			;counter for inner loop
	sta.b TempBuffer+3			;counter for outer loop
	stz.b TempBuffer			;clear "highest priority" variable

	
;	ldx.w #0				;reset pointer to priority list
	txy					;reset pointer to zsort list
	



ZsortSortingInnerLoop:
	lda.w OamZSortBuffer & $FFFF+4,y	;entry present?
	bpl ZsortNotPresent

;sprite present
	lda.w OamZSortBuffer & $FFFF+5,y	;get priority
	cmp.b TempBuffer			;check if bigger than last highest sprite
	bcc ZsortNotBiggest
	
	sta.b TempBuffer			;new highest priority

	rep #$31
	tya					;get pointer in sprite list, save to sort listing
	sta.w OamZSortObjList & $FFFF,x


ZsortNotBiggest:
ZsortNotPresent:
	rep #$31
	tya
	adc.w #OamZSortBufferSize		;get next entry
	tay
	sep #$20

	dec.b TempBuffer+2			;decrease inner counter	
	bne ZsortSortingInnerLoop

;sorted through all once
	lda.b OamZsortSpriteNumber
	sta.b TempBuffer+2			;set counter for inner loop	
	stz.b TempBuffer			;clear highest

;clear "present"-bit of highest in this round	
	phy
	lda.w OamZSortObjList & $FFFF,x		;get pointer to highest
	tay
	lda.w OamZSortBuffer & $FFFF+4,y	;clear "present"-bit
	and.b #$7f
	sta.w OamZSortBuffer & $FFFF+4,y
	
	ply
	
	inx					;figure out next one
	inx
	ldy.w #0				;reset source buffer pointer
	
	dec.b TempBuffer+3			;decrease counter
	bne ZsortSortingInnerLoop

ZSortToOamBufferDoneSorting:	
;	lda.b OamZsortSpriteNumber
	stz.b TempBuffer+2			;clear pointer in priority list, counter
	ldx.b OamBufferPointer			;get oam pointer, write after priorityless sprites
;	ldy.w #0

ZSortWriteToOamLoop:
	rep #$31				;get pointer to current priority list entry
	lda.b TempBuffer+2
	and.w #$ff
	tay	

	lda.w OamZSortObjList & $FFFF,y		;get first pointer
	tay

;copy data from zbuffer to oam buffer
	lda.w OamZSortBuffer & $FFFF,y
	sta.w OamBuffer & $ffff,x	;store in oam buffer
	lda.w OamZSortBuffer & $FFFF+2,y
	sta.w OamBuffer & $ffff+2,x	;store in oam buffer

	phx
	phy

	txa
	and.w #$01f0				;get byte target of size buffer
;	and.w #$0100				;get byte target of size buffer
;	xba					;divide by 16

	lsr a					;divide by 16
	lsr a
	lsr a
	lsr a
	sta.b TempBuffer			;this is the 16bit pointer to the current byte in size buffer

;calculate bit offset in spritesize buffer:	
	txa
	clc
	and.w #$c				;get target bits of size buffer
	lsr a					;shift right twice to get correct jump offset
	tax
	lda.w OamZSortBuffer & $FFFF+4,y
	ldy.b TempBuffer
	and.w #%11
	jmp (PrioObjectProcessorSizeTableJTbl,x)
	
PrioObjectProcessorSizeTableJTbl:
	.dw PrioObjProcSizeJtblBit0
	.dw PrioObjProcSizeJtblBit1
	.dw PrioObjProcSizeJtblBit2
	.dw PrioObjProcSizeJtblBit3

PrioObjProcSizeJtblBit3:
	asl a
	asl a
PrioObjProcSizeJtblBit2:
	asl a
	asl a
PrioObjProcSizeJtblBit1:
	asl a
	asl a
PrioObjProcSizeJtblBit0:

	ora.w OamPriorityBuffer & $ffff,y	;store in corresponding bits/bytes
	sta.w OamPriorityBuffer & $ffff,y	;store in corresponding bits/bytes
	ply
	plx
	lda.w #0
;	sta.w OamZSortBuffer & $FFFF,y		;clear zsort buffer entries right after use so we dont have to clear the whole list each frame
;	sta.w OamZSortBuffer & $FFFF+2,y
;	sta.w OamZSortBuffer & $FFFF+3,y

	inx
	inx
	inx
	inx

	sep #$20
	lda.b TempBuffer+2			;process next entry
	inc a
	inc a
	sta.b TempBuffer+2			;process next entry
	lsr a
	cmp.b OamZsortSpriteNumber		;done copying?

	bne ZSortWriteToOamLoop
	
	stx.b OamBufferPointer
	
	plp
	rts	

;in: a,16bit: desired scanline to draw sprite to.
;out: y,16bit: relative pointer to correct entry in zsort buffer
ObjectZSortGetFreeSlot:
;	php
;	rep #$31
;	tya
	sta.b TempBuffer			;get desired scanline
	asl a			;multiply by 5 to get pointer into zsort buffer
	asl a
	clc
	adc.b TempBuffer
	tay

ObjectZSortGetFreeSlotLoop:
	lda.w OamZSortBuffer & $FFFF,y
	beq ObjectZSortGetFreeSlotDone 
	
	tya
	sec				;increase scanline by one if theres a sprite here already
	sbc.w #OamZSortBufferSize
	bcc ObjZSortGetSlotUnderrun			
	tay
	bra ObjectZSortGetFreeSlotLoop


ObjZSortGetSlotUnderrun:
	ldy.w #210*OamZSortBufferSize			;select max entry
	bra ObjectZSortGetFreeSlotLoop

ObjectZSortGetFreeSlotDone:
;	tya
;	clc
;	adc.w #OamZSortBuffer		;make this a direct pointer
;	tay
;	plp
	rts

ObjProcScreenBound:
	sep #$20	
	lda.w ObjEntryZDisplacement,x			;calculate priority?
;	and.w #$ff												;only get z-priority
	beq ObjProcScreenBoundNoPriorityCalc

;****************************************************
;write sprite to oam zsort buffer instead
	ldy.b OamZsortBufferPointer		;get buffer pointer



;	rep #$31

	lda.w ObjEntryType2,x			;pseudo-3d sprite?
	bmi ObjProc3dSprite


	lda.w ObjEntryZDisplacement,x		;store z


	sta.w OamZSortBuffer & $ffff+5,y
	rep #$31
	lda.w ObjEntryXPos,x	;get xy-coordinates
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a	
;	sep #$20
	sta.w OamZSortBuffer & $ffff,y	;store x coordinate
;	rep #$31
	lda.w ObjEntryYPos,x		;get xy-coordinates
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a
	sep #$20	
	sta.w OamZSortBuffer & $ffff+1,y		;store x coordinate
	jmp ObjProcZsortWriteExit


ObjProc3dSprite:
	lda.w ObjEntryZDisplacement,x
	eor.b #$ff					;invert priority for 3d sprites

	sta.w OamZSortBuffer & $ffff+5,y

	rep #$31
	lda.w ObjEntryXPos,x	;get xy-coordinates
	lsr a			;divide by 16, high precision position
	lsr a
	lsr a
	lsr a
;	lsr a
;	lsr a
	sep #$20
	sta.w OamZSortBuffer & $ffff,y	;store x coordinate
	rep #$31
	lda.w ObjEntryYPos,x		;get xy-coordinates
	lsr a			;divide by 16, high precision position
	lsr a
	lsr a
	lsr a
;	lsr a
;	lsr a
	sep #$20
	sta.w OamZSortBuffer & $ffff+1,y		;store x coordinate
	jmp ObjProcZsortWriteExit



ObjProcScreenBoundNoPriorityCalc:	

	rep #$31
	lda.w ObjEntryXPos,x	;get xy-coordinates
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a	
	sta.w OamBuffer & $ffff,y			;store x coordinate
	lda.w ObjEntryYPos,x	;get xy-coordinates
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a
	sep #$20
	sta.w OamBuffer & $ffff+1,y		;store y coordinate
	
	jmp ObjProcDrawConfig

ObjectSingleSpriteToOam:
;dont upload directly to sprite buffer, but put sprite in a seperate 256entry-list according to y+ydisplacement value so the sprites get z-sorted. ;upload sprite to oam buffer:

;	rep #$31
	ldy.b OamBufferPointer			;get pointer to current oam buffer entry

	lda.w ObjEntryType,x
	bit.w #%00100000				;check if screenbound
	bne ObjProcScreenBound


	lda.w ObjEntryZDisplacement,x
	and.w #$ff
	beq ObjProcNoPriorityCalc

ObjProcPriorityCalc:
;****************************************************
;write sprite to oam zsort buffer instead
	ldy.b OamZsortBufferPointer		;get buffer pointer
	lda.w ObjEntryYPos,x		;get xy-coordinates
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a
	sta.b TempBuffer
		
	lda.w ObjEntryZDisplacement,x
	and.w #$ff
	clc
;	adc.w ObjEntryYPos,x		;get xy-coordinates
	adc.b TempBuffer
	sec
	sbc.b ScreenPixelPositionY	;substract screen position to get sprite position on screen. before doing this, we made sure the sprite is actually onscreen.

	sep #$20
	sta.w OamZSortBuffer & $ffff+5,y
	rep #$31

	lda.w ObjEntryXPos,x	;get xy-coordinates
;	clc
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a
;	adc.w #16			;add 16 because of left border
	sec
	sbc.w ObjEntryXDisplacement,x
	sec
	sbc.b ScreenPixelPositionX	;substract screen position to get sprite position on screen. before doing this, we made sure the sprite is actually onscreen.
	sta.w OamZSortBuffer & $ffff,y	;store x coordinate
	bcs ObjProcPriorSprNoWrap1		;if sprite wraps around left screen edge, carry is clear

	lda.w ObjEntryType,x		;set x-position sign bit if sprite wraps around the edge
	ora.w #%1
	sta.w ObjEntryType,x
	bra ObjProcPriorSprWrapDone

ObjProcPriorSprNoWrap1:
	lda.w ObjEntryType,x		;clear x-position sign bit if sprite wraps around the edge
	and.w #$fffe
	sta.w ObjEntryType,x
	
ObjProcPriorSprWrapDone:
	lda.w ObjEntryYPos,x		;get xy-coordinates
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a	
	sec
	sbc.b ScreenPixelPositionY	;substract screen position to get sprite position on screen. before doing this, we made sure the sprite is actually onscreen.
	sec
	sbc.w ObjEntryYDisplacement,x
	sep #$20
	sta.w OamZSortBuffer & $ffff+1,y		;store x coordinate

ObjProcZsortWriteExit:
	lda.w ObjEntryType,x		;get size/x-sign bits
	and.b #%11
	ora.b #$80			;set "present" bit
	sta.w OamZSortBuffer & $ffff+4,y
	inc.b OamZsortSpriteNumber		;increment number of sortable sprites
	rep #$31
	lda.w ObjEntryVramTile,x	;starting tile and config
	sta.w OamZSortBuffer & $ffff+2,y		;store tile and config
	
	tya						;calc new value for buffer
	adc.w #OamZSortBufferSize
	sta.b OamZsortBufferPointer
	
	rts
;****************************************************




ObjProcNoPriorityCalc:	
	lda.w ObjEntryXPos,x	;get xy-coordinates
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a
	clc
;	adc.w #16			;add 16 because of left border
	sec
	sbc.w ObjEntryXDisplacement,x

	sec
	sbc.b ScreenPixelPositionX	;substract screen position to get sprite position on screen. before doing this, we made sure the sprite is actually onscreen.

	sta.w OamBuffer & $ffff,y	;store x coordinate
	bcs ObjProcSprNoWrap1		;if sprite wraps around left screen edge, carry is clear

	lda.w ObjEntryType,x		;set x-position sign bit if sprite wraps around the edge
	ora.w #%1
	sta.w ObjEntryType,x
	bra ObjProcSprWrapDone

ObjProcSprNoWrap1:
	lda.w ObjEntryType,x		;clear x-position sign bit if sprite wraps around the edge
	and.w #$fffe
	sta.w ObjEntryType,x

	
ObjProcSprWrapDone:
	lda.w ObjEntryYPos,x	;get xy-coordinates
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a	
	sec
	sbc.b ScreenPixelPositionY	;substract screen position to get sprite position on screen. before doing this, we made sure the sprite is actually onscreen.
	sec
	sbc.w ObjEntryYDisplacement,x

	sep #$20
	sta.w OamBuffer & $ffff+1,y		;store x coordinate



ObjProcDrawConfig:

;calculate byte offset in spritesize buffer:	
	rep #$31


;	ldy.b OamBufferPointer			;get pointer to current oam buffer entry
	phx
	phy
	tya
	and.w #$1f0				;get byte target of size buffer
	lsr a
	lsr a
	lsr a
	lsr a
	sta.b TempBuffer			;this is the 16bit pointer to the current byte in size buffer
;calculate bit offset in spritesize buffer:	
	tya
	clc
	and.w #$c				;get target bits of size buffer
	lsr a					;shift right twice to get correct jump offset
	tay
	lda.w ObjEntryType,x		;get size/x-sign bits
	tyx
	ldy.b TempBuffer
	and.w #%11
	jmp (ObjectProcessorSizeTableJTbl,x)
	
ObjectProcessorSizeTableJTbl:
	.dw ObjProcSizeJtblBit0
	.dw ObjProcSizeJtblBit1
	.dw ObjProcSizeJtblBit2
	.dw ObjProcSizeJtblBit3

ObjProcSizeJtblBit3:
	asl a
	asl a
ObjProcSizeJtblBit2:
	asl a
	asl a
ObjProcSizeJtblBit1:
	asl a
	asl a
ObjProcSizeJtblBit0:

	ora.w OamPriorityBuffer & $ffff,y	;store in corresponding bits/bytes
	sta.w OamPriorityBuffer & $ffff,y	;store in corresponding bits/bytes
		
	ply
	plx

;	sep #$20



	rep #$31
	lda.w ObjEntryVramTile,x	;starting tile and config
	sta.w OamBuffer & $ffff+2,y		;store tile and config

	iny						;move pointer to next tile
	iny
	iny
	iny
	sty.b OamBufferPointer				;store new buffer to next oam object
	
	rts



InitOam:
	php
	jsr ClearOamBuffer
	jsr ClearOamPriorityBuffer
	jsr ClearObjectList
	
	sep #$20
	stz.w GravObjectCounter				;reset amount of gravity objects
	stz.b NMIOamUploadFlag			;clear this flag so that oam processor is executed at least once (in case irq oam uploader fucks up)

.IF DEBUG == 1
	lda.b #1			;create cpu-usage object in every scene if debug is enabled
	jsr CreateObject

.endif		
	plp
	rts
	
ClearOamPriorityBuffer:
	php

	rep #$31
	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #$20
	ldx.w #OamPriorityBuffer&$ffff
	jsr ClearWRAM		
	

	plp
	rts
/*
ClearOamPriorityBuffer:
	php
	rep #$31
	lda.w #$0000			;clear with y-position at line 255
	ldx.w #$0020
ClearOamPriorityBufferLoop:
	sta.l (OamPriorityBuffer &$ffff + $7e0000-2),x
	dex
	dex
	bne ClearOamPriorityBufferLoop
	plp
	rts
*/




ClearOamBufferPart:
/*
	php
	rep #$31
	lda.w #$c900			;clear with y-position at line 201
	ldx.b OamBufferPointer
	bra ClearOamBufferLoop
*/
	php
	rep #$31
	sep #$20
	lda.b #3		;clear word: $0000
	ldy.b OamBufferPointer
	beq ClearOamBufferPartNoTrans		;need to do this, else $ffff long transfer occurs
	
	ldx.w #OamBuffer&$ffff
	jsr ClearWRAM
ClearOamBufferPartNoTrans:	
	rep #$31
	stz.b OamBufferPointer
	stz.b OamZsortBufferPointer
	stz.b OamZsortSpriteNumber	
	plp
	rts

	
ClearOamBuffer:

/*
	php
	rep #$31
	lda.w #$c900			;clear with y-position at line 201
	ldx.w #$0200
ClearOamBufferLoop:
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex
	sta.l (OamBuffer &$ffff + $7e0000-2),x
	dex
	dex


	bpl ClearOamBufferLoop
*/


	php

	rep #$31
	sep #$20
	lda.b #3		;clear word: $0000
	ldy.w #$200
	ldx.w #OamBuffer&$ffff
	jsr ClearWRAM		
	
	rep #$31
	stz.b OamBufferPointer
	stz.b OamZsortBufferPointer
	stz.b OamZsortSpriteNumber
	plp
	rts




ClearColObjList:
	php

	rep #$31
	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #ColObjFileSize*32			;old one was: ldx.w #ColObjFileSize*32-2, maybe -2 needed?
	ldx.w #ColObjList&$ffff
	jsr ClearWRAM	
	plp
	rts


/*
	php
	rep #$31

	lda.w #0			;clear with y-position at line 201
	ldx.w #ColObjFileSize*32-2
ClearColObjListLoop:
	sta.l (ColObjList &$ffff + $7e0000-2),x
	dex
	dex
	bne ClearColObjListLoop

	stz.w ColObjListPointer
	plp
	rts
*/


ClearZBuffer:
	php

	rep #$31
	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #OamZSortBufferSize*256
	ldx.w #OamZSortBuffer&$ffff
	jsr ClearWRAM	
	plp
	rts
	
/*
	php
	rep #$31

	lda.w #0			;clear with y-position at line 201
	ldx.w #OamZSortBufferSize*256
ClearZBufferLoop:
	sta.l (OamZSortBuffer & $ffff + $7e0000-2),x
	dex
	dex
	bne ClearZBufferLoop

;	stz.w ColObjListPointer
	plp
	rts
*/

ClearObjectList:
	php
	rep #$31
	sep #$20
	lda.b #0		;clear word: $0000
	ldy.w #ObjectFileSize*64
	ldx.w #ObjectList&$ffff
	jsr ClearWRAM	
	plp
	rts


/*
	php
	rep #$31
	lda.w #$0000
	ldx.w #ObjectFileSize*64
ClearObjectListLoop:
	sta.l (ObjectList &$ffff + $7e0000-2),x
	dex
	dex
	bne ClearObjectListLoop
	plp
	rts
*/


;in: a,16bit: type of objects to delete
;uses TempBuffer
;takes around 11 scanlines
SeekDeleteObjects:
	php
	rep #$31
	sta.b TempBuffer			;store 16bit compare number
	ldx.w #0

SeekDeleteObjectsLoop:
	lda.l ObjEntryObjectNumber+$7e0000,x		;get object number
	cmp.b TempBuffer
	bne SeekDeleteObjectsNoMatch
	
	lda.w #0
	sta.l ObjEntryType+$7e0000,x			;delete object if number matches.

SeekDeleteObjectsNoMatch:
	txa
	clc
	adc.w #ObjectFileSize			;goto next entry
	tax
	cpx.w #ObjectFileSize*64		;check all 64 entries.
	bne SeekDeleteObjectsLoop

	
	plp
	rts

;in: a,16bit: type of object to seek
;uses TempBuffer
;takes around 11 scanlines max.
;searches for first object of selected type.
;returns relative pointer to object in x
;if object was not found, carry is set
SeekObject:
	php
	rep #$31
	sta.b TempBuffer			;store 16bit compare number
	ldx.w #0

SeekObjectLoop:
	lda.l ObjEntryObjectNumber+$7e0000,x		;get object number
	cmp.b TempBuffer
	bne SeekObjectNoMatch
;match found:
	plp
	clc
	rts


SeekObjectNoMatch:
	txa
	clc
	adc.w #ObjectFileSize			;goto next entry
	tax
	cpx.w #ObjectFileSize*64		;check all 64 entries.
	bne SeekObjectLoop

	
	plp
	sec
	rts
	
;same as above, but don't reset pointer into obj list. useful for finding multiple objects of the same type
SeekObjectContinue:
	php
	rep #$31
	sta.b TempBuffer
	bra SeekObjectLoop
	
	
	

ObjVectorMoveHandler:
	php
	rep #$31
	sep #$20
	ldx.b ObjectListPointerCurrent
	lda.w ObjEntryVectorSpeed,x
	bmi ObjVectorSpeedMet

	and.b #%111111
	sta.b TempBuffer
	lda.w ObjEntryVectorTarSpeed,x
	pha
	and.b #%111111
	sta.b TempBuffer+1	
	pla									;get speed change mode (upper 2 bits) and use it as pointer to jump LUT
	rol a
	rol a
	rol a
	asl a
	rep #$31
	and.w #%110
	phx
	tax
	sep #$20

	jsr (ObjVectorSpeedChangeModeLUT,x)
	
	plx
	sta.b TempBuffer+2					;store inc/dec value


;speed inc/dec	
	lda.b TempBuffer+1
	cmp.b TempBuffer					;check if target speed is bigger than actual speed
	bcs ObjVectorTarSpeedBiggerLinear

;target speed is lower than actual speed
	lda.b TempBuffer
	sec
	sbc.b TempBuffer+2		;#%1000			;decrease speed by one full unit
	cmp.b TempBuffer+1		;has actual speed become => target?
	bmi	ObjVectorTarSpeedMetLin		;has speed wrapped around to $ff? if yes, target has also been reached(only needed if target is smaller than 1.0)
	bcs ObjVectorTarSpeedNotMetLinInc

;target speed met:
	bra ObjVectorTarSpeedMetLin


;target speed is bigger than actual speed
ObjVectorTarSpeedBiggerLinear:
	lda.b TempBuffer
	clc
	adc.b TempBuffer+2		;#%1000			;increase speed by one full unit
	cmp.b TempBuffer+1		;has actual speed become => target?
	bcc ObjVectorTarSpeedNotMetLinInc

ObjVectorTarSpeedMetLin:
	lda.b TempBuffer+1			;if target speed has been met or exceeded, write target speed into current and set "speed met"-flag.
	and.b #%111111
	ora.b #$80

ObjVectorTarSpeedNotMetLinInc:
	sta.w ObjEntryVectorSpeed,x
ObjVectorSpeedMet:	

	rep #$31
	ldx.b ObjectListPointerCurrent
	lda.w ObjEntryVectorDir-1,x
	bmi ObjVectorTargetDirMet


	asl a
	asl a
	and.w #%1111110000000000			;put into high byte
	sta.b TempBuffer
	lda.w ObjEntryVectorTurnSpeed,x		;get subpixel precision, 5bits starting at bit3
	asl a								;put into bits 5-9
	asl a
	and.w #%0000001111100000			;mask off unwanted stuff
	ora.b TempBuffer
	sta.b TempBuffer
	lda.w ObjEntryVectorTarDir,x
	
	pha
	xba
	asl a
	asl a
	and.w #%1111110000000000			;put into high byte
	sta.b TempBuffer+2
	
	pla
	lsr a
	lsr a								;get speed change mode (upper 2 bits) and use it as pointer to jump LUT
	lsr a
	lsr a
	lsr a
	
	and.w #%110
	phx
	tax

	jsr (ObjVectorDirChangeModeLUT,x)

	plx
	sta.b TempBuffer+4					;store inc/dec value


;	lda.b TempBuffer+1
;	cmp.b TempBuffer					;check if target speed is bigger than actual speed
	lda.w ObjEntryVectorDir,x
	bit.w #%01000000
	bne ObjVectorRotateClockwise		;bcs ObjVectorTarSpeedBiggerLinear

;rotate anti-clockwise to meet target dir
	lda.b TempBuffer				;get current angle
	cmp.b TempBuffer+2				;is target angle smaller? (=must 0 be crossed to reach target?)
	bcs AntiClockwiseNoZeroCross

;must cross 0 to reach target
	sec
	sbc.b TempBuffer+4				;substract speed value
	bcs AntiClockwiseTargetNotMet		;if 0 wasn't passed, target value couldn't have possibly been reached
	cmp.b TempBuffer+2				;compare target to new angle
	bcc AntiClockwiseTargetMet			;target met if new angle is smaller than old	
	bra AntiClockwiseTargetNotMet

ObjVectorTargetDirMet:
	jmp ObjVectorTargetDirMetLong


AntiClockwiseNoZeroCross:
	sec
	sbc.b TempBuffer+4				;substract speed value
	bcc AntiClockwiseTargetMet			;target met if angle wraps around through 0.
	cmp.b TempBuffer+2				;compare target to new angle
	bcc AntiClockwiseTargetMet			;target met if new angle is smaller than old
	bra AntiClockwiseTargetNotMet
	
ObjVectorRotateClockwise:

;rotate clockwise to meet target dir
	lda.b TempBuffer				;get current angle
	cmp.b TempBuffer+2				;is target angle smaller? (=must 0 be crossed to reach target?)
	bcc ClockwiseNoZeroCross

;must cross 0 to reach target
	clc
	adc.b TempBuffer+4				;add speed value
	bcc AntiClockwiseTargetNotMet		;if 0 wasn't passed, target value couldn't have possibly been reached
	cmp.b TempBuffer+2				;compare target to new angle
	bcs AntiClockwiseTargetMet		;target met if new angle is bigger than old	
	bra AntiClockwiseTargetNotMet
	


ClockwiseNoZeroCross:
	clc
	adc.b TempBuffer+4				;add speed value
	bcs AntiClockwiseTargetMet		;target met if angle wraps around through 0.
	cmp.b TempBuffer+2				;compare target to new angle
	bcs AntiClockwiseTargetMet		;target met if new angle is bigger than old
	bra AntiClockwiseTargetNotMet

AntiClockwiseTargetMet:
	lda.b TempBuffer+2				;target met, set target without subpixel as new angle
	and.w #%1111110000000000
	lsr a
	lsr a
	ora.w #$8000					;set "target met" flag
	bra AntiClockwiseTargetWrite
	
AntiClockwiseTargetNotMet:
	and.w #%1111111111100000		;angle with subpixel precision
	lsr a
	lsr a
AntiClockwiseTargetWrite:	

	sta.b TempBuffer				;save new angle
	sep #$20
	lda.w ObjEntryVectorDir,x
	and.b #%01000000
	ora.b TempBuffer+1				;save back new direction
	sta.w ObjEntryVectorDir,x
	
	lda.w ObjEntryVectorTurnSpeed,x
	and.b #%111
	ora.b TempBuffer
	sta.w ObjEntryVectorTurnSpeed,x



ObjVectorTargetDirMetLong:	
	rep #$31					;get current position in vector angle, 16 max
	lda.w ObjEntryType2,x
	and.w #$f
	sta.b TempBuffer
	lda.w ObjEntryVectorTurnSpeed,x	;get subpixel precision for later usage
	and.w #%111
	sta.b TempBuffer+2
	lda.w ObjEntryVectorSpeed,x
	and.w #%111111
	pha
	lda.w ObjEntryVectorDir,x	;get direction, multiply with 32, add current position in angle
	and.w #%111111
	asl a
	asl a
	asl a
	asl a
	asl a						
	adc.b TempBuffer
	clc
	adc.w #VectorAngleCodeLUT
	tax							;source offset
	ldy.w #VectorAngleSMCode&$ffff	;target offset
	
	pla							;get vector speed
	clc
	adc.b TempBuffer+2			;add sub-pixel buffer
	pha
	and.w #%111
	sta.b TempBuffer+2			;save back for next frame
	
	pla
	lsr a						;remove sub-pixel precision
	lsr a
	lsr a
	and.w #%111
	pha
	beq VectorAngleNoSpeed		;triggers when vector move distance is lower than 1 and prevents ram thrashing.
	
	dec a						;decrease length by one for mvn

	.db $54						;MVN
	.db $7e						;destination bank
	.db :VectorAngleCodeLUT+$c0	;target bank

	lda.w #$6b							;rtl opcode
	sta.w $0,y							;store after sm-code. y conveniently points to the byte after the last one transferred by MVN.	
	
	ldx.b ObjectListPointerCurrent
	lda.w ObjEntryYPos,x
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a	
	
	tay
	lda.w ObjEntryXPos,x
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a
	phx
	tax
	jsl VectorAngleSMCode
	
	txa
	plx
	asl a												;add subpixel precision
	asl a
	asl a
	asl a	
	sta.w ObjEntryXPos,x
	tya
	asl a												;add subpixel precision
	asl a
	asl a
	asl a	
	sta.w ObjEntryYPos,x

VectorAngleNoSpeed:	
	pla							;update vector angle position
	clc
	adc.b TempBuffer
	and.w #$f
	sta.b TempBuffer
	
	sep #$20

	
	ldx.b ObjectListPointerCurrent

	lda.w ObjEntryType2,x				;update vector angle position
	and.b #%11110000
	ora.b TempBuffer
	sta.w ObjEntryType2,x
	
	lda.w ObjEntryVectorTurnSpeed,x		;update vector speed sub-pixel position
	and.b #%11111000
	ora.b TempBuffer+2
	sta.w ObjEntryVectorTurnSpeed,x


	plp
	rts	


ObjVectorSpeedChangeModeLUT:
	.dw ObjVectorSpeedChangeDirect
	.dw ObjVectorSpeedChangeLinearSlow
	.dw ObjVectorSpeedChangeLinearFast
	.dw ObjVectorSpeedChangeLinearSmooth
	
ObjVectorSpeedChangeDirect:
	lda.b #$40
	rts	
	
ObjVectorSpeedChangeLinearSlow:
	lda.b #1					;smallest possible unit, takes 64 frames max to reach target speed
	rts
	
ObjVectorSpeedChangeLinearFast:
	lda.b #8					;one full unit, takes 8 frames max to reach target speed
	rts	


;smooth: divide the difference between target and current speed by two and use that to alter the current speed
ObjVectorSpeedChangeLinearSmooth:
	lda.b TempBuffer+1
	sec
	sbc.b TempBuffer					;check if target speed is bigger than actual speed
	bcc ObjVectorTarSpeedBiggerSmooth
	
	bra ObjVectorTarSpeedSmooth


ObjVectorTarSpeedBiggerSmooth:
	lda.b TempBuffer
	sec
	sbc.b TempBuffer+1

ObjVectorTarSpeedSmooth:	
	lsr a
	bne ObjVectorTarSpeedMinOKSmooth
	
	lda.b #1							;minimum speed: 1
	rts
	
ObjVectorTarSpeedMinOKSmooth:
	cmp.b #8
	bcc ObjVectorTarSpeedMaxOKSmooth
	
	lda.b #8							;maximum speed: 8
ObjVectorTarSpeedMaxOKSmooth:	
	rts
	
	
ObjVectorDirChangeModeLUT:
	.dw ObjVectorDirChangeDirect
	.dw ObjVectorDirChangeLinearSlow
	.dw ObjVectorDirChangeLinearFast
	.dw ObjVectorDirChangeLinearSmooth
	

ObjVectorDirChangeDirect:
	lda.w #$fc00
	rts	
	
ObjVectorDirChangeLinearSlow:
	lda.w #$20					;smallest possible unit, takes 64 frames max to reach target speed
	rts
	
ObjVectorDirChangeLinearFast:
	lda.w #$400					;one full unit, takes 8 frames max to reach target speed
	rts	


;smooth: divide the difference between target and current speed by two and use that to alter the current speed
ObjVectorDirChangeLinearSmooth:
	lda.b TempBuffer+2
	sec
	sbc.b TempBuffer					;check if target speed is bigger than actual speed
	bcc ObjVectorDirTarSpeedBiggerSmooth
	
	bra ObjVectorDirTarSpeedSmooth


ObjVectorDirTarSpeedBiggerSmooth:
	lda.b TempBuffer
	sec
	sbc.b TempBuffer+2

ObjVectorDirTarSpeedSmooth:	
	lsr a
	cmp.w #$20							;smaller than minimum speed?
	bcs ObjVectorDirTarSpeedMinOKSmooth
	
	lda.w #$20							;minimum speed: 1
	rts
	
ObjVectorDirTarSpeedMinOKSmooth:
	cmp.w #$400
	bcc ObjVectorDirTarSpeedMaxOKSmooth
	
	lda.w #$400							;maximum speed: 8
ObjVectorDirTarSpeedMaxOKSmooth:	
	rts
	
OamSubCreateParticles:
;create a couple of particles, position and palette same as victim:
;input: x=pointer to object that is emitting the particles
;		a=number to add to the random number(0-3) of particles to generate. (valid ranges:0-3)
;modifies:paltemp
;calls: createobject (uses TempBuffers), updates random a couple of times
	php
	rep #$31
	phx
	sep #$20
	pha
	lda.b R4
	and.b #%11
	sta.b PalTemp
	pla
	and.b #%11
	clc
	adc.b PalTemp
	beq ParticleCreateExit
	sta.b PalTemp
	
CreateParticleLoop:
	rep #$31
	lda.w ObjEntryYPos,x
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a

	lsr a
	lsr a
	lsr a
	and.w #$ff
	xba
	sta.b TempBuffer

	
	lda.w ObjEntryXDisplacement,x
	and.w #$ff
	sta.b TempBuffer+2

	lda.w ObjEntryXPos,x
	lsr a										;remove subpixel precision
	lsr a
	lsr a
	lsr a
	sec
	sbc.w #10

	
	lsr a
	lsr a
	lsr a
	inc a
	and.w #$ff
	ora.b TempBuffer
	tax
	lda.w #54
	jsr CreateObjectPosition
	sep #$20

	plx
	lda.w ObjEntryPalConf,x
	and.b #%1110
	sta.b TempBuffer
	phx
	ldx.b ObjectListPointerCurrent
	lda.w ObjEntryPalConf,x
	and.b #%11110001
	ora.b TempBuffer					;store palette of player for this particle
	sta.w ObjEntryPalConf,x
	jsr Random							;update random numbers so that all particles look different
	jsr Random							;update random numbers so that all particles look different
	dec.b PalTemp
	bne	CreateParticleLoop

ParticleCreateExit:
	plx
	plp
	rts
	
ObjProcUpdateGravity:	
	lda.w ObjEntryLifeCounter,x
	and.w #$ff
	beq ObjProcGravityKill

	dec.w ObjEntryLifeCounter,x		;decrease life counter
	lda.w ObjEntryXSpeed,x		;add speed x-dir
	and.w #$ff
	clc
	adc.w #$ff80							;align in middle so that x-speed can be both positive and negative
	clc
	adc.w ObjEntryXPos,x
	sta.w ObjEntryXPos,x
	
	lda.w ObjEntryGravity,x
	and.w #$ff
	clc
	adc.w ObjEntryYSpeed,x
	sta.w ObjEntryYSpeed,x
	clc
	adc.w ObjEntryYPos,x
	sta.w ObjEntryYPos,x
	lsr a										;remove subpixel prec.
	lsr a
	lsr a
	lsr a
	cmp.w GravityCutOffYPos	;check if object is too low and should be deleted
	bcs ObjProcGravityKill
	rts	
	
ObjProcGravityKill:
	dec.w GravObjectCounter
	stz.w ObjEntryType,x		;kill object if life is up
	rts
	
	
ObjectProcessGravObjInc:
	php
	sep #$20
	lda.w GravObjectCounter
	cmp.w MaxGravObjCount				;number bigger than max count?
	bcc ObjectProcessGravObjNoDel

	stz.w ObjEntryType,x					;delete object if count is too high
	stz.w ObjEntryType2,x
	plp
	rts

ObjectProcessGravObjNoDel:
	inc.w GravObjectCounter
	plp
	rts	
	