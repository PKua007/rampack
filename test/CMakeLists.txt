project(rampack_test)

add_executable(unit_tests TestMain.cpp unit_tests/core/PeriodicBoundaryConditionsTest.cpp
        unit_tests/core/LatticeArrangingModelTest.cpp unit_tests/core/PackingTest.cpp
        matchers/PackingApproxPositionsCatchMatcher.h unit_tests/core/ShapeTest.cpp
        unit_tests/core/ShapeFactoryTest.cpp unit_tests/core/SphereTest.cpp)
target_link_libraries(unit_tests PRIVATE rampack_src Catch2::Catch2 trompeloeil)
target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(valid_tests TestMain.cpp validation_tests/SimulationTest.cpp)
target_link_libraries(valid_tests PRIVATE rampack_src Catch2::Catch2 trompeloeil)
target_include_directories(valid_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})