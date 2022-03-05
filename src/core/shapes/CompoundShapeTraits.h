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
    std::unique_ptr<ShapeTraits> mainShapeTraits;
    [[maybe_unused]] std::unique_ptr<ShapeTraits> auxShapeTraits;
    CompoundInteraction interaction;

public:
    CompoundShapeTraits(std::unique_ptr<ShapeTraits> mainShapeTraits, std::unique_ptr<ShapeTraits> auxShapeTraits)
            : mainShapeTraits{std::move(auxShapeTraits)}, auxShapeTraits{std::move(auxShapeTraits)},
              interaction(this->mainShapeTraits->getInteraction(), this->auxShapeTraits->getInteraction())
    { }

    [[nodiscard]] const Interaction &getInteraction() const override { return this->mainShapeTraits->getInteraction(); }
    [[nodiscard]] double getVolume() const override { return this->mainShapeTraits->getVolume(); }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return this->mainShapeTraits->getPrinter(); }

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override {
        return this->mainShapeTraits->getPrimaryAxis(shape);
    }
};


#endif //RAMPACK_COMPOUNDSHAPETRAITS_H
