; ***********************************************************
; *** Music routine taken from 'ZOOM.ASM'.                ***
; *** Optimized by Yoshi                                  ***
; ***********************************************************

play_music:
		sep #$30
        lda #$ff
        sta $2140
        rep #$10
        ldx.w #$7fff
-       lda.l $028000,x
        sta.l $7f0000,x
        lda.l $038000,x
        sta.l $7f8000,x
        dex
        bpl -


        stz $00fd
        stz $00fe

        lda #$7f
        sta $00ff
        ina                     ; Oh boy. Actually stores #$80. :-)
        sta $2100
        
        stz $4200
        sei
        jsr res1
        sep #$30
-       lda $2140
        bne -

        lda #$e0
        sta $2143
        lda #$ff
        sta $2142
        lda #$01
        sta $2141
        sta $2140
-       lda $2140
        cmp #$01
        bne -

-       lda $2140
        cmp #$55
        bne -

        lda $0207
        sta $2141
        lda #$07
        sta $2140
-       lda $2140
        cmp #$07
        bne -

-       lda $2140
        cmp #$55
        bne -

        cli
        rts

res1    php
        jsr res2
        plp
        stz $2140
        rts

res2    php
        rep #$30
        ;ldy.w #$0000
        lda.w #$bbaa
-       cmp $2140
        bne -

        sep #$20
        lda #$cc
        bra B1

B4      lda [$fd],y
        iny
        xba
        lda #$00
        bra B2

B3      xba
        lda [$fd],y
        iny
        xba
B5      cmp $2140
        bne B5

        ina
B2      rep #$20
        sta $2140
        sep #$20
        dex
        bne B3

-       cmp $2140
        bne -
-       adc #$03
        beq -

B1      pha
        rep #$20
        lda [$fd],y
        iny
        iny
        tax
        lda [$fd],y
        iny
        iny
        sta $2142
        sep #$20
        cpx.w #$0001
        lda #$00
        rol
        sta $2141
        adc #$7f
        pla
        sta $2140
-       cmp $2140
        bne -
        bvs B4

        plp
        rts
        pla
        sta $2140
-       cmp $2140
        bne -
        bvs B5

        plp
        rts


