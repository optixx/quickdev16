.Section "BGmode" superfree
BgModeLut:
	.dw BgModeTable0
	.dw BgModeTable1
	.dw BgModeTable2
	.dw BgModeTable3
	.dw BgModeTable4
	.dw BgModeTable5
	.dw BgModeTable6
	.dw BgModeTable7
	.dw BgModeTable8


;normal mode1 16color background mode. debug menu
BgModeTable0:
	.db $00				;00	SetIni		Hires screen settings 			(reg $2133)
	.db %00001001				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010100			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00000000			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %00000000				;04	CGWsel		color add/sub settings 		(reg $2130)
	.db %00111111				;05	CgadsubConfig	color add/sub settings 2 	(reg $2131)
	.db $25				;06	BGTilesVram12	bg1/bg2 tile base adress 		(reg $210b)
	.db $01				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db $00				;08	BG1TilemapVram	bg1 screen size and tilemap base adress (reg $2107)
	.db $08				;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $10				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $18				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %00100011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $00				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00000000			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00100000			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 1				;17	W1L		$2126 window 1 left pos
	.db 0				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00000000			;23	WMS		$212e window main screen designation
	.db %00000000			;24	WSS		$212f window sub screen designation

;normal levels. player select, battle level, results screen
;mode3 256color level mode with overscan and irq cutoff for dma transfers with sprites. sprites=8x8 + 32x32
BgModeTable1:
	.db %00000100			;00	SetIni		Hires/overscan screen settings 		(reg $2133)
	.db $03				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010001			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00010001			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %10100000			;04	CGWsel		color add/sub settings 			(reg $2130)
	.db $00				;05	CgadsubConfig	color add/sub settings 2 		(reg $2131)
	.db $70				;06	BGTilesVram12	bg1/bg2 tile base adress 		(reg $210b)
	.db $00				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db $5c				;08	BG1TilemapVram	bg1 screen size and tilemap base adress (reg $2107)
	.db $7c				;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $00				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $00				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %00100011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $01				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00000011			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00000011			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 16				;17	W1L		$2126 window 1 left pos
	.db 239				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00010001			;23	WMS		$212e window main screen designation
	.db %00000000			;24	WSS		$212f window sub screen designation

;battles	
;mode3 256color level mode with overscan and irq cutoff for dma transfers with sprites. sprites=8x8 + 32x32
BgModeTable2:
	.db %00000100			;00	SetIni		Hires/overscan screen settings 		(reg $2133)
	.db $03				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010011			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00010011			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %10100000			;04	CGWsel		color add/sub settings 			(reg $2130)
	.db $00				;05	CgadsubConfig	color add/sub settings 2 		(reg $2131)
	.db $40				;06	BGTilesVram12	bit0-3:bg1/bit4-7:bg2 tile base adress	(reg $210b)
	.db $00				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db %1011001			;08	BG1TilemapVram	bg1 screen size and tilemap base adress (reg $2107)
	.db %111100			;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $00				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $00				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %00100011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $01				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00110011			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00110011			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 16				;17	W1L		$2126 window 1 left pos
	.db 239				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00010011			;23	WMS		$212e window main screen designation
	.db %00000000			;24	WSS		$212f window sub screen designation

;intro scene 2, moon with corona
;mode3 256color level mode with overscan and irq cutoff for dma transfers with sprites. sprites=8x8 + 64x64
BgModeTable3:
	.db %00000100			;00	SetIni		Hires/overscan screen settings 		(reg $2133)
	.db $03				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010000			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00010001			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %00000010			;04	CGWsel		color add/sub settings 			(reg $2130)
	.db %00110001			;05	CgadsubConfig	color add/sub settings 2 		(reg $2131)
	.db $00				;06	BGTilesVram12	bg1/bg2 tile base adress 		(reg $210b)
	.db $00				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db $5c				;08	BG1TilemapVram	bg1 screen size and tilemap base adress (reg $2107)
	.db $00				;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $00				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $00				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %01000011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $01				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00000011			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00000011			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 16				;17	W1L		$2126 window 1 left pos
	.db 239				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00010001			;23	WMS		$212e window main screen designation
	.db %00010001			;24	WSS		$212f window sub screen designation

;titlescreen	
;mode3 256color level mode with overscan and irq cutoff for dma transfers with sprites. sprites=8x8 + 32x32
BgModeTable4:
	.db %00000100			;00	SetIni		Hires/overscan screen settings 		(reg $2133)
	.db $03				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010011			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00010011			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %10100000			;04	CGWsel		color add/sub settings 			(reg $2130)
	.db %10100001				;05	CgadsubConfig	color add/sub settings 2 		(reg $2131)
	.db $50				;06	BGTilesVram12	bit0-3:bg1/bit4-7:bg2 tile base adress	(reg $210b)
	.db $00				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db %01011100			;08	BG1TilemapVram	bg1 screen size and tilemap base adress (reg $2107)
	.db %01001100			;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $00				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $00				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %01000011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $01				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00110011			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00110011			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 16				;17	W1L		$2126 window 1 left pos
	.db 239				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00010011			;23	WMS		$212e window main screen designation
	.db %00000000			;24	WSS		$212f window sub screen designation
	
;normal mode1 16color background mode. gra logo
BgModeTable5:
	.db $00				;00	SetIni		Hires screen settings 			(reg $2133)
	.db %00001001				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010001			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00000000			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %00000000				;04	CGWsel		color add/sub settings 		(reg $2130)
	.db %00110011				;05	CgadsubConfig	color add/sub settings 2 	(reg $2131)
	.db $25				;06	BGTilesVram12	bg1/bg2 tile base adress 		(reg $210b)
	.db $01				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db $00				;08	BG1TilemapVram	bg1 screen size and tilemap base adress (reg $2107)
	.db $08				;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $10				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $18				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %01000011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $01				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00000000			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00100000			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 1				;17	W1L		$2126 window 1 left pos
	.db 0				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00000000			;23	WMS		$212e window main screen designation
	.db %00000000			;24	WSS		$212f window sub screen designation


;titlescreen flash	
;mode3 256color level mode with overscan and irq cutoff for dma transfers with sprites. sprites=8x8 + 32x32
BgModeTable6:
	.db %00000100			;00	SetIni		Hires/overscan screen settings 		(reg $2133)
	.db $03				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010011			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00010011			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %10100000			;04	CGWsel		color add/sub settings 			(reg $2130)
	.db %00111111				;05	CgadsubConfig	color add/sub settings 2 		(reg $2131)
	.db $50				;06	BGTilesVram12	bit0-3:bg1/bit4-7:bg2 tile base adress	(reg $210b)
	.db $00				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db %01011100			;08	BG1TilemapVram	bg1 screen size and tilemap base adress (reg $2107)
	.db %01001100			;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $00				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $00				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %01000011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $01				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00110011			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00110011			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 16				;17	W1L		$2126 window 1 left pos
	.db 239				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00010011			;23	WMS		$212e window main screen designation
	.db %00000000			;24	WSS		$212f window sub screen designation	

;16 color videos, 256 cc background 64x32 bg1
BgModeTable7:
	.db $00				;00	SetIni		Hires screen settings 			(reg $2133)
	.db %00001011				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010011			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00000000			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %00000000				;04	CGWsel		color add/sub settings 		(reg $2130)
	.db %00110011				;05	CgadsubConfig	color add/sub settings 2 	(reg $2131)
	.db $12				;06	BGTilesVram12	bg1/bg2 tile base adress 		(reg $210b)
	.db $00				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db $01				;08	BG1TilemapVram	bg1 screen size and tilemap base adress aaaaaayx (reg $2107)
	.db $0c				;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $00				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $00				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %00100011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $01				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00000000			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00100000			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 1				;17	W1L		$2126 window 1 left pos
	.db 0				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00000000			;23	WMS		$212e window main screen designation
	.db %00000000			;24	WSS		$212f window sub screen designation	
	
;16 color videos, 256 cc background 32x32 bg1, credits
BgModeTable8:
	.db $00				;00	SetIni		Hires screen settings 			(reg $2133)
	.db %00001011				;01	ScreenMode	screen mode/bg tilesize/bg3 priority 	(reg $2105)
	.db %00010011			;02	MainScreen	mainscreen designation 			(reg $212c)
	.db %00000000			;03	SubScreen	subscreen designation 			(reg $212d)
	.db %00000000				;04	CGWsel		color add/sub settings 		(reg $2130)
	.db %00110011				;05	CgadsubConfig	color add/sub settings 2 	(reg $2131)
	.db $12				;06	BGTilesVram12	bg1/bg2 tile base adress 		(reg $210b)
	.db $00				;07	BGTilesVram34	bg3/bg4 tile base adress 		(reg $210c)
	.db $00				;08	BG1TilemapVram	bg1 screen size and tilemap base adress aaaaaayx (reg $2107)
	.db $0c				;09	BG2TilemapVram	bg2 screen size and tilemap base adress (reg $2107)
	.db $00				;10	BG3TilemapVram	bg3 screen size and tilemap base adress (reg $2107)
	.db $00				;11	BG4TilemapVram	bg4 screen size and tilemap base adress (reg $2107)
	.db %01000011			;12	ObjSel		Sprite size, name and base adress 	(reg $2101)
	.db $01				;13	IrqRoutineNumber number of irq routine to use. 0 means irq disabled
	.db %00000000			;14	W12SEL		$2123 window enable/inverse bg1,2
	.db $00				;15	W34SEL		$2124 window enable/inverse bg3,4
	.db %00100000			;16	WOBJSEL		$2125 window enable/inverse obj/col
	.db 1				;17	W1L		$2126 window 1 left pos
	.db 0				;18	W1R		$2127 window 1 right pos
	.db $00				;19	W2L		$2128 window 2 left pos
	.db $00				;20	W2R		$2129 window 2 right pos
	.db %00000000			;21	WBGLOG		$212a window mask logic bg
	.db $00				;22	WOBJLOG		$212b window mask logic obj/col
	.db %00000000			;23	WMS		$212e window main screen designation
	.db %00000000			;24	WSS		$212f window sub screen designation		
	
.ends	