.define	debug_pointer			$080 

.MACRO printdb
    ldy.w   #\1                 ; load addr of the marco arg string
    sty.w   debug_pointer       ; store address in defined mem area
    ldy     #0                  ; zero y index
    jsr     do_printf           
.ENDM


.MACRO prints

    ldy.w   #++                ; load addr of the marco arg string
    sty.w   debug_pointer       ; store address in defined mem area
    ldy     #0                  ; zero y index
    jsr     do_printf           
    jmp     +++
++:
    .db     \1,10,0
+++:

.ENDM

.BANK 0
.SECTION "debug" SEMIFREE

do_printf:
    lda     (debug_pointer),y   ; get address of string via indirect mem area
    sta     $3000               ; write to MMIO debug reg
    iny                         ; inc index
    cmp     #0                  ; look for null byte
    bne     do_printf
    rts
.ENDs
