    lui $a0, 6

    lui $t1, 1
    beq $a0, $t0, R0
    beq $a0, $t1, END

    lui $t2, 0
    lui $t3, 0
    lui $t4, 1
    lui $t5, 1

LOOP:
    sub $t6, $a0, $t5 #8
    blez $t6, ENDLOOP
    add $t2, $t3, $t4
    addi $t3, $t4, 0
    addi $t4, $t2, 0
    addi $t5, $t5, 1
    j LOOP

ENDLOOP:
    addi $a0, $t2, 0
    j END

R0:
    j END

END:
    addi $v0, $a0, 0