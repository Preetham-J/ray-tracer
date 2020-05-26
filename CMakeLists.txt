cmake_minimum_required (VERSION 2.8)

project(ray-tracer)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

add_definitions("-Wall")
add_definitions("-Wextra")
add_definitions("-std=c++11")
add_definitions("-pedantic")
add_definitions("-O3")
# add_definitions("-fopenmp")

# Files to include
file(GLOB SOURCES
    "${SRC_DIR}/*.hpp"
    "${SRC_DIR}/*.cpp"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")

# STB library
set(STB_DIR "${LIB_DIR}/stb")
target_include_directories(${PROJECT_NAME} PRIVATE "${STB_DIR}")
