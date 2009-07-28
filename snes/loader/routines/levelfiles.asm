
.Section "LevelLUT" superfree
LevelLUT:
	.dw Map1 & $ffff
	.db (:Map1+BaseAdress>>16)
	.dw Map2 & $ffff
	.db (:Map2+BaseAdress>>16)
	.dw Map3 & $ffff
	.db (:Map3+BaseAdress>>16)
	.dw Map4 & $ffff
	.db (:Map4+BaseAdress>>16)
	.dw Map5 & $ffff
	.db (:Map5+BaseAdress>>16)
	.dw Map6 & $ffff
	.db (:Map6+BaseAdress>>16)
	.dw Map7 & $ffff
	.db (:Map7+BaseAdress>>16)
	.dw Map8 & $ffff
	.db (:Map8+BaseAdress>>16)
	.dw Map9 & $ffff
	.db (:Map9+BaseAdress>>16)
.ends

.Section "level file 1" superfree
Map1:
	.db 28		;size in tiles, x
	.db 26		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map1Pal-Map1	;relative pointer(16bit) to palette
	.dw Map1Col-Map1	;relative pointer(16bit) to collision map
	.dw Map1Tile-Map1	;relative pointer(16bit) to image
	.dw Map1EOF	;direct pointer(24bit) to EOF
	.db (:Map1EOF)-(:Map1)+$c0
	.dw Map1Exits-Map1
;from here, total 19 additional bytes:
	.dw Map1Objs-Map1
Map1Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map1Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8
*/
;male 
	.db 4		;object number
	.db $80		;obj present flag
	.db 7+2		;x-position/8
	.db 6		;y-position/8
;male 
	.db 5		;object number
	.db $80		;obj present flag
	.db 10+2		;x-position/8
	.db 8		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 11+2		;x-position/8
	.db 11		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 25+2		;x-position/8
	.db 12		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 22+2		;x-position/8
	.db 14		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 17+2		;x-position/8
	.db 6		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 12+2		;x-position/8
	.db 16		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 18		;y-position/8	
	
											
;obj list terminator:
	.dw 0

Map1Pal:
	.incbin "data/levels/selectmap.clr" READ 124*2			;leave last 4 colors for onscreen text
Map1Col:
	.incbin "data/levels/selectmapcoli.bmp" SKIP $3e
Map1Tile:
	.incbin "data/levels/selectmap.pic" ;READ $7eb2		;$8000+32000+496


	
Map1EOF:
.ends


.Section "level file 2" superfree
Map2:
	.db 28		;size in tiles, x
	.db 26		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map2Pal-Map2	;relative pointer(16bit) to palette
	.dw Map2Col-Map2	;relative pointer(16bit) to collision map
	.dw Map2Tile-Map2	;relative pointer(16bit) to image
	.dw Map2EOF	;direct pointer(24bit) to EOF
	.db (:Map2EOF)-(:Map2)+$c0
	.dw Map2Exits-Map2
;from here, total 19 additional bytes:
	.dw Map2Objs-Map2
Map2Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map2Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8
*/
;male 
	.db 4		;object number
	.db $80		;obj present flag
	.db 10+2		;x-position/8
	.db 18		;y-position/8
;male 
	.db 5		;object number
	.db $80		;obj present flag
	.db 7+2		;x-position/8
	.db 20		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 2+2		;x-position/8
	.db 17		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 2+2		;x-position/8
	.db 13		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 9		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 5+2		;x-position/8
	.db 6		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 8+2		;x-position/8
	.db 4		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 19		;y-position/8	
	
											
;obj list terminator:
	.dw 0

Map2Pal:
	.incbin "data/levels/resultsmap.clr"
Map2Col:
;	.incbin "data/levels/map1coli.bmp" SKIP $3e
Map2Tile:
	.incbin "data/levels/resultsmap.pic" ;READ $7eb2		;$8000+32000+496
Map2EOF:
.ends


.Section "level file 3" superfree
Map3:
	.db 28		;size in tiles, x
	.db 26		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map3Pal-Map3	;relative pointer(16bit) to palette
	.dw Map3Col-Map3	;relative pointer(16bit) to collision map
	.dw Map3Tile-Map3	;relative pointer(16bit) to image
	.dw Map3EOF	;direct pointer(24bit) to EOF
	.db (:Map3EOF)-(:Map3)+$c0
	.dw Map3Exits-Map3
;from here, total 19 additional bytes:
	.dw Map3Objs-Map3
Map3Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map3Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8
*/
;male 
	.db 4		;object number
	.db $80		;obj present flag
	.db 2+2		;x-position/8
	.db 6		;y-position/8
;male 
	.db 5		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 6		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 6+2		;x-position/8
	.db 6		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 8+2		;x-position/8
	.db 8		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 10+2		;x-position/8
	.db 10		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 12+2		;x-position/8
	.db 12		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 14+2		;x-position/8
	.db 14		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 16		;y-position/8	
	
;healthmeter
	.db 12		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 1		;y-position/8	

;healthmeter
	.db 13		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 14		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 1		;y-position/8
	
;healthmeter
	.db 15		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 16		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 21		;y-position/8

;healthmeter
	.db 17		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 18		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 19		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 21		;y-position/8												
;obj list terminator:
	.dw 0

Map3Pal:
;	.incbin "data/levels/map3.clr"
Map3Col:
;	.incbin "data/levels/map3coli.bmp" SKIP $3e
Map3Tile:
;	.incbin "data/levels/map3.pic" ;READ $7eb2		;$8000+32000+496
Map3EOF:	
.ends


.Section "level file 4" superfree
Map4:
	.db 28		;size in tiles, x
	.db 26		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map4Pal-Map4	;relative pointer(16bit) to palette
	.dw Map4Col-Map4	;relative pointer(16bit) to collision map
	.dw Map4Tile-Map4	;relative pointer(16bit) to image
	.dw Map4EOF	;direct pointer(24bit) to EOF
	.db (:Map4EOF)-(:Map4)+$c0
	.dw Map4Exits-Map4
;from here, total 19 additional bytes:
	.dw Map4Objs-Map4
Map4Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map4Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8
*/
;male 
	.db 4		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 5		;y-position/8
;male 
	.db 5		;object number
	.db $80		;obj present flag
	.db 18+2		;x-position/8
	.db 5		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 14		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 18+2		;x-position/8
	.db 12		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 20		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 19		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 18		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 23+2		;x-position/8
	.db 19		;y-position/8	
	
;healthmeter
	.db 12		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 1		;y-position/8	

;healthmeter
	.db 13		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 14		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 1		;y-position/8
	
;healthmeter
	.db 15		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 16		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 21		;y-position/8

;healthmeter
	.db 17		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 18		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 19		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 21		;y-position/8												
;obj list terminator:
	.dw 0

Map4Pal:
	.incbin "data/levels/map9.clr"
Map4Col:
	.incbin "data/levels/map9coli.bmp" SKIP $3e
Map4Tile:
	.incbin "data/levels/map9.pic" ;READ $7eb2		;$8000+32000+496
Map4EOF:	
.ends


.Section "level file 5" superfree
Map5:
	.db 28		;size in tiles, x
	.db 26		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map5Pal-Map5	;relative pointer(16bit) to palette
	.dw Map5Col-Map5	;relative pointer(16bit) to collision map
	.dw Map5Tile-Map5	;relative pointer(16bit) to image
	.dw Map5EOF	;direct pointer(24bit) to EOF
	.db (:Map5EOF)-(:Map5)+$c0
	.dw Map5Exits-Map5
;from here, total 19 additional bytes:
	.dw Map5Objs-Map5
Map5Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map5Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8
*/
;male 
	.db 4		;object number
	.db $80		;obj present flag
	.db 5+2		;x-position/8
	.db 17		;y-position/8
;male 
	.db 5		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 14		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 5+2		;x-position/8
	.db 9		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 7		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 14+2		;x-position/8
	.db 6		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 20+2		;x-position/8
	.db 7		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 23+2		;x-position/8
	.db 10		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 24+2		;x-position/8
	.db 14		;y-position/8	
	
;healthmeter
	.db 12		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 1		;y-position/8	

;healthmeter
	.db 13		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 14		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 1		;y-position/8
	
;healthmeter
	.db 15		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 16		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 21		;y-position/8

;healthmeter
	.db 17		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 18		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 19		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 21		;y-position/8												
;obj list terminator:
	.dw 0

Map5Pal:
	.incbin "data/levels/map5.clr"
Map5Col:
	.incbin "data/levels/map5coli.bmp" SKIP $3e
Map5Tile:
	.incbin "data/levels/map5.pic" ;READ $7eb2		;$8000+32000+496
Map5EOF:
.ends


.Section "level file 6" superfree
Map6:
	.db 28		;size in tiles, x
	.db 26		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map6Pal-Map6	;relative pointer(16bit) to palette
	.dw Map6Col-Map6	;relative pointer(16bit) to collision map
	.dw Map6Tile-Map6	;relative pointer(16bit) to image
	.dw Map6EOF	;direct pointer(24bit) to EOF
	.db (:Map6EOF)-(:Map6)+$c0
	.dw Map6Exits-Map6
;from here, total 19 additional bytes:
	.dw Map6Objs-Map6
Map6Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map6Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8
*/
;male 
	.db 4		;object number
	.db $80		;obj present flag
	.db 8+2		;x-position/8
	.db 8		;y-position/8
;male 
	.db 5		;object number
	.db $80		;obj present flag
	.db 12+2		;x-position/8
	.db 8		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 8		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 20+2		;x-position/8
	.db 8		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 19		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 10+2		;x-position/8
	.db 19		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 17+2		;x-position/8
	.db 19		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 24+2		;x-position/8
	.db 19		;y-position/8	
	
;healthmeter
	.db 12		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 1		;y-position/8	

;healthmeter
	.db 13		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 14		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 1		;y-position/8
	
;healthmeter
	.db 15		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 16		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 21		;y-position/8

;healthmeter
	.db 17		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 18		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 19		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 21		;y-position/8												
;obj list terminator:
	.dw 0

Map6Pal:
	.incbin "data/levels/map6.clr"
Map6Col:
	.incbin "data/levels/map6coli.bmp" SKIP $3e
Map6Tile:
	.incbin "data/levels/map6.pic" ;READ $7eb2		;$8000+32000+496
Map6EOF:	
.ends


.Section "level file 7" superfree
Map7:
	.db 28		;size in tiles, x
	.db 26		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map7Pal-Map7	;relative pointer(16bit) to palette
	.dw Map7Col-Map7	;relative pointer(16bit) to collision map
	.dw Map7Tile-Map7	;relative pointer(16bit) to image
	.dw Map7EOF	;direct pointer(24bit) to EOF
	.db (:Map7EOF)-(:Map7)+$c0
	.dw Map7Exits-Map7
;from here, total 19 additional bytes:
	.dw Map7Objs-Map7
Map7Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map7Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8
*/
;male 
	.db 4		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 15		;y-position/8
;male 
	.db 5		;object number
	.db $80		;obj present flag
	.db 5+2		;x-position/8
	.db 20		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 10+2		;x-position/8
	.db 20		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 14+2		;x-position/8
	.db 4		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 7		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 17+2		;x-position/8
	.db 10		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 20+2		;x-position/8
	.db 13		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 24+2		;x-position/8
	.db 15		;y-position/8	
	
;healthmeter
	.db 12		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 1		;y-position/8	

;healthmeter
	.db 13		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 14		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 1		;y-position/8
	
;healthmeter
	.db 15		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 16		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 21		;y-position/8

;healthmeter
	.db 17		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 18		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 19		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 21		;y-position/8												
;obj list terminator:
	.dw 0

Map7Pal:
	.incbin "data/levels/map7.clr"
Map7Col:
	.incbin "data/levels/map7coli.bmp" SKIP $3e
Map7Tile:
	.incbin "data/levels/map7.pic" ;READ $7eb2		;$8000+32000+496
Map7EOF:	
.ends




.bank 12 slot 0
.org $0
.Section "nightsky"


Map8:
	.db 28		;size in tiles, x
	.db 64		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map8Pal-Map8	;relative pointer(16bit) to palette
	.dw Map8Col-Map8	;relative pointer(16bit) to collision map
	.dw Map8Tile-Map8	;relative pointer(16bit) to image
	.dw Map8EOF	;direct pointer(24bit) to EOF
	.db (:Map8EOF)-(:Map8)+$c0
	.dw Map8Exits-Map8
;from here, total 19 additional bytes:
	.dw Map8Objs-Map8
Map8Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map8Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8

;mond 1 
	.db 20		;object number
	.db $00		;obj present flag
	.db 3+2		;x-position/8
	.db 15		;y-position/8
;male 
	.db 5		;object number
	.db $00		;obj present flag
	.db 5+2		;x-position/8
	.db 20		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 10+2		;x-position/8
	.db 23		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 14+2		;x-position/8
	.db 3		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 7		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 17+2		;x-position/8
	.db 10		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 20+2		;x-position/8
	.db 13		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 24+2		;x-position/8
	.db 15		;y-position/8	
	
;healthmeter
	.db 12		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 1		;y-position/8	

;healthmeter
	.db 13		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 14		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 1		;y-position/8
	
;healthmeter
	.db 15		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 16		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 21		;y-position/8

;healthmeter
	.db 17		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 18		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 19		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 21		;y-position/8	
*/												
;obj list terminator:
	

Map8Pal:
	.incbin "data/levels/nightsky.clr"
	
	
Map8Col:
;	.incbin "data/levels/map8coli.bmp" SKIP $3e

.incbin "data/levels/dummy.bin"	
.dw 0
.dw 0
Map8Tile:
	.incbin "data/levels/nightsky.pic" READ $f500		;$8000+32000+496

.ends

.bank 13 slot 0
.org $0
.Section "nightsky 2"

	.incbin "data/levels/nightsky.pic" SKIP $f500 READ $cb00	;$8000+32000+496

Map8EOF:	
.ends




.Section "level file 9" superfree
Map9:
	.db 28		;size in tiles, x
	.db 26		;size in tiles, y
	.db 0		;background color,r
	.db 0		;background color,g
	.db 0		;background color,b
	.dw Map9Pal-Map9	;relative pointer(16bit) to palette
	.dw Map9Col-Map9	;relative pointer(16bit) to collision map
	.dw Map9Tile-Map9	;relative pointer(16bit) to image
	.dw Map9EOF	;direct pointer(24bit) to EOF
	.db (:Map9EOF)-(:Map9)+$c0
	.dw Map9Exits-Map9
;from here, total 19 additional bytes:
	.dw Map9Objs-Map9
Map9Exits:
;exit 0:	

;exit list terminator:	
	.dw $0

Map9Objs:
/*
;cpu usage
	.db 1		;object number
	.db $80		;obj present flag
	.db 16+2		;x-position/8
	.db 00		;y-position/8
*/
;male 
	.db 4		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 2		;y-position/8
;male 
	.db 5		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 6		;y-position/8
;male 
	.db 6		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 10		;y-position/8
;male 
	.db 7		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 14		;y-position/8
;male 
	.db 8		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 18		;y-position/8		
;male 
	.db 9		;object number
	.db $80		;obj present flag
	.db 4+2		;x-position/8
	.db 22		;y-position/8	
;male 
	.db 10		;object number
	.db $80		;obj present flag
	.db 6+2		;x-position/8
	.db 18		;y-position/8	
;male 
	.db 11		;object number
	.db $80		;obj present flag
	.db 6+2		;x-position/8
	.db 22		;y-position/8	
	
;healthmeter
	.db 12		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 1		;y-position/8	

;healthmeter
	.db 13		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 14		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 1		;y-position/8
	
;healthmeter
	.db 15		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 1		;y-position/8

;healthmeter
	.db 16		;object number
	.db $80		;obj present flag
	.db 3+2		;x-position/8
	.db 21		;y-position/8

;healthmeter
	.db 17		;object number
	.db $80		;obj present flag
	.db 9+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 18		;object number
	.db $80		;obj present flag
	.db 15+2		;x-position/8
	.db 21		;y-position/8
	
;healthmeter
	.db 19		;object number
	.db $80		;obj present flag
	.db 21+2		;x-position/8
	.db 21		;y-position/8												
;obj list terminator:
	.dw 0

Map9Pal:
	.incbin "data/levels/debugmap.clr"
Map9Col:
	.incbin "data/levels/debugmap.bmp" SKIP $3e
	.db 0
Map9Tile:
	.incbin "data/levels/debugmap.pic" ;READ $7eb2		;$8000+32000+496
Map9EOF:	
	
.ends
