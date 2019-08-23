    lui $a0, 5
    lui $t1, 0
    lui $t2, 1
    lui $t3, 2
    beq $a0, $t1, J1
    beq $a0, $t2, J2
    beq $a0, $t3, J3
    j J4
J1:
    add $t4, $zero, $zero
    j J5
J2:
J3:
    addi $t4, $zero, 1
    j J5
J4:
    addi $t4, $zero, 2
J5: