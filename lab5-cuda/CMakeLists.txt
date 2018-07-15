cmake_minimum_required(VERSION 3.10)
project(lab2)
find_package( OpenCV REQUIRED )

find_package(CUDA REQUIRED)
cuda_add_executable(cuda-erode cuda-erode.cu)

target_link_libraries(cuda-erode ${OpenCV_LIBS})
