add $t0, $zero,$zero # $t0 = 0
addi $t0, $t0, 10 # copia o valor 10 para $t0
add $t1, $t0, $zero # copia o valor 10 para $t1
addi $t1, $t1, 3 # 10+3
mul $t2, $t1, $t0 # $t2 = (10+3)*10