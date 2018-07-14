cmake_minimum_required(VERSION 3.10)
project(lab2)
find_package( OpenCV REQUIRED )


find_package(MPI REQUIRED)
add_executable(mpi-erode mpi-erode.cpp )
target_link_libraries(mpi-erode ${OpenCV_LIBS})
target_include_directories(mpi-erode PUBLIC "${MPI_INCLUDE_PATH}")
target_link_libraries(mpi-erode ${MPI_LIBRARIES})
set_target_properties(mpi-erode PROPERTIES
  COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
set_target_properties(mpi-erode PROPERTIES
  LINK_FLAGS "${MPI_LINK_FLAGS}")
