
.Section "BackgroundfileLUT" superfree	
UploadBackgroundFileLUT:
	.dw BackgroundFile0 & $ffff
	.db (:BackgroundFile0 + BaseAdress>>16)
.ends

.Section "background file 0" superfree
;battlefile 0 sky	
BackgroundFile0:
	.dw BackgroundFile0Tiles-BackgroundFile0
	.dw BackgroundFile0Tilemap-BackgroundFile0
	.dw BackgroundFile0Palette-BackgroundFile0
	.dw BackgroundFile0EOF-BackgroundFile0
BackgroundFile0Tiles:
;	.incbin "data/battle_sky.pic"
BackgroundFile0Tilemap:
;	.incbin "data/battle_sky.map"
BackgroundFile0Palette:
	.incbin "data/battle_sky.clr" READ 32		;only get a single 16color palette
BackgroundFile0EOF:
.ends
