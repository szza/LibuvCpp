cmake_minimum_required(VERSION 3.5.1)
project(LibuvCpp VERSION 1.0)

# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILE "g++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(CXX_FLAGS 
-g
-Wall
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH     ${PROJECT_BINARY_DIR}/lib )

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(src)
add_subdirectory(src/Http)
add_subdirectory(example)
