//
// Created by pkua on 18.05.22.
//

#include "UnitCellFactory.h"

UnitCell UnitCellFactory::createScCell(const std::array<double, 3> &linearSize) {
    return UnitCell(TriclinicBox(linearSize), {Shape({0.5, 0.5, 0.5})});
}

UnitCell UnitCellFactory::createBccCell(const std::array<double, 3> &linearSize) {
    return UnitCell(TriclinicBox(linearSize), {Shape({0.25, 0.25, 0.25}), Shape({0.75, 0.75, 0.75})});
}

UnitCell UnitCellFactory::createFccCell(const std::array<double, 3> &linearSize) {
    return UnitCell(TriclinicBox(linearSize), {Shape({0.25, 0.25, 0.25}),
                                               Shape({0.75, 0.25, 0.25}),
                                               Shape({0.25, 0.75, 0.25}),
                                               Shape({0.25, 0.25, 0.75})});
}

UnitCell UnitCellFactory::createHcpCell(const std::array<double, 3> &cuboidalCellSize) {
    std::vector<Shape> shapes{Shape({0, 0, 0}),
                              Shape({0.5, 0.5, 0}),
                              Shape({0, 1./3, 0.5}),
                              Shape({0.5, 5./6, 0.5})};
    // Center shapes in the cell
    for (auto &shape : shapes)
        shape.setPosition(shape.getPosition() + Vector<3>{0.25, 1./12, 0.25});

    return UnitCell(TriclinicBox(cuboidalCellSize), std::move(shapes));
}

UnitCell UnitCellFactory::createHexagonalCell(const std::array<double, 3> &cuboidalCellSize) {
    return UnitCell(TriclinicBox(cuboidalCellSize), {Shape({0.25, 0.25, 0.5}), Shape({0.75, 0.75, 0.5})});
}

UnitCell UnitCellFactory::createScCell(double linearSize) {
    return UnitCellFactory::createScCell({linearSize, linearSize, linearSize});
}

UnitCell UnitCellFactory::createBccCell(double linearSize) {
    return UnitCellFactory::createBccCell({linearSize, linearSize, linearSize});
}

UnitCell UnitCellFactory::createFccCell(double linearSize) {
    return UnitCellFactory::createFccCell({linearSize, linearSize, linearSize});
}

UnitCell UnitCellFactory::createHcpCell(double ballDiameter) {
    return UnitCellFactory::createHcpCell({ballDiameter, std::sqrt(3)*ballDiameter, 2*std::sqrt(6)*ballDiameter/3});
}

UnitCell UnitCellFactory::createHexagonalCell(double ballDiameter) {
    return UnitCellFactory::createHexagonalCell({ballDiameter, std::sqrt(3)*ballDiameter, ballDiameter});
}
