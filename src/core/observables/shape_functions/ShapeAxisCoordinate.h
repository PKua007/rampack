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
    std::string name;

    [[nodiscard]] std::string constructName() const;

public:
    ShapeAxisCoordinate(ShapeGeometry::Axis axis, std::size_t coord);

    [[nodiscard]] double calculate(const Shape &shape, const ShapeTraits &traits) const override;
    [[nodiscard]] std::string getName() const override { return this->name; }
};


#endif //RAMPACK_SHAPEAXISCOORDINATE_H
