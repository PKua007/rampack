cmake_minimum_required(VERSION 3.10)
project(rampack)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_CONCEPT_CHECKS -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")

# Automatic git submodule init based on
# https://cliutils.gitlab.io/modern-cmake/chapters/projects/submodule.html
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive RESULT_VARIABLE GIT_SUBMOD_RESULT)

    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/Catch2/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/cxxopts/CMakeLists.txt" OR
        NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/cxxopts/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# Now we can safely add submodule subdirectories
add_subdirectory(extern/Catch2)
add_subdirectory(extern/trompeloeil)
add_subdirectory(extern/cxxopts)

add_subdirectory(src)
add_subdirectory(test)