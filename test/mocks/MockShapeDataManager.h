//
// Created by Piotr Kubala on 08/02/2024.
//

#ifndef RAMPACK_MOCKSHAPEDATAMANAGER_H
#define RAMPACK_MOCKSHAPEDATAMANAGER_H

#include <catch2/trompeloeil.hpp>

#include "core/ShapeDataManager.h"


class MockShapeDataManager : public trompeloeil::mock_interface<ShapeDataManager>  {
public:
    IMPLEMENT_CONST_MOCK0(getShapeDataSize);
    IMPLEMENT_CONST_MOCK1(validateShapeData);
};


#endif //RAMPACK_MOCKSHAPEDATAMANAGER_H
