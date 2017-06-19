cmake_minimum_required(VERSION 3.3)
project(mpiLatency)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(SOURCE_FILES main.c t3.c)
add_executable(mpiLatency ${SOURCE_FILES})