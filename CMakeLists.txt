cmake_minimum_required(VERSION 3.25)
project(open-rastertotspl C)

set(CMAKE_C_STANDARD 23)

find_package(Cups REQUIRED)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic")
endif()

add_executable(rastertotspl main.c c23_compat.h)
target_link_libraries(rastertotspl Cups::Cups)
