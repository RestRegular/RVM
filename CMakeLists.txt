cmake_minimum_required(VERSION 3.29)
project(RVM)

set(CMAKE_CXX_STANDARD 20)

add_executable(RVM rvm_main.cpp
        ra_base.cpp
        ra_base.h
        ra_core.cpp
        ra_core.h
        ra_parser.cpp
        ra_parser.h
        ra_utils.cpp
        ra_utils.h
        rvm_ris.cpp
        rvm_ris.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")