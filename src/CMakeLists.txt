project(rampack_app)

add_library(rampack_src STATIC core/Simulation.cpp utils/Assertions.h utils/Config.cpp utils/Fold.cpp utils/Logger.h
            utils/Quantity.cpp utils/Utils.cpp frontend/Frontend.cpp frontend/Parameters.cpp core/Shape.cpp
            core/BoundaryConditions.h core/Packing.cpp core/LatticeArrangingModel.cpp
            core/PeriodicBoundaryConditions.cpp core/Sphere.cpp core/ShapeFactory.cpp core/FreeBoundaryConditions.cpp
            geometry/Matrix.h geometry/Matrix.tpp geometry/Vector.h geometry/Vector.tpp)

target_include_directories(rampack_src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rampack_src PUBLIC cxxopts stdc++fs)

add_executable(rampack Main.cpp)
target_link_libraries(rampack PRIVATE rampack_src)