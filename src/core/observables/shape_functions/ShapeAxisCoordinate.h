//
// Created by Piotr Kubala on 23/03/2023.
//

#ifndef RAMPACK_SHAPEAXISCOORDINATE_H
#define RAMPACK_SHAPEAXISCOORDINATE_H

#include "core/observables/ShapeFunction.h"


class ShapeAxisCoordinate : public ShapeFunction {
private:
    ShapeGeometry::Axis axis{};
    std::size_t coord{};

public:
    ShapeAxisCoordinate(ShapeGeometry::Axis axis, std::size_t coord);

    [[nodiscard]] double calculate(const Shape &shape, const ShapeTraits &traits) const override;
};


#endif //RAMPACK_SHAPEAXISCOORDINATE_H
