    lui $t0, 1
    lui $t1, 100
    lui $t3, 100
    bne $t1, $t3, B1
    mult $t1, $t3
B1:
    sub $t3, $t3, $t0
    bne $zero, $t3, B1