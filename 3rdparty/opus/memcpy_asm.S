    .file           "memcpy_asm.S"
    .section        .text.memcpy_asm,"ax",@progbits
    .align          2
    .global         memcpy_asm
    .type           memcpy_asm, @function

memcpy_asm:
    mov t0, a0
    or t1, a1, a0
    andi t2, t1, 3

    bez t2, .L23
    bez a2, .L5
.Lloop1start:
    ldbi.b a3, (a1)
.Lloop1end:
    stbi.b a3, (t0)
    bloop a2, .Lloop1start, .Lloop1end
.L5:
    rts
.L23:
    lsri  t1, a2, 4
    bez t1, .L20
.Lloop16start:
    pldbi.d t2, (a1)
    pldbi.d t4, (a1)
    st.w t2, (t0)
    st.w t3, (t0, 4)
    st.w t4, (t0, 8)
    st.w t5, (t0, 12)
.Lloop16end:
    addi t0, t0, 16
    bloop t1, .Lloop16start, .Lloop16end
.L20:
    andi  a2, a2, 0xf
    lsri  t1, a2, 2
    bez t1, .L21
.Lloop4start:
    ldbi.w a3, (a1)
.Lloop4end:
    stbi.w a3, (t0)
    bloop t1, .Lloop4start, .Lloop4end
.L21:
    andi  a2, a2, 3
    bez a2, .L5
    ldbi.b a3, (a1)
    subi a2, a2, 1
    stbi.b a3, (t0)
    br .L21
    .size memcpy_asm, .-memcpy_asm
