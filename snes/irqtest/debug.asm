.define	debug_pointer			$0080 

.MACRO printf
    ldy.w   #\1                 ; load add of the string
    sty.w   debug_pointer       ; store address in defined mem area
    ldy     #0                  ; zero y
    jsr     do_printf           
.ENDM



.BANK 0
.SECTION "debug" SEMIFREE

do_printf:
    lda     (debug_pointer),y    ; get address of string
	sta     $3000               ; write to MMIO debug reg
	iny
	cmp     #0                  ; len 5
	bne     do_printf
    rts
.ENDs
