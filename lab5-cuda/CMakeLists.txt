cmake_minimum_required(VERSION 3.10)
project(lab5)
find_package( OpenCV REQUIRED )

find_package(CUDA)
cuda_add_executable(cuda-erode cuda-erode.cu)

add_executable(fakemain fakemain.cpp)
target_link_libraries(cuda-erode ${OpenCV_LIBS})
