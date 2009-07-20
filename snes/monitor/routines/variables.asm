
.ENUM $00				;1byte direct page variables for kernel and game mechanics
TempBuffer		ds 20
CurrentEvent		db
FrameCounterLo		db
FrameCounterHi		db
SetIni			db
ScreenMode		db
MainScreen		db
SubScreen		db
ScreenBrightness	db
BGTilesVram12		db
BGTilesVram34		db
BG1TilemapVram		db
BG2TilemapVram		db
BG3TilemapVram		db
BG4TilemapVram		db
SpriteTileOffsetVram	dw
ObjSel			db
BG1HOfLo		db
BG1HOfHi		db
BG1VOfLo		db
BG1VOfHi		db
BG2HOfLo		db
BG2HOfHi		db
BG2VOfLo		db
BG2VOfHi		db
HdmaFlags		db
HdmaPause		db		;if msb is set, don't initiate further hdma transfers. this is useful when the rom hdma is trying to read from isnt present atm.
ThreeBytePointerLo	db
ThreeBytePointerHi	db
ThreeBytePointerBank	db
ThreeBytePointerLo2	db
ThreeBytePointerHi2	db
ThreeBytePointerBank2 	db
BrightnessSpeed		db
LastFrameLo		db
LastFrameHi		db
CGWsel			db
CgadsubConfig		db
FixedColourR		db
FixedColourG		db
FixedColourB		db
InterruptEnableFlags	db
IrqRoutineNumber	db
IrqRoutineNumberBuffer	db				;backup in case multiple irqs are executed per frame. nmi writes this number back to irq number every frame
IrqVCounter		dw
IrqHCounter		dw
W12SEL			db
W34SEL			db
WOBJSEL			db
W1L			db
W1R			db
W2L			db
W2R			db
WBGLOG			db
WOBJLOG			db
WMS			db
WSS			db
Mosaic			db
BGTilesVram2		dw
Bg3Status		db
PrintStringThreeBytePointerLo	db
PrintStringThreeBytePointerHi	db
PrintStringThreeBytePointerBank	db
VramBg1Tilemap			dw
VramBg2Tilemap			dw
VramBg3Tilemap			dw
VramBg4Tilemap			dw
VramBg1Tiles			dw
VramBg2Tiles			dw
VramBg3Tiles			dw
VramBg4Tiles			dw
SetBGThreeBytePointerLo		db
SetBGThreeBytePointerHi		db
SetBGThreeBytePointerBank	db

NMIOamUploadFlag	db
NMIPaletteUploadFlag	db	;refresh palette during nmi?
NMIBg1UploadFlag	db	;refresh bg1 tilemap during nmi?
NMIBg2UploadFlag	db
NMIBg3UploadFlag  db	;UploadBg3Map			db			;if this is not zero, upload tilemap to bg3
CurrentMapNumber	db	;number of currently loaded map
CurrentMapPointer	ds 3	;24bit pointer to current map file
CurrentColMapPointer	ds 3	;24 bit pointer to current collision map
BgScrollCounterX		db	;ranges from 0-7 inside one tile
BgScrollCounterY		db
BgMapCurrentPositionX		db	;current upper left tile on screen
BgMapCurrentPositionY		db
ScreenPixelPositionX		dw	;exact pixel position of upper left border for sprite position calculation
ScreenPixelPositionY		dw	;exact pixel position of upper left border for sprite position calculation
BgScrollRowUploadDisplace	db	;number of tiles to add/substract when uploading tilerows. this gets added to current position in map depending on the scrolling direction.(scroll right: add 28/scroll left: substract 1)
BgScrollTilemapRowUploadDisplaceX	db	;number of tiles to add/substract when uploading tilerows. this gets added to current position in map depending on the scrolling direction.(scroll right: add 28/scroll left: substract 1)
BgScrollTilemapRowUploadDisplaceY	db	;number of tiles to add/substract when uploading tilerows. this gets added to current position in map depending on the scrolling direction.(scroll right: add 28/scroll left: substract 1)
BgScrollOffsetPointerTilesX		db	;pointer to current vertical column that holds the leftmost tiles onscreen(goes from 0-27)
BgScrollOffsetPointerTilesY		db
BgScrollOffsetPointerTilemapX		db	;pointer to current vertical column that holds the leftmost tiles onscreen for tilemap (goes from 0-32)
BgScrollOffsetPointerTilemapY		db
BgScrollTileSourcePointer		ds 3		;pointer to first tile of tileline to be uplodaded

DmaFifoPointer				dw		;relative pointer to current free entry in buffer
DmaFifoPointerIrq			dw
DmaFifoSourcePointerLo			dw	
DmaFifoSourcePointerBa			db
DmaFifoOverhang				dw
DmaFifoTotalBytesTransferred		dw		;used to guesstimate how much time we have left for transfers

ObjectListPointer			dw		;pointer to next free object in object list
ObjectListPointerCurrent		dw		;pointer to current object in object list, used to set direct register
CurrentObjectNumber			db		;number of currently selected object in list
OamBufferPointer			dw		;pointer to current sprite in oam buffer
OamZsortBufferPointer			dw		;pointer to current sprite in oam zsort buffer
OamZsortSpriteNumber			db		;number of sprites to sort
OamAniListStraightRepeatFlag		dw

FocusScreenFlags	db		;flags for focus
					;bit0=enable focus on object
FocusScreenObject	db		;number of object in object list to focus to
FocusScreenSpline	db		;number of preset table to use for scrolling depending on distance to object(linear,sine,exp etc)
FocusScreenXWait	db
FocusScreenYWait	db

CollisionPixelX		dw
CollisionPixelY		dw
CollisionTemp		dw
PalTemp				db		;used for object subroutine
ExitCollisionPointer	dw		;bits0-14 contain number of exit.   old:;bits0-14 contain pointer to an entry in the exit list. if bit15 is set, this exit has been hit and needs to be procesed.
R1			db		;random number generator buffers
R2			db
R3			db
R4			db
ColObjListPointer	dw
HdmaListPointer		dw
VwfFontPointerLo	ds 3


SpcTempBuffer			ds 8		;temp buffer for spc stuff
SpcCurrentStreamSet		db
SpcHandlerState			db
SpcHandlerArgument0		db
SpcHandlerArgument1		db
SpcHandlerArgument2		db
SpcCmdFifoStart			db
SpcCmdFifoEnd			db

PtPlayerDataPointerLo		db	;assumes dreg: $0000
PtPlayerDataPointerHi		db	;assumes dreg: $0000
PtPlayerDataPointerBa		db	;assumes dreg: $0000
PtPlayerCurrentSong		db	;assumes dreg: $0000
PtPlayerCurrentSamplePack	db
PtPlayerCurrentSoundEffect	db

PtPlayerSmplBufferPosLo		db	;not needed at all
PtPlayerSmplBufferPosHi		db
SpcUploadedFlag				db	;msb set=song upload complete and playing. bit6 set=sample pack uploaded
VideoHandlerState			db

CheckJoypadMode				db	;0=1 player, 1=8 players, 2=instruments + 1 joypad

FontSelector		db
FixedStringLength		db
PrintStringPalette		db

MenuFileThreeBytePointerLo		db
MenuFileThreeBytePointerHi		db
MenuFileThreeBytePointerBank	db
MenuRowsThreeBytePointerLo		db
MenuRowsThreeBytePointerHi		db
MenuRowsThreeBytePointerBank	db
MenuRowsThreeByteOptionPointerLo	db
MenuRowsThreeByteOptionPointerHi	db
MenuRowsThreeByteOptionPointerBank	db
MenuRowsThreeByteCodePointerLo	db
MenuRowsThreeByteCodePointerHi	db
MenuRowsThreeByteCodePointerBank	db
LoadMenuInitialOffset		dw
LoadMenuInitialOptionOffset	dw
LoadMenuVerticalSpacing		dw
LoadMenuNumberOfRows		db
LoadMenuCurrentRow		db
LoadMenuStringPosLo		db
LoadMenuStringPosHi		db
LoadMenuPalUnselSel		db
LoadMenuDoInit			db
BattleSubroutine		db
HdmaListCounter			db	;counter used by hdma handler to switch through channels

TempBufferIrq			ds 8		;Temporary Buffer for irq routines
BattleMusicState		db
CurrentTablistPointer		ds 3
.ENDE



.ENUM $200		;2-byte adresses for game status stuff
TempBufferTest		ds 4
Hdma3dScrollCountV	db	;amount of lines to wait before hdma table
CurrentBattleFile	db
MapStartPosX	db		;map start position set by external routine
MapStartPosY	db
MapSizeX	db		;/must not be seperated cause they may be written to both at the same time in word-mode
MapSizeY	db		;\
BGMapStartPosX	db		;start position for bg upload(may differ from sprite location if near a border)
BGMapStartPosY	db

CurrentStringTarget	db	;current position in bg1 tilemap to write to, 2 bytes
CurrentStringTargetHi	db
FixedColourRsave	db
FixedColourGsave	db
FixedColourBsave	db
CpuUsageScanline		db
SpcStreamVolume			db
SpcSEVolume			db
SpcSEPitch			db
SpcSongSpeed			db		;default $a0
SpcSongChMask			db		;default $0f
SpcReportType			db		;0=none 1=timecode 2=channel-levels(vol out) 3=special mod command

SpcCmdFifo			ds 64
MessageDeleteCounter		dw		;inactive when zero, delete message when 1(then set zero), decrease each frame when not 0 or 1						

HdmaFadeInOutState		db		;bit7set=fade in/bit7clear=fade out,bit6set=done fading bit0-5=current state in LUT for window and mosiac size
MainCharaObjectNumber		db
MainCharaXPos			dw
MainCharaYPos			dw
ExitMapTarget			dw		;target map
ExitXTarget			dw		;target x-pos on map
ExitYTarget			dw		;target y-pos on map

Pseudo3dScrollUpdateFlag	db		;if this isn't zero, update scroll table
CurrentLevel			dw
PlayerState			db		;current mode of player characters. 0=active battle 1=player select menu(start sitting. when start pressed for that player, switch to subroutine that is similar to battle, but without being able to fight) 2=results screen.
PlayersPresentFlags		db		;flags to signalize players connected.
ActivePlayers			db		;number of alive players 		
WinningPlayer			db		;number of player that has won the match. $ff=invalid
IrqBrightnessIncDec		db		;just a flag. bit0 set=increase. bit1 clear=decrease. bit8=ack/done
BrightnessEventBuffer		db		;buffer event routine to jum p to after brightness inc/dec
SpcStreamFrame			dw
SpcSoundEffectFlipFlag		dw		;flag alternating between each sound effect upload so that spc doesnt trigger the same one twice.
BattleFinished			db		;$80 if battle is finished
IntroScene3ScrollPoint		dw
VideoFrames				db
VideoFrameRate			dw		;ANDed with framecounter. 0=60hz, 1=30hz, 3=15hz etc.
CurrentVideo			db
CurrentVideoFrame		db
PlayerSelectScrollCounter	dw
WinningPlayerPointer		dw
RandomLevelCounter			db	;msb=shuffle direction. bits0-3 level number
RandomStreamCounter			db	;msb=shuffle direction. bits0-3 Streamset number
SpecialReportOld			db
HdmaScrollPointerBuffer	dw
GravityCutOffYPos					dw	;y-pos of object(with 4bit precision)	that triggers delete if bigger and gravity affected
GravObjectCounter						db	;counter of particle/gravity objects to create to prevent slowdown and such
MaxGravObjCount						db		;max allowed particles per frame. dont create new ones if exceeded
EventJumper								db		;variable to select which event to jump to in debug menu
CollisionObjPointer				dw	;pointer to object that just has collided with calling obj
Reg3000WriteVar				db
VIrqCounter					dw
ExtIrqCounter				dw
CartChecksum				db
.ENDE

.ENUM $7e0300
JoyPort1Data1Io0Buffer		ds 4
JoyPort1Data2Io0Buffer		ds 4
JoyPort2Data1Io0Buffer		ds 4
JoyPort2Data2Io0Buffer		ds 4

JoyPort1Data1Io1Buffer		ds 4
JoyPort1Data2Io1Buffer		ds 4
JoyPort2Data1Io1Buffer		ds 4
JoyPort2Data2Io1Buffer		ds 4
VectorAngleSMCode			ds 9	;8 inc/decs per frame + rtl

LoadMenuStringBuffer		ds 9
PrintStringBuffer0		ds 16
;.ende

;.ENUM $7e0300
JoyPortBuffer			ds 16		;8 joypads max, word entries.
						;0=port1 joy 1 
						;2=port1 joy 2 
						;4=port1 joy 3 
						;6=port1 joy 4 
						;8=port2 joy 1
						;a=port2 joy 2
						;c=port2 joy 3
						;e=port2 joy 4
JoyPortBufferOld			ds 16
JoyPortBufferTrigger			ds 16

;joypad buttons:
;15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
;b  y  se st U  D  L R a x l r 0 0 0 0



PaletteBuffer			ds $0200	;bg3 palettes: buffer $0-$20 (4 different 4-color palettes, for text
Bg1MapBuffer			ds $1000
Bg2MapBuffer			ds $800
Bg3MapBuffer			ds $0800
Bg4MapBuffer			ds $1
OamZSortBuffer			ds $0500	;256 entries, each 6 bytes
OamZSortObjList			ds 128*2		;prioritized list of objects that need priority processing(2 byte relative pointers)
;ZSortOamPriorityBuffer		ds $0040
OamBuffer			ds $0200
OamPriorityBuffer		ds $0020
DmaFifoBuffer			ds $1000 	;DmaFifoEntryLength*256
ObjectList			ds $0800	;32 entries, each 32 bytes
ExitList			ds $0080	;16 entries, each 8 bytes
ColObjList			ds $00c0	;32 entries, each 4 bytes
HdmaBuffer			ds $0340
HdmaBuffer1			ds $0340
HdmaBuffer2			ds $0340
HdmaBuffer3			ds $0340
HdmaBuffer4			ds $0340
HdmaBuffer5			ds $0340
HdmaDataBuffer1			ds $0400
HdmaSpcBuffer			ds 200		;streaming table, uses channel 7
TextBoxMenuBuffer		ds $0040	;8 entries * 8
SpcReportBuffer			ds 16		;8 entries, each 2 bytes
									;0=none
									;2=timecode
									;4=channel levels
									;6=special mod cmd
Hdma3dScrollBuffer		ds 400		;80 entries, each 5 bytes
WinnerArray				ds 8*1
.ENDE
