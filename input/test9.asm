    lui $t1, 2
    lui $t3, 3
    bne $t1, $t3, B1
    mult $t1, $t3
B1:
    lui $t3, 2
    bne $t1, $t3, B1