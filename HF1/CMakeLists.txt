cmake_minimum_required(VERSION 3.24)
project(Skeleton)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(.)

add_executable(Skeleton
        framework.cpp
        framework.h
        Skeleton.cpp)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    target_link_libraries(Skeleton "-framework OpenGL")
    target_link_libraries(Skeleton "-framework GLUT")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    target_link_libraries(Skeleton freeglut)
    target_link_libraries(Skeleton glew32)
    target_link_libraries(Skeleton opengl32)
endif()