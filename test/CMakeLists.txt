project(rampack_lib)

file(GLOB_RECURSE unit_tests_sources CONFIGURE_DEPENDS "unit_test/*.cpp" "unit_test/*.h" "matchers/*.cpp"
        "matchers/*.h")

file(GLOB_RECURSE valid_tests_sources CONFIGURE_DEPENDS "validation_tests/*.cpp" "validation_tests/*.h")

add_executable(unit_tests TestMain.cpp ${unit_tests_sources})
target_link_libraries(unit_tests PRIVATE rampack_lib Catch2::Catch2 trompeloeil)
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(valid_tests TestMain.cpp ${valid_tests_sources})
target_link_libraries(valid_tests PRIVATE rampack_lib Catch2::Catch2 trompeloeil)
target_include_directories(valid_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})