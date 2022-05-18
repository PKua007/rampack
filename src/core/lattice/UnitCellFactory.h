//
// Created by pkua on 18.05.22.
//

#ifndef RAMPACK_UNITCELLFACTORY_H
#define RAMPACK_UNITCELLFACTORY_H

#include "UnitCell.h"


class UnitCellFactory {
public:
    static UnitCell createScCell(double linearSize);
    static UnitCell createScCell(const std::array<double, 3> &linearSize);
    static UnitCell createBccCell(double linearSize);
    static UnitCell createBccCell(const std::array<double, 3> &linearSize);
    static UnitCell createFccCell(double linearSize);
    static UnitCell createFccCell(const std::array<double, 3> &linearSize);
    static UnitCell createHcpCell(double ballDiameter);
    static UnitCell createHcpCell(const std::array<double, 3> &cuboidalCellSize);
    static UnitCell createHexagonalCell(double ballDiameter);
    static UnitCell createHexagonalCell(const std::array<double, 3> &cuboidalCellSize);
};


#endif //RAMPACK_UNITCELLFACTORY_H
