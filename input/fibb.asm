    lui $a0, 30

    lui $t1, 1
    beq $a0, $zero, END
    beq $a0, $t1, END

    lui $t2, 0
    lui $t3, 0
    lui $t4, 1
    lui $t5, 1

LOOP:
    sub $t6, $a0, $t5
    blez $t6, ENDLOOP
    add $t2, $t3, $t4
    movz $t3, $t4, $zero
    movz $t4, $t2, $zero
    addi $t5, $t5, 1
    j LOOP

ENDLOOP:
    addi $a0, $t2, 0

END:
    addi $v0, $a0, 0