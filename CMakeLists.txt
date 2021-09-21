cmake_minimum_required(VERSION 3.0.0)
project(cpp-service VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories("deps/eigen-3.3.9")

add_executable(cpp-service src/main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
