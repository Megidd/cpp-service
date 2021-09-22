cmake_minimum_required(VERSION 3.0.0)
project(cpp-service VERSION 0.1.0)

include(CTest)
enable_testing()

### To fix: error: ‘is_same_v’ is not a member of ‘std’
### Note: location of these statements matter.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("deps/eigen-3.3.9") # header-only =)
include_directories("deps/libigl-2.3.0/include") # header-only =)
include_directories("src")
include_directories("deps/tbb/include")

# https://stackoverflow.com/a/41909627/3405291
find_library(TBB_LIB NAMES tbb HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(TBBbind_LIB NAMES tbbbind HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(TBBmalloc_proxy_LIB NAMES tbbmalloc_proxy HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(TBBmalloc_LIB NAMES tbbmalloc HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(TBB_preview_LIB NAMES tbb_preview HINTS "deps/tbb/lib/intel64/gcc4.8")

# https://stackoverflow.com/a/2110796/3405291
AUX_SOURCE_DIRECTORY(src/libslic3r/ SRC_libslic3r)
AUX_SOURCE_DIRECTORY(src/libslic3r/SLA/ SRC_libslic3r_SLA)
AUX_SOURCE_DIRECTORY(src/libslic3r/Optimize/ SRC_libslic3r_Optimize)

add_executable(${PROJECT_NAME}
src/main.cpp
src/trianglemesh.cpp
${SRC_libslic3r}
${SRC_libslic3r_SLA}
${SRC_libslic3r_Optimize}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
${TBB_LIB}
${TBBbind_LIB}
${TBBmalloc_proxy_LIB}
${TBBmalloc_LIB}
${TBB_preview_LIB}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
