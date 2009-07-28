.Section "sprite data" superfree
;24bit pointers to sprite tilesets + 16bit length
SpriteTilesetLUT:
	.dw SpriteTileset0
	.db :SpriteTileset0+$c0

			
;16bit pointers to sprite palettes + 16bit length
SpritePaletteLUT:	
	.dw SpritePalette0-SpritePaletteLUT
	.dw (SpritePalette0End-SpritePalette0)
		
		
SpritePalette0:
	;.incbin "data/cpuusage.clr" READ 32
    .db 0
SpritePalette0End:


.ends



.Section "spritetileset 0" superfree
SpriteTileset0:
	;.incbin "data/cpuusage.pic"	
    .db 0
.ends
