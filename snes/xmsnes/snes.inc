;-----------;
; SNES.ASM   \
;---------------------------;
; SNES Register Definitions ;
; by eKid                   ;
;---------------------------+--;
; Information taken from-      ;
; QWERTIE'S SNES DOCUMENTATION ;
; AND                          ;
; YOSHI'S DOCUMENTS            ;
;------------------------------;

.EQU REG_INIDISP		$2100  ; Screen Display Register	1B/W
.EQU REG_OBSEL			$2101  ; OAM Size Control			1B/W
.EQU REG_OAMADDL		$2102  ; OAM Access Address			2B/W
.EQU REG_OAMADDH		$2103
.EQU REG_OAMDATA		$2104  ; OAM Data Write				1B/W
.EQU REG_BGMODE			$2105  ; Screen Mode Register		1B/W
.EQU REG_MOSAIC			$2106  ; Screen Pixelation Register 1B/W
.EQU REG_BG0SC			$2107  ; BG0 VRAM Location Register	1B/W
.EQU REG_BG1SC			$2108  ; BG1 VRAM Location Register 1B/W
.EQU REG_BG2SC			$2109  ; BG2 VRAM Location Register 1B/W
.EQU REG_BG3SC			$210A  ; BG3 VRAM Location Register 1B/W
.EQU REG_BG01NBA		$210B  ; BG0/1 Character Bases		1B/W
.EQU REG_BG23NBA		$210C  ; BG2/3 Character Bases		1B/W
.EQU REG_BG0HOFS		$210D  ; BG0 Vertical Scroll		1B/W D
.EQU REG_BG0VOFS		$210E  ; BG0 Vertical Scroll		1B/W D
.EQU REG_BG1HOFS		$210F  ; BG1 Horizontal Scroll		1B/W D
.EQU REG_BG1VOFS		$2110  ; BG1 Vertical Scroll		1B/W D
.EQU REG_BG2HOFS		$2111  ; BG2 Horizontal Scroll		1B/W D
.EQU REG_BG2VOFS		$2112  ; BG2 Vertical Scroll		1B/W D
.EQU REG_BG3HOFS		$2113  ; BG3 Horiztonal Scroll		1B/W D
.EQU REG_BG3VOFS		$2114  ; BG3 Vertical Scroll		1B/W D
.EQU REG_VMAIN			$2115  ; Video Port Control			1B/W
.EQU REG_VMADDL			$2116  ; Video Port Address			2B/W
.EQU REG_VMADDH			$2117
.EQU REG_VMDATAL		$2118  ; Video Port Data			2B/W
.EQU REG_VMDATAH		$2119
.EQU REG_M7SEL			$211A  ; MODE7 settings register	1B/W
.EQU REG_M7A			$211B  ; MODE7 COSINE A				1B/W
.EQU REG_M7B			$211C  ; MODE7 SINE	A				1B/W
.EQU REG_M7C			$211D  ; MODE7 SINE	B				1B/W
.EQU REG_M7D			$211E  ; MODE7 COSINE B				1B/W
.EQU REG_M7X			$211F  ; MODE7 Center Pos X			1B/W D
.EQU REG_M7Y			$2120  ; MODE7 Center Pos Y			1B/W D
.EQU REG_CGADD			$2121  ; CGRAM Address				1B/W
.EQU REG_CGDATA			$2122  ; CGRAM Data Write			1B/W D
.EQU REG_W01SEL			$2123  ; Window Mask Settings Reg1	1B/W
.EQU REG_W23SEL			$2124  ; Window Mask Settings Reg2	1B/W
.EQU REG_WH0			$2126  ; Window 1 Left Posision Reg	1B/W
.EQU REG_WH1			$2127  ; Window 1 Right Position Reg	1B/W
.EQU REG_WH2			$2128  ; Window 2 Left Position Reg 1B/W
.EQU REG_WH3			$2129  ; Window 2 Right Position Reg	1B/W
.EQU REG_WBGLOG			$212A  ; Mask Logic for Window 1 & 2	1B/W
.EQU REG_WOBJLOG		$212B  ; Mask Logic for Color&OBJ Windows	1B/W
.EQU REG_TM				$212C  ; Main Screen Designation	1B/W
.EQU REG_TD				$212D  ; Sub-Screen Designation		1B/W
.EQU REG_TMW			$212E  ; WinMask Main Designation Reg	1B/W
.EQU REG_TSW			$212F  ; WinMask Sub Designation Reg	1B/W
.EQU REG_CGWSEL			$2130  ; Fixed Color/Screen Addition Reg	1B/W
.EQU REG_CGADSUB		$2131  ; +/- For Screens/BGs/OBJs	1B/W
.EQU REG_COLDATA		$2132  ; Fixed Color Data for +/-	1B/W
.EQU REG_SETINI			$2133  ; Screen Mode Select Reg		1B/W
.EQU REG_MPYL			$2134  ; Multiplication Result Low	1B/R
.EQU REG_MPYM			$2135  ; Middle						1B/R
.EQU REG_MPYH			$2136  ; High						1B/R
.EQU REG_SLHV			$2137  ; Sofware Latch For H/V Counter	1B/R
.EQU REG_OAMDATAREAD	$2138  ; OAM Data Read				1B/R
.EQU REG_VMDATALREAD	$2139  ; VRAM Data Read	High		1B/R
.EQU REG_VMDATAHREAD	$213A  ; VRAM Data Read Low			1B/R
.EQU REG_CGDATAREAD		$213B  ; CGRAM Data Read			1B/R
.EQU REG_OPHCT			$213C  ; X Scanline Location		1B/R D
.EQU REG_OPVCT			$213D  ; Y Scanline Location		1B/R D
.EQU REG_STAT77			$213E  ; PPU Status Flag & Version	1B/R
.EQU REG_STAT78			$213F  ; PPU Status Flag & Version	1B/R
.EQU REG_APUI00			$2140  ; Sound Register				1B/RW
.EQU REG_APUI01			$2141  ; Sound Register				1B/RW
.EQU REG_APUI02			$2142  ; Sound Register				1B/RW
.EQU REG_APUI03			$2143  ; Sound Register				1B/RW
.EQU REG_WMDATA			$2180  ; WRAM Data Read/Write		1B/RW
.EQU REG_WMADDL			$2181  ; WRAM Address (Low)			1B/RW
.EQU REG_WMADDM			$2182  ; WRAM Address (Middle)		1B/RW
.EQU REG_WMADDH			$2183  ; WRAM Address (High)		1B/RW
.EQU REG_NMITIMEN		$4200  ; Counter Enable				1B/W
.EQU REG_WRIO			$4201  ; Programmable I/O Port		1B/W
.EQU REG_WRMPYA			$4202  ; Multiplicand				1B/W
.EQU REG_WRMPYB			$4203  ; Multiplier					1B/W
.EQU REG_WRDIVL			$4204  ; Dividend					2B/W ???
.EQU REG_WRDIVH			$4205  ;								 ???
.EQU REG_WRDIVB			$4206  ; Divisor						 ???
.EQU REG_HTIMEL			$4207  ; Video X IRQ Beam Pointer	2B/W
.EQU REG_HTIMEH			$4208
.EQU REG_VTIMEL			$4209  ; Video Y IRQ Beam Pointer	2B/W
.EQU REG_VTIMEH			$420A
.EQU REG_MDMAEN			$420B  ; DMA Enable Register		1B/W
.EQU REG_MHDMAEN		$420C  ; HDMA Enable Register		1B/W
.EQU REG_MEMSEL			$420D  ; Cycle Speed Register		1B/W
.EQU REG_RDNMI			$4210  ; NMI Register				1B/R
.EQU REG_TIMEUP			$4211  ; Video IRQ Register			1B/RW
.EQU REG_HVBJOY			$4212  ; Status Register			1B/RW
.EQU REG_RDIO			$4213  ; Programmable I/O Port		1B/RW
.EQU REG_RDDIVL			$4214  ; Quotient Of Divide Result	2B/R
.EQU REG_RDDIVH			$4215
.EQU REG_RDMPYL			$4216  ; Multiplication Or Divide Result	2B/R
.EQU REG_RDMPYH			$4217
.EQU REG_JOY1L			$4218  ; Joypad #1 Status			2B/R
.EQU REG_JOY1H			$4219
.EQU REG_JOY2L			$421A  ; Joypad #2 Status			2B/R
.EQU REG_JOY2H			$421B
.EQU REG_JOY3L			$421C  ; Joypad #3 Status			2B/R
.EQU REG_JOY3H			$421D
.EQU REG_JOY4L			$421E  ; Joypad #4 Status			2B/R
.EQU REG_JOY4H			$421F

.EQU REG_DMAP0			$4300  ; DMA Control Register		1B/W
.EQU REG_BBAD0			$4301  ; DMA Destination Register	1B/W
.EQU REG_A1T0L			$4302  ; DMA Source Address			2B/W
.EQU REG_A1T0H			$4303
.EQU REG_A1B0			$4304  ; Source Bank Address		1B/W
.EQU REG_DAS0L			$4305  ; DMA Transfer size/HDMA Address	2B/W
.EQU REG_DAS0H			$4306
.EQU REG_NTRL0			$430A  ; Number Of Lines For HDMA	1B/W
.EQU REG_DMAP1			$4310  ; DMA 1
.EQU REG_BBAD1			$4311
.EQU REG_A1T1L			$4312
.EQU REG_A1T1H			$4313
.EQU REG_A1B1			$4314
.EQU REG_DAS1L			$4315
.EQU REG_DAS1H			$4316
.EQU REG_NTRL1			$431A
.EQU REG_DMAP2			$4310  ; DMA 2
.EQU REG_BBAD2			$4311
.EQU REG_A1T2L			$4312
.EQU REG_A1T2H			$4313
.EQU REG_A1B2			$4314
.EQU REG_DAS2L			$4315
.EQU REG_DAS2H			$4316
.EQU REG_NTRL2			$431A
.EQU REG_DMAP3			$4310  ; DMA 3
.EQU REG_BBAD3			$4311
.EQU REG_A1T3L			$4312
.EQU REG_A1T3H			$4313
.EQU REG_A1B3			$4314
.EQU REG_DAS3L			$4315
.EQU REG_DAS3H			$4316
.EQU REG_NTRL3			$431A
.EQU REG_DMAP4			$4310  ; DMA 4
.EQU REG_BBAD4			$4311
.EQU REG_A1T4L			$4312
.EQU REG_A1T4H			$4313
.EQU REG_A1B4			$4314
.EQU REG_DAS4L			$4315
.EQU REG_DAS4H			$4316
.EQU REG_NTRL4			$431A
.EQU REG_DMAP5			$4310  ; DMA 5
.EQU REG_BBAD5			$4311
.EQU REG_A1T5L			$4312
.EQU REG_A1T5H			$4313
.EQU REG_A1B5			$4314
.EQU REG_DAS5L			$4315
.EQU REG_DAS5H			$4316
.EQU REG_NTRL5			$431A
.EQU REG_DMAP6			$4310  ; DMA 6
.EQU REG_BBAD6			$4311
.EQU REG_A1T6L			$4312
.EQU REG_A1T6H			$4313
.EQU REG_A1B6			$4314
.EQU REG_DAS6L			$4315
.EQU REG_DAS6H			$4316
.EQU REG_NTRL6			$431A
.EQU REG_DMAP7			$4310  ; DMA 7
.EQU REG_BBAD7			$4311
.EQU REG_A1T7L			$4312
.EQU REG_A1T7H			$4313
.EQU REG_A1B7			$4314
.EQU REG_DAS7L			$4315
.EQU REG_DAS7H			$4316
.EQU REG_NTRL7			$431A
.EQU REG_FBNANACNT		$FEED  ; Banana Register...			1B/RW
