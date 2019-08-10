lui $t1, 10
lui $t2, 5
lui $t3, 3
lui $t4, 6
lui $t5, 2354
lui $t6, 87
lui $t7, 45
mul $t1, $t1, $t2
sub $t2, $t4, $t5
add $t4, $t2, $t6
div $t3, $t4
mflo $t5
mul $t6, $t5, $t7
add $v0, $t1, $t6