cmake_minimum_required(VERSION 3.24)
project(Skeleton)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Skeleton
        framework.cpp
        framework.h
        Skeleton.cpp)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -framework OpenGL -framework GLUT")
endif()