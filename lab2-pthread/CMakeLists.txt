cmake_minimum_required(VERSION 3.10)
project(naive)

add_executable(baseline baseline.cpp)
add_executable(pthread-add pthread-add.cpp)
target_link_libraries(pthread-add pthread)
