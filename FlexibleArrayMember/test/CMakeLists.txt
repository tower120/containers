cmake_minimum_required(VERSION 3.8)
project(flexible_array_member)

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

set(SOURCE_FILES main.cpp ../src/FlexibleArrayMember.h)
add_executable(flexible_array_member_test ${SOURCE_FILES})