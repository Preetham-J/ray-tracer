cmake_minimum_required (VERSION 2.8)

project(ray-tracer)

add_definitions("-Wall")
add_definitions("-Wextra")
add_definitions("-std=c++11")
add_definitions("-pedantic")
add_definitions("-O3")
# add_definitions("-fopenmp")

file(GLOB SOURCES *.hpp *.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
