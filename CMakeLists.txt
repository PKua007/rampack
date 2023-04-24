cmake_minimum_required(VERSION 3.10)
project(rampack)


# Check compiler support
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0)
        message(FATAL_ERROR "Only versions 7+ of GNU are supported")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7.0)
        message(FATAL_ERROR "Only versions 7+ of Clang are supported")
    endif()
else()
    message(WARNING "Compilers other than GNU 7+ and Clang 7+ were not tested; there may be problems")
endif()


set(CMAKE_CXX_STANDARD 17)

# Check build type
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "No CMAKE_BUILD_TYPE specified; using Release. Use -DCMAKE_BUILD_TYPE=... to specify it manually")
endif()

# Configure compiler options
add_compile_options(-Wall -Wextra -pedantic)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Silence stupid warnings about the fix of std::pair API
    add_compile_options(-Wno-psabi)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Release")
    # Apple silicon does not like -march=native option
    if (NOT (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64"))
        add_compile_options(-march=native)
    endif()
    add_compile_options(-O3)
elseif (CMAKE_BUILD_TYPE MATCHES "Debug")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_definitions(_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC)
    endif()
    add_compile_options(-gdwarf-4)
endif()

# Check for submodules
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/Catch2/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/trompeloeil/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/cxxopts/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/ZipIterator/ZipIterator.hpp" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/quickhull/QuickHull.hpp" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/eigen/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/Root-Finder/root_finder/CMakeLists.txt")
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

# Explicitly link filesystem library for older versions of GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    link_libraries(stdc++fs)
endif()

add_library(quickhull STATIC extern/quickhull/QuickHull.cpp)
target_include_directories(quickhull PUBLIC extern/quickhull/)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(quickhull PRIVATE -Wno-maybe-uninitialized)
    # unset _GLIBCXX_CONCEPT_CHECKS for quickhull because it causes compilation errors
    target_compile_options(quickhull PRIVATE -U_GLIBCXX_CONCEPT_CHECKS)
endif()

# Allow overriding CMake options with set() and turn off tests for cxxopts
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CXXOPTS_BUILD_TESTS OFF)

# Turn off tests and documentation for Eigen
set(EIGEN_BUILD_DOC OFF)
set(BUILD_TESTING OFF)

# Now we can safely add submodule subdirectories
add_subdirectory(extern/Catch2 EXCLUDE_FROM_ALL)
add_subdirectory(extern/trompeloeil EXCLUDE_FROM_ALL)
add_subdirectory(extern/cxxopts EXCLUDE_FROM_ALL)
add_subdirectory(extern/eigen EXCLUDE_FROM_ALL)

# Set CXXOPTS_VECTOR_DELIMITER globally to avoid potential errors when used in more than one translation unit
set_property(TARGET cxxopts APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS CXXOPTS_VECTOR_DELIMITER='|')

add_subdirectory(src)
add_subdirectory(test)

# Install rampack bin
install(TARGETS rampack DESTINATION "bin")

# Install zsh autocompletion
find_program(ZHS_FOUND "zsh")
if(ZHS_FOUND)
    install(FILES "${PROJECT_SOURCE_DIR}/script/rampack_completion.zsh"
            DESTINATION "share/zsh/site-functions"
            RENAME "_rampack")
endif()

# Install bash autocompletion
find_program(BASH_FOUND "bash")
if(BASH_FOUND)
    install(FILES "${PROJECT_SOURCE_DIR}/script/rampack_completion.bash"
            DESTINATION "share/bash-completion/completions"
            RENAME "rampack")
endif()
