L1: add $t1, $t2, $t3
    addi $t1, $t2, 23412
    and $t1, $t2, $t3
L2: andi $t1, $t2, 67434
    b L1
    beq $t1, $t2, L2
L5: beql $t1, $t2, L3
    bgez $t1, L3
    bgtz $t1, L4
    blez $t1, L5
    b L6
    bltz $t1, L6
    bne $t1, $t2, L7
L7: div $t1, $t2
    j L2
L6:
    jr $t1
    lui $t1, 34125
    madd $t1, $t2
L3:
    mfhi $t1
    mflo $t1
    movn $t1, $t2, $t3
    movz $t1, $t2, $t3
    msub $t1, $t2
    mthi $t1
L4: mtlo $t1
    mul $t1, $t2, $t3
    mult $t1, $t2
    nop
    nor $t1, $t2, $t3
    or $t1, $t2, $t3
    ori $t1, $t2, 23434
    sub $t1, $t2, $t3
    syscall
    xor $t1, $t2, $t3
    xori $t1, $t2, 34514
