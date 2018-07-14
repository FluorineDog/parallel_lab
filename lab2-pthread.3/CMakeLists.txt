cmake_minimum_required(VERSION 3.10)
project(lab2)
find_package( OpenCV REQUIRED )

add_executable(pthread-erode pthread-erode.cpp)
target_link_libraries(pthread-erode pthread ${OpenCV_LIBS} )
