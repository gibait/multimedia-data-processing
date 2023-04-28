cmake_minimum_required(VERSION 3.24)
project(multimedia_data_processing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

add_executable(sort_int sort_int/sort_int.cpp)
add_executable(sort_int_c sort_int/sort_int.c)
add_executable(frequencies frequencies/frequencies.cpp)
add_executable(write_int32 write_int32/write_int32.cpp)
add_executable(read_int32 read_int32/read_int32.cpp)
add_executable(write_int11 write_int11/write_int11.cpp)
add_executable(read_int11 read_int11/read_int11.cpp)

add_executable(elias elias/elias.cpp include/bitrw/bitwriter.h include/bitrw/bitreader.h)

add_executable(packbits packbits/packbits.cpp
        include/bitrw/bitwriter.h
        include/bitrw/bitreader.h)

add_executable(pam pam/pam.cpp pam/matrix.h)
