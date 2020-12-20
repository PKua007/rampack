//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SHAPETRAITS_H
#define RAMPACK_SHAPETRAITS_H

#include "Shape.h"
#include "Interaction.h"
#include "ShapePrinter.h"

class ShapeTraits {
public:
    virtual ~ShapeTraits() = default;

    [[nodiscard]] virtual const Interaction &getInteraction() const = 0;
    [[nodiscard]] virtual double getVolume() const = 0;
    [[nodiscard]] virtual const ShapePrinter &getPrinter() const = 0;
};

#endif //RAMPACK_SHAPETRAITS_H
