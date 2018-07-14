cmake_minimum_required(VERSION 3.10)
project(lab2)
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
find_package( OpenCV REQUIRED )

add_executable(openmp-erode openmp-erode.cpp)

target_link_libraries(openmp-erode ${OpenCV_LIBS} )
