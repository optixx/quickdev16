	








.Section "Misc Data 1" superfree
;mosaic and hdma count value table used when fading bg1&2 in/out:
Bg12FadeLUT:
;0
	.db 2				;window count value (1-99)
	.db %00000011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void
;1
	.db 3				;window count value (1-99)
	.db %00010011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;2
	.db 4				;window count value (1-99)
	.db %00100011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;3
	.db 6				;window count value (1-99)
	.db %00110011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;4
	.db 8				;window count value (1-99)
	.db %01000011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;5
	.db 11				;window count value (1-99)
	.db %01010011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;6
	.db 15				;window count value (1-99)
	.db %01100011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;7
	.db 18				;window count value (1-99)
	.db %01110011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;8
	.db 23				;window count value (1-99)
	.db %10000011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;9
	.db 27				;window count value (1-99)
	.db %10010011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;10
	.db 35				;window count value (1-99)
	.db %10100011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;11
	.db 45				;window count value (1-99)
	.db %10110011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;12
	.db 57				;window count value (1-99)
	.db %11000011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;13
	.db 70				;window count value (1-99)
	.db %11010011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111111			;and value with mainscreen settings
	.db 0				;void

;14
	.db 88				;window count value (1-99)
	.db %11100011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111100			;and value with mainscreen settings
	.db 0				;void

;15
	.db 99				;window count value (1-99)
	.db %11110011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111100			;and value with mainscreen settings
	.db 0				;void

;16
	.db 99				;window count value (1-99)
	.db %11110011			;mosaic value (xxxx4321, x is mosaic size)
	.db %11111100			;and value with mainscreen settings
	.db 0				;void
.ends

.Section "MemoryViewLUT" superfree
MemoryViewerLetterLUT:
	.db $f0
	.db $f1
	.db $f2	
	.db $f3	
	.db $f4	
	.db $f5	
	.db $f6	
	.db $f7	
	.db $f8	
	.db $f9	
	.db $b0	
	.db $b1	
	.db $b2	
	.db $b3	
	.db $b4	
	.db $b5
.ends	


.Section "ScreenFocusLUT" superfree
FocusScreenSplineLut:
	.dw FocusScreenSpline0
	.dw FocusScreenSpline1
	.dw FocusScreenSpline2
	.dw FocusScreenSpline3
	.dw FocusScreenSpline4
	.dw FocusScreenSpline5
	.dw FocusScreenSpline6
	.dw FocusScreenSpline7			;linear ramp

FocusScreenSpline0:
FocusScreenSpline1:
FocusScreenSpline2:
FocusScreenSpline3:
FocusScreenSpline4:
FocusScreenSpline5:
FocusScreenSpline6:
FocusScreenSpline7:
	.db $60
	.db $60
	.db $60
	.db $60
	.db $50
	.db $50
	.db $50
	.db $50
	.db $40
	.db $40
	.db $40
	.db $40
	.db $30
	.db $30
	.db $30
	.db $30
	.db $20
	.db $20
	.db $20
	.db $20
	.db $10
	.db $10
	.db $10
	.db $10
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 1
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 2
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 3
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 4
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 5
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 6
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
	.db 7
.ends

.Section "blanktile" superfree
BlankTile:
	.incbin "data/blanktile.pic"
.ends	

/*
this is the table that defines the transfer length and number for the different sprites.
bit0-3 of adress: obj size designation
bit4 of adress: objs own size flag
1st byte of data: transfer length
2nd byte of data: transfer number

xsize:	ysize:	number of lines to upload:	number of bytes to transfer per line:
8	8	1				32
16	16	2				64
32	32	4				128
64	64	8				256
16	32	4				64
32	64	8				128
*/
.Section "ObjsizeLUT" superfree
ObjSizeLUT:

;8x8
	.db 31		;transfer length
	.db 1		;number of transfers
	.db 1		;transfer type
;8x8
	.db 31
	.db 1
	.db 1
;8x8
	.db 31
	.db 1
	.db 1
;16x16
	.db 63
	.db 2
	.db 3
;16x16
	.db 63
	.db 2
	.db 3
;32x32
	.db 127
	.db 4
	.db 4
;16x32
	.db 63
	.db 4
	.db 0		;not supported atm
;16x32
	.db 63
	.db 4
	.db 0		;not supported atm	
;size flag=1
;16x16
	.db 63
	.db 2
	.db 3
;32x32
	.db 127
	.db 4
	.db 4
;64x64
	.db 255
	.db 8
	.db 5
;32x32
	.db 127
	.db 4
	.db 4
;64x64
	.db 255
	.db 8
	.db 5
;64x64
	.db 255
	.db 8
	.db 5
;32x64
	.db 127
	.db 8
	.db 0		;not supported atm	
;32x32
	.db 127
	.db 4
	.db 4
.ends



.Section "Fonts" superfree
Bg38x8FontLUT:
	.dw Font0
	.dw Font0End-Font0
	.dw Font1
	.dw Font1End-Font1
	.dw Font2
	.dw Font2End-Font2
	.dw Font3
	.dw Font3End-Font3

Font0:
	.INCBIN "data/font/font1.pic"
Font0End:	
Font1:
;	.INCBIN "data/font2.pic"
Font1End:	
Font2:
;	.INCBIN "data/font2.pic"
Font2End:	
Font3:
;	.INCBIN "data/font2.pic"
Font3End:	


BG38x8FontPalette:
	.INCBIN "data/font/font1pal.clr" READ 32
BG38x8FontPaletteEnd:

.ends

.Section "ascii tables" superfree
ASCIITable:
	.incbin "data/font/asciitablejap.tbl"
	
ASCIITable8x16:
;	.incbin "data/font/asciitable8x16.tbl"
.ends






	
	
.Section "Audio Player" superfree
PtplayerSpcCode:
	.dw (PtplayerSpcCodeEnd-PtplayerSpcCode-2)
	
	.incbin "data/apu/apucode.bin"			
	
PtplayerSpcCodeEnd:
	.dw $0000		;termination code
	.dw $0000
	.incbin "data/apu/apucode.bin" READ 2		;spc start adress
.ends	




	

.section "general tilesets" superfree
GeneralTilesetsLUT:
		.dw GeneralTileset0
		.db (:GeneralTileset0+BaseAdress>>16)
		.dw (GeneralTileset0End-GeneralTileset0)
		.dw GeneralTileset1
		.db (:GeneralTileset1+BaseAdress>>16)
		.dw (GeneralTileset1End-GeneralTileset1)
		.dw GeneralTileset2
		.db (:GeneralTileset2+BaseAdress>>16)
		.dw (GeneralTileset2End-GeneralTileset2)

.ends

.section "general tileset 2" superfree
GeneralTileset0:
GeneralTileset0End:
GeneralTileset1:
GeneralTileset1End:
GeneralTileset2:
	.incbin "data/blanktile.pic"
GeneralTileset2End:
.ends





.section "hex2dec lut" superfree
VwfHex2DecLUT:
	.db    $0, $0, $0, $0, $0, $1,  $0, $0, $0, $0, $0, $2,  $0, $0, $0, $0, $0, $4,  $0, $0, $0, $0, $0, $8
 	.db    $0, $0, $0, $0, $0,$16,  $0, $0, $0, $0, $0,$32,  $0, $0, $0, $0, $0,$64,  $0, $0, $0, $0, $1,$28
  .db    $0, $0, $0, $0, $2,$56,  $0, $0, $0, $0, $5,$12,  $0, $0, $0, $0,$10,$24,  $0, $0, $0, $0,$20,$48
  .db    $0, $0, $0, $0,$40,$96,  $0, $0, $0, $0,$81,$92,  $0, $0, $0, $1,$63,$84,  $0, $0, $0, $3,$27,$68

	.db    $0, $0, $0, $6,$55,$36,  $0, $0, $0,$13,$10,$72,  $0, $0, $0,$26,$21,$44,  $0, $0, $0,$52,$42,$88
 	.db    $0, $0, $1,$04,$85,$76,  $0, $0, $2,$09,$71,$52,  $0, $0, $4,$19,$43,$04,  $0, $0, $8,$38,$86,$08
  .db    $0, $0,$16,$77,$72,$16,  $0, $0,$33,$55,$44,$32,  $0, $0,$67,$10,$88,$64,  $0, $1,$34,$21,$77,$28
  .db    $0, $2,$68,$43,$54,$56,  $0, $5,$36,$87,$09,$12,  $0,$10,$73,$74,$18,$24,  $0,$21,$47,$48,$36,$48

.ends  






.Section "playerselectscroll" superfree	
PlayerSelectScrollCounterTable:
	.dw 2
	.dw 2
	.dw 1
	.dw 1
	.dw 0						;wait for "hit any button"
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 1
	.dw 1
	.dw 2
	.dw 2
	.dw 2
	.dw 2
	.dw 2
		
	.dw 2
	.dw 2
	.dw 1
	.dw 1	
	.dw 0						;wait for "waiting for challengers"
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 1
	.dw 1
	.dw 2
	.dw 2
	.dw 2
.ends	