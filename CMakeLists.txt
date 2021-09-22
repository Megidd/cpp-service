cmake_minimum_required(VERSION 3.0.0)
project(cpp-service VERSION 0.1.0)

include(CTest)
enable_testing()

### To fix: error: ‘is_same_v’ is not a member of ‘std’
### Note: location of these statements matter.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("deps/eigen-3.3.9")                 # header-only =)
include_directories("deps/libigl-2.3.0/include")        # header-only =)
include_directories("src")
include_directories("deps/tbb/include")                 # linked to
include_directories("src/clipper")                      # as source code
include_directories("deps/cereal-1.3.0/include")        # header-only =)

# https://stackoverflow.com/a/41909627/3405291
find_library(LIB_TBB                NAMES tbb               HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(LIB_TBBbind            NAMES tbbbind           HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(LIB_TBBmalloc_proxy    NAMES tbbmalloc_proxy   HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(LIB_TBBmalloc          NAMES tbbmalloc         HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(LIB_TBB_preview        NAMES tbb_preview       HINTS "deps/tbb/lib/intel64/gcc4.8")

# https://stackoverflow.com/a/2110796/3405291
AUX_SOURCE_DIRECTORY(src/libslic3r/             SRC_libslic3r)
AUX_SOURCE_DIRECTORY(src/libslic3r/SLA/         SRC_libslic3r_SLA)
AUX_SOURCE_DIRECTORY(src/libslic3r/Optimize/    SRC_libslic3r_Optimize)
AUX_SOURCE_DIRECTORY(src/clipper/               SRC_clipper)

add_executable(${PROJECT_NAME}
src/main.cpp
src/trianglemesh.cpp
${SRC_libslic3r}
${SRC_libslic3r_SLA}
${SRC_libslic3r_Optimize}
${SRC_clipper}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
${LIB_TBB}
${LIB_TBBbind}
${LIB_TBBmalloc_proxy}
${LIB_TBBmalloc}
${LIB_TBB_preview}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
