    lui $a0, 7

    lui $t1, 1
    beq $a0, $zero, END
    beq $a0, $t1, END

    lui $t2, 1

LOOP:
    addi $t2, $t2, 1
    mul $t1, $t1, $t2
    beq $a0, $t2, ENDLOOP
    j LOOP

ENDLOOP:
    addi $a0, $t1, 0

END:
    addi $v0, $a0, 0