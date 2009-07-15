.extern __init, __vector_1
.global __vector_default, exit
.section .vectors.bootloader

/* micro-jumptable, we are using just reset and int0 vectors */
exit:
__vector_default:
rjmp __init
rjmp __vector_1
