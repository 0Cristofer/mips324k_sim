all:
	flex -osim.lex.c src/parsers/sim.l
	bison src/parsers/sim.y -d -osim.tab.c
	gcc *.c src/*c src/helpers/*c src/parsers/*.c src/simulator/*.c -O3 -o mips324k_sim
	rm *.c *.h