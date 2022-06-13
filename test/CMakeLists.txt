cmake_minimum_required(VERSION 3.21)
project(test_image_processor)

set(CMAKE_CXX_STANDARD 20)

add_executable(test_image_processor
    test.cpp
    ../console_read.cpp
    ../bmp_processing.cpp
    ../filters_processing.cpp
    ../filters.cpp)
