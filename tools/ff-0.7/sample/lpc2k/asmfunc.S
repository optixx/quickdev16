@-----------------------------------------------------------@
@ LPC2300 startup code and asm functions
@-----------------------------------------------------------@


.equ UND_Stack_Size,	0
.equ SVC_Stack_Size,	256	@ Used by ISRs and SWIs
.equ ABT_Stack_Size,	0
.equ FIQ_Stack_Size,	0	@ Used by FIQ handler
.equ IRQ_Stack_Size,	256	@ Requres ISR nesting level * 28 bytes
.equ USR_Stack_Size,	1024	@ Used by user mode programs
.equ Stack_Size,	(UND_Stack_Size + SVC_Stack_Size + ABT_Stack_Size + \
			FIQ_Stack_Size + IRQ_Stack_Size + USR_Stack_Size)
.equ Heap_Size,		0

.equ B_Irq,	0x80
.equ B_Fiq,	0x40
.equ B_Thumb,	0x20
.equ M_USR,	0x10
.equ M_FIQ,	0x11
.equ M_IRQ,	0x12
.equ M_SVC,	0x13
.equ M_ABT,	0x17
.equ M_UND,	0x1B
.equ M_SYS,	0x1F



@-----------------------------------------------------------@
@ Stack area (located in RAM)
@-----------------------------------------------------------@

.arm
.section .STACK, "w"
.align 3
Stack_Mem:
.space Stack_Size
.equ Stack_Top, Stack_Mem + Stack_Size



@-----------------------------------------------------------@
@ Heap area (located in RAM)
@-----------------------------------------------------------@

.section .HEAP, "w"
.align 3
HeapMem:
.if (Heap_Size > 0)
.space Heap_Size
.endif



@-----------------------------------------------------------@
@ Exception entries (located in ROM, address 0x00000000)
@-----------------------------------------------------------@

.section .VECTOR, "ax"
.arm

	LDR	PC, [PC, #24]	@ Reset entry, jump to reset handler
	LDR	PC, [PC, #24]	@ Undef entry, trap
	LDR	PC, [PC, #24]	@ SWI entry, jump to SWI handler
	LDR	PC, [PC, #24]	@ PAbt entry, trap
	LDR	PC, [PC, #24]	@ DAbt entry, trap
	.word	0
	LDR	PC, [PC, #20]	@ IRQ entry, jump to IRQ handler
	LDR     PC, [PC, #20]	@ FIQ entry, trap

	.word	Reset_Handler	@ Reset handler
	.word	Trap		@ Undefined Instruction handler
	.word	SWI_Handler	@ Software Interrupt handler
	.word	Trap		@ Prefetch Abort handler
	.word	Trap		@ Data Abort handler
	.word	IRQ_Handler	@ IRQ handler
	.word	Trap		@ FIQ handler

Trap:	B	Trap		@ Unused exception trap (infinite loop)



@-----------------------------------------------------------@
@ Reset Handler
@-----------------------------------------------------------@

.section .text, "ax"
.arm
Reset_Handler:

@.extern TargetResetInit
@	LDR	SP, =Stack_Top    @ Temporary stack for TargetResetInit()
@	LDR	R0, =TargetResetInit
@	MOV	LR, PC
@	BX	R0

@ Setup Stack for each mode
	LDR	R0, =Stack_Top

@ Enter Undefined Instruction Mode and set its Stack Pointer
	MSR	CPSR_c, #M_UND | B_Irq | B_Fiq
	MOV	SP, R0
	SUB	R0, R0, #UND_Stack_Size

@ Enter Abort Mode and set its Stack Pointer
	MSR	CPSR_c, #M_ABT | B_Irq | B_Fiq
	MOV	SP, R0
	SUB	R0, R0, #ABT_Stack_Size

@ Enter FIQ Mode and set its Stack Pointer
	MSR	CPSR_c, #M_FIQ | B_Irq | B_Fiq
	MOV	SP, R0
	SUB	R0, R0, #FIQ_Stack_Size

@ Enter IRQ Mode and set its Stack Pointer
	MSR	CPSR_c, #M_IRQ | B_Irq | B_Fiq
	MOV	SP, R0
	SUB	R0, R0, #IRQ_Stack_Size

@ Enter Supervisor Mode and set its Stack Pointer
	MSR	CPSR_c, #M_SVC | B_Irq | B_Fiq
	MOV	SP, R0
	SUB	R0, R0, #SVC_Stack_Size

@ Enter User Mode and set its Stack Pointer
	MSR	CPSR_c, #M_USR | B_Irq | B_Fiq
	MOV	SP, R0
	SUB	SL, SP, #USR_Stack_Size

@ Relocate .data section (Initialize with ROM data)
	LDR	R1, =_etext
	LDR	R2, =_data
	LDR	R3, =_edata
	CMP	R2, R3
	BEQ	DataIsEmpty
LoopRel:CMP	R2, R3
	LDRLO	R0, [R1], #4
	STRLO	R0, [R2], #4
	BLO	LoopRel
DataIsEmpty:

@ Clear .bss section (Initialize with 0)
	MOV	R0, #0
	LDR	R1, =__bss_start__
	LDR	R2, =__bss_end__
	CMP	R1,R2
	BEQ	BSSIsEmpty
LoopZI:	CMP	R1, R2
	STRLO	R0, [R1], #4
	BLO	LoopZI
BSSIsEmpty:

@ Start main()
.extern main
	LDR	R0, =main
	MOV	LR, PC
	BX	R0
MTrap:	B	MTrap	@ Trap if main() terminated



@-----------------------------------------------------------@
@ IRQ Handler
@ Prologue and Epilog for all ISRs are handled here
@-----------------------------------------------------------@

.equ LPC_BASE_VIC,	0xFFFFF000
.equ VIC_IntSelect,	0x00C
.equ VIC_IntEnable,	0x010
.equ VIC_IntEnClear,	0x014
.equ VIC_Protection,	0x020
.equ VIC_SWPriorityMask,0x024
.equ VIC_VectAddr0,	0x100
.equ VIC_VectPriority0,	0x200
.equ VIC_VectAddr,	0xF00

.arm
IRQ_Handler:
	SUB	LR, LR, #4		@ Adjust LR_irq and push it
	STMFD	SP!, {LR}

	MRS	LR, SPSR		@ Save SPSR need to be saved for nested interrupt
	STMFD	SP!, {R0-R3,IP,LR}	@ Push scratch/used registers and SPSR
	LDR	R0, =LPC_BASE_VIC	@ Get the ISR address pointed by VIC_VectAddr
	LDR	R0, [R0, #VIC_VectAddr]
	MSR	CPSR_c, #M_SVC		@ Enter SVC mode and enable Irq and Fiq

	STMFD	SP!, {LR}		@ Call the ISR
	MOV	LR, PC
	BX	R0
	LDMIA	SP!, {LR}

	MSR	CPSR_c, #M_IRQ | B_Irq	@ Enter IRQ mode and disable Irq
	LDMIA	SP!, {R0-R3,IP,LR}	@ Restore scratch/used registers and SPSR
	MSR	SPSR_cxsf, LR		@ Restore SPSR_irq

	LDR	LR, =LPC_BASE_VIC	@ Issue EOI command to the VIC
	STR	LR, [LR, #VIC_VectAddr]

	LDMIA	SP!, {PC}^		@ Reruen from the IRQ handler



@-----------------------------------------------------------@
@ SWI Service (declared in interrupt.h)
@-----------------------------------------------------------@

.equ SWI_IRQ_DIS,	0
.equ SWI_IRQ_EN,	1
.equ SWI_FIQ_DIS,	2
.equ SWI_FIQ_EN,	3
.equ SWI_CLR_VECT,	4
.equ SWI_REG_VECT,	5

.equ NUM_SWI,		6

.arm
SWI_Handler:
	LDR	IP, [LR, #-4]		@ Get swi instruction code (assuming ARM state)
	AND	IP, #0xFF		@ Get swi comment field (lower 8 bit)
	CMP	IP, #NUM_SWI		@ Check range
	LDRLO	PC, [PC, IP, LSL #2]	@ Jump each service function when code is valid
	MOVS	PC, LR			@ Otherwise return
	.word	IRQDisable
	.word	IRQEnable
	.word	FIQDisable
	.word	FIQEnable
	.word	ClearVect
	.word	RegVect

IRQDisable:
	MRS	R0, SPSR
	ORR	R0, R0, #B_Irq
	MSR	SPSR_c, R0
	MOVS	PC, LR

IRQEnable:
	MRS	R0, SPSR
	BIC	R0, R0, #B_Irq
	MSR	SPSR_c, R0
	MOVS	PC, LR

FIQDisable:
	MRS	R0, SPSR
	ORR	R0, R0, #B_Fiq
	MSR	SPSR_c, R0
	MOVS	PC, LR

FIQEnable:
	MRS	R0, SPSR
	BIC	R0, R0, #B_Fiq
	MSR	SPSR_c, R0
	MOVS	PC, LR

ClearVect:
	LDR	IP, =LPC_BASE_VIC
	MVN	R0, #0				@ Disable all interrupts
	STR	R0, [IP, #VIC_IntEnClear]
	MOV	R0, R0, LSR #16			@ Unmask all interrupt levels
	STR	R0, [IP, #VIC_SWPriorityMask]
	MOV	R0, #1				@ Enable protection
	STR	R0, [IP, #VIC_Protection]
	STR	R0, [IP, #VIC_VectAddr]		@ Issule EOI command
	MOVS	PC, LR

RegVect:
	CMP	R0, #32				@ Range check
	MOVCSS	PC, LR
	LDR	IP, =(LPC_BASE_VIC+VIC_VectAddr0)
	STR	R1, [IP, R0, LSL #2]		@ Set VICVectVectAddr<n>
	LDR	IP, =(LPC_BASE_VIC+VIC_VectPriority0)
	STR	R2, [IP, R0, LSL #2]		@ Set VICVectPriority<n>
	MOV	R1, #1
	MOV	R1, R1, LSL R0
	LDR	IP, =LPC_BASE_VIC
	LDR	R2, [IP, #VIC_IntSelect]	@ Set corresponding bit in the VICIntSelect
	BIC	R2, R1
	CMP	R3, #1
	ORREQ	R2, R1
	STR	R2, [IP, #VIC_IntSelect]
	STR	R1, [IP, #VIC_IntEnable]	@ Enable corresponding interrupt
	MOVS	PC, LR

.global IrqDisable
.arm
IrqDisable:
	SWI	SWI_IRQ_DIS
	BX	LR

.global IrqEnable
.arm
IrqEnable:
	SWI	SWI_IRQ_EN
	BX	LR

.global FiqDisable
.arm
FiqDisable:
	SWI	SWI_FIQ_DIS
	BX	LR

.global FiqEnable
.arm
FiqEnable:
	SWI	SWI_FIQ_EN
	BX	LR

.global ClearVector
.arm
ClearVector:
	SWI	SWI_CLR_VECT
	BX	LR

.global RegisterVector
.arm
RegisterVector:
	SWI	SWI_REG_VECT
	BX	LR



@-----------------------------------------------------------@
@ Fast Block Copy (declared in mci.c)
@-----------------------------------------------------------@

.global Load_Block
.arm
Load_Block:
	STMFD	SP!, {R4-R8}
	ANDS	IP, R1, #3
	BEQ	lb_align

	BIC	R1, #3
	MOV	IP, IP, LSL #3
	RSB	R8, IP, #32
	LDMIA	R1!, {R7}
lb_l1:	MOV	R3, R7
	LDMIA	R1!, {R4-R7}
	MOV	R3, R3, LSR IP
	ORR	R3, R3, R4, LSL R8
	MOV	R4, R4, LSR IP
	ORR	R4, R4, R5, LSL R8
	MOV	R5, R5, LSR IP
	ORR	R5, R5, R6, LSL R8
	MOV	R6, R6, LSR IP
	ORR	R6, R6, R7, LSL R8
	SUBS	R2, R2, #16
	STMIA	R0!, {R3-R6}
	BNE	lb_l1
	LDMFD	SP!, {R4-R8}
	BX	LR

lb_align:
	LDMIA	R1!, {R3-R6}
	SUBS	R2, R2, #16
	STMIA	R0!, {R3-R6}
	BNE	lb_align
	LDMFD	SP!, {R4-R8}
	BX	LR


.global Store_Block
.arm
Store_Block:
	STMFD	SP!, {R4-R8}
	ANDS	IP, R0, #3
	BEQ	sb_align

	MOV	IP, IP, LSL #3
	RSB	R8, IP, #32

	LDMIA	R1!, {R4-R7}
sb_p1:	STRB	R4, [R0], #1
	MOV	R4, R4, LSR #8
	TST	R0, #3
	BNE	sb_p1
	ORR	R4, R4, R5, LSL IP
	MOV	R5, R5, LSR R8
	ORR	R5, R5, R6, LSL IP
	MOV	R6, R6, LSR R8
	ORR	R6, R6, R7, LSL IP
	SUBS	R2, R2, #16
	STMIA	R0!, {R4-R6}

sb_l1:	MOV	R3, R7
	LDMIA	R1!, {R4-R7}
	MOV	R3, R3, LSR R8
	ORR	R3, R3, R4, LSL IP
	MOV	R4, R4, LSR R8
	ORR	R4, R4, R5, LSL IP
	MOV	R5, R5, LSR R8
	ORR	R5, R5, R6, LSL IP
	MOV	R6, R6, LSR R8
	ORR	R6, R6, R7, LSL IP
	SUBS	R2, R2, #16
	STMIA	R0!, {R3-R6}
	BNE	sb_l1

	MOV	R7, R7, LSR R8
sb_p2:	SUBS	IP, IP, #8
	STRB	R7, [R0], #1
	MOV	R7, R7, LSR #8
	BNE	sb_p2

	LDMFD	SP!, {R4-R8}
	BX	LR

sb_align:
	LDMIA	R1!, {R3-R6}
	SUBS	R2, #16
	STMIA	R0!, {R3-R6}
	BNE	sb_align
	LDMFD	SP!, {R4-R8}
	BX	LR

.end

