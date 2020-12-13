project(rampack_test)

add_executable(tests TestMain.cpp tests/core/PeriodicBoundaryConditionsTest.cpp tests/core/LatticeArrangingModelTest.cpp
               tests/core/PackingTest.cpp matchers/PackingApproxPositionsCatchMatcher.h tests/core/ShapeTest.cpp)

target_link_libraries(tests PRIVATE rampack_src Catch2::Catch2 trompeloeil)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})