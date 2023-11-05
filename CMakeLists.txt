cmake_minimum_required(VERSION 3.25)

if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif (POLICY CMP0077)

project(vec2poly)

set(CMAKE_CXX_STANDARD 20)

find_package(boost_headers REQUIRED COMPONENTS graph)

add_executable(vec2poly main.cpp
        point.cpp
        point.h
        lineseg.cpp
        lineseg.h
        tests.cpp
        world.cpp
        world.h
        pntalloc.cpp
        pntalloc.h
        graph-path.cpp
        graph-path.h
)

