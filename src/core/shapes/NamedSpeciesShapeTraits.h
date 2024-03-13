//
// Created by Piotr Kubala on 13/03/2024.
//

#ifndef RAMPACK_NAMEDSPECIESSHAPETRAITS_H
#define RAMPACK_NAMEDSPECIESSHAPETRAITS_H

#include <utility>

#include "core/ShapeTraits.h"


class NamedSpeciesShapeTraits : public ShapeTraits, public ShapeDataManager {
private:
    using const_iterator = std::map<std::string, ShapeData>::const_iterator;

    [[nodiscard]] const_iterator findByName(const std::string &name) const;
    [[nodiscard]] const_iterator findByData(const ShapeData &data) const;

    std::shared_ptr<ShapeTraits> underlyingTraits;
    const Interaction &underlyingInteraction;
    const ShapeGeometry &underlyingGeometry;
    const ShapeDataManager &underlyingDataManager;
    std::map<std::string, ShapeData> speciesMap;

public:
    explicit NamedSpeciesShapeTraits(const std::shared_ptr<ShapeTraits> &underlyingTraits)
            : underlyingTraits{underlyingTraits}, underlyingInteraction{underlyingTraits->getInteraction()},
              underlyingGeometry{underlyingTraits->getGeometry()},
              underlyingDataManager{underlyingTraits->getDataManager()}
    { }

    NamedSpeciesShapeTraits(const std::shared_ptr<ShapeTraits> &underlyingTraits, const std::string &defaultName,
                            const ShapeData &defaultData);

    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
    [[nodiscard]] const Interaction &getInteraction() const override { return this->underlyingInteraction; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return this->underlyingGeometry; }
    [[nodiscard]] std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const override {
        return this->underlyingTraits->getPrinter(format, params);
    }

    [[nodiscard]] std::size_t getShapeDataSize() const override {
        return this->underlyingDataManager.getShapeDataSize();
    }

    void validateShapeData(const ShapeData &data) const override;

    [[nodiscard]] ShapeData::Comparator getComparator() const override {
        return this->underlyingDataManager.getComparator();
    }

    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const override;
    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const override;

    void addSpecies(const std::string &name, const ShapeData &data);
    [[nodiscard]] bool hasSpecies(const std::string &name) const;
    [[nodiscard]] bool hasSpecies(const ShapeData &data) const;
    void setDefaultSpecies(const std::string &name);
    [[nodiscard]] ShapeData shapeDataForSpecies(const std::string &name) const;
    [[nodiscard]] ShapeData shapeDataForDefaultSpecies() const;
};


#endif //RAMPACK_NAMEDSPECIESSHAPETRAITS_H
