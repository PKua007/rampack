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
    std::shared_ptr<ShapeTraits> helperShapeTraits;
    CompoundInteraction compoundInteraction;

public:
    /**
     * @brief Construct the class with @a mainShapeTraits as main ShapeTraits, while @a auxShapeTraits only provide
     * additional interaction.
     */
    CompoundShapeTraits(const std::shared_ptr<ShapeTraits> &mainShapeTraits,
                        const std::shared_ptr<ShapeTraits> &helperShapeTraits, ShapeData helperData)
            : mainShapeTraits{mainShapeTraits}, helperShapeTraits{helperShapeTraits},
              compoundInteraction(mainShapeTraits->getInteraction(), helperShapeTraits->getInteraction(), std::move(helperData))
    { }

    [[nodiscard]] const Interaction &getInteraction() const override { return this->compoundInteraction; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return this->mainShapeTraits->getGeometry(); }

    [[nodiscard]] const ShapeDataManager &getDataManager() const override {
        return this->mainShapeTraits->getDataManager();
    }

    [[nodiscard]] std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const override
    {
        return this->mainShapeTraits->getPrinter(format, params);
    }
};


#endif //RAMPACK_COMPOUNDSHAPETRAITS_H
