.Section "SongLUT" superfree
PtPlayerSongPointertable:
		.dw Song0
		.db (:Song0+BaseAdress>>16)
				

.ends	

.Section "song 0" superfree
Song0:
	.dw (Song0End-Song0)
 .incbin "data/songs/maf_atomanic_4_hi.bin"
;	.incbin "data/songs/maf - atomaniac en rab.bin"
Song0End:
.ends
