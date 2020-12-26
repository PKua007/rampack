project(rampack_app)

add_library(rampack_src STATIC core/Simulation.cpp utils/Assertions.h utils/Config.cpp utils/Fold.cpp utils/Logger.h
            utils/Quantity.cpp utils/Utils.cpp frontend/Frontend.cpp frontend/Parameters.cpp core/Shape.cpp
            core/BoundaryConditions.h core/Packing.cpp core/LatticeArrangingModel.cpp core/FreeBoundaryConditions.h
            core/PeriodicBoundaryConditions.cpp frontend/ShapeFactory.cpp geometry/Matrix.h geometry/Matrix.tpp
            geometry/Vector.h geometry/Vector.tpp core/Interaction.h core/ShapePrinter.h core/shapes/SphereTraits.cpp
            core/shapes/SpherocylinderTraits.cpp core/interactions/CentralInteraction.cpp
            core/interactions/LennardJonesInteraction.cpp  core/interactions/RepulsiveLennardJonesInteraction.cpp
            core/shapes/PolysphereTraits.cpp core/shapes/PolysphereBananaTraits.cpp core/NeighbourGrid.cpp)

target_include_directories(rampack_src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rampack_src PUBLIC cxxopts stdc++fs)

add_executable(rampack Main.cpp)
target_link_libraries(rampack PRIVATE rampack_src)