cmake_minimum_required(VERSION 3.11)
project(parallel_lab)
add_subdirectory(lab1-naive)
add_subdirectory(lab2-pthread)
add_subdirectory(lab3-openmp)
add_subdirectory(lab4-mpi)
add_subdirectory(lab5-cuda)
add_subdirectory(project-sudoku)
enable_testing()


add_test(
  NAME bench-baseline-add
  COMMAND baseline-add
)
add_test(
  NAME bench-pthread-add
  COMMAND pthread-add
)
add_test(
  NAME bench-openmp-add
  COMMAND openmp-add
)
add_test(
  NAME bench-mpi-add
  COMMAND mpiexec -np 2 ${CMAKE_CURRENT_BINARY_DIR}/lab1-naive/mpi-add
)
add_test(
  NAME bench-cuda-add
  COMMAND cuda-add
)

set(lena ${CMAKE_CURRENT_SOURCE_DIR}/data/lena.png)
set(png_out ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary)
add_test(
  NAME bench-pthread-erode
  COMMAND pthread-erode 
    ${lena}
    ${png_out}/pthread-erode
)
add_test(
  NAME bench-openmp-erode
  COMMAND openmp-erode 
    ${lena}
    ${png_out}/openmp-erode
)
add_test(
  NAME bench-mpi-erode
  COMMAND mpiexec -np 4 ${CMAKE_CURRENT_BINARY_DIR}/lab4-mpi/mpi-erode 
    ${lena}
    ${png_out}/mpi-erode
)
add_test(
  NAME bench-cuda-erode
  COMMAND cuda-erode 
    ${lena}
    ${png_out}/cuda-erode
)

set(grid ${CMAKE_CURRENT_SOURCE_DIR}/project-sudoku/data/16grid.txt)
add_test(
  NAME bench-sudoku
  COMMAND sudoku 
    ${grid}
)
add_test(
  NAME bench-sudoku-parallel
  COMMAND sudoku-parallel
    ${grid}
)

