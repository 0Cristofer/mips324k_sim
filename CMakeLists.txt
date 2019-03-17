cmake_minimum_required(VERSION 3.13)
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

add_executable(mips324k_sim ${BISON_SimParser_OUTPUTS} ${FLEX_SimScanner_OUTPUTS} src/include/parser.h)