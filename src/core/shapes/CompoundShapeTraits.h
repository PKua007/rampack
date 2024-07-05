//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_COMPOUNDSHAPETRAITS_H
#define RAMPACK_COMPOUNDSHAPETRAITS_H

#include <memory>

#include "core/ShapeTraits.h"
#include "core/shapes/interactions/CompoundInteraction.h"


/**
 * @brief Compound shape traits, where one ShapeTraits instance (main traits) acts as a base and the second one (helper
 * traits) only provides additional Interaction type on top of the main one.
 * @details ShapeGeometry, ShapeDataManager, and ShapePrinter -s are delegated from the main traits. It means that, in
 * particular, the associated ShapeData of CompoundShapeTraits correspond to the one of the main traits. Helper traits
 * have a constant ShapeData, which is passed in the constructor and cannot be altered from the outside.
 * @sa CompoundInteraction for the restrictions on underlying Interaction instances
 */
class CompoundShapeTraits : public ShapeTraits {
private:
    std::shared_ptr<ShapeTraits> mainShapeTraits;
    std::shared_ptr<ShapeTraits> helperShapeTraits;
    CompoundInteraction compoundInteraction;

public:
    /**
     * @brief Construct the class for two given ShapeTraits.
     * @param mainShapeTraits main shaped traits used as a base
     * @param helperShapeTraits helper traits, supplying only additional interaction type on top of the one of
     * @a mainShapeTraits
     * @param helperData constant ShapeData associated with @a helperShapeTraits
     */
    CompoundShapeTraits(const std::shared_ptr<ShapeTraits> &mainShapeTraits,
                        const std::shared_ptr<ShapeTraits> &helperShapeTraits, ShapeData helperData = {})
            : mainShapeTraits{mainShapeTraits}, helperShapeTraits{helperShapeTraits},
              compoundInteraction(mainShapeTraits->getInteraction(), helperShapeTraits->getInteraction(),
                                  std::move(helperData))
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
