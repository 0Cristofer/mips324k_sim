    lui $t0, 10
    lui $t1, 20
    add $t3, $t0, $t1
    addi $t0, $t3, 15
    b J1
J2:
    xor $t6, $t0, $t1
    xori $t5, $t6, 0
    lui $t0, 1
    lui $t1, 1
    beq $t0, $t1, J3
J1:
    or $t4, $t0, $t1
    ori $t5, $t3, 1
    nor $t4, $t3, $t2
    beql $t4, $t5, J1
    j J2
J3:
    lui $t1, 1
    lui $t3, 2
    lui $t4, 0
    movn $t2, $t3, $t0
    movz $t3, $t1, $t4
    and $t3, $t4, $t1
    andi $t3, $t3, 1
    bgez $t3, J4
J5:
    mfhi $t2
    blez $t2, J6
J4:
    lui $t1, 3
    lui $t2, 5
    mult $t1, $t2
    mflo $t3
    bgtz $t3, J5
J6:
    lui $t4, 10
    sub $t1, $t4, $t3
    bltz $t1, J7
J8:
    mtlo $t1
    madd $t1, $t0
    mtlo $t0
    msub $t1, $t0
    j J9
J7:
    lui $t5, 9
    lui $t3, 3
    div $t5, $t3
    mflo $t0
    mfhi $t1
    bne $t1, $t0, J8
J10:
    syscall
J9:
    lui $t0, 5
    lui $t1, 7
    mul $t2, $t0, $t1
    j J10