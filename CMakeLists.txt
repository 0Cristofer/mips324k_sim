cmake_minimum_required(VERSION 3.12)
project(mips324k_sim C)

set(CMAKE_C_STANDARD 99)

find_package(BISON)
find_package(FLEX)

BISON_TARGET(SimParser ${CMAKE_SOURCE_DIR}/src/parsers/sim.y ${CMAKE_CURRENT_BINARY_DIR}/sim.tab.c
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/sim.tab.h VERBOSE)
FLEX_TARGET(SimScanner ${CMAKE_SOURCE_DIR}/src/parsers/sim.l  ${CMAKE_CURRENT_BINARY_DIR}/sim.lex.c)

ADD_FLEX_BISON_DEPENDENCY(SimScanner SimParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(mips324k_sim ${BISON_SimParser_OUTPUTS} ${FLEX_SimScanner_OUTPUTS} src/parsers/include/parser.h src/mips324k_sim.c src/include/mips324k_sim.h src/parsers/parser.c src/util.c src/include/util.h src/simulator/simulator.c src/simulator/include/simulator.h src/simulator/util.c src/simulator/include/util.h src/helpers/queue.c src/helpers/include/queue.h src/simulator/branchComponent.c src/simulator/include/branchComponent.h src/simulator/alu.c src/simulator/include/alu.h src/helpers/linked_list.c src/helpers/include/linked_list.h)
