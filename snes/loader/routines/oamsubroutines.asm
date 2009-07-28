AniSubroutineJumpLUT:
	.dw AniSubroutineVoid		;0
	.dw AniSubroutineCpuUsage


AniSubroutineCpuUsage:
	sep #$20
	ldx.b ObjectListPointerCurrent
	lda.w CpuUsageScanline
	asl a												;add subpixel precision
	asl a
	asl a
	asl a	
	sta.w ObjEntryYPos,x
	lda.b #16*16-1								;*16=subpixel precision
	sta.w ObjEntryXPos,x
	rts

AniSubroutineVoid:
	rts	
