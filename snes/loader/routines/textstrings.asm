/*
normal ASCII chars. $e0-$ff is reserved for special characters (countries etc)

text byte commands:

#$00 - terminate string
#$01 - set new offset, only executed if not first byte in string(has 2 additional bytes)
#$02 - set font
#$03 - draw string from special adress, but with fixed length(has 4 additional bytes, first byte length, last 3 bytes string vector, maximum string length: 32 letters)
#$04 - draw string from special adress(has 3 additional bytes for string vector)
#$05 - draw byte in hexadecimal(has 3 additional bytes for string vector)
#$06 - draw byte in binary(has 3 additional bytes for string vector)
#$07 - change palette number(AND'ed with 0x7)
#$08 - draw byte in decimal
#$09 - set snes mood


rows:

line 00: 0x0000
line 01: 0x0040
line 02: 0x0080
line 03: 0x00C0
line 04: 0x0100
line 05: 0x0140
line 06: 0x0180
line 07: 0x01c0
line 08: 0x0200
line 09: 0x0240
line 10: 0x0280
line 11: 0x02c0
line 12: 0x0300
line 13: 0x0340
line 14: 0x0380
line 15: 0x03c0
line 16: 0x0400
line 17: 0x0440
line 18: 0x0480
line 19: 0x04c0
line 20: 0x0500
line 21: 0x0540
line 22: 0x0580
line 23: 0x05c0
line 24: 0x0600
line 25: 0x0640
line 26: 0x0680
line 27: 0x06c0
*/

.Section "TextstringLUT" superfree	
TextStringPTable:	
	.dw TextString0
	.dw TextString1
	.dw TextString2
	.dw TextString3
	.dw TextString4
	.dw TextString5
	.dw TextString6
	.dw TextString7
	.dw TextString8
	.dw TextString9

	.dw TextString10
	.dw TextString11
	.dw TextString12
	.dw TextString13
	.dw TextString14
	.dw TextString15
	.dw TextString16
	.dw TextString17
	.dw TextString18
	.dw TextString19

	.dw TextString20
	.dw TextString21
	.dw TextString22
	.dw TextString23
	.dw TextString24
	.dw TextString25
	.dw TextString26
	.dw TextString27
	.dw TextString28
	.dw TextString29

	.dw TextString30
	.dw TextString31
	.dw TextString32
	.dw TextString33
	.dw TextString34
	.dw TextString35
	.dw TextString36
	.dw TextString37
	.dw TextString38
	.dw TextString39

	.dw TextString40
	.dw TextString41
	.dw TextString42
	.dw TextString43
	.dw TextString44
	.dw TextString45
	.dw TextString46
	.dw TextString47
	.dw TextString48
	.dw TextString49

	.dw TextString50
	.dw TextString51
	.dw TextString52
	.dw TextString53
	.dw TextString54
	.dw TextString55
	.dw TextString56
	.dw TextString57
	.dw TextString58
	.dw TextString59

	.dw TextString60
	.dw TextString61
	.dw TextString62
	.dw TextString63
	.dw TextString64
	.dw TextString65
	.dw TextString66
	.dw TextString67
	.dw TextString68
	.dw TextString69

	.dw TextString70
	.dw TextString71
	.dw TextString72
	.dw TextString73
	.dw TextString74
	.dw TextString75
	.dw TextString76
	.dw TextString77
	.dw TextString78
	.dw TextString79

	.dw TextString80
	.dw TextString81
	.dw TextString82
	.dw TextString83
	.dw TextString84
	.dw TextString85
	.dw TextString86
	.dw TextString87
	.dw TextString88
	.dw TextString89

	.dw TextString90
	.dw TextString91
	.dw TextString92
	.dw TextString93
	.dw TextString94
	.dw TextString95
	.dw TextString96
	.dw TextString97
	.dw TextString98
	.dw TextString99
	
	.dw TextString100
	.dw TextString101
	.dw TextString102
	.dw TextString103
	.dw TextString104
	.dw TextString105
	.dw TextString106
	.dw TextString107
	.dw TextString108
	.dw TextString109

	.dw TextString110
	.dw TextString111
	.dw TextString112
	.dw TextString113
	.dw TextString114
	.dw TextString115
	.dw TextString116
	.dw TextString117
	.dw TextString118
	.dw TextString119

	.dw TextString120
	.dw TextString121
	.dw TextString122
	.dw TextString123
	.dw TextString124
	.dw TextString125
	.dw TextString126
	.dw TextString127
	.dw TextString128
	.dw TextString129

	.dw TextString130
	.dw TextString131
	.dw TextString132
	.dw TextString133
	.dw TextString134
	.dw TextString135
	.dw TextString136
	.dw TextString137
	.dw TextString138
	.dw TextString139

	.dw TextString140
	.dw TextString141
	.dw TextString142
	.dw TextString143
	.dw TextString144
	.dw TextString145
	.dw TextString146
	.dw TextString147
	.dw TextString148
	.dw TextString149

	.dw TextString150
	.dw TextString151
	.dw TextString152
	.dw TextString153
	.dw TextString154
	.dw TextString155
	.dw TextString156
	.dw TextString157
	.dw TextString158
	.dw TextString159

	.dw TextString160
	.dw TextString161
	.dw TextString162
	.dw TextString163
	.dw TextString164
	.dw TextString165
	.dw TextString166
	.dw TextString167
	.dw TextString168
	.dw TextString169

	.dw TextString170
	.dw TextString171
	.dw TextString172
	.dw TextString173
	.dw TextString174
	.dw TextString175
	.dw TextString176
	.dw TextString177
	.dw TextString178
	.dw TextString179
	
	.dw TextString180
	.dw TextString181
	.dw TextString182
	.dw TextString183
	.dw TextString184
	.dw TextString185
	.dw TextString186
	
.ends	

.Section "text strings" superfree





TextStrings:


TextString0:
	.dw $004c

	.db "- OPTIXX test -"			;textstring

	.db 1
	.dw $00d0
	.db "Debug: Main"
	.db $00					;terminator
	
TextString1:
	.dw $0146
	.db "Video IRQs:"			;textstring
	.db 1
	.dw $0146+11*2
	.db 5
	.dw VIrqCounter
	.db $7e
	.db $00

;this is the textstring for the text buffer of the menu system
TextString2:
	.dw $0000				;offset on bg1 tilemap
	.db $04					;terminator
	.dw (LoadMenuStringBuffer&$ffff)
	.db $7e
TextString3:
	.dw $004c

	.db "- OPTIXX test -"			;textstring

	.db 1
	.dw $00d0
	.db "Debug: Audio"
	.db $00	
TextString4:
	.dw $0186
	.db "EXT IRQs:"			;textstring
	.db 1
	.dw $0186+11*2
	.db 5
	.dw ExtIrqCounter
	.db $7e
	.db $00
TextString5:
	.dw $01c6
	.db "$00:3000"			;textstring
	.db 1
	.dw $01c6+11*2
	.db 5
	.dw $3000
	.db $00
	.db $00
TextString6:
	.dw $0186+$e
	.db 5
	.dw JoyPortBuffer&$ffff+1
	.db $7e
	.db 0	
TextString7:
	.dw $01c6
	.db "Joy2:"
	.db 5
	.dw JoyPortBuffer&$ffff+2
	.db $7e
	.db 0
TextString8:
	.dw $01c6+$e
	.db 5
	.dw JoyPortBuffer&$ffff+3
	.db $7e
	.db 0	
TextString9:
	.dw $0206
	.db "Joy3:"
	.db 5
	.dw JoyPortBuffer&$ffff+4
	.db $7e
	.db 0
TextString10:
	.dw $0206+$e
	.db 5
	.dw JoyPortBuffer&$ffff+5
	.db $7e
	.db 0	
TextString11:
	.dw $0246
	.db "Joy4:"
	.db 5
	.dw JoyPortBuffer&$ffff+6
	.db $7e
	.db 0
TextString12:
	.dw $0246+$e
	.db 5
	.dw JoyPortBuffer&$ffff+7
	.db $7e
	.db 0	
TextString13:
	.dw $0146
	.db "Timecode:"			;textstring
	.db 1
	.dw $0146+$12
	.db 5
	.dw SpcReportBuffer&$ffff+3
	.db $7e
	.db $00
TextString14:
	.dw $146+$12+$4
	.db 5
	.dw SpcReportBuffer&$ffff+2
	.db $7e

	.db $00
TextString15:
	.dw $004c

	.db "- Nwarp Daisakusen -"			;textstring

	.db 1
	.dw $00d0
	.db "Debug: Tablist Recorder"
	.db $00	
TextString16:
	.dw $0206

	.db "chsum ok "			;textstring
	.db $00	
TextString17:
	.dw $0206

	.db "chsum bad"			;textstring
	.db $00	
TextString18:
	.dw $0146+13*2
	.db 5
	.dw VIrqCounter+1
	.db $7e
	.db $00

TextString19:

TextString20:

TextString21:

TextString22:

TextString23:

TextString24:

TextString25:

TextString26:

TextString27:

TextString28:

TextString29:

TextString30:
	.dw $0186
	.db "Volout:"			;textstring
	.db 1
	.dw $0186+$12
	.db 5
	.dw SpcReportBuffer&$ffff+5
	.db $7e
	.db $00
TextString31:
	.dw $186+$12+$4
	.db 5
	.dw SpcReportBuffer&$ffff+4
	.db $7e

	.db $00
	
TextString32:
	.dw $0186+$18
	.db "Joy5:"
	.db 5
	.dw JoyPortBuffer&$ffff+8
	.db $7e
	.db 0
TextString33:
	.dw $0186+$e+$18
	.db 5
	.dw JoyPortBuffer&$ffff+9
	.db $7e
	.db 0	
TextString34:
	.dw $01c6+$18
	.db "Joy6:"
	.db 5
	.dw JoyPortBuffer&$ffff+10
	.db $7e
	.db 0
TextString35:
	.dw $01c6+$e+$18
	.db 5
	.dw JoyPortBuffer&$ffff+11
	.db $7e
	.db 0	
TextString36:
	.dw $0206+$18
	.db "Joy7:"
	.db 5
	.dw JoyPortBuffer&$ffff+12
	.db $7e
	.db 0
TextString37:
	.dw $0206+$e+$18
	.db 5
	.dw JoyPortBuffer&$ffff+13
	.db $7e
	.db 0	
TextString38:
	.dw $0246+$18
	.db "Joy8:"
	.db 5
	.dw JoyPortBuffer&$ffff+14
	.db $7e
	.db 0
TextString39:
	.dw $0246+$e+$18
	.db 5
	.dw JoyPortBuffer&$ffff+15
	.db $7e
	.db 0
TextString40:
TextString41:
TextString42:
TextString43:
TextString44:
TextString45:
TextString46:
TextString47:
TextString48:
TextString49:
TextString50:
TextString51:
TextString52:
TextString53:
TextString54:
TextString55:
TextString56:
TextString57:
TextString58:
TextString59:
TextString60:
TextString61:
TextString62:
TextString63:
TextString64:
TextString65:
TextString66:
TextString67:
TextString68:
TextString69:
TextString70:
TextString71:
TextString72:
TextString73:
TextString74:
TextString75:
TextString76:
TextString77:
TextString78:
TextString79:
TextString80:
TextString81:
TextString82:
TextString83:
TextString84:
TextString85:
TextString86:
TextString87:
TextString88:
TextString89:
TextString90:
TextString91:
TextString92:
TextString93:
TextString94:
TextString95:
TextString96:
TextString97:
TextString98:
TextString99:
TextString100:
TextString101:
TextString102:
TextString103:
TextString104:
TextString105:
TextString106:
TextString107:
TextString108:
TextString109:
TextString110:
TextString111:
TextString112:
TextString113:
TextString114:
TextString115:
TextString116:
TextString117:
TextString118:
TextString119:
TextString120:
TextString121:
TextString122:
TextString123:
TextString124:
TextString125:
TextString126:
TextString127:
TextString128:
TextString129:
TextString130:
TextString131:
TextString132:
TextString133:
TextString134:
TextString135:
TextString136:
TextString137:
TextString138:
TextString139:
TextString140:
TextString141:
TextString142:
TextString143:
TextString144:
TextString145:
TextString146:
TextString147:
TextString148:
TextString149:
TextString150:
TextString151:
TextString152:
TextString153:
TextString154:
TextString155:
TextString156:
TextString157:
TextString158:
TextString159:
TextString160:
TextString161:
TextString162:
TextString163:
TextString164:
TextString165:
TextString166:
TextString167:
TextString168:
TextString169:
TextString170:
TextString171:
TextString172:
TextString173:
TextString174:
TextString175:
TextString176:
TextString177:
TextString178:
TextString179:
TextString180:
TextString181:
TextString182:
TextString183:
TextString184:
TextString185:
TextString186:

.ends