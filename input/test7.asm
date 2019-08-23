    lui $t0, 0
    lui $t1, 2
    lui $t2, 5
    lui $t3, 1
    lui $t4, 10
    lui $t5, 9
    lui $t6, 0
    lui $t7, 20
    lui $t8, 100
    lui $a0, 5
    lui $a1, 2

    sub $v0, $t0, $t1
    blez $v0, B1
    j OUT1
B1:
    add $t0, $t1, $zero
OUT1:
    sub $v0, $t0, $t2
    blez $v0, B2
    j OUT2
B2:
    add $t0, $t2, $zero
OUT2:
    sub $v0, $t0, $t3
    blez $v0, B3
    j OUT3
B3:
    add $t0, $t3, $zero
OUT3:
    sub $v0, $t0, $t4
    blez $v0, B4
    j OUT4
B4:
    add $t0, $t4, $zero
OUT4:
    sub $v0, $t0, $t5
    blez $v0, B5
    j OUT5
B5:
    add $t0, $t5, $zero
OUT5:
    sub $v0, $t0, $t6
    blez $v0, B6
    j OUT6
B6:
    add $t0, $t6, $zero
OUT6:
    sub $v0, $t0, $t7
    blez $v0, B7
    j OUT7
B7:
    add $t0, $t7, $zero
OUT7:
    sub $v0, $t0, $t8
    blez $v0, B8
    j OUT8
B8:
    add $t0, $t8, $zero
OUT8:
    sub $v0, $t0, $a0
    blez $v0, B9
    j OUT9
B9:
    add $t0, $a0, $zero
OUT9:
    sub $v0, $t0, $a1
    blez $v0, B10
    j OUT10
B10:
    add $t0, $a1, $zero
OUT10:
