.Section "oam objects" superfree
;relative pointers to objects:
ObjectLUT:
	.dw (Object000-ObjectLUT)
	.dw (Object001-ObjectLUT)


								
Object000:
	.db %11011010		;object type designation
				;bit3=subroutine? if set, this object has its own subroutine that must be executed every frame
				;bit4=animate? if set, this object is animated(a special table for each object specifies the exact animation with commands for tileloading, waiting, animation loop etc)
				;bit5=screen-bound? if set, this object must move in accordance with background layer 0
				;bit6=object active? if set, this object is active and needs to be processed. if clear, this sprite doesnt need to be processed
				;bit7=object present? if set, this slot has an object. if clear, its considered empty and can be overwritten
	.db %00000000		;object type designation 2
	.db $02			;number of subroutine. executed if bit6 of object type is set
	.db 0			;tileset to use
	.db $00			;current "frame" of tileset to display
	.db $08			;starting tile in vram
	.db %00110110		;palette and config
	.dw $80			;x position
	.dw $80			;y position
	.db 0			;current frame in animation list
	.db 0			;object command list to use	
	.db 0			;object offset in object list.
	.db 0			;palette number to upload for this sprite
	.dw 0			;object number

	.db 0			;x-displacement
	.db 0			;y-displacement
	.db 0			;z-displacement
	.db 0			;animation repeat counter for nop
	.db 0			;collision subroutine
	.db 0			;vector speed. 3bit + 3bit sub-pixel accuracy. msb set=target speed met.(speed=0: don't calc vector movement)
	.db 0			;void
	.db 0			;void
	.db 0			;void
	.db 7			;spare variable
	.db 0			;spare variable
	.db 0			;spare variable
	.db 0			;spare variable
	.db 0			;spare variable
	.db 0			;spare variable



;cpu usage indicator
Object001:
	.db %11101000	;object type designation
					;bit0=X position sign of sprite(usually 0)
					;bit1=Object size flag
					;bit2=collidable
					;bit3=subroutine? if set, this object has its own subroutine that must be executed every frame
					;bit4=animate? if set, this object is animated(a special table for each object specifies the exact animation with commands for tileloading, waiting, animation loop etc)
					;bit5=bg-bound? if set, this object must move in accordance with background layer 0
					;bit6=object active? if set, this object is active and needs to be drawn. if clear, this sprite doesnt need to be drawn, but must be processed, anyway
					;bit7=object present? if set, this slot has an object. if clear, its considered empty and can be overwritten
	.db %00100000	;object type designation 2
					;bit7=pseudo 3d sprite that needs to be moved according to z-value when background moves
					;bit6=don't upload tiles for this sprite. useful for stuff like particles and such where lots of sprites share the same tiles.
					;bit5=don't upload palette for this sprite.
					;bits0-3: current position in vector angle LUT (didn't fit anywhere else)
	
	.db 1			;number of subroutine. executed if bit6 of object type is set
	.db 0			;tileset to use
	.db $00			;current "frame" of tileset to display
	.db $ff			;starting tile in vram
	.db %00110001	;palette and config
	.dw 1*16			;x position
	.dw 1			;y position
	.db 0			;current frame in animation list
	.db 0			;object command list to use	
	.db 0			;object offset in object list.
	.db 0			;palette number to upload for this sprite
	.dw 0			;object number

	.db 0			;x-displacement
	.db 0			;y-displacement
	.db 0			;z-displacement
	.db 0			;animation repeat counter for nop
	.db 0			;collision subroutine
	.db 0			;vector speed. bit0-2:subpixel speed. bit3-5:pixel speed.  bit7 set: target speed met  
	.db 0			;vector target speed. bit0-2:subpixel speed target. bit3-5:pixel speed target. bit6,7: movement type(direct, linear slow, linear fast, smooth)
	.db 0			;vector direction. bit0-5:direction.0=facing up. bit6=turning direction.(set=clockwise) msb set=target direction met.
	.db 0			;vector target dir. bit0-5:target direction.0=facing up.  bit6,7: movement type(direct, linear slow, linear fast, smooth)
	.db 0			;subpixel buffer. bit0-2:vector speed subpixel buffer. bit3-7: direction turn speed sub-pixel buffer. 
	.db 0			;spare variable
	.db 0			;spare variable
	.db 0			;spare variable
	.db 0			;spare variable
	.db 0			;spare variable

.ends