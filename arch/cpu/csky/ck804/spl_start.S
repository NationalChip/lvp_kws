#include <autoconf.h>
#include <board_config.h>
#include <soc.h>

.import reset_handler

.section .spl.vectors
    .align  10
    .globl  __spl_vectors
    .type   __spl_vectors, @object
__spl_vectors:
    .long   spl_reset_handler
    .rept   31
    .long   spl_default_handler
    .endr

    /* External interrupts */
    .rept NR_IRQS
    .long spl_default_handler
    .endr

    .size    __spl_vectors, . - __spl_vectors

    .text
    .align   2
_spl_start:
    .text
    .align   2
    .globl   spl_reset_handler
    .type    spl_reset_handler, %function
spl_reset_handler:
    lrw      r0, 0x80000200
    mtcr     r0, psr

    // stop RTS to predict
    mfcr     r1, cr<31,0>
    bclri    r1, 3
    mtcr     r1, cr<31,0>

    // set spl vector table
    lrw      r0, __spl_vectors
    mtcr     r0, vbr

    // set spl stack
    lrw      r0, CONFIG_STAGE1_STACK
    mov      sp, r0

stage1_init:
    jbsr    spl_board_init_r

    jbsr    reset_handler
    .size   spl_reset_handler, . - spl_reset_handler

    .text
    .align  2
    .type   spl_default_handler, %function
spl_default_handler:
    br      spl_default_handler
    .size   spl_default_handler, . - spl_default_handler
    .text
    .align   2
    .globl   spl_clear_bss
    .type    spl_clear_bss, %function
spl_clear_bss:
    rts
