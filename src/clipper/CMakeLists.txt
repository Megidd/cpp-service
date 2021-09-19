project(clipper)
cmake_minimum_required(VERSION 2.6)

# require C++11
# Aparently, should be set before add_library
# To be able to build on MacBook with Xcode version 9.4.1
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(clipper STATIC
    clipper.cpp
    clipper.hpp
    clipper_z.cpp
    clipper_z.hpp
)

if(SLIC3R_PROFILE)
    target_link_libraries(clipper Shiny)
endif()

target_include_directories(clipper PUBLIC
    ${CMAKE_SOURCE_DIR}/../
)
