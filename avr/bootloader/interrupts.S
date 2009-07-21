.extern __init, __vector_1
.global __vector_default, exit
.section .vectors.bootloader

/* micro-jumptable, we are using just reset and int0 vectors */
exit:
__vector_default:
jmp __init
jmp __vector_1
