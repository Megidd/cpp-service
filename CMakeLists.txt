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
include_directories("src/glu-libtess/include/")         # linked to
include_directories("deps/libnest2d/build/installdir/include/") # linked to
include_directories("deps/nlopt/build/installdir/include/") # linked to

# https://stackoverflow.com/a/41909627/3405291
find_library(LIB_TBB                NAMES tbb               HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(LIB_TBBbind            NAMES tbbbind           HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(LIB_TBBmalloc_proxy    NAMES tbbmalloc_proxy   HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(LIB_TBBmalloc          NAMES tbbmalloc         HINTS "deps/tbb/lib/intel64/gcc4.8")
find_library(LIB_TBB_preview        NAMES tbb_preview       HINTS "deps/tbb/lib/intel64/gcc4.8")

find_library(LIB_glu_libtess NAMES glu-libtess HINTS "src/glu-libtess/build/")
find_library(LIB_nest2d NAMES nest2d_clipper_nlopt HINTS "deps/libnest2d/build/installdir/lib/")
find_library(LIB_nlopt NAMES nlopt HINTS "deps/nlopt/build/installdir/lib64/")

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
${LIB_glu_libtess}
${LIB_nest2d}
${LIB_nlopt}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

#################################################################################
# Eigen, libigl, and cereal
#################################################################################

# Are header-only.
# Header-only libraries don't have binaries to LINK to.
# Just download and place headers in proper location.

#################################################################################
# TBB on Windows
#################################################################################

### TBB 2020 Update 1 is used on Windows with Visual Studio 2017
### https://github.com/oneapi-src/oneTBB/issues/274
#
### Have to download tbb-2020.1-win and place it in proper location.
### Note that tbb-2020.1-win DLLs must overwrite VCPKG TBB DDLs, when copying for deployment.

#################################################################################
# TBB on Linux
#################################################################################

### This version is tested:
### tbb-2020.3-lin.tgz
### i.e. 2020 Update 3
###
### As far as tested,
### newer TBB versions are incompatible,
### i.e. header files are different
###
###
### TBB provided by distro package manager is not needed.
### It can be removed:
#
# sudo zypper rm tbb-devel

    ### On Linux, TBB is dependent upon libhwloc.so which needs to be installed by Linux distro package manager
    ###
    ### Recent Linux distributions are installing libhwloc.so.15
    ### But tbb-2020.3-lin.tgz depends upon libhwloc.so.5
    ### To work around it, e.g. on openSUSE Leap 15.3, run:
    #
    # sudo zypper in hwloc-devel
    # cd /usr/lib64/
    # sudo ln -sv libhwloc.so libhwloc.so.5

#################################################################################
# Windows, Linux and macOS
# libtess
#################################################################################

### Just the libtess tesselation library to be statically compilable without OpenGL dependencies.

### README warns:
### Do include gl.h, glu.h or glut.h together with glu-libtess.h, you would get symbol clashes!
###
### Not sure how this warning would affect the statically linked library.

### How to build
## lib is already in src\glu-libtess
## cd to src\glu-libtess
# mkdir build
# cd build/
# cmake  -G "Visual Studio 15 2017 Win64" ..
## On Linux and macOS, rather than the above command, just run: cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON ..
# cmake --build . --config Debug
# cmake --build . --config Release

#################################################################################
# Linux and macOS
# libnest2d
#################################################################################

### On terminal:
#
# git clone https://github.com/tamasmeszaros/libnest2d.git
# cd libnest2d/
# mkdir build
# cd build/
# cmake .. -DLIBNEST2D_HEADER_ONLY=OFF -DCMAKE_INSTALL_PREFIX=./installdir -DRP_ENABLE_DOWNLOADING=ON
# cmake --build . --target install --config Debug
# cmake --build . --target install --config Release

#################################################################################
# Windows:
# libnest2d
#################################################################################

### Command Prompt:
#
# C:\Users\m3\repos\vcpkg>vcpkg install polyclipping:x64-windows
# C:\Users\m3\repos\vcpkg>vcpkg install nlopt:x64-windows
# C:\Users\m3\repos\vcpkg>vcpkg install boost-geometry:x64-windows
#
# git clone https://github.com/tamasmeszaros/libnest2d.git
# cd libnest2d
# mkdir build
# cd build
### 1st option:
### Will download and build dependencies automatically:
# cmake .. -DLIBNEST2D_HEADER_ONLY=OFF -DCMAKE_INSTALL_PREFIX=./installdir -DRP_ENABLE_DOWNLOADING=ON
### Other option:
### Will use VCPKG installations:
# cmake .. -DLIBNEST2D_HEADER_ONLY=OFF -DCMAKE_INSTALL_PREFIX=./installdir -DCMAKE_TOOLCHAIN_FILE=/c/Users/m3/repos/vcpkg/scripts/buildsystems/vcpkg.cmake
# cmake --build . --target install --config Debug
# cmake --build . --target install --config Release
# cmake --build . --target install

#################################################################################
# NLopt
# Linux
#################################################################################

### Installation
#
# git clone https://github.com/stevengj/nlopt.git
# cd nlopt/
# git checkout v2.7.0
# mkdir build
# cd build
# cmake .. -DCMAKE_INSTALL_PREFIX=./installdir
# cmake --build . --target install --config Release
#

#################################################################################
# NLopt
# macOS
#################################################################################

### Installation
#
# git clone https://github.com/stevengj/nlopt.git
# cd nlopt/
# git checkout v2.7.0
# mkdir build
# cd build
# cmake .. -DCMAKE_INSTALL_PREFIX=./installdir
# cmake --build . --target install --config Release
#

#################################################################################
# NLopt
# Windows
#################################################################################

### Installation
#
# git clone https://github.com/stevengj/nlopt.git
# cd nlopt
# git checkout v2.7.0
# mkdir build
# cd build
### Specifiy 64-bit to avoid linker errors:
# cmake  -G "Visual Studio 15 2017 Win64" .. -DCMAKE_INSTALL_PREFIX=./installdir
# cmake --build . --target install --config Release
