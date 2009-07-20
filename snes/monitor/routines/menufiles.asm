.Section "menu files" superfree
/*
these are the menu files for menusystem.asm

menu table format:
byte		function
2		starting position of first option text on bg3 tilemap
1		relative starting position of first option variable(*2+starting position of first option text on bg3 tilemap)
1		number of seperating rows between options
1		number of rows/options
2xrow quantity	relative pointer to data for each row


row table format:
1		option data type (maximum number:7)
		0x0=no options, only exec subroutine
		0x1=1 byte
		0x2=8 bits
		0x3=interchangeable options with description text (eg: "sound: mono/stereo) . number of options is variable and limited by the bitmask
		0x4=3byte adress with individual control over each byte(not implemented yet)
1		bitmask for changeable bits if data type=0x2; bitmask(usually only one bit) to determine the string to choose if data type=0x3(if zero=string 1, if not zero=string 2)
1		minimum value for option, wrap to maximum value if changed value equals this
1		maximum value for option, wrap to minimum value if changed value equals this
3		24bit adress of option byte to change
1		Number of subroutine to execute when option is changed(number can be found in MenuSubroutineLUT)
1		palette number when unselected
1		palette number when selected
2xstring quant.	relative pointer to text string
x		option text string(s), #$00 terminated


*/
MenuFilesPTable:
	.dw MenuFile0
	.dw MenuFile1
	.dw MenuFile2


MenuFiles:
MenuFile0:
	.dw $0306			;starting position of first option text on bg3 tilemap
	.db 16				;relative starting position of first option variable
	.db 0				;number of seperating rows between options
	.db 6				;number of rows/options
	.dw (MenuFile0Row0-MenuFile0)
	.dw (MenuFile0Row1-MenuFile0)
	.dw (MenuFile0Row2-MenuFile0)
	.dw (MenuFile0Row3-MenuFile0)
	.dw (MenuFile0Row4-MenuFile0)
	.dw (MenuFile0Row5-MenuFile0)


	MenuFile0Row0:
		.db 0				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db $ff				;maximum value for option, wrap to minimum value if changed value equals this
		.dw CurrentLevel		;24bit adress of option byte to change
		.db $7E
		.db 1				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile0Row0String0-MenuFile0)
		MenuFile0Row0String0:
			.db "Jump to ROM"
			.db $00				;terminator

	MenuFile0Row1:
		.db 0				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db $ff				;maximum value for option, wrap to minimum value if changed value equals this
		.dw CurrentBattleFile				;24bit adress of option byte to change
		.db $7e
		.db 2				;2 ;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile0Row1String0-MenuFile0)
		MenuFile0Row1String0:
			.db "Jump to RAM"
			.db $00				;terminator

	MenuFile0Row2:
		.db 3				;option data type (maximum number:7)
		.db 7				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db 1				;maximum value for option, wrap to minimum value if changed value equals this
		.dw IrqRoutineNumberBuffer				;24bit adress of option byte to change
		.db $7e
		.db 0				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile0Row2String0-MenuFile0)
		.dw (MenuFile0Row2String1-MenuFile0)
		.dw (MenuFile0Row2String2-MenuFile0)

		MenuFile0Row2String0:
			.db "Video-IRQ"
			.db $00				;terminator		

		MenuFile0Row2String1:
			.db "Disable"
			.db $00				;terminator		

		MenuFile0Row2String2:
			.db "Enable "
			.db $00				;terminator		


	MenuFile0Row3:
		.db 1				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db $ff				;maximum value for option, wrap to minimum value if changed value equals this
		.dw Reg3000WriteVar				;24bit adress of option byte to change
		.db $7e
		.db 4				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile0Row3String0-MenuFile0)
		MenuFile0Row3String0:
			.db "$00:3000 Write"
			.db $00				;terminator


	MenuFile0Row4:
		.db 0				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db 31				;maximum value for option, wrap to minimum value if changed value equals this
		.dw PtPlayerCurrentSoundEffect				;24bit adress of option byte to change
		.db $7e
		.db 3				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile0Row4String0-MenuFile0)
		MenuFile0Row4String0:
			.db "Audio Menu"
			.db $00				;terminator

	MenuFile0Row5:
		.db 0				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db $7f				;maximum value for option, wrap to minimum value if changed value equals this
		.dw SpcSEVolume				;24bit adress of option byte to change
		.db $7e
		.db 12				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile0Row5String0-MenuFile0)
		MenuFile0Row5String0:
			.db "Calc Chsum"
			.db $00				;terminator


		
			
			
;audio menu
MenuFile1:
	.dw $0306			;starting position of first option text on bg3 tilemap
	.db 20				;relative starting position of first option variable
	.db 0				;number of seperating rows between options
	.db 6				;number of rows/options
	.dw (MenuFile1Row2-MenuFile1)
	.dw (MenuFile1Row7-MenuFile1)
	.dw (MenuFile1Row8-MenuFile1)
	.dw (MenuFile1Row9-MenuFile1)
	.dw (MenuFile1Row10-MenuFile1)
	.dw (MenuFile1Row11-MenuFile1)




	MenuFile1Row2:
		.db 0				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db 0				;maximum value for option, wrap to minimum value if changed value equals this
		.dw PtPlayerCurrentSong				;24bit adress of option byte to change
		.db $7e
		.db 5				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile1Row2String0-MenuFile1)
		MenuFile1Row2String0:
			.db "Upload,play song"
			.db $00				;terminator



	MenuFile1Row7:
		.db 0				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db 59				;maximum value for option, wrap to minimum value if changed value equals this
		.dw SpcSEPitch				;24bit adress of option byte to change
		.db $7e
		.db 8				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile1Row7String0-MenuFile1)
		MenuFile1Row7String0:
			.db "Stop song"
			.db $00				;terminator						
			

	MenuFile1Row8:
		.db 1				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db $ff				;maximum value for option, wrap to minimum value if changed value equals this
		.dw SpcSongSpeed				;24bit adress of option byte to change
		.db $7e
		.db 9				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile1Row8String0-MenuFile1)
		MenuFile1Row8String0:
			.db "Set song speed"
			.db $00				;terminator		
			

	MenuFile1Row9:
		.db 2				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db $0f				;maximum value for option, wrap to minimum value if changed value equals this
		.dw SpcSongChMask				;24bit adress of option byte to change
		.db $7e
		.db 10				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile1Row9String0-MenuFile1)
		MenuFile1Row9String0:
			.db "Song channel mask"
			.db $00				;terminator						
			
			
	MenuFile1Row10:
		.db 3				;option data type (maximum number:7)
		.db 7				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db 7				;maximum value for option, wrap to minimum value if changed value equals this
		.dw SpcReportType				;24bit adress of option byte to change
		.db $7e
		.db 11				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile1Row10String0-MenuFile1)
		.dw (MenuFile1Row10String1-MenuFile1)
		.dw (MenuFile1Row10String2-MenuFile1)
		.dw (MenuFile1Row10String3-MenuFile1)
		.dw (MenuFile1Row10String4-MenuFile1)
		.dw (MenuFile1Row10String1-MenuFile1)
		.dw (MenuFile1Row10String1-MenuFile1)
		.dw (MenuFile1Row10String1-MenuFile1)
		.dw (MenuFile1Row10String1-MenuFile1)
		MenuFile1Row10String0:
			.db "Spc Report Type"
			.db $00				;terminator		

		MenuFile1Row10String1:
			.db "None    "
			.db $00				;terminator		

		MenuFile1Row10String2:
			.db "Timecode"
			.db $00				;terminator		

		MenuFile1Row10String3:
			.db "Vol Out "
			.db $00				;terminator		

		MenuFile1Row10String4:
			.db "Mod Cmd "
			.db $00				;terminator										
			
	MenuFile1Row11:
		.db 0				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db $0f				;maximum value for option, wrap to minimum value if changed value equals this
		.dw SpcSongChMask				;24bit adress of option byte to change
		.db $7e
		.db 13				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile1Row11String0-MenuFile1)
		MenuFile1Row11String0:
			.db "Return"
			.db $00		
			
MenuFile2:
	.dw $0306			;starting position of first option text on bg3 tilemap
	.db 16				;relative starting position of first option variable
	.db 0				;number of seperating rows between options
	.db 1				;number of rows/options
	.dw (MenuFile2Row0-MenuFile2)

	MenuFile2Row0:
		.db 0				;option data type (maximum number:7)
		.db 0				;bitmask for changeable bits
		.db 0				;minimum value for option, wrap to maximum value if changed value equals this
		.db $ff				;maximum value for option, wrap to minimum value if changed value equals this
		.dw CurrentLevel		;24bit adress of option byte to change
		.db $7E
		.db 13				;number of subroutine to execute
		.db 1				;palette number when unselected
		.db 0				;palette number when selected
		.dw (MenuFile2Row0String0-MenuFile2)
		MenuFile2Row0String0:
			.db "Return"
			.db $00				;terminator					
			
			
			

.ends			