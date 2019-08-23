    lui $a0, 5
    lui $a1, 12
    lui $t1, 0
LOOP:
    sub $t2, $a0, $t1
    blez $t2, END
    add $t0, $t0, $a1
    addi $t1, $t1, 1
    j LOOP
END: