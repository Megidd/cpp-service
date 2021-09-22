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


find_library(TBB_LIB
    NAMES
    tbbbind_debug
    tbbbind
    tbb_debug
    tbbmalloc_debug
    tbbmalloc_proxy_debug
    tbbmalloc_proxy
    tbbmalloc
    tbb_preview_debug
    tbb_preview
    tbb
    HINTS "deps/tbb/lib/intel64/gcc4.8"
)

add_executable(${PROJECT_NAME}
src/main.cpp
src/trianglemesh.cpp
src/libslic3r/SLA/IndexedMesh.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${TBB_LIB})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
