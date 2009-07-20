;*************************************************************************
.define DEBUG			1			;enable debugmenu
;*************************************************************************

;*************************************************************************
;spc interface definitions:
.define  SpcFrameSize		144
.define	 SpcFramesPerBank	455	

.define SpcCmdUploadSong	$f1		;indicates that a song is to be uploaded
.define SpcCmdUploadSongT1	$f2		;indicates that data for transfer1 is on apu ports
.define SpcCmdUploadSongT2	$f3		;indicates that data for transfer2 is on apu ports
.define SpcCmdUploadSongDone	$f4		;indicates that song upload is complete
.define SpcCmdReceiveStream	$f5		;indicates that 65816 wants to stream brr data to spc
.define SpcCmdSubmitStreamNumber	$f7		;indicates that hdma transfer has started.  It's important that bit0 of this command is set.(brr end bit)
.define SpcCmdReceiveStreamComplete	$f6	;spc wants to end stream transmission.
.define SpcCmdUploadSamplePack	$f8		;indicates that a sample pack is to be uploaded. the rest of the commands are taken from normal song upload
.define SpcCmdUploadSamplePackDone	$f9		;indicates that sample pack upload is complete
.define SpcCmdPlaySoundEffect	$fa		;play a sound effect
.define SpcCmdStopSong		$fb		;stop song or stream
.define SpcCmdSetSongSpeed	$fc		;set timer speed of mod playback routine
.define SpcCmdSetSongChMask	$fd		;song channel mask
.define SpcCmdReportType	$fe		;type of data spc should respond with

.define SpcScanlineWaitCount	5		;amount of scanlines to wait before frame send

;spc fifo commands for other routines to write to
.define SpcFifoCmdUploadSong	$02		;arg 0 is song number
.define SpcFifoCmdUploadSamplePack	$04		;arg 0 is song number
.define SpcFifoCmdStreamBrr	$06		;arg 0 is stream frameset number
.define SpcFifoCmdPlaySE	$08		;arg 0 is stream frameset number
.define SpcFifoCmdStopSong	$0a		;arg 0 is stream frameset number
.define SpcFifoCmdSetSpeed	$0c		;arg 0 is speed
.define SpcFifoCmdSetChMask	$0e		;arg 0 is mask
.define SpcFifoCmdSetReportType	$10		;arg 0 is mask
;*************************************************************************

;*************************************************************************
;background settings
.define FontVramOffset		$2500	;$2800	;vram offset of the font, starting tile index is $180
.define FontTileConfig			$20	;priority bit set, tiles $100+, palette 1

.define TileBufferSizeX	28		;number of tiles in tilebuffer per x-line
.define TileBufferSizeY	26		;number of tiles in tilebuffer per y-line

.define TileMapSizeX	31		;number of tiles in tilebuffer per x-line
.define TileMapSizeY	31		;number of tiles in tilebuffer per y-line
;*************************************************************************


;*************************************************************************
;fifo buffer entry definitions:
.define DmaFifoEntryLength	8	;length of an entry in the dma fifo buffer
.define DmaFifoEntryType	DmaFifoBuffer &$ffff + $7e0000
.define DmaFifoEntryCount	DmaFifoBuffer &$ffff + $7e0000+1
.define DmaFifoEntryTarget	DmaFifoBuffer &$ffff + $7e0000+3
.define DmaFifoEntrySrcLo	DmaFifoBuffer &$ffff + $7e0000+5
.define DmaFifoEntrySrcHi	DmaFifoBuffer &$ffff + $7e0000+6
.define DmaFifoEntrySrcBank	DmaFifoBuffer &$ffff + $7e0000+7

.define DmaFifoBytesPerFrame	$1000-$220-$200-$200		;substract oam and cgram transfer sizes
.define DmaFifoMinTransferSize	128
;*************************************************************************



;*************************************************************************
;object buffer entry definitions:
.define ObjectFileSize	32
.define ObjEntryType			ObjectList & $ffff		;0 object type designation
													;bit0=X position sign of sprite(usually 0)
													;bit1=Object size flag
													;bit2=collidable
													;bit3=subroutine? if set, this object has its own subroutine that must be executed every frame
													;bit4=animate? if set, this object is animated(a special table for each object specifies the exact animation with commands for tileloading, waiting, animation loop etc)
													;bit5=bg-bound? if set, this object must move in accordance with background layer 0
													;bit6=object active? if set, this object is active and needs to be drawn. if clear, this sprite doesnt need to be drawn, but must be processed, anyway
													;bit7=object present? if set, this slot has an object. if clear, its considered empty and can be overwritten
.define ObjEntryType2			ObjectList & $ffff + 1	;extra type definitions. msb set=pseudo-3d sprite that has to be moved when scrolling.
													;bit7=pseudo 3d sprite that needs to be moved according to z-value when background moves
													;bit6=don't upload tiles for this sprite. useful for stuff like particles and such where lots of sprites share the same tiles.
													;bit5=don't upload palette for this sprite.
													;bit4=decaying gravity sprite
													;bits0-3: current position in vector angle LUT (didn't fit anywhere else)
													
.define ObjEntrySubRout			ObjectList & $ffff + 2
.define ObjEntryTileset			ObjectList & $ffff + 3
.define ObjEntryTilesetFrame	ObjectList & $ffff + 4
.define ObjEntryVramTile		ObjectList & $ffff + 5	;vram tile and
.define ObjEntryPalConf			ObjectList & $ffff + 6	;pal config must be in that order because they are accessed word-wise together
.define ObjEntryXPos			ObjectList & $ffff + 7	;word size
.define ObjEntryYPos			ObjectList & $ffff + 9	;word size
.define ObjEntryAniFrame		ObjectList & $ffff + 11
.define ObjEntryAniList			ObjectList & $ffff + 12
.define ObjEntryListOffset		ObjectList & $ffff + 13
.define ObjEntryPalNumber		ObjectList & $ffff + 14
.define ObjEntryObjectNumber	ObjectList & $ffff + 15	;number of object type so other routines know what object the`re dealing with. this is not the number of the object in the lookup table, but an arbitrary number that defines certain object types/groups so they can be moved/deleted alltogether.
.define ObjEntryXDisplacement	ObjectList & $ffff + 17	;used only by obj2oam writer to center objects
.define ObjEntryYDisplacement	ObjectList & $ffff + 18	;used only by obj2oam writer to center objects
.define ObjEntryZDisplacement	ObjectList & $ffff + 19	;used to calculate priority. also used as direct z-value in pseudo-3d scenes.
.define ObjEntryAniCmdRepeat	ObjectList & $ffff + 20	;used by animation list processor. allows repeated nops with a single command
.define ObjEntryColliSub		ObjectList & $ffff + 21	;subroutine to execute when mainchara hits this object(only active if object is collidable)

.define ObjEntryVectorSpeed		ObjectList & $ffff + 22	;bit0-2:subpixel speed. bit3-5:pixel speed.  bit7 set: target speed met  
.define ObjEntryVectorTarSpeed	ObjectList & $ffff + 23	;bit0-2:subpixel speed target. bit3-5:pixel speed target. bit6,7: movement type(direct, linear slow, linear fast, smooth)
.define ObjEntryVectorDir		ObjectList & $ffff + 24	;bit0-5:direction.0=facing up. bit6=turning direction.(set=clockwise) msb set=target direction met.
.define ObjEntryVectorTarDir	ObjectList & $ffff + 25	;bit0-5:target direction.0=facing up.  bit6,7: movement type(direct, linear slow, linear fast, smooth)
.define ObjEntryVectorTurnSpeed	ObjectList & $ffff + 26	;bit0-2:vector speed subpixel buffer. bit3-7: direction turn speed sub-pixel buffer. 


.define ObjEntryXSpeed				ObjectList & $ffff + 22	;byte, gets added to x-pos every frame. lower 4bit subpixel precision
.define ObjEntryYSpeed				ObjectList & $ffff + 23	;word, gets added to y-pos every frame. lower 4bit subpixel precision. needs to be word because of +/- handling and gravity
.define ObjEntryGravity				ObjectList & $ffff + 25	;byte, gets added to y-speed every frame.
.define ObjEntryLifeCounter		ObjectList & $ffff + 26	;word, decreased every frame

.define ObjEntrySpareVar		ObjectList & $ffff + 27	;5 bytes of data that can be used as a variable depending on the objects needs. for example, npcs uses this to store direction and walk count

;.define MaxGravObjCount	20											;maximum amount of allowed particles onscreen

;zsort buffer definitions:
.define OamZSortBufferSize 6				;entries:
							;0	- sprite x
							;1	- sprite y
							;2	- sprite config
							;3	- sprite config
							;4	- size/sign bits (bit0,1) bit7 clear: sprite present flag
							;5	- z priority (scanline number)
							
.define SpriteTileSizeX	8		;pixel size of one tile for sprite processor
.define SpriteTileSizeY	8		;pixel size of one tile for sprite processor

;*************************************************************************



;*************************************************************************
;exit & collision definitions
.define ExitFileSize	8
.define ExitTargetMap		ExitList & $ffff 
.define ExitSubroutine		ExitList & $ffff +1	;bit 7 indicates that an exit is present and is masked off when jumping. it can conveniently be checked by loading the first word and checking for the sign flag
.define ExitXPosition		ExitList & $ffff +2
.define ExitYPosition		ExitList & $ffff +3
.define ExitXTargetPosition		ExitList & $ffff +4
.define ExitYTargetPosition		ExitList & $ffff +5

;collidable objects definitions
.define ColObjFileSize	6
.define ColObjNumber		ColObjList & $ffff	;object number in object list
.define ColObjEnableFlagsSub	ColObjList & $ffff +1	;msb is enable flag, lower 7 bits are subroutine number
;.define ColObjSubroutine	ColObjList & $ffff +2
.define ColObjXPos		ColObjList & $ffff +2	;xpos/8
.define ColObjYPos		ColObjList & $ffff +4	;ypos/8

.define NumberOfCollidableObjects	32			;how far to check into object array to find collidable objects. slows down object processor alot when many people are walking
;*************************************************************************


;*************************************************************************
;hdma buffer definitions
.define HdmaBufferTableEntry	HdmaBuffer & $ffff
.define HdmaBufferFlags				HdmaBuffer & $ffff +1
.define HdmaBufferBBusTarget	HdmaBuffer & $ffff +2
.define HdmaBufferWriteConfig	HdmaBuffer & $ffff +3
.define HdmaBufferRomDataTbl	HdmaBuffer & $ffff +4
.define HdmaBufferRomCountTbl	HdmaBuffer & $ffff +5
.define HdmaBufferSubRout	HdmaBuffer & $ffff +6	
.define HdmaBufferRamDataPnt	HdmaBuffer & $ffff +7	;2 bytes
.define HdmaBufferRamCountPnt	HdmaBuffer & $ffff +9	;2 bytes
.define HdmaBufferEntryPnt	HdmaBuffer & $ffff +11	;2 bytes simple relative pointer to hdma table in ram to speed up processing
.define HdmaBufferRomDataPnt	HdmaBuffer & $ffff +13	;3 bytes
.define HdmaBufferRomDataLength	HdmaBuffer & $ffff +16	;3 bytes
.define HdmaBufferRomCountPnt	HdmaBuffer & $ffff +19	;3 bytes
.define HdmaBufferTable		HdmaBuffer & $ffff +24	;start of actual table

.define HdmaBufferSize		$340
.define HdmaEffectFileSize	6			;size of a hdma effect file in rom
.define HdmaBufferNumber	6

.define Hdma3dScrollLineNumber	80	;number of lines to process for this effect.
;*************************************************************************


;*************************************************************************
;Textbox menu definitions:
.define TextBoxMenuSubrout	TextBoxMenuBuffer & $ffff	;uses same subroutines as script commands
.define TextBoxMenuFlags	TextBoxMenuBuffer & $ffff + 1
.define TextBoxMenuParam1	TextBoxMenuBuffer & $ffff + 2
.define TextBoxMenuParam2	TextBoxMenuBuffer & $ffff + 3
.define TextBoxMenuParam3	TextBoxMenuBuffer & $ffff + 4
.define TextBoxMenuTilePos	TextBoxMenuBuffer & $ffff + 5	;x-pixelposition inside buffer
.define TextBoxMenuLinePos	TextBoxMenuBuffer & $ffff + 6	;y-pixelposition inside buffer
.define TextBoxMenuVoid		TextBoxMenuBuffer & $ffff + 7	;future expansion

.define TextBoxMenuEntrySize	8
;*************************************************************************


;*************************************************************************
;script commands ($00-$1f)
.define StringEnd		0	;end string and close textbox immediately
.define StringBreak		1
.define StringCls		2
.define StringButtonCls		3
.define StringFontChng		4	;takes one additional byte
.define StringEndButton		5	;wait for button press, then end string and close textbox
.define StringMenuEntry		6	;create menu entry. takes 4 parameters
.define StringMenuWait		7	;wait for button press/menu decision
.define StringLoadString	8	;immediately clear screen and load another textstring. takes 2 parameters(15bit textstring number)
.define StringLoadLevel		9	;immediately clear screen and load another level. takes 3 parameter(8bit level number, 8bit x-pos, 8bit y-pos target/8)
.define StringPlaySong		10	;upload & play a mod song. arg0: song number
.define StringGotoBattle	11	;goto battle mode. arg0: battle number arg1: event routine to load after returning from battle. arg2,3: vwf string to load after returning from battle. 0=don't load any textstring at all 
.define StringGotoSubstring	12	;goto substring, returnable, can be nested 16 times. arg0,1: string number(word)
.define StringSubstringReturn	13	;return to previous string, no argument
.define StringPrintDecimal 14 ;prints byte in decimal mode. uses substring in ram with return to print number. takes 4 parameter bytes, 3-byte adress of number to print, 1 byte for number length(valid values: 1-4). number can be 32bits long. (4294967295/$ffffffff max). if longer than 16bit, store low word first, then high word.
.define StringEndNoCls		15 ;same as end string, but dont clear screen. (used for statusbox)
.define StringSetPosition	16 ;changes tile draw position in vwf buffer. 1 argument, 0-128 valid
.define StringSetSpeed		17	;set drawing speed. 1 argument: 0=slowest, 7f=fastest. msb set: draw instantly
.define StringClsNoUpload	18	;clear screen, but don't upload vwf buffer so textbox doesnt flicker.
.define StringGotoEvent 	19	;reset to debug mode
.define StringCondGotoSub 20	;conditional goto substring. substring is only printed if all bits set in target bitmask are set in target byte. 6 arguments. arg0,1: substring number(word). arg2-4:target byte to check for condition. arg 5:bitmask to check for with target

.define VwfPlotBufferLength	18*16*6	;tile number * tile length * line count
.define VwfSpriteUpdFlagsLength	16	;10 sprites to update, upper 5 sprites at $0, lower 5 sprites at $8
;*************************************************************************


;*************************************************************************
;nwarp daisakusen specific stuff:
.define MaleHitpoints			8		;hp - health

.define SpasmToRespawn		20
.define DefaultBlockCounter 25		;number of frames to block after trigger

;player objects:
.define ObjEntryMaleDirection	ObjectList & $ffff + 28	;direction in which the object is currently facing
							;0=down 1=up 2=left 3=right
.define ObjEntryMaleHP		ObjectList & $ffff + 29	;amount of health this player has. 0=dead
.define ObjEntryMaleInvinc	ObjectList & $ffff + 30	;if not zero, player is invincible
.define ObjEntryMaleSpasmCount	ObjectList & $ffff + 30	;this is the revival counter. if a certain value is reached, player is revived with one hp.
.define ObjEntryMaleBlockCounter	ObjectList & $ffff + 31	;this is the revival counter. if a certain value is reached, player is revived with one hp.
.define OamTypeMainCharaCtrl		3		;main character top/controlling sprite
.define MainCharaHotspotSize	32			;size of mainchara hotspot square in pixels
.define NPCHotspotSizeX	14				;size of walking player hotspot square in pixels
.define NPCHotspotSizeY	20				;size of walking player hotspot square in pixels
.define NPCHitPointSizeX 12
.define NPCHitPointSizeY 20


;hdma gradient colors
.define CreditsGradientColor	247	;cgram entry for color gradient
.define TextBoxGradientColor	113	;cgram entry for color gradient
.define BattleBoxGradientColor	113	;cgram entry for color gradient
;*************************************************************************


