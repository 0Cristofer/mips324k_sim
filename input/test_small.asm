    add $0, $0, $0
    add $t1, $t2, $t3
    addi $t1, $t2, 1000
    j L2
L1: addi $t1, $t2, 1000
    add $0, $0, $0
    add $0, $0, $0
L2: add $0, $0, $0
    j L1