cmake_minimum_required(VERSION 3.24)
project(multimedia_data_processing)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)


add_executable(sort_int sort_int/sort_int.cpp)
add_executable(sort_int_c sort_int/sort_int.c)
add_executable(frequencies frequencies/frequencies.cpp)
add_executable(write_int32 write_int32/write_int32.cpp)
add_executable(read_int32 read_int32/read_int32.cpp)
add_executable(write_int11 write_int11/write_int11.cpp)
add_executable(read_int11 read_int11/read_int11.cpp)

add_executable(elias elias/elias.cpp include/bitrw/bitreader.h include/bitrw/bitwriter.h)
target_include_directories(elias PUBLIC include/bitrw)