#include <csi_config.h>
#include <autoconf.h>
#include <board_config.h>
#include <soc.h>

.import tspend_handler
.import CORET_IRQHandler
.import Default_Handler

.section .vectors
    .align  10
    .globl  __Vectors
    .type   __Vectors, @object
__Vectors:
    .long   reset_handler
    .rept   21
    .long   Default_Handler
    .endr
    .long   tspend_handler
    .rept   9
    .long   Default_Handler
    .endr

    /* External interrupts */
    .rept NR_IRQS
    .long gx_irq_handler
    .endr

    .size    __Vectors, . - __Vectors

    .text
    .align   2
_start:
    .text
    .align   2
    .globl   reset_handler
    .type    reset_handler, %function
reset_handler:
    lrw      r0, 0x80000200
    mtcr     r0, psr

    // stop RTS to predict
    mfcr     r1, cr<31,0>
    bclri    r1, 3
    mtcr     r1, cr<31,0>


stage2_stack_set:
    lrw     r3, CONFIG_STAGE2_STACK
    mov     sp, r3
    jbsr    system_init

    jbsr    main
    .size   reset_handler, . - reset_handler

__exit:
    br __exit

    .text
    .align   2
    .globl   clear_bss
    .type    clear_bss, %function
clear_bss:
    push    r4-r6
    lrw     r5, _end_bss_
    lrw     r6, _start_bss_
    cmphs   r6, r5
    bt      2f
    xor     r4,r4
1:
    st      r4,(r6)
    addi    r6,4
    cmphs   r6, r5
    bf      1b
2:
    pop     r4-r6
    rts
    bkpt
