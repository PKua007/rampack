project(rampack_app)

file(GLOB_RECURSE rampack_lib_sources CONFIGURE_DEPENDS "*/*.cpp" "*/*.h")

add_library(rampack_lib STATIC ${rampack_lib_sources})

target_include_directories(rampack_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(rampack_lib PUBLIC ../extern/ZipIterator)
target_include_directories(rampack_lib PUBLIC ../extern/Root-Finder/root_finder/include)
target_link_libraries(rampack_lib PUBLIC cxxopts quickhull Eigen3::Eigen)

add_executable(rampack Main.cpp)
target_link_libraries(rampack PRIVATE rampack_lib)