Random:
	php
	sep #$20
	LDA R3
	STA R4								;R4=R3
	LDA R2
	STA R3								;R3=R2
	LDA R1
	STA R2								;R2=R1
	CMP R3
	BMI R3_Greater				;If R3>R2 Then Goto R3_Greater
	
	LDA R3
	CLC
	ADC R4
	clc
	eor.w JoyPortBuffer&$ffff			;use button presses for rng aswell
	eor.w JoyPortBuffer&$ffff+2
	eor.w JoyPortBuffer&$ffff+4
	eor.w JoyPortBuffer&$ffff+6
	eor.w JoyPortBuffer&$ffff+8
	eor.w JoyPortBuffer&$ffff+10
	eor.w JoyPortBuffer&$ffff+12
	eor.w JoyPortBuffer&$ffff+14
	eor.b FrameCounterLo

	STA R1								;R1=R3+R4 MOD 256
	plp
	RTS									 ;Return R1
	
R3_Greater:
	CLC
	ADC R4
	clc
	eor.w JoyPortBuffer&$ffff+8
	eor.b FrameCounterLo

	STA R1								;R1=R2+R4 MOD 256
	plp
	RTS									 ;Return R1