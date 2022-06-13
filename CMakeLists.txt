cmake_minimum_required(VERSION 3.21)
project(image_processor)

set(CMAKE_CXX_STANDARD 20)

add_executable(image_processor
    image_processor.cpp
    console_read.cpp
    bmp_processing.cpp
    filters_processing.cpp
    filters.cpp)
add_subdirectory(test)
