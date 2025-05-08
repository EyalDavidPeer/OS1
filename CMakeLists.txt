cmake_minimum_required(VERSION 3.28)
project(OS1)

set(CMAKE_CXX_STANDARD 14)

add_executable(OS1
        smash.cpp
signals.cpp
Commands.cpp)
