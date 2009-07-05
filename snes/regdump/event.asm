;:ts=8
R0	equ	1
R1	equ	5
R2	equ	9
R3	equ	13
	code
	xdef	__initEvents
	func
__initEvents:
	longa	on
	longi	on
	stz	|__events
	rts
L2	equ	0
L3	equ	1
	ends
	efunc
	code
	xdef	__createEvent
	func
__createEvent:
	longa	on
	longi	on
	tsc
	sec
	sbc	#L5
	tcs
	phd
	tcd
callback_0	set	3
myEvent_1	set	0
	pea	#<$8
	jsr	__malloc
	sta	<L6+myEvent_1
	lda	#$0
	sta	(<L6+myEvent_1)
	lda	<L5+callback_0
	ldy	#$2
	sta	(<L6+myEvent_1),Y
	lda	#$0
	ldy	#$6
	sta	(<L6+myEvent_1),Y
	dey
	dey
	sta	(<L6+myEvent_1),Y
	lda	<L6+myEvent_1
	tay
	lda	<L5+1
	sta	<L5+1+2
	pld
	tsc
	clc
	adc	#L5+2
	tcs
	tya
	rts
L5	equ	2
L6	equ	1
	ends
	efunc
	code
	xdef	__addEvent
	func
__addEvent:
	longa	on
	longi	on
	tsc
	sec
	sbc	#L8
	tcs
	phd
	tcd
callback_0	set	3
noDuplicateCallback_0	set	5
lastEvent_1	set	0
myEvent_1	set	2
	lda	|__events
	bne	L10001
	pei	<L8+callback_0
	jsr	__createEvent
	sta	|__events
L11:
	tay
	lda	<L8+1
	sta	<L8+1+4
	pld
	tsc
	clc
	adc	#L8+4
	tcs
	tya
	rts
L10001:
	lda	|__events
	bra	L20000
L20004:
	ldy	#$2
	lda	(<L9+lastEvent_1),Y
	cmp	<L8+callback_0
	bne	L10005
L20005:
	lda	#$0
	bra	L11
L20002:
	lda	<L8+noDuplicateCallback_0
	cmp	#<$1
	bne	L10004
	ldy	#$2
	lda	(<L9+lastEvent_1),Y
	cmp	<L8+callback_0
	beq	L20005
L10004:
	ldy	#$6
	lda	(<L9+lastEvent_1),Y
L20000:
	sta	<L9+lastEvent_1
	ldy	#$6
	lda	(<L9+lastEvent_1),Y
	bne	L20002
	lda	<L8+noDuplicateCallback_0
	cmp	#<$1
	beq	L20004
L10005:
	pei	<L8+callback_0
	jsr	__createEvent
	sta	<L9+myEvent_1
	lda	<L9+lastEvent_1
	ldy	#$4
	sta	(<L9+myEvent_1),Y
	lda	<L9+myEvent_1
	iny
	iny
	sta	(<L9+lastEvent_1),Y
	lda	<L9+myEvent_1
	bra	L11
L8	equ	4
L9	equ	1
	ends
	efunc
	code
	xdef	__removeEvent
	func
__removeEvent:
	longa	on
	longi	on
	tsc
	sec
	sbc	#L17
	tcs
	phd
	tcd
eventElement_0	set	3
alone_1	set	0
next_1	set	1
previous_1	set	3
	sep	#$20
	longa	off
	stz	<L18+alone_1
	rep	#$20
	longa	on
	ldy	#$6
	lda	(<L17+eventElement_0),Y
	sta	<L18+next_1
	dey
	dey
	lda	(<L17+eventElement_0),Y
	sta	<L18+previous_1
	iny
	iny
	lda	(<L17+eventElement_0),Y
	beq	L10006
	dey
	dey
	lda	(<L17+eventElement_0),Y
	beq	L10006
	sep	#$20
	longa	off
	inc	<L18+alone_1
	rep	#$20
	longa	on
	lda	<L18+previous_1
	sta	(<L18+next_1),Y
	lda	<L18+next_1
	bra	L20007
L10006:
	ldy	#$6
	lda	(<L17+eventElement_0),Y
	beq	L10008
	sep	#$20
	longa	off
	inc	<L18+alone_1
	rep	#$20
	longa	on
	lda	#$0
	dey
	dey
	sta	(<L18+next_1),Y
	lda	<L18+next_1
	sta	|__events
	bra	L10007
L10008:
	ldy	#$4
	lda	(<L17+eventElement_0),Y
	beq	L10007
	sep	#$20
	longa	off
	inc	<L18+alone_1
	rep	#$20
	longa	on
	lda	#$0
L20007:
	ldy	#$6
	sta	(<L18+previous_1),Y
L10007:
	pei	<L17+eventElement_0
	jsr	__free
	lda	<L18+alone_1
	and	#$ff
	bne	L24
	stz	|__events
L24:
	lda	<L17+1
	sta	<L17+1+2
	pld
	tsc
	clc
	adc	#L17+2
	tcs
	rts
L17	equ	5
L18	equ	1
	ends
	efunc
	code
	xdef	__processEvents
	func
__processEvents:
	longa	on
	longi	on
	tsc
	sec
	sbc	#L25
	tcs
	phd
	tcd
currentEvent_1	set	0
returnValue_1	set	2
	lda	|__events
L20008:
	sta	<L26+currentEvent_1
	lda	<L26+currentEvent_1
	bne	L20010
	pld
	tsc
	clc
	adc	#L25
	tcs
	rts
L20010:
	lda	(<L26+currentEvent_1)
	pha
	ldy	#$2
	lda	(<L26+currentEvent_1),Y
	xref	__~cal
	jsr	__~cal
	sep	#$20
	longa	off
	sta	<L26+returnValue_1
	cmp	#<$1
	rep	#$20
	longa	on
	bne	L10014
	lda	(<L26+currentEvent_1)
	ina
	sta	(<L26+currentEvent_1)
	bra	L10015
L10014:
	pei	<L26+currentEvent_1
	jsr	__removeEvent
L10015:
	ldy	#$6
	lda	(<L26+currentEvent_1),Y
	bra	L20008
L25	equ	3
L26	equ	1
	ends
	efunc
	xref	__malloc
	xref	__free
	udata
	xdef	__events
__events
	ds	2
	ends
