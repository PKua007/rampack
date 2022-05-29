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
public:
    static UnitCell createScCell(const TriclinicBox &box);

    /**
     * @brief Creates a simple cubic cell with a single particle in the middle, however of a possible cuboidal shape
     * with dimensions @a linearSize.
     */
    static UnitCell createScCell(const std::array<double, 3> &linearSize);

    /**
     * @brief Creates a simple cubic cell with a single particle in the middle, with a cube side length @a linearSize.
     */
    static UnitCell createScCell(double linearSize);


    static UnitCell createBccCell(const TriclinicBox &box);

    /**
     * @brief Creates a body centered cubic cell with two particles, however of a possible cuboidal shape with
     * dimensions @a linearSize.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25} and
     * {0.75, 0.75, 0.75}).
     */
    static UnitCell createBccCell(const std::array<double, 3> &linearSize);

    /**
     * @brief Creates a body centered cubic cell with two particles, with a cube side length @a linearSize.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25} and
     * {0.75, 0.75, 0.75}).
     */
    static UnitCell createBccCell(double linearSize);


    static UnitCell createFccCell(const TriclinicBox &box);

    /**
     * @brief Creates a face centered cubic cell with four particles, however of a possible cuboidal shape with
     * dimensions @a linearSize.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25},
     * {0.25, 0.75, 0.75}, {0.75, 0.25, 0.75} and {0.75, 0.75, 0.25}).
     */
    static UnitCell createFccCell(const std::array<double, 3> &linearSize);

    /**
     * @brief Creates a face centered cubic cell with four particles, with a cube side length @a linearSize.
     * @details Particles are centered in the box (their cell vector coordinates are {0.25, 0.25, 0.25},
     * {0.25, 0.75, 0.75}, {0.75, 0.25, 0.75} and {0.75, 0.75, 0.25}).
     */
    static UnitCell createFccCell(double linearSize);


    static UnitCell createHcpCell(const TriclinicBox &box, LatticeTraits::Axis axis = LatticeTraits::Axis::Z);

    /**
     * @brief Creates a hexagonal close packed unit cell with four molecules within a cuboid of side lengths
     * @a cuboidalCellSize.
     * @details @a axis controls the orientation of the honeycombs. If @a axis is LatticeTraits::Axis::Z and assuming
     * that the first molecule is in the corner, relative cell coordinates of the molecules are {0, 0, 0},
     * {1/2, 1/2, 0}, {0, 1/3, 1/2} and {1/2, 5/6, 1/2}. However, those coordinates are moved by vector {1/4, 1/12, 1/4}
     * (in relative coordinates) so that the molecules are equally distant from the cell box faces. If @a axis is
     * LatticeTraits::Axis::Y, the relative coordinates are cycled so that XYZ becomes YZX. Similarly, if @a axis is
     * LatticeTraits::Axis::X, XYZ becomes ZXY.
     */
    static UnitCell createHcpCell(const std::array<double, 3> &cuboidalCellSize,
                                  LatticeTraits::Axis axis = LatticeTraits::Axis::Z);

    /**
     * @brief Creates a hexagonal close packed unit cell with four molecules in cuboidal cell with such side lengths,
     * that the distance between the nearest neighbours is @a ballDiameter.
     * @details Relative coordinates of molecules and @a axis parameter are the same as for
     * createHcpCell(const std::array<double, 3>&, LatticeTraits::Axis).
     */
    static UnitCell createHcpCell(double ballDiameter, LatticeTraits::Axis axis = LatticeTraits::Axis::Z);


    static UnitCell createHexagonalCell(const TriclinicBox &box, LatticeTraits::Axis axis = LatticeTraits::Axis::Z);

    /**
     * @brief Creates a hexagonal cell giving stacked honeycombs, but not alternating as for hcp lattice.
     * @details @a axis behaves in identical way as for
     * createHcpCell(const std::array<double, 3>&, LatticeTraits::Axis). If @a axis is LatticeTraits::Axis::Z, relative
     * cell coordinates of the molecules are {0.5, 0.25, 0.25} and {0.5, 0.75, 0.75}.
     */
    static UnitCell createHexagonalCell(const std::array<double, 3> &cuboidalCellSize,
                                        LatticeTraits::Axis axis = LatticeTraits::Axis::Z);

    /**
     * @brief Creates a hexagonal cell giving stacked honeycombs in cuboidal cell with such side lengths, that the
     * distance between the nearest neighbours is @a ballDiameter.
     * @details Relative coordinates of molecules and @a axis parameter are the same as for
     * createHexagonalCell(const std::array<double, 3> &, LatticeTraits::Axis).
     */
    static UnitCell createHexagonalCell(double ballDiameter, LatticeTraits::Axis axis = LatticeTraits::Axis::Z);
};


#endif //RAMPACK_UNITCELLFACTORY_H
