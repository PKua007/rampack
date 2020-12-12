cmake_minimum_required(VERSION 3.10)
project(rampack)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_subdirectory(extern/Catch2)
add_subdirectory(extern/trompeloeil)
add_subdirectory(src)
add_subdirectory(test)