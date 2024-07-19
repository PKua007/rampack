//
// Created by pkua on 18.05.22.
//

#include "UnitCellFactory.h"


Shape UnitCellFactory::createShape(const Vector<3> &pos, const ShapeData &data) {
    return Shape(pos, Matrix<3, 3>::identity(), data);
}

UnitCell UnitCellFactory::createScCell(const TriclinicBox &box, const ShapeData &data) {
    return UnitCell(box, {createShape({0.5, 0.5, 0.5}, data)});
}

UnitCell UnitCellFactory::createScCell(const std::array<double, 3> &sideLengths, const ShapeData &data) {
    return UnitCellFactory::createScCell(TriclinicBox(sideLengths), data);
}

UnitCell UnitCellFactory::createScCell(double sideLength, const ShapeData &data) {
    return UnitCellFactory::createScCell(TriclinicBox(sideLength), data);
}

UnitCell UnitCellFactory::createBccCell(const TriclinicBox &box, const ShapeData &data) {
    return UnitCell(box, {
        createShape({0.25, 0.25, 0.25}, data),
        createShape({0.75, 0.75, 0.75}, data)
    });
}

UnitCell UnitCellFactory::createBccCell(const std::array<double, 3> &sideLengths, const ShapeData &data) {
    return UnitCellFactory::createBccCell(TriclinicBox(sideLengths), data);
}

UnitCell UnitCellFactory::createBccCell(double nearestNeighborDistance, const ShapeData &data) {
    double linearSize = 2 * nearestNeighborDistance / std::sqrt(3);
    return UnitCellFactory::createBccCell(TriclinicBox(linearSize), data);
}

UnitCell UnitCellFactory::createFccCell(const TriclinicBox &box, const ShapeData &data) {
    return UnitCell(box, {
        createShape({0.25, 0.25, 0.25}, data),
        createShape({0.25, 0.75, 0.75}, data),
        createShape({0.75, 0.25, 0.75}, data),
        createShape({0.75, 0.75, 0.25}, data)
    });
}

UnitCell UnitCellFactory::createFccCell(const std::array<double, 3> &sideLengths, const ShapeData &data) {
    return UnitCellFactory::createFccCell(TriclinicBox(sideLengths), data);
}

UnitCell UnitCellFactory::createFccCell(double nearestNeighborDistance, const ShapeData &data) {
    double linearSize = std::sqrt(2) * nearestNeighborDistance;
    return UnitCellFactory::createFccCell(TriclinicBox(linearSize), data);
}

UnitCell UnitCellFactory::createHcpCell(const TriclinicBox &box, LatticeTraits::Axis axis, const ShapeData &data) {
    std::vector<Shape> shapes{
        createShape({  0,    0,   0}, data),
        createShape({0.5,  0.5,   0}, data),
        createShape({  0, 1./3, 0.5}, data),
        createShape({0.5, 5./6, 0.5}, data)
    };

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

UnitCell UnitCellFactory::createHcpCell(const std::array<double, 3> &sideLengths, LatticeTraits::Axis axis,
                                        const ShapeData &data)
{
    return UnitCellFactory::createHcpCell(TriclinicBox(sideLengths), axis, data);
}

UnitCell UnitCellFactory::createHcpCell(double nearestNeighborDistance, LatticeTraits::Axis axis, const ShapeData &data) {
    double a = nearestNeighborDistance;
    double b = std::sqrt(3) * nearestNeighborDistance;
    double c = 2 * std::sqrt(6) * nearestNeighborDistance / 3;
    switch (axis) {
        case LatticeTraits::Axis::X:
            return UnitCellFactory::createHcpCell({c, a, b}, axis, data);
        case LatticeTraits::Axis::Y:
            return UnitCellFactory::createHcpCell({b, c, a}, axis, data);
        case LatticeTraits::Axis::Z:
            return UnitCellFactory::createHcpCell({a, b, c}, axis, data);
    }
    AssertThrow("");
}

UnitCell UnitCellFactory::createHexagonalCell(const TriclinicBox &box, LatticeTraits::Axis axis,
                                              const ShapeData &data)
{
    switch (axis) {
        case LatticeTraits::Axis::X:
            return UnitCell(box, {
                createShape({0.5, 0.25, 0.25}, data),
                createShape({0.5, 0.75, 0.75}, data)
            });
        case LatticeTraits::Axis::Y:
            return UnitCell(box, {
                createShape({0.25, 0.5, 0.25}, data),
                createShape({0.75, 0.5, 0.75}, data)
            });
        case LatticeTraits::Axis::Z:
            return UnitCell(box, {
                createShape({0.25, 0.25, 0.5}, data),
                createShape({0.75, 0.75, 0.5}, data)
            });
    }
    AssertThrow("");
}

UnitCell UnitCellFactory::createHexagonalCell(const std::array<double, 3> &sideLengths, LatticeTraits::Axis axis,
                                              const ShapeData &data)
{
    return UnitCellFactory::createHexagonalCell(TriclinicBox(sideLengths), axis, data);
}

UnitCell UnitCellFactory::createHexagonalCell(double nearestNeighborDistance, LatticeTraits::Axis axis, const ShapeData &data) {
    double a = nearestNeighborDistance;
    double b = std::sqrt(3) * nearestNeighborDistance;
    double c = nearestNeighborDistance;
    switch (axis) {
        case LatticeTraits::Axis::X:
            return UnitCellFactory::createHexagonalCell({c, a, b}, axis, data);
        case LatticeTraits::Axis::Y:
            return UnitCellFactory::createHexagonalCell({b, c, a}, axis, data);
        case LatticeTraits::Axis::Z:
            return UnitCellFactory::createHexagonalCell({a, b, c}, axis, data);
    }
    AssertThrow("");
}
