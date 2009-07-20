.Section "oam anilists" superfree
;relative pointers to object animation files
ObjectAnimationLUT:
	.dw (ObjAniList000-ObjectAnimationLUT)
	.dw (ObjAniList001-ObjectAnimationLUT)
	.dw (ObjAniList002-ObjectAnimationLUT)
	.dw (ObjAniList003-ObjectAnimationLUT)
	.dw (ObjAniList004-ObjectAnimationLUT)
	.dw (ObjAniList005-ObjectAnimationLUT)
	.dw (ObjAniList006-ObjectAnimationLUT)
	.dw (ObjAniList007-ObjectAnimationLUT)
	.dw (ObjAniList008-ObjectAnimationLUT)
	.dw (ObjAniList009-ObjectAnimationLUT)
	.dw (ObjAniList010-ObjectAnimationLUT)
	.dw (ObjAniList011-ObjectAnimationLUT)
	.dw (ObjAniList012-ObjectAnimationLUT)		
	.dw (ObjAniList013-ObjectAnimationLUT)
	.dw (ObjAniList014-ObjectAnimationLUT)
	.dw (ObjAniList015-ObjectAnimationLUT)
	.dw (ObjAniList016-ObjectAnimationLUT)
	.dw (ObjAniList017-ObjectAnimationLUT)
	.dw (ObjAniList018-ObjectAnimationLUT)
	.dw (ObjAniList019-ObjectAnimationLUT)
	.dw (ObjAniList020-ObjectAnimationLUT)
	.dw (ObjAniList021-ObjectAnimationLUT)
	.dw (ObjAniList022-ObjectAnimationLUT)
	.dw (ObjAniList023-ObjectAnimationLUT)
	.dw (ObjAniList024-ObjectAnimationLUT)
	.dw (ObjAniList025-ObjectAnimationLUT)
	.dw (ObjAniList026-ObjectAnimationLUT)
	.dw (ObjAniList027-ObjectAnimationLUT)
	.dw (ObjAniList028-ObjectAnimationLUT)
	.dw (ObjAniList029-ObjectAnimationLUT)
	.dw (ObjAniList030-ObjectAnimationLUT)
	.dw (ObjAniList031-ObjectAnimationLUT)
	.dw (ObjAniList032-ObjectAnimationLUT)
	.dw (ObjAniList033-ObjectAnimationLUT)
	
	
ObjAniList000:
;testsprite walk downwards:
;	.dw $0100			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel
	
	
	.dw $000d			;nop
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel

	

	.dw $000d			;nop
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel

	.dw $000d			;nop
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel

	.dw $000d			;nop
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel

	
	.dw $000d			;nop
	.dw $0184			;goto tileset frame 1
	.dw $7685			;set palette and config
	.dw $1002			;and scroll right a pixel

	.dw $000d			;nop
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel
	
	.dw $000d			;nop
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel

	.dw $000d			;nop
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel

	.dw $000d			;nop
	.dw $0183			;next animation frame
	.dw $1002			;and scroll right a pixel


	.dw $000d			;nop
	.dw $0184			;goto tileset frame 1
	.dw $3685			;set palette and config
	
;	.dw $0001
	.dw $0606			;goto frame 6 in this animation

	
ObjAniList001:
;main chara top body walking down
	.dw $0580			;create object 5
	.dw $0287			;goto command list 3

ObjAniList002:
;male walking
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
;walking animation
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop

	.dw $0284			;add 2 tileset frames

	.dw $0406			;loop walking animation

	
ObjAniList003:
;male punching
	.dw $0203			;add 2 tileset frames
	.dw $020a			;set subroutine to void	
	.dw $0103			;add 2 tileset frames
	.dw $0103			;add 2 tileset frames
	
	.dw $010d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $010d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $010d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $010d			;nop
	.dw $ff03			;add 2 tileset frames

	.dw $0d0a			;reset subroutine to normal
	.dw $000e			;infinite waitloop

ObjAniList004:
;male standing still top
	.dw $0004			;goto tileset frame 1
	.dw $000e			;goto animation frame 0(endless loop)

ObjAniList005:
;male falling
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
;standing up, not moving anymore:
	.dw $020a			;set subroutine to void	
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames

	.dw $000d			;nop
	.dw $000d			;nop
	
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0e0a			;reset subroutine to normal again, turn player around
	.dw $000e			;infinite waitloop


ObjAniList006:
;main chara top body walking down
	.dw $0780			;create object 7
	.dw $0487			;goto command list 4

ObjAniList007:
;male fierce punch
	.dw $0103			;add 2 tileset frames
	.dw $000d
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames



	.dw $000c			;play soundeffect
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $050a			;set subroutine to fierce punch
	.dw $0103			;add 2 tileset frames
	.dw $020a			;set subroutine to void
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames


	.dw $0d0a			;reset subroutine to normal again
	.dw $000e			;infinite waitloop

ObjAniList008:
;male falling far (fierce punch)
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop

	
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop

	
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop

	
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
		
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	

	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
;standing up, not moving anymore:
	.dw $020a			;set subroutine to void	
	
	
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames

	.dw $000d			;nop
	.dw $000d			;nop
	
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $0e0a			;reset subroutine to normal again, turn player around
	.dw $000e			;infinite waitloop
	

ObjAniList009:
;male death/fail continuous
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames

;animation loops here
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $100d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $0b84			;reset tileset frame
	.dw $1606			;loop back to shake head sequence


	

ObjAniList010:
;male falling far (death)
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop

	
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop

	
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop

	
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
		
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	

	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
	.dw $000d			;nop
;not moving anymore:
	.dw $0a0a			;set subroutine to dead	
	.dw $000e			;infinite waitloop

ObjAniList011:
;male sitting in menu

	.dw $0103			;add 2 tileset frames
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $200d			;nop

	.dw $0004			;reset tileset frame
	.dw $050d			;nop	

	.dw $050d			;nop	
	.dw $050d			;nop	
	.dw $0006			;repeat animation




ObjAniList012:
;male standing up in menu
	.dw $0b04			;add 2 tileset frames

	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames	
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop

	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop


	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames	
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $050d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop

	.dw $070a			;set subroutine to menu running
	.dw $000d			;nop
	.dw $0006			;repeat animation
	



ObjAniList013:
;male spasm for revival
	.dw $028a			;set subroutine to void (so player has to wait and autofire doesn't help much)
	.dw $0504			;set frame 6
	.dw $060d			;nop

	.dw $0103			;add 2 tileset frames
	.dw $060d			;nop

	
	.dw $0103			;add 2 tileset frames
	.dw $080d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $000d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0a0a			;set subroutine to dead again
	.dw $000e			;infinite waitloop

;male winner cheering:
ObjAniList014:
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames

;animation loops here:
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames

	.dw $030d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $030d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $030d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $030d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $030d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $1206			;loop animation

ObjAniList015:
;main chara top body battle steady still
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop

	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $fe03			;substract 2 tileset frames
	.dw $000d			;nop	


	.dw $0006			;loop walking animation

ObjAniList016:
;small main chara top body battle steady still
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop

	.dw $ff03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $ff03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $ff03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $ff03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $ff03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $ff03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $ff03			;substract 2 tileset frames
	.dw $000d			;nop	
	.dw $ff03			;substract 2 tileset frames
	.dw $000d			;nop	


	.dw $0006			;loop walking animation

;explosion init
ObjAniList017:


;explosion play
ObjAniList018:
	.dw $010d
	.dw $0403
	.dw $010d
	.dw $0403
	.dw $010d
	.dw $0403
	.dw $010d
	.dw $0403
	.dw $010d
	.dw $0403
	.dw $010d
	.dw $0403	
;	.dw $050f
	.dw $010d
	.dw $0001			;delete

;gra g
ObjAniList019:
	.dw $050d
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop




	.dw $1606			;loop walking animation
	
	
ObjAniList020:
;gra gra
	.dw $000d
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $040d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $030d			;nop
	.dw $1d06			;loop walking animation
	
	
ObjAniList021:
;male being stunned
	.dw $0103			;add 2 tileset frames
	.dw $0103			;add 2 tileset frames
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $010d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $020d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop


	.dw $0d0a			;reset subroutine to normal
	.dw $000e			;infinite waitloop
	
ObjAniList022:
;male blocking
	.dw $0103			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0103			;add 2 tileset frames
	.dw $0f0a			;set subroutine to block
	.dw $0103			;add 2 tileset frames
	.dw $0a0d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $010d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $020d			;nop
	.dw $ff03			;add 2 tileset frames
	.dw $0d0a			;reset subroutine to normal
	
	.dw $000e			;infinite waitloop		
	
	
ObjAniList023:
;male block success, pushing back
	.dw $0203			;add 2 tileset frames
	.dw $000d			;nop
	.dw $0203			;add 2 tileset frames
	.dw $0103			;add 2 tileset frames
	
	.dw $0f0a			;set subroutine to block
	.dw $ff03			;sub 2 tileset frames
	.dw $000d			;nop
	.dw $ff03			;sub 2 tileset frames
	.dw $000d			;nop
	.dw $ff03			;sub 2 tileset frames
	.dw $000d			;nop
	.dw $ff03			;sub 2 tileset frames
	.dw $000d			;nop

	.dw $ff03			;sub 2 tileset frames
	.dw $000d			;nop
	.dw $0d0a			;reset subroutine to normal
	
	.dw $000e			;infinite waitloop		
	
ObjAniList024:
	.dw $070d			;nop
	.dw $0102			;move down
	.dw $070d			;nop
	.dw $0102			;move down	
	.dw $030d			;nop
	.dw $0102			;move down	
	.dw $010d			;nop
	.dw $0102			;move down	
	.dw $000d			;nop
	.dw $0102			;move down	
	.dw $000d			;nop
	.dw $0102			;move down	
	.dw $0102			;move down	
	.dw $0102			;move down
	.dw $0102			;move down

	


	
	.dw $0102			;move down
	.dw $000d			;nop	
	.dw $0102			;move down
	.dw $000d			;nop
	.dw $0102			;move down
	.dw $010d			;nop		
	.dw $0102			;move down
	.dw $030d			;nop	
	.dw $0102			;move down
	.dw $070d			;nop	
	.dw $0102			;move down	

	.dw $070d			;nop
	
	.dw $0902			;move down
	.dw $070d			;nop
	.dw $0902			;move down	
	.dw $030d			;nop
	.dw $0902			;move down	
	.dw $010d			;nop
	.dw $0902			;move down	
	.dw $000d			;nop
	.dw $0902			;move down	
	.dw $000d			;nop
	.dw $0902			;move down	
	.dw $0902			;move down	
	.dw $0902			;move down
	.dw $0902			;move down


	


	
	.dw $0902			;move down
	.dw $000d			;nop	
	.dw $0902			;move down
	.dw $000d			;nop
	.dw $0902			;move down
	.dw $010d			;nop		
	.dw $0902			;move down
	.dw $030d			;nop	
	.dw $0902			;move down
	.dw $070d			;nop	
	.dw $0902			;move down	
	

	.dw $0006			;reset
	
;particle
ObjAniList025:
	.dw $0110			;vector speed
	.dw $b411			;vector dir
	.dw $020d			;nop
	.dw $c310			;vector speed
	.dw $040d			;nop
	.dw $e411			;vector dir	
	.dw $080d			;nop
	.dw $c110			;vector speed

	.dw $0b0d			;nop
	.dw $3411
	.dw $c310			;vector speed
	.dw $e411			;vector dir
	
	.dw $0b0d			;nop
	
	.dw $0001			;del

;particle
ObjAniList026:

	.dw $db11			;vector dir
	.dw $020d			;nop
	.dw $df10			;vector speed
	.dw $040d			;nop
	.dw $e011			;vector dir	
	.dw $080d			;nop
	.dw $f010			;vector speed

	.dw $0b0d			;nop
	.dw $1911
	.dw $d810			;vector speed
	.dw $dc11			;vector dir
	
	.dw $0b0d			;nop
	
	.dw $0001			;del	
	
	
;particle
ObjAniList027:

	.dw $8411			;vector dir	
	.dw $080d			;nop
	.dw $c810			;vector speed
	.dw $080d			;nop
	.dw $c310			;vector speed

	.dw $180d			;nop
	
	.dw $0001			;del


;particle
ObjAniList028:

	.dw $9711			;vector dir	
	.dw $080d			;nop
	.dw $c810			;vector speed
	.dw $080d			;nop
	.dw $c310			;vector speed

	.dw $120d			;nop
	
	.dw $0001			;del
	

;particle
ObjAniList029:

	.dw $b811			;vector dir
	.dw $020d			;nop
	.dw $df10			;vector speed
	.dw $020d			;nop
	.dw $e011			;vector dir	
	.dw $080d			;nop
	.dw $f010			;vector speed

	.dw $080d			;nop
	.dw $3911
	.dw $d710			;vector speed
	.dw $e211			;vector dir
	
	.dw $060d			;nop
	
	.dw $0001			;del
;particle
ObjAniList030:

	.dw $c611			;vector dir
	.dw $050d			;nop
	.dw $d910			;vector speed
	.dw $e011			;vector dir	
	.dw $050d			;nop

	.dw $080d			;nop
;	.dw $d810			;vector speed

	.dw $050d			;nop
	.dw $0211			;vector dir
	.dw $c610			;vector speed
	.dw $d711			;vector dir
	
	.dw $240d			;nop
	
	.dw $0001			;del
	
;particle
ObjAniList031:

	.dw $9911			;vector dir
	.dw $050d			;nop
	.dw $df10			;vector speed
	.dw $dd11			;vector dir	
	.dw $080d			;nop
	.dw $d510			;vector speed
	.dw $240d			;nop
	
	.dw $0001			;del
;particle
ObjAniList032:

	.dw $f411			;vector dir
	.dw $060d			;nop
	.dw $df10			;vector speed
	.dw $f011			;vector dir	
	.dw $040d			;nop
	.dw $cf10			;vector speed
	.dw $e911			;vector dir	
	.dw $080d			;nop
	
	.dw $c310			;vector speed
	.dw $080d			;nop
	.dw $0001			;del
;particle
ObjAniList033:

	.dw $ff11			;vector dir
	.dw $030d			;nop
	.dw $df10			;vector speed
	.dw $fc11			;vector dir	
	.dw $020d			;nop
	.dw $cf10			;vector speed
	.dw $fb11			;vector dir	
	.dw $050d			;nop
	
	.dw $c310			;vector speed
	.dw $080d			;nop
	.dw $0001			;del
	
.ends