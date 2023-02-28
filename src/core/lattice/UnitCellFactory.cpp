//
// Created by pkua on 18.05.22.
//

#include "UnitCellFactory.h"


UnitCell UnitCellFactory::createScCell(const TriclinicBox &box) {
    return UnitCell(box, {Shape({0.5, 0.5, 0.5})});
}

UnitCell UnitCellFactory::createScCell(const std::array<double, 3> &linearSize) {
    return UnitCell(TriclinicBox(linearSize), {Shape({0.5, 0.5, 0.5})});
}

UnitCell UnitCellFactory::createScCell(double linearSize) {
    return UnitCellFactory::createScCell(TriclinicBox(linearSize));
}

UnitCell UnitCellFactory::createBccCell(const std::array<double, 3> &linearSize) {
    return UnitCell(TriclinicBox(linearSize), {Shape({0.25, 0.25, 0.25}), Shape({0.75, 0.75, 0.75})});
}

UnitCell UnitCellFactory::createBccCell(const TriclinicBox &box) {
    return UnitCell(box, {Shape({0.25, 0.25, 0.25}), Shape({0.75, 0.75, 0.75})});
}

UnitCell UnitCellFactory::createBccCell(double ballDiameter) {
    double linearSize = 2 * ballDiameter / std::sqrt(3);
    return UnitCellFactory::createBccCell(TriclinicBox(linearSize));
}

UnitCell UnitCellFactory::createFccCell(const TriclinicBox &box) {
    return UnitCell(box, {Shape({0.25, 0.25, 0.25}), Shape({0.25, 0.75, 0.75}),
                          Shape({0.75, 0.25, 0.75}), Shape({0.75, 0.75, 0.25})});
}

UnitCell UnitCellFactory::createFccCell(const std::array<double, 3> &linearSize) {
    return UnitCellFactory::createFccCell(TriclinicBox(linearSize));
}

UnitCell UnitCellFactory::createFccCell(double ballDiameter) {
    double linearSize = std::sqrt(2) * ballDiameter;
    return UnitCellFactory::createFccCell(TriclinicBox(linearSize));
}

UnitCell UnitCellFactory::createHcpCell(const TriclinicBox &box, LatticeTraits::Axis axis) {
    std::vector<Shape> shapes{Shape({0,    0,   0}), Shape({0.5,  0.5,   0}),
                              Shape({0, 1./3, 0.5}), Shape({0.5, 5./6, 0.5})};
    // Center shapes in the cell
    for (auto &shape : shapes)
        shape.setPosition(shape.getPosition() + Vector<3>{0.25, 1./12, 0.25});

    std::size_t shift{};
    switch (axis) {
        case LatticeTraits::Axis::X: shift = 1; break;
        case LatticeTraits::Axis::Y: shift = 2; break;
        case LatticeTraits::Axis::Z: shift = 0; break;
    }

    for (auto &shape : shapes) {
        auto oldPos = shape.getPosition();
        auto newPos = oldPos;
        for (std::size_t i{}; i < 3; i++)
            newPos[(i + shift) % 3] = oldPos[i];
        shape.setPosition(newPos);
    }

    return UnitCell(box, std::move(shapes));
}

UnitCell UnitCellFactory::createHcpCell(const std::array<double, 3> &cuboidalCellSize, LatticeTraits::Axis axis) {
    return UnitCellFactory::createHcpCell(TriclinicBox(cuboidalCellSize), axis);
}

UnitCell UnitCellFactory::createHcpCell(double ballDiameter, LatticeTraits::Axis axis) {
    double a = ballDiameter;
    double b = std::sqrt(3)*ballDiameter;
    double c = 2*std::sqrt(6)*ballDiameter/3;
    switch (axis) {
        case LatticeTraits::Axis::X:
            return UnitCellFactory::createHcpCell({c, a, b}, axis);
        case LatticeTraits::Axis::Y:
            return UnitCellFactory::createHcpCell({b, c, a}, axis);
        case LatticeTraits::Axis::Z:
            return UnitCellFactory::createHcpCell({a, b, c}, axis);
    }
    AssertThrow("");
}

UnitCell UnitCellFactory::createHexagonalCell(const TriclinicBox &box, LatticeTraits::Axis axis) {
    switch (axis) {
        case LatticeTraits::Axis::X:
            return UnitCell(box, {Shape({0.5, 0.25, 0.25}), Shape({0.5, 0.75, 0.75})});
        case LatticeTraits::Axis::Y:
            return UnitCell(box, {Shape({0.25, 0.5, 0.25}), Shape({0.75, 0.5, 0.75})});
        case LatticeTraits::Axis::Z:
            return UnitCell(box, {Shape({0.25, 0.25, 0.5}), Shape({0.75, 0.75, 0.5})});
    }
    AssertThrow("");
}

UnitCell UnitCellFactory::createHexagonalCell(const std::array<double, 3> &cuboidalCellSize, LatticeTraits::Axis axis) {
    return UnitCellFactory::createHexagonalCell(TriclinicBox(cuboidalCellSize), axis);
}

UnitCell UnitCellFactory::createHexagonalCell(double ballDiameter, LatticeTraits::Axis axis) {
    double a = ballDiameter;
    double b = std::sqrt(3)*ballDiameter;
    double c = ballDiameter;
    switch (axis) {
        case LatticeTraits::Axis::X:
            return UnitCellFactory::createHexagonalCell({c, a, b}, axis);
        case LatticeTraits::Axis::Y:
            return UnitCellFactory::createHexagonalCell({b, c, a}, axis);
        case LatticeTraits::Axis::Z:
            return UnitCellFactory::createHexagonalCell({a, b, c}, axis);
    }
    AssertThrow("");
}
