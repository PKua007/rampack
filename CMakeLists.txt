cmake_minimum_required(VERSION 3.10)
project(rampack VERSION 0.15.0)

# Available options
option(RAMPACK_STATIC_LINKING "Build no-dependency static executable" OFF)
option(RAMPACK_BUILD_TESTS "Build unit and validation tests" OFF)

add_compile_definitions(RAMPACK_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
                        RAMPACK_VERSION_MINOR=${PROJECT_VERSION_MINOR}
                        RAMPACK_VERSION_PATCH=${PROJECT_VERSION_PATCH})

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

# Static linking only available in GCC
if (RAMPACK_STATIC_LINKING)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "RAMPACK_STATIC_LINKING: Static linking is supported only for the GNU compiler")
    endif()
endif()

# Link OpenMP if available
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    if (RAMPACK_STATIC_LINKING)
        # We manually create StaticOpenMP target with static versions of libgomp and libpthread
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=libgomp.a
                        OUTPUT_VARIABLE RAMPACK_LIBGOMP_PATH
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=libpthread.a
                        OUTPUT_VARIABLE RAMPACK_LIBPTHREAD_PATH
                        OUTPUT_STRIP_TRAILING_WHITESPACE)

        if (NOT EXISTS "${RAMPACK_LIBGOMP_PATH}")
            message(FATAL_ERROR "RAMPACK_STATIC_LINKING: Cannot find static version of libgomp")
        endif()
        if (NOT EXISTS "${RAMPACK_LIBPTHREAD_PATH}")
            message(FATAL_ERROR "RAMPACK_STATIC_LINKING: Cannot find static version of libpthread")
        endif()
        message(STATUS "Found libgomp.a and libpthread.a for static linking")

        add_library(StaticOpenMP IMPORTED INTERFACE)
        set_property(TARGET StaticOpenMP PROPERTY INTERFACE_COMPILE_OPTIONS ${OpenMP_CXX_FLAGS})
        set_property(TARGET StaticOpenMP
                     PROPERTY INTERFACE_LINK_LIBRARIES "${RAMPACK_LIBGOMP_PATH}" "${RAMPACK_LIBPTHREAD_PATH}")
        link_libraries(StaticOpenMP)

        link_libraries(-static)
    else()
        link_libraries(OpenMP::OpenMP_CXX)
    endif()
endif()

# Explicitly link filesystem library for older versions of GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    link_libraries(stdc++fs)
endif()

add_library(quickhull STATIC extern/quickhull/QuickHull.cpp)
target_include_directories(quickhull PUBLIC extern/quickhull/)

# Allow overriding CMake options with set() and turn off tests for cxxopts
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CXXOPTS_BUILD_TESTS OFF)

# Turn off tests and documentation for Eigen
set(EIGEN_BUILD_DOC OFF)
# This is quite precarious, because it changes the global behavior of CMake:
# https://cmake.org/cmake/help/latest/command/enable_testing.html
# But as long as using rampack as static library is not officially supported, I don't care >:)
set(BUILD_TESTING OFF)

# Now we can safely add submodule subdirectories
add_subdirectory(extern/Catch2 EXCLUDE_FROM_ALL)
add_subdirectory(extern/trompeloeil EXCLUDE_FROM_ALL)
add_subdirectory(extern/cxxopts EXCLUDE_FROM_ALL)

# Add Eigen but silence it, because it displays unhelpful message about targets
message(STATUS "Configuring Eigen...")
set(_saved_CMAKE_MESSAGE_LOG_LEVEL ${CMAKE_MESSAGE_LOG_LEVEL})
set(CMAKE_MESSAGE_LOG_LEVEL NOTICE)
add_subdirectory(extern/eigen EXCLUDE_FROM_ALL)
set(CMAKE_MESSAGE_LOG_LEVEL ${_saved_CMAKE_MESSAGE_LOG_LEVEL})
message(STATUS "Configuring Eigen done")

# Set CXXOPTS_VECTOR_DELIMITER globally to avoid potential errors when used in more than one translation unit
set_property(TARGET cxxopts APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS CXXOPTS_VECTOR_DELIMITER='|')

add_subdirectory(src)
if (RAMPACK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

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

# Prepare DEB and RPM packaging
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Piotr Kubala and rampack Contributors")
set(CPACK_PACKAGE_DESCRIPTION "The software for simulating particle systems using different flavors of Monte Carlo sampling.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Random And Maximal PACKing PACKage.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/PKua007/rampack")
set(CPACK_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_MAINTAINER}")

include(CPack)