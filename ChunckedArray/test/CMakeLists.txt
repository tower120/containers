cmake_minimum_required(VERSION 3.8)
project(chuncked_array)

set(CMAKE_CXX_STANDARD 17)
# Visual Studio 2017 support C++17 https://stackoverflow.com/a/44964919
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()


include_directories(../src)

set(SOURCE_FILES main.cpp ../src/ChunckedArray.h ../src/ReusableChunckedArray.h)
add_executable(chuncked_array_test ${SOURCE_FILES})