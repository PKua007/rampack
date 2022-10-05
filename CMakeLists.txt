cmake_minimum_required(VERSION 3.10)
project(rampack)

if (NOT (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0))
    message(FATAL_ERROR "Only GCC 7+ is supporter")
endif()

set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "No CMAKE_BUILD_TYPE specified; using Release. Use -DCMAKE_BUILD_TYPE=... to specify it manually")
endif()

add_compile_options(-Wall -Wextra -pedantic)
if (CMAKE_BUILD_TYPE MATCHES "Release")
    add_compile_options(-O3 -march=native)
elseif (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_compile_definitions(_GLIBCXX_CONCEPT_CHECKS _GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC _GLIBCXX_CONCEPT_CHECKS)
endif()

# Check for submodules
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/Catch2/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/trompeloeil/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/cxxopts/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/ZipIterator/ZipIterator.hpp")
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
        message(FATAL_ERROR "Some submodules are not present. Execute: git submodule update --init")
    else()
        message(FATAL_ERROR "This is not a git repository. Clone it from a repo using \n"
                            "git clone https://github.com/PKua007/rampack.git --recursive")
    endif()
endif()

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    link_libraries(OpenMP::OpenMP_CXX)
endif()

add_library(quickhull STATIC extern/quickhull/QuickHull.cpp)
target_include_directories(quickhull PUBLIC extern/quickhull/)
target_compile_options(quickhull PRIVATE -Wno-maybe-uninitialized)
# unset _GLIBCXX_CONCEPT_CHECKS for quickhull because it causes compilation errors
target_compile_options(quickhull PRIVATE -U_GLIBCXX_CONCEPT_CHECKS)

# Explicitly link filesystem library for older versions of GCC
link_libraries(stdc++fs)

# Allow overriding CMake options with set() and turn off tests for cxxopts
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CXXOPTS_BUILD_TESTS OFF)

# Now we can safely add submodule subdirectories
add_subdirectory(extern/Catch2 EXCLUDE_FROM_ALL)
add_subdirectory(extern/trompeloeil EXCLUDE_FROM_ALL)
add_subdirectory(extern/cxxopts EXCLUDE_FROM_ALL)

add_subdirectory(src)
add_subdirectory(test)