//
// Created by pkua on 25.05.22.
//

#ifndef RAMPACK_MOCKSHAPEGEOMETRY_H
#define RAMPACK_MOCKSHAPEGEOMETRY_H

#include <catch2/trompeloeil.hpp>

#include "core/ShapeGeometry.h"


class MockShapeGeometry : public trompeloeil::mock_interface<ShapeGeometry> {
public:
    IMPLEMENT_CONST_MOCK1(getVolume);
    IMPLEMENT_CONST_MOCK1(getPrimaryAxis);
    IMPLEMENT_CONST_MOCK1(getSecondaryAxis);
    IMPLEMENT_CONST_MOCK1(getGeometricOrigin);

    void publicRegisterStaticNamedPoint(const std::string &pointName, const Vector<3> &point) {
        this->registerStaticNamedPoint(pointName, point);
    }

    void publicRegisterDynamicNamedPoint(const std::string &pointName, NamedPoint::DynamicEvaluator point) {
        this->registerDynamicNamedPoint(pointName, std::move(point));
    }

    void publicRegisterTransientNamedPoint(NamedPoint::TransientEvaluator evaluator,
                                           NamedPoint::TransientLister lister)
    {
        this->registerTransientNamedPoint(std::move(evaluator), std::move(lister));
    }
};

#endif //RAMPACK_MOCKSHAPEGEOMETRY_H
