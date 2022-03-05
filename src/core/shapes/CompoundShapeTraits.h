//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_COMPOUNDSHAPETRAITS_H
#define RAMPACK_COMPOUNDSHAPETRAITS_H

#include <memory>

#include "core/ShapeTraits.h"
#include "core/interactions/CompoundInteraction.h"


class CompoundShapeTraits : public ShapeTraits {
private:
    std::shared_ptr<ShapeTraits> mainShapeTraits;
    std::shared_ptr<ShapeTraits> auxShapeTraits;
    CompoundInteraction compoundInteraction;

public:
    CompoundShapeTraits(const std::shared_ptr<ShapeTraits> &mainShapeTraits,
                        const std::shared_ptr<ShapeTraits> &auxShapeTraits)
            : mainShapeTraits{mainShapeTraits}, auxShapeTraits{auxShapeTraits},
              compoundInteraction(mainShapeTraits->getInteraction(), auxShapeTraits->getInteraction())
    { }

    [[nodiscard]] const Interaction &getInteraction() const override { return this->compoundInteraction; }
    [[nodiscard]] double getVolume() const override { return this->mainShapeTraits->getVolume(); }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return this->mainShapeTraits->getPrinter(); }

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override {
        return this->mainShapeTraits->getPrimaryAxis(shape);
    }
};


#endif //RAMPACK_COMPOUNDSHAPETRAITS_H
