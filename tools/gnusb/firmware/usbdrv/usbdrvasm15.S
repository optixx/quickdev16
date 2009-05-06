/* Name: usbdrvasm15.S
 * Project: AVR USB driver
 * Author: contributed by V. Bosch
 * Creation Date: 2007-08-06
 * Tabsize: 4
 * Copyright: (c) 2007 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
 * Revision: $Id$
 */


/* Do not link this file! Link usbdrvasm.S instead, which includes the
 * appropriate implementation!
 */

/*
General Description:
This file is the 15 MHz version of the asssembler part of the USB driver. It
requires a 15 MHz crystal (not a ceramic resonator and not a calibrated RC
oscillator).

See usbdrv.h for a description of the entire driver.

Since almost all of this code is timing critical, don't change unless you
really know what you are doing! Many parts require not only a maximum number
of CPU cycles, but even an exact number of cycles!
*/

;max stack usage: [ret(2), YL, SREG, YH, bitcnt, shift, x1, x2, x3, x4, cnt] = 12 bytes
;nominal frequency: 15 MHz -> 10.0 cycles per bit, 80.0 cycles per byte
; Numbers in brackets are clocks counted from center of last sync bit
; when instruction starts

;----------------------------------------------------------------------------
; order of registers pushed: 
;	YL, SREG [sofError] YH, shift, x1, x2, x3, bitcnt, cnt, x4
;----------------------------------------------------------------------------
USB_INTR_VECTOR:              
    push    YL                   ;2 	push only what is necessary to sync with edge ASAP
    in      YL, SREG             ;1 
    push    YL                   ;2 
;----------------------------------------------------------------------------
; Synchronize with sync pattern:
;
;   sync byte (D-) pattern LSb to MSb: 01010100 [1 = idle = J, 0 = K]
;   sync up with J to K edge during sync pattern -- use fastest possible loops
;   first part has no timeout because it waits for IDLE or SE1 (== disconnected)
;-------------------------------------------------------------------------------
waitForJ:			 ;- 
    sbis    USBIN, USBMINUS      ;1 <-- sample: wait for D- == 1
    rjmp    waitForJ		 ;2 
;-------------------------------------------------------------------------------
; The following code results in a sampling window of < 1/4 bit 
;	which meets the spec.
;-------------------------------------------------------------------------------
waitForK:			 ;- 
    sbis    USBIN, USBMINUS      ;1 [00] <-- sample
    rjmp    foundK               ;2 [01]
    sbis    USBIN, USBMINUS	 ;	 <-- sample
    rjmp    foundK
    sbis    USBIN, USBMINUS	 ;	 <-- sample
    rjmp    foundK
    sbis    USBIN, USBMINUS	 ;	 <-- sample
    rjmp    foundK
    sbis    USBIN, USBMINUS	 ;	 <-- sample
    rjmp    foundK
    sbis    USBIN, USBMINUS	 ;	 <-- sample
    rjmp    foundK
#if USB_COUNT_SOF
    lds     YL, usbSofCount
    inc     YL
    sts     usbSofCount, YL
#endif  /* USB_COUNT_SOF */
    rjmp    sofError
;------------------------------------------------------------------------------
; {3, 5} after falling D- edge, average delay: 4 cycles [we want 5 for 
;	center sampling] 
; 	we have 1 bit time for setup purposes, then sample again. 
;	Numbers in brackets are cycles from center of first sync (double K) 
;	bit after the instruction
;------------------------------------------------------------------------------
foundK:                          ;- [02]
    lds     YL, usbInputBufOffset;2 [03+04]	tx loop
    push    YH                   ;2 [05+06]
    clr     YH                   ;1 [07]
    subi    YL, lo8(-(usbRxBuf)) ;1 [08] 	[rx loop init]
    sbci    YH, hi8(-(usbRxBuf)) ;1 [09] 	[rx loop init]
    push    shift                ;2 [10+11]
    ser	    shift		 ;1 [12]
    sbis    USBIN, USBMINUS      ;1 [-1] [13] <--sample:we want two bits K (sample 1 cycle too early)
    rjmp    haveTwoBitsK         ;2 [00] [14]
    pop     shift                ;2 	 [15+16] undo the push from before
    pop     YH 			 ;2 	 [17+18] undo the push from before
    rjmp    waitForK             ;2 	 [19+20] this was not the end of sync, retry
; The entire loop from waitForK until rjmp waitForK above must not exceed two
; bit times (= 20 cycles).

;----------------------------------------------------------------------------
; push more registers and initialize values while we sample the first bits:
;----------------------------------------------------------------------------
haveTwoBitsK:			;- [01]
    push    x1              	;2 [02+03]
    push    x2              	;2 [04+05]
    push    x3              	;2 [06+07]
    push    bitcnt              ;2 [08+09]	
    in      x1, USBIN       	;1 [00] [10] <-- sample bit 0
    bst     x1, USBMINUS    	;1 [01]
    bld     shift, 0        	;1 [02]
    push    cnt             	;2 [03+04]
    ldi     cnt, USB_BUFSIZE	;1 [05] 
    push    x4              	;2 [06+07] tx loop
    rjmp    rxLoop          	;2 [08]
;----------------------------------------------------------------------------
; Receiver loop (numbers in brackets are cycles within byte after instr)
;----------------------------------------------------------------------------
unstuff0:               	;- [07] (branch taken)
    andi    x3, ~0x01   	;1 [08]
    mov     x1, x2      	;1 [09] x2 contains last sampled (stuffed) bit
    in      x2, USBIN   	;1 [00] [10] <-- sample bit 1 again
    andi    x2, USBMASK 	;1 [01]
    breq    se0Hop         	;1 [02] SE0 check for bit 1 
    ori     shift, 0x01 	;1 [03] 0b00000001
    nop				;1 [04]
    rjmp    didUnstuff0 	;2 [05]
;-----------------------------------------------------
unstuff1:               	;- [05] (branch taken)
    mov     x2, x1      	;1 [06] x1 contains last sampled (stuffed) bit
    andi    x3, ~0x02   	;1 [07]
    ori     shift, 0x02 	;1 [08] 0b00000010
    nop                 	;1 [09]
    in      x1, USBIN   	;1 [00] [10] <-- sample bit 2 again
    andi    x1, USBMASK 	;1 [01]
    breq    se0Hop         	;1 [02] SE0 check for bit 2 
    rjmp    didUnstuff1 	;2 [03]
;-----------------------------------------------------
unstuff2:               	;- [05] (branch taken)
    andi    x3, ~0x04   	;1 [06]
    ori     shift, 0x04 	;1 [07] 0b00000100
    mov     x1, x2      	;1 [08] x2 contains last sampled (stuffed) bit
    nop                 	;1 [09]
    in      x2, USBIN   	;1 [00] [10] <-- sample bit 3
    andi    x2, USBMASK 	;1 [01]
    breq    se0Hop         	;1 [02] SE0 check for bit 3 
    rjmp    didUnstuff2 	;2 [03]
;-----------------------------------------------------
unstuff3:               	;- [00] [10]  (branch taken)
    in      x2, USBIN   	;1 [01] [11] <-- sample stuffed bit 3 one cycle too late
    andi    x2, USBMASK 	;1 [02]
    breq    se0Hop         	;1 [03] SE0 check for stuffed bit 3 
    andi    x3, ~0x08   	;1 [04]
    ori     shift, 0x08 	;1 [05] 0b00001000
    rjmp    didUnstuff3 	;2 [06]
;----------------------------------------------------------------------------
; extra jobs done during bit interval:
;
; bit 0:    store, clear [SE0 is unreliable here due to bit dribbling in hubs], 
; 		overflow check, jump to the head of rxLoop
; bit 1:    SE0 check
; bit 2:    SE0 check, recovery from delay [bit 0 tasks took too long]
; bit 3:    SE0 check, recovery from delay [bit 0 tasks took too long]
; bit 4:    SE0 check, none
; bit 5:    SE0 check, none
; bit 6:    SE0 check, none
; bit 7:    SE0 check, reconstruct: x3 is 0 at bit locations we changed, 1 at others
;----------------------------------------------------------------------------
rxLoop:				;- [09]
    in      x2, USBIN   	;1 [00] [10] <-- sample bit 1 (or possibly bit 0 stuffed)
    andi    x2, USBMASK 	;1 [01]
    brne    SkipSe0Hop		;1 [02]
se0Hop:				;- [02]
    rjmp    se0         	;2 [03] SE0 check for bit 1 
SkipSe0Hop:			;- [03]
    ser     x3          	;1 [04]
    andi    shift, 0xf9 	;1 [05] 0b11111001
    breq    unstuff0    	;1 [06]
didUnstuff0:			;- [06]
    eor     x1, x2      	;1 [07]
    bst     x1, USBMINUS	;1 [08]
    bld     shift, 1    	;1 [09] 
    in      x1, USBIN   	;1 [00] [10] <-- sample bit 2 (or possibly bit 1 stuffed)
    andi    x1, USBMASK 	;1 [01]
    breq    se0Hop         	;1 [02] SE0 check for bit 2 
    andi    shift, 0xf3 	;1 [03] 0b11110011
    breq    unstuff1    	;1 [04] do remaining work for bit 1
didUnstuff1:			;- [04]
    eor     x2, x1      	;1 [05]
    bst     x2, USBMINUS	;1 [06]
    bld     shift, 2    	;1 [07]
    nop2			;2 [08+09]
    in      x2, USBIN   	;1 [00] [10] <-- sample bit 3 (or possibly bit 2 stuffed)
    andi    x2, USBMASK 	;1 [01]
    breq    se0Hop         	;1 [02] SE0 check for bit 3 
    andi    shift, 0xe7 	;1 [03] 0b11100111
    breq    unstuff2    	;1 [04]
didUnstuff2:			;- [04]
    eor     x1, x2      	;1 [05]
    bst     x1, USBMINUS	;1 [06]
    bld     shift, 3    	;1 [07]
didUnstuff3:			;- [07]
    andi    shift, 0xcf 	;1 [08] 0b11001111
    breq    unstuff3    	;1 [09]
    in      x1, USBIN   	;1 [00] [10] <-- sample bit 4
    andi    x1, USBMASK 	;1 [01]
    breq    se0Hop         	;1 [02] SE0 check for bit 4
    eor     x2, x1      	;1 [03]
    bst     x2, USBMINUS	;1 [04]
    bld     shift, 4    	;1 [05]
didUnstuff4:			;- [05]
    andi    shift, 0x9f 	;1 [06] 0b10011111
    breq    unstuff4    	;1 [07]
    nop2			;2 [08+09]
    in      x2, USBIN   	;1 [00] [10] <-- sample bit 5
    andi    x2, USBMASK 	;1 [01]
    breq    se0         	;1 [02] SE0 check for bit 5
    eor     x1, x2      	;1 [03]
    bst     x1, USBMINUS	;1 [04]
    bld     shift, 5    	;1 [05]
didUnstuff5:			;- [05]
    andi    shift, 0x3f 	;1 [06] 0b00111111
    breq    unstuff5    	;1 [07]
    nop2			;2 [08+09]
    in      x1, USBIN   	;1 [00] [10] <-- sample bit 6
    andi    x1, USBMASK 	;1 [01]
    breq    se0         	;1 [02] SE0 check for bit 6
    eor     x2, x1      	;1 [03]
    bst     x2, USBMINUS	;1 [04]
    bld     shift, 6   	 	;1 [05]
didUnstuff6:			;- [05]
    cpi     shift, 0x02 	;1 [06] 0b00000010
    brlo    unstuff6    	;1 [07]
    nop2			;2 [08+09]
    in      x2, USBIN   	;1 [00] [10] <-- sample bit 7
    andi    x2, USBMASK 	;1 [01]
    breq    se0         	;1 [02] SE0 check for bit 7
    eor     x1, x2      	;1 [03]
    bst     x1, USBMINUS	;1 [04]
    bld     shift, 7    	;1 [05]
didUnstuff7:			;- [05] 
    cpi     shift, 0x04 	;1 [06] 0b00000100
    brlo    unstuff7		;1 [07]
    eor     x3, shift   	;1 [08] reconstruct: x3 is 0 at bit locations we changed, 1 at others
    nop				;1 [09]
    in      x1, USBIN   	;1 [00]	[10] <-- sample bit 0
    st      y+, x3      	;2 [01+02] store data
    eor     x2, x1      	;1 [03]
    bst     x2, USBMINUS	;1 [04]
    bld     shift, 0    	;1 [05]
    subi    cnt, 1		;1 [06]
    brcs    ignorePacket	;1 [07]
    rjmp    rxLoop		;2 [08]
;-----------------------------------------------------
unstuff4:               	;- [08] 
    andi    x3, ~0x10   	;1 [09]
    in      x1, USBIN   	;1 [00] [10] <-- sample stuffed bit 4
    andi    x1, USBMASK 	;1 [01]
    breq    se0         	;1 [02] SE0 check for stuffed bit 4
    ori     shift, 0x10 	;1 [03]
    rjmp    didUnstuff4 	;2 [04]
;-----------------------------------------------------
unstuff5:               	;- [08] 
    ori     shift, 0x20 	;1 [09]
    in      x2, USBIN   	;1 [00] [10] <-- sample stuffed bit 5
    andi    x2, USBMASK 	;1 [01]
    breq    se0         	;1 [02] SE0 check for stuffed bit 5
    andi    x3, ~0x20   	;1 [03]
    rjmp    didUnstuff5		;2 [04]
;-----------------------------------------------------
unstuff6:               	;- [08] 
    andi    x3, ~0x40   	;1 [09]
    in      x1, USBIN   	;1 [00] [10] <-- sample stuffed bit 6
    andi    x1, USBMASK 	;1 [01]
    breq    se0         	;1 [02] SE0 check for stuffed bit 6
    ori     shift, 0x40 	;1 [03]
    rjmp    didUnstuff6 	;2 [04]
;-----------------------------------------------------
unstuff7:			;- [08]
    andi    x3, ~0x80   	;1 [09]
    in      x2, USBIN   	;1 [00] [10] <-- sample stuffed bit 7
    andi    x2, USBMASK 	;1 [01]
    breq    se0         	;1 [02] SE0 check for stuffed bit 7
    ori     shift, 0x80 	;1 [03]
    rjmp    didUnstuff7 	;2 [04]
;----------------------------------------------------------------------------
; Processing of received packet (numbers in brackets are cycles after center of SE0)
;----------------------------------------------------------------------------
;This is the only non-error exit point for the software receiver loop
;we don't check any CRCs here because there is no time left.
#define token   x1
se0:					;- [04]
    subi    cnt, USB_BUFSIZE    	;1 [05]
    neg     cnt                 	;1 [06]
    cpi     cnt, 3              	;1 [07]
    ldi     x2, 1<<USB_INTR_PENDING_BIT ;1 [08]
    USB_STORE_PENDING(x2)           ;1 [09] clear pending intr and check flag later. SE0 should be over.
    brlo    doReturn            	;1 [10] this is probably an ACK, NAK or similar packet
    sub     YL, cnt             	;1 [11]
    sbci    YH, 0               	;1 [12]
    ld      token, y            	;2 [13+14]
    cpi     token, USBPID_DATA0 	;1 [15]
    breq    handleData          	;1 [16]
    cpi     token, USBPID_DATA1 	;1 [17]
    breq    handleData          	;1 [18]
    ldd     x2, y+1             	;2 [19+20] ADDR and 1 bit endpoint number
    mov     x3, x2              	;1 [21] store for endpoint number
    andi    x2, 0x7f            	;1 [22] x2 is now ADDR
    lds     shift, usbDeviceAddr	;2 [23+24]
    cp      x2, shift           	;1 [25]
    brne    ignorePacket        	;1 [26] packet for different address
    cpi     token, USBPID_IN    	;1 [27]
    breq    handleIn			;1 [28]
    cpi     token, USBPID_SETUP 	;1 [29]
    breq    handleSetupOrOut    	;1 [30]
    cpi     token, USBPID_OUT   	;1 [31]
    breq    handleSetupOrOut    	;1 [32]
;   rjmp    ignorePacket        	;       fallthrough, should not happen anyway.
ignorePacket:				;- [32]
    clr     shift			;1 [33]
    sts     usbCurrentTok, shift	;2 [34+35]
doReturn:
    pop     x4    
    pop     cnt
    pop     bitcnt
    pop     x3
    pop     x2
    pop     x1
    pop     shift
    pop     YH
sofError:
    pop     YL
    out     SREG, YL
    pop     YL
    reti
;-------------------------------------------------------------------------
#if USB_CFG_HAVE_INTRIN_ENDPOINT && USB_CFG_HAVE_INTRIN_ENDPOINT3
handleIn3:				;- [42]
    lds     cnt, usbTxLen3      	;2 [43+44]
    sbrc    cnt, 4              	;1 [45]
    rjmp    sendCntAndReti      	;1 [46] 46 + 16 = 62 until SOP
    sts     usbTxLen3, x1       	;2 [47+48] x1 == USBPID_NAK from above
    ldi     YL, lo8(usbTxBuf3)  	;1 [49]
    ldi     YH, hi8(usbTxBuf3)  	;1 [50]
    rjmp    usbSendAndReti      	;1 [51] 51 + 13 = 64 until SOP
#endif

;Setup and Out are followed by a data packet two bit times (20 cycles) after
;the end of SE0. The sync code allows up to 50 cycles delay from the start of
;the sync pattern until the first bit is sampled. That's a total of 70 cycles.
handleSetupOrOut:               	;[31/33]
#if USB_CFG_IMPLEMENT_FN_WRITEOUT   /* if we have data for second OUT endpoint, set usbCurrentTok to -1 */
    sbrc    x3, 7               	;[32] skip if endpoint 0
    ldi     token, -1           	;[33] indicate that this is endpoint 1 OUT
#endif
    sts     usbCurrentTok, token	;[34+35]
    pop     x4                  	;[36+37]
    pop     cnt                 	;[38+39]
    pop     bitcnt              	;[40+41]
    pop     x3                  	;[42+43]
    pop     x2                  	;[44+45]
    pop     x1                  	;[46+47]
    pop     shift               	;[48+49]
    pop	    YH                      ;[50+51]
    USB_LOAD_PENDING(YL)            ;[52]
    sbrc    YL, USB_INTR_PENDING_BIT	;[53] check whether data is already arriving
    rjmp    waitForJ            	;[54] save the pops and pushes -- a new interrupt is aready pending
    rjmp    sofError            	;[55] not an error, but it does the pops and reti we want
;--------------------------------------------------------------------
handleData:				;- [16/18]
    lds     token, usbCurrentTok	;2 [19+20]
    tst     token               	;1 [21]
    breq    doReturn            	;1 [22]
    lds     x2, usbRxLen        	;2 [23+24]
    tst     x2                  	;1 [25]
    brne    sendNakAndReti      	;1 [26]
; 2006-03-11: The following two lines fix a problem where the device was not
; recognized if usbPoll() was called less frequently than once every 4 ms.
    cpi     cnt, 4              	;1 [27] 0 sized data packets are status phase only -- ignore and ack
    brmi    sendAckAndReti      	;1 [28] keep rx buffer clean -- we must not NAK next SETUP
    sts     usbRxLen, cnt       	;2 [29+30] store received data, swap buffers
    sts     usbRxToken, token   	;2 [31+23]
    lds     x2, usbInputBufOffset	;2 [33+34] swap buffers
    ldi     cnt, USB_BUFSIZE    	;1 [35]
    sub     cnt, x2             	;1 [36]
    sts     usbInputBufOffset, cnt	;2 [37+38] buffers now swapped
    rjmp    sendAckAndReti      	;1 [39] 39 + 17 = 56 until SOP
;--------------------------------------------------------------------
;We don't send any data as long as the C code has not processed the current
;input data and potentially updated the output data. That's more efficient
;in terms of code size than clearing the tx buffers when a packet is received.
handleIn:				;- [29]
    lds     x1, usbRxLen        	;2 [30+31]
    cpi     x1, 1               	;1 [32] negative values are flow control, 0 means "buffer free"
    brge    sendNakAndReti      	;1 [33] unprocessed input packet?
    ldi     x1, USBPID_NAK      	;1 [34] prepare value for usbTxLen
#if USB_CFG_HAVE_INTRIN_ENDPOINT
    sbrc    x3, 7               	;1 [35] x3 contains addr + endpoint
    rjmp    handleIn1           	;1 [36]
#endif
    lds     cnt, usbTxLen       	;2 [37+38]
    sbrc    cnt, 4              	;2 [39] all handshake tokens have bit 4 set
    rjmp    sendCntAndReti      	;1 [40] 42 + 16 = 58 until SOP
    sts     usbTxLen, x1        	;2 [41+42] x1 == USBPID_NAK from above
    ldi     YL, lo8(usbTxBuf)   	;1 [43]
    ldi     YH, hi8(usbTxBuf)   	;1 [44]
    rjmp    usbSendAndReti      	;1 [45] 45 + 13 = 58 until SOP
;---------------------------------------------------------------------------------------
; Comment about when to set usbTxLen to USBPID_NAK:
; We should set it back when we receive the ACK from the host. This would
; be simple to implement: One static variable which stores whether the last
; tx was for endpoint 0 or 1 and a compare in the receiver to distinguish the
; ACK. However, we set it back immediately when we send the package,
; assuming that no error occurs and the host sends an ACK. We save one byte
; RAM this way and avoid potential problems with endless retries. The rest of
; the driver assumes error-free transfers anyway.

#if USB_CFG_HAVE_INTRIN_ENDPOINT    /* placed here due to relative jump range */
handleIn1:                      	;- [37]
#if USB_CFG_HAVE_INTRIN_ENDPOINT3
; 2006-06-10 as suggested by O.Tamura: support second INTR IN / BULK IN endpoint
    ldd     x2, y+2             	;2 [38+39]
    sbrc    x2, 0               	;2 [40]
    rjmp    handleIn3           	;1 [41]
#endif
    lds     cnt, usbTxLen1      	;2 [42+43]
    sbrc    cnt, 4              	;2 [44] all handshake tokens have bit 4 set
    rjmp    sendCntAndReti      	;1 [45] 45 + 16 = 61 until SOP
    sts     usbTxLen1, x1       	;2 [46+47] x1 == USBPID_NAK from above
    ldi     YL, lo8(usbTxBuf1)  	;1 [48]
    ldi     YH, hi8(usbTxBuf1)  	;1 [49]
    rjmp    usbSendAndReti      	;1 [50] 50 + 13 + 63 until SOP
#endif
;---------------------------------------------------------------------------
; USB spec says:
; idle = J
; J = (D+ = 0), (D- = 1)
; K = (D+ = 1), (D- = 0)
; Spec allows 7.5 bit times from EOP to SOP for replies
;---------------------------------------------------------------------------
bitstuffN:		    	;- [04]
    eor     x1, x4          	;1 [05]
    clr	    x2			;1 [06]
    nop				;1 [07]
    rjmp    didStuffN       	;1 [08]
;---------------------------------------------------------------------------    
bitstuff6:		    	;- [04]
    eor     x1, x4          	;1 [05]
    clr	    x2			;1 [06]
    rjmp    didStuff6       	;1 [07]
;---------------------------------------------------------------------------
bitstuff7:		    	;- [02]
    eor     x1, x4          	;1 [03]
    clr	    x2			;1 [06]
    nop			    	;1 [05]
    rjmp    didStuff7       	;1 [06]
;---------------------------------------------------------------------------
sendNakAndReti:			;- [-19]
    ldi     x3, USBPID_NAK  	;1 [-18]
    rjmp    sendX3AndReti   	;1 [-17]
;---------------------------------------------------------------------------
sendAckAndReti:			;- [-17]
    ldi     cnt, USBPID_ACK 	;1 [-16]
sendCntAndReti:			;- [-16]
    mov     x3, cnt         	;1 [-15]
sendX3AndReti:			;- [-15]
    ldi     YL, 20          	;1 [-14] x3==r20 address is 20
    ldi     YH, 0           	;1 [-13]
    ldi     cnt, 2          	;1 [-12]
;   rjmp    usbSendAndReti      fallthrough
;---------------------------------------------------------------------------
;usbSend:
;pointer to data in 'Y'
;number of bytes in 'cnt' -- including sync byte [range 2 ... 12]
;uses: x1...x4, btcnt, shift, cnt, Y
;Numbers in brackets are time since first bit of sync pattern is sent
;We need not to match the transfer rate exactly because the spec demands 
;only 1.5% precision anyway.
usbSendAndReti:             	;- [-13] 13 cycles until SOP
    in      x2, USBDDR      	;1 [-12]
    ori     x2, USBMASK     	;1 [-11]
    sbi     USBOUT, USBMINUS	;2 [-09-10] prepare idle state; D+ and D- must have been 0 (no pullups)
    in      x1, USBOUT      	;1 [-08] port mirror for tx loop
    out     USBDDR, x2      	;1 [-07] <- acquire bus
	; need not init x2 (bitstuff history) because sync starts with 0 
    ldi     x4, USBMASK     	;1 [-06] 	exor mask
    ldi     shift, 0x80     	;1 [-05] 	sync byte is first byte sent
    ldi     bitcnt, 6    	;1 [-04] 
txBitLoop:		    	;- [-04] [06]
    sbrs    shift, 0        	;1 [-03] [07]
    eor     x1, x4          	;1 [-02] [08] 
    ror     shift           	;1 [-01] [09]  
didStuffN:		    	;-       [09]
    out     USBOUT, x1      	;1 [00]  [10] <-- out N
    ror     x2              	;1 [01]
    cpi     x2, 0xfc        	;1 [02]
    brcc    bitstuffN       	;1 [03]
    dec     bitcnt          	;1 [04]
    brne    txBitLoop       	;1 [05]
    sbrs    shift, 0        	;1 [06]
    eor     x1, x4          	;1 [07]
    ror     shift           	;1 [08]
didStuff6:			;- [08]
    nop				;1 [09]
    out     USBOUT, x1      	;1 [00] [10] <-- out 6
    ror     x2              	;1 [01] 
    cpi     x2, 0xfc        	;1 [02]
    brcc    bitstuff6       	;1 [03]
    sbrs    shift, 0        	;1 [04]
    eor     x1, x4          	;1 [05]
    ror     shift           	;1 [06]
    ror     x2              	;1 [07]
didStuff7:			;- [07]
    ldi     bitcnt, 6    	;1 [08]
    cpi     x2, 0xfc        	;1 [09]
    out     USBOUT, x1      	;1 [00] [10] <-- out 7
    brcc    bitstuff7       	;1 [01]
    ld      shift, y+       	;2 [02+03]
    dec     cnt             	;1 [04]
    brne    txBitLoop      	;1 [05]
makeSE0:
    cbr     x1, USBMASK     	;1 [06] 	prepare SE0 [spec says EOP may be 19 to 23 cycles]
    lds     x2, usbNewDeviceAddr;2 [07+08]
;2006-03-06: moved transfer of new address to usbDeviceAddr from C-Code to asm:
;set address only after data packet was sent, not after handshake
    subi    YL, 2           	;1 [09]
    out     USBOUT, x1      	;1 [00] [10] <-- out SE0-- from now 2 bits==20 cycl. until bus idle
    sbci    YH, 0           	;1 [01]
    breq    skipAddrAssign  	;1 [02]
    sts     usbDeviceAddr, x2	;2 [03+04] if not skipped: SE0 is one cycle longer
;----------------------------------------------------------------------------
;end of usbDeviceAddress transfer
skipAddrAssign:				;- [03/04]
    ldi     x2, 1<<USB_INTR_PENDING_BIT	;1 [04] int0 occurred during TX -- clear pending flag
    USB_STORE_PENDING(x2)           ;1 [05]
    ori     x1, USBIDLE     		;1 [06]
    in      x2, USBDDR      		;1 [07]
    cbr     x2, USBMASK     		;1 [08] set both pins to input
    mov     x3, x1          		;1 [09]
    cbr     x3, USBMASK     		;1 [10] configure no pullup on both pins
    ldi     x4, 3           		;1 [11]
se0Delay:				;- [11] [14] 
    dec     x4              		;1 [12] [15] 
    brne    se0Delay        		;1 [13] [16] 
    nop2				;2      [17+18]
    out     USBOUT, x1      		;1      [19] <--out J (idle) -- end of SE0 (EOP sig.)
    out     USBDDR, x2      		;1      [20] <--release bus now
    out     USBOUT, x3      		;1      [21] <--ensure no pull-up resistors are active
    rjmp    doReturn			;1	[22]
;---------------------------------------------------------------------------
