project(sudoku)
add_executable(sudoku sudoku.cpp)
target_link_libraries(sudoku pthread)