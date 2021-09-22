cmake_minimum_required(VERSION 3.0.0)
project(cpp-service VERSION 0.1.0)

include(CTest)
enable_testing()

### To fix: error: ‘is_same_v’ is not a member of ‘std’
### Note: location of these statements matter.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("deps/eigen-3.3.9")
include_directories("src")
include_directories("deps/tbb/include")

add_library(TBBLIB SHARED IMPORTED)
set_target_properties(TBBLIB PROPERTIES
        IMPORTED_LOCATION "deps/tbb/lib/intel64/gcc4.8/libtbb.so"
        INTERFACE_INCLUDE_DIRECTORIES "deps/tbb/include")

add_executable(${PROJECT_NAME} src/main.cpp src/trianglemesh.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC ${TBBLIB})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
