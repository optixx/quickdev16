.Section "SamplepackLUT" superfree				
PtPlayerSamplePackPointertable:
		.dw SamplePack0
		.db (:SamplePack0+BaseAdress>>16)

.ends

.Section "sample pack 0" superfree
SamplePack0:
	.dw (SamplePack0End-SamplePack0)

SamplePackStart0:
	.db 1				;number of samples in this pack

Sample0Header:
	.dw (Sample0-SamplePackStart0)	;relative pointer to sample	
	.dw (Sample0-SamplePackStart0)	;relative loop pointer
	.db $7f				;volume l
	.db $7f				;volume r
	.dw $400			;pitch
	.dw $0000			;adsr
	.db %00011111				;gain
	.db 0
	.db 0
	.db 0
	.db 0
	.db 0


Sample0:
	.incbin "data/sounds/hit.brr"

SamplePack0End:
.ends
