    lui $a0, 10
    lui $a1, 14
    sub $t2, $a0, $a1
    blez $t2, SKIP
    add $t2, $zero, $zero
SKIP: