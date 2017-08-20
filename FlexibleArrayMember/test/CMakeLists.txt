cmake_minimum_required(VERSION 3.8)
project(flexible_array_member)

set(CMAKE_CXX_STANDARD 17)

include_directories(../src)

set(SOURCE_FILES main.cpp ../src/FlexibleArrayMember.h)
add_executable(flexible_array_member_test ${SOURCE_FILES})