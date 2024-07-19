//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_UNITCELLFACTORY_H
#define RAMPACK_UNITCELLFACTORY_H

#include "UnitCell.h"
#include "LatticeTraits.h"


/**
 * @brief A class to conveniently create most common unit cell types.
 */
class UnitCellFactory {
private:
    static Shape createShape(const Vector<3> &pos, const ShapeData &data);

public:
    /**
     * @brief Creates a simple cubic cell of a general, triclinic shape given by @a box, accommodating a single particle
     * with ShapeData @a data in the middle.
     */
    static UnitCell createScCell(const TriclinicBox &box, const ShapeData &data = {});

    /**
     * @brief Creates a simple cubic cell of a cuboidal shape, whose side lengths are @a sideLengths, accommodating a
     * single particle with ShapeData @a data in the middle.
     */
    static UnitCell createScCell(const std::array<double, 3> &sideLengths, const ShapeData &data = {});

    /**
     * @brief Creates a simple cubic cell of a cubic shape, whose side length is @a sideLength, accommodating a single
     * particle with ShapeData @a data in the middle.
     */
    static UnitCell createScCell(double sideLength, const ShapeData &data = {});

    /**
     * @brief Creates a body centered cubic cell of a general, triclinic shape given by @a box, accommodating two
     * particles with ShapeData @a data.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25} and
     * {0.75, 0.75, 0.75}).
     */
    static UnitCell createBccCell(const TriclinicBox &box, const ShapeData &data = {});

    /**
     * @brief Creates a body centered cubic cell of a cuboidal, whose side lengths are @a sideLengths, accommodating two
     * particles with ShapeData @a data.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25} and
     * {0.75, 0.75, 0.75}).
     */
    static UnitCell createBccCell(const std::array<double, 3> &sideLengths, const ShapeData &data = {});

    /**
     * @brief Creates a body centered cubic cell of a cubic shape, accommodating two particles with ShapeData @a data.
     * The side length of the cell is such that the distance between the nearest neighbors is
     * @a nearestNeighborDistance.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25} and
     * {0.75, 0.75, 0.75}).
     */
    static UnitCell createBccCell(double nearestNeighborDistance, const ShapeData &data = {});

    /**
     * @brief Creates a face centered cubic cell of a general, triclinic shape given by @a box, accommodating four
     * particles with ShapeData @a data.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25},
     * {0.25, 0.75, 0.75}, {0.75, 0.25, 0.75}, and {0.75, 0.75, 0.25}).
     */
    static UnitCell createFccCell(const TriclinicBox &box, const ShapeData &data = {});

    /**
     * @brief Creates a face centered cubic cell of a cuboidal shape, whose side lengths are @a sideLengths,
     * accommodating four particles with ShapeData @a data.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25},
     * {0.25, 0.75, 0.75}, {0.75, 0.25, 0.75}, and {0.75, 0.75, 0.25}).
     */
    static UnitCell createFccCell(const std::array<double, 3> &sideLengths, const ShapeData &data = {});

    /**
     * @brief Creates a face centered cubic cell of a cubic shape, accommodating four particles with ShapeData @a data.
     * The side length of the cell is such that the distance between the nearest neighbors is
     * @a nearestNeighborDistance.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25},
     * {0.25, 0.75, 0.75}, {0.75, 0.25, 0.75}, and {0.75, 0.75, 0.25}).
     */
    static UnitCell createFccCell(double nearestNeighborDistance, const ShapeData &data = {});

    /**
     * @brief Creates a hexagonal close packed unit cell of a general, triclinic shape given by @a box, accommodating
     * four particles with ShapeData @a data.
     * @details @a axis controls the direction along which the (alternating) honeycombs are stacked. Relative
     * coordinates of particles for the particular values of @a axis are the following:
     * - LatticeTraits::Axis::X: {1/4, 1/4, 1/12}, {1/4, 3/4, 7/12}, {3/4, 1/4, 5/12}, {3/4, 3/4, 11/12}
     * - LatticeTraits::Axis::Y: {1/12, 1/4, 1/4}, {7/12, 1/4, 3/4}, {5/12, 3/4, 1/4}, {11/12, 3/4, 3/4}
     * - LatticeTraits::Axis::Z: {1/4, 1/12, 1/4}, {3/4, 7/12, 1/4}, {1/4, 5/12, 3/4}, {3/4, 11/12, 3/4}
     */
    static UnitCell createHcpCell(const TriclinicBox &box, LatticeTraits::Axis axis = LatticeTraits::Axis::Z,
                                  const ShapeData &data = {});

    /**
     * @brief Creates a hexagonal close packed unit cell of a cuboidal shape, whose side lengths are @a sideLengths,
     * accommodating four particles with ShapeData @a data.
     * @details @a axis controls the direction along which the (alternating) honeycombs are stacked; see
     * createHcpCell(const TriclinicBox&, LatticeTraits::Axis, const ShapeData&).
     */
    static UnitCell createHcpCell(const std::array<double, 3> &sideLengths,
                                  LatticeTraits::Axis axis = LatticeTraits::Axis::Z, const ShapeData &data = {});

    /**
     * @brief Creates a hexagonal close packed unit cell of a cuboidal shape, accommodating four particles with
     * ShapeData @a data. The side lengths of the cell are such that the distance between the nearest neighbors is
     * @a nearestNeighborDistance.
     * @details @a axis controls the direction along which the (alternating) honeycombs are stacked; see
     * createHcpCell(const TriclinicBox&, LatticeTraits::Axis, const ShapeData&).
     */
    static UnitCell createHcpCell(double nearestNeighborDistance, LatticeTraits::Axis axis = LatticeTraits::Axis::Z,
                                  const ShapeData &data = {});


    /**
     * @brief Creates a hexagonal cell accommodating two particles with ShapeData @a data, where hexatic honeycombs are
     * stack directly on top of each other, without alternating offsets as for the hexagonal close packed unit cell. The
     * cell has a general, triclinic shape given by @a box.
     * @details @a axis controls the direction along which the honeycombs are stacked. Relative coordinates of particles
     * for the particular values of @a axis are the following:
     * - LatticeTraits::Axis::X: {1/2, 1/4, 1/4}, {1/2, 3/4, 3/4}
     * - LatticeTraits::Axis::Y: {1/4, 1/2, 1/4}, {3/4, 1/2, 3/4}
     * - LatticeTraits::Axis::Z: {1/4, 1/4, 1/2}, {3/4, 3/4, 1/2}
     */
    static UnitCell createHexagonalCell(const TriclinicBox &box, LatticeTraits::Axis axis = LatticeTraits::Axis::Z,
                                        const ShapeData &data = {});

    /**
     * @brief Creates a hexagonal cell accommodating two particles with ShapeData @a data, where hexatic honeycombs are
     * stack directly on top of each other, without alternating offsets as for the hexagonal close packed unit cell. The
     * cell has a cuboidal shape, whose side lengths are @a sideLengths.
     * @details @a axis controls the direction along which the honeycombs are stacked; see
     * createHexagonalCell(const TriclinicBox&, LatticeTraits::Axis, const ShapeData&).
     */
    static UnitCell createHexagonalCell(const std::array<double, 3> &sideLengths,
                                        LatticeTraits::Axis axis = LatticeTraits::Axis::Z, const ShapeData &data = {});

    /**
     * @brief Creates a hexagonal cell accommodating two particles with ShapeData @a data, where hexatic honeycombs are
     * stack directly on top of each other, without alternating offsets as for the hexagonal close packed unit cell. The
     * cell has a cuboidal shape, whose side lengths are such that the distance between the nearest neighbors is
     * @a nearestNeighborDistance.
     * @details @a axis controls the direction along which the honeycombs are stacked; see
     * createHexagonalCell(const TriclinicBox&, LatticeTraits::Axis, const ShapeData&).
     */
    static UnitCell createHexagonalCell(double nearestNeighborDistance,
                                        LatticeTraits::Axis axis = LatticeTraits::Axis::Z, const ShapeData &data = {});
};


#endif //RAMPACK_UNITCELLFACTORY_H
