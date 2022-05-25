//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_UNITCELLFACTORY_H
#define RAMPACK_UNITCELLFACTORY_H

#include "UnitCell.h"
#include "LatticeTraits.h"


class UnitCellFactory {
public:
    static UnitCell createScCell(const std::array<double, 3> &linearSize);
    static UnitCell createScCell(double linearSize);

    static UnitCell createBccCell(const std::array<double, 3> &linearSize);
    static UnitCell createBccCell(double linearSize);

    static UnitCell createFccCell(const std::array<double, 3> &linearSize);
    static UnitCell createFccCell(double linearSize);

    static UnitCell createHcpCell(const std::array<double, 3> &cuboidalCellSize,
                                  LatticeTraits::Axis axis = LatticeTraits::Axis::Z);
    static UnitCell createHcpCell(double ballDiameter, LatticeTraits::Axis axis = LatticeTraits::Axis::Z);

    static UnitCell createHexagonalCell(const std::array<double, 3> &cuboidalCellSize,
                                        LatticeTraits::Axis axis = LatticeTraits::Axis::Z);
    static UnitCell createHexagonalCell(double ballDiameter, LatticeTraits::Axis axis = LatticeTraits::Axis::Z);
};


#endif //RAMPACK_UNITCELLFACTORY_H
