cmake_minimum_required(VERSION 3.10)
project(sudoku)
add_executable(sudoku sudoku.cpp)
target_link_libraries(sudoku pthread)