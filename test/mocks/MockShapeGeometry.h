//
// Created by pkua on 25.05.22.
//

#ifndef RAMPACK_MOCKSHAPEGEOMETRY_H
#define RAMPACK_MOCKSHAPEGEOMETRY_H

#include <catch2/trompeloeil.hpp>

#include "core/ShapeGeometry.h"


class MockShapeGeometry : public ShapeGeometry {
public:
    MAKE_CONST_MOCK0(getVolume, double(), override);
    MAKE_CONST_MOCK1(getPrimaryAxis, Vector<3>(const Shape &), override);
    MAKE_CONST_MOCK1(getSecondaryAxis, Vector<3>(const Shape &), override);
    MAKE_CONST_MOCK1(getGeometricOrigin, Vector<3>(const Shape &), override);
    void addNamedPoints(const std::map<std::string, Vector<3>> &namedPoints) { this->registerNamedPoints(namedPoints); }
};

#endif //RAMPACK_MOCKSHAPEGEOMETRY_H
