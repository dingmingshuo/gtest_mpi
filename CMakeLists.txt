cmake_minimum_required(VERSION 3.0)
project(gtest_mpi)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(googletest)
include_directories(googletest/include googletest)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

add_executable(gtest_mpi example.cpp gtest_mpi.h)

target_link_libraries(gtest_mpi gtest gtest_main)
target_link_libraries(gtest_mpi ${MPI_CXX_LIBRARIES})
