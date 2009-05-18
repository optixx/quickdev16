
.MACRO printf
    REP #$38	; mem/A = 16 bit, X/Y = 16 bit
    SEP #$20
    lda.w str_NMI
    sta.w debug_pointer
    ldy #0
    jsr do_printf
    REP #$30	; mem/A = 16 bit, X/Y = 16 bit
    SEP #$20
    
.ENDM


.define	debug_pointer			$0000 


.BANK 0
.ORG 0
.SECTION "debug" SEMIFREE


do_printf:
	lda     (debug_pointer),y       ; get ascii text data
	sta     $3000
	iny
	cpy     #5
	bne     do_printf
    rts
.ENDs
