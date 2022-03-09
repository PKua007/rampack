//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_COMPOUNDSHAPETRAITS_H
#define RAMPACK_COMPOUNDSHAPETRAITS_H

#include <memory>

#include "core/ShapeTraits.h"
#include "core/interactions/CompoundInteraction.h"


/**
 * @brief Construct compound shape traits, where one ShapeTraits instance acts as a base and the second one provides
 * only additional interaction.
 * @details For example, shapes are printed according to main ShapeTraits instance.
 */
class CompoundShapeTraits : public ShapeTraits {
private:
    std::shared_ptr<ShapeTraits> mainShapeTraits;
    std::shared_ptr<ShapeTraits> auxShapeTraits;
    CompoundInteraction compoundInteraction;

public:
    /**
     * @brief Construct the class with @a mainShapeTraits as main ShapeTraits, while @a auxShapeTraits only provide
     * additional interaction.
     */
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
