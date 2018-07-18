cmake_minimum_required(VERSION 3.10)
project(naive)

# parallel set randow value
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()


add_executable(baseline-add baseline-add.cpp)

add_executable(pthread-add pthread-add.cpp)
target_link_libraries(pthread-add pthread)
add_executable(openmp-add openmp-add.cpp)

find_package(MPI REQUIRED)
add_executable(mpi-add mpi-add.cpp)
target_include_directories(mpi-add PUBLIC "${MPI_INCLUDE_PATH}")
target_link_libraries(mpi-add ${MPI_LIBRARIES})
set_target_properties(mpi-add PROPERTIES
  COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
set_target_properties(mpi-add PROPERTIES
  LINK_FLAGS "${MPI_LINK_FLAGS}")

find_package(CUDA REQUIRED)
cuda_add_executable(cuda-add cuda-add.cu)

