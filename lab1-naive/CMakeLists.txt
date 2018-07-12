cmake_minimum_required(VERSION 3.10)
project(naive)

add_executable(baseline baseline.cpp)

add_executable(pthread-add pthread-add.cpp)
target_link_libraries(pthread-add pthread)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
# message("${OpenMP_CXX_FLAGS}")
# message("${OpenMP_EXE_LINKER_FLAGS}")
add_executable(openmp-add openmp-add.cpp)

