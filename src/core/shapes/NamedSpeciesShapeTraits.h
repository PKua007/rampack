//
// Created by Piotr Kubala on 13/03/2024.
//

#ifndef RAMPACK_NAMEDSPECIESSHAPETRAITS_H
#define RAMPACK_NAMEDSPECIESSHAPETRAITS_H

#include <utility>

#include "core/ShapeTraits.h"


/**
 * @brief ShapeTraits decorator adding the registry of named shape species.
 * @details <p> This class turns the ShapeTraits whose ShapeData parameters can have arbitrary values into a class with
 * a predefined (registered before the usage) set of quantized ShapeData instances with associated species names. As a
 * result, its behavior and some of the methods resemble GenericShapeRegistry. It is, however, important to note that
 * due to performance and compatibility reasons, the underlying ShapeData type is not changed. The only altered behavior
 * is the one of serialization methods.
 */
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
    /**
     * @brief Creates the class for underlying ShapeTraits @a underlyingTraits.
     */
    explicit NamedSpeciesShapeTraits(const std::shared_ptr<ShapeTraits> &underlyingTraits)
            : underlyingTraits{underlyingTraits}, underlyingInteraction{underlyingTraits->getInteraction()},
              underlyingGeometry{underlyingTraits->getGeometry()},
              underlyingDataManager{underlyingTraits->getDataManager()}
    { }

    /**
     * @brief Creates the class for underlying ShapeTraits @a underlyingTraits and also sets the default shape data
     * as @a defaultData with an associated species name @a defaultName.
     */
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

    /**
     * @brief Serializes the ShapeData @a data into a single element `species=[name]` map, where `[name]` is the name
     * of a species associated to @a data in the registry.
     * @throws ShapeDataFormatException if @a data is not present in the registry
     */
    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const override;

    /**
     * @brief Deserializes the ShapeData @a data form a single element `species=[name]` map, where `[name]` is the name
     * of a species associated to @a data in the registry.
     * @throws ShapeDataSerializationException if @a data has incorrect keys or the species' name is not in the registry
     */
    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const override;

    /**
     * @brief Add a new species with ShapeData @a data and the name @a name to the registry.
     * @throws PreconditionException if @a name contains whitespace or quotes, as well as if there already exists a
     * species with a given @a name or @a data in the registry
     */
    void addSpecies(const std::string &name, const ShapeData &data);

    /**
     * @brief Returns @a true, if there exists a species with name @a name in the registry, @a false otherwise
     */
    [[nodiscard]] bool hasSpecies(const std::string &name) const;

    /**
     * @brief Returns @a true, if there exists a species with ShapeData @a data in the registry, @a false otherwise
     */
    [[nodiscard]] bool hasSpecies(const ShapeData &data) const;

    /**
     * @brief Sets the default species (corresponding ShapeDataManager::getDefaultShapeData)
     */
    void setDefaultSpecies(const std::string &name);

    /**
     * @brief Retrieves ShapeData of the species named @a name
     * @throws PreconditionException if the species with name @a name is not present in the registry
     */
    [[nodiscard]] ShapeData shapeDataForSpecies(const std::string &name) const;

    /**
     * @brief Retrieves ShapeData of the default species (as set in the constructor or by setDefaultSpecies())
     * @throws PreconditionException if the default species was not set
     */
    [[nodiscard]] ShapeData shapeDataForDefaultSpecies() const;
};


#endif //RAMPACK_NAMEDSPECIESSHAPETRAITS_H
