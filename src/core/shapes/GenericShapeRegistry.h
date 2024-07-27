//
// Created by Piotr Kubala on 28/02/2024.
//

#ifndef RAMPACK_GENERICSHAPEREGISTRY_H
#define RAMPACK_GENERICSHAPEREGISTRY_H

#include <map>
#include <string>

#include "core/ShapeGeometry.h"
#include "core/ShapeDataManager.h"
#include "utils/Exceptions.h"
#include "core/io/ShapeDataSerializer.h"
#include "core/io/ShapeDataDeserializer.h"


/**
 * @brief Fully working implementation of ShapeDataManager and ShapeGeometry, which acts as a storage for a set of named
 * shape species, whose ShapeData are their index (represented by GenericShapeRegistry::Data) in the storage.
 * @details The class acts as a database of named shapes, whose data do not have to be trivially-copyable. Contrary to
 * DynamicShapeCache, where it is dynamically determined whether the species is new and should be registered, or already
 * exists and should be retrieved from the cache, in GenericShapeRegistry species should be explicitly named and
 * registered using addSpecies().
 * @tparam ConcreteSpecies Class representing concrete species. It does not have to be trivially copyable (in contrast
 * tocvalid ShapeData types). It must contain the following methods used for implementing ShapeGeometry methods:
 * @code
 * // Returns the primary axis for a default oriented species
 * Vector<3> getPrimaryAxis() const
 *
 * // Returns the secondary axis for a default oriented species
 * Vector<3> getSecondaryAxis() const
 *
 * // Returns the geometric origin for a default oriented species
 * Vector<3> getGeometricOrigin() const
 *
 * // Returns the volume of the species
 * double getVolume() const
 *
 * // Returns the map of named points (name mapped to a point on a default oriented species)
 * const std::map<std::string, Vector<3>> &getNamedPoints() const
 * @endcode
 * @sa DynamicShapeCache
 */
template <typename ConcreteSpecies>
class GenericShapeRegistry : public ShapeGeometry, public ShapeDataManager {
private:
    std::map<std::string, std::size_t> speciesNameIdxMap;
    std::vector<ConcreteSpecies> speciesStore;

protected:
    /**
     * @brief Returns @a ConcreteSpecies reference for raw ShapeData @a data. As raw ShapeData are used in
     * performance-critical code path, the registry bounds are not checked.
     */
    [[nodiscard]] const ConcreteSpecies &speciesFor(const std::byte *data) const {
        std::size_t speciesIdx = ShapeData::as<Data>(data).speciesIdx;
        return this->speciesStore[speciesIdx];
    }

    /**
     * @brief Returns @a ConcreteSpecies reference for ShapeData @a data. The registry bounds are validated.
     */
    [[nodiscard]] const ConcreteSpecies &speciesFor(const ShapeData &data) const {
        std::size_t speciesIdx = data.as<Data>().speciesIdx;
        Expects(speciesIdx < this->speciesStore.size());
        return this->speciesStore[speciesIdx];
    }

    /**
     * @brief Returns @a ConcreteSpecies reference for ShapeData of the Shape @a shape. The registry bounds are
     * validated.
     */
    [[nodiscard]] const ConcreteSpecies &speciesFor(const Shape &shape) const {
        return this->speciesFor(shape.getData());
    }

    /**
     * @brief Returns modifiable reference to @a ConcreteSpecies for a given @a speciesIdx.
     */
    [[nodiscard]] ConcreteSpecies &modifySpecies(std::size_t speciesIdx) {
        Expects(speciesIdx < this->speciesStore.size());
        return this->speciesStore[speciesIdx];
    }

public:
    /**
     * @brief ShapeData compatible species index holder.
     */
    struct Data {
        /**
         * @brief Index of the species in the species registry, recognized by methods such as
         * GenericShapeRegistry::getSpecies(std::size_t) const
         */
        std::size_t speciesIdx{};

        friend bool operator==(Data lhs, Data rhs) { return lhs.speciesIdx == rhs.speciesIdx; }
    };

    GenericShapeRegistry() {
        NamedPoint::TransientEvaluator evaluator = [this](const std::string &pointName, const ShapeData &data) {
            const ConcreteSpecies &species = this->speciesFor(data);
            const std::map<std::string, Vector<3>> &namedPoints = species.getNamedPoints();

            auto it = namedPoints.find(pointName);
            if (it == namedPoints.end()) {
                const auto &speciesName = this->getSpeciesName(data.as<Data>().speciesIdx);
                throw NoSuchNamedPointForShapeException("No point named " + pointName + " for species " + speciesName);
            }

            return it->second;
        };

        NamedPoint::TransientLister lister = [this](const ShapeData &data) {
            const ConcreteSpecies &species = this->speciesFor(data);
            const std::map<std::string, Vector<3>> &namedPoints = species.getNamedPoints();

            std::set<std::string> pointNames;
            for (const auto &[pointName, pointCoords] : namedPoints)
                pointNames.insert(pointName);
            return pointNames;
        };

        this->registerTransientNamedPoint(std::move(evaluator), std::move(lister));
    }

    GenericShapeRegistry(const GenericShapeRegistry &) = delete;
    GenericShapeRegistry &operator=(const GenericShapeRegistry &) = delete;

    /**
     * @brief Return shape's primary axis based on `ConcreteSpecies::getPrimaryAxis`.
     */
    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const final {
        const auto &species = this->speciesFor(shape);
        return shape.getOrientation() * species.getPrimaryAxis();
    }

    /**
     * @brief Return shape's secondary axis based on `ConcreteSpecies::getSecondaryAxis`.
     */
    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const final {
        const auto &species = this->speciesFor(shape);
        return shape.getOrientation() * species.getSecondaryAxis();
    }

    /**
     * @brief Return shape's geometric origin based on `ConcreteSpecies::getGeometricOrigin`.
     */
    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const final {
        const auto &species = this->speciesFor(shape);
        return shape.getOrientation() * species.getGeometricOrigin();
    }

    /**
     * @brief Return shape's volume based on `ConcreteSpecies::getVolume`.
     */
    [[nodiscard]] double getVolume(const Shape &shape) const final {
        return this->speciesFor(shape).getVolume();
    }

    [[nodiscard]] std::size_t getShapeDataSize() const final {
        return sizeof(Data);
    }

    void validateShapeData(const ShapeData &data) const final {
        const auto &speciesData = data.as<Data>();
        ShapeDataValidateMsg(speciesData.speciesIdx < this->speciesStore.size(), "Species index out of range");
    }

    [[nodiscard]] ShapeData::Comparator getComparator() const final {
        return ShapeData::Comparator::forType<Data>();
    }

    /**
     * @brief Serializes the shape data into a single-element map `species=[name]`, where `[name]` is the species' name.
     */
    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const final {
        std::size_t speciesIdx = data.as<Data>().speciesIdx;
        const std::string &speciesName = this->getSpeciesName(speciesIdx);

        ShapeDataSerializer serializer;
        serializer["species"] = speciesName;
        return serializer.toTextualShapeData();
    }

    /**
     * @brief Deserializes the shape data from a single-element map `species=[name]`, where `[name]` is the species'
     * name.
     * @throws ShapeDataSerializationException if @a data has incorrect keys or the species' name is not in the registry
     */
    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const final {
        ShapeDataDeserializer deserializer(data);
        auto speciesName = deserializer.as<std::string>("species");
        deserializer.throwIfNotAccessed();

        if (!this->hasSpecies(speciesName))
            throw ShapeDataSerializationException("Unknown shape species: " + speciesName);
        return ShapeData(Data{this->getSpeciesIdx(speciesName)});
    }

    /**
     * @brief Adds a new species to the registry.
     * @details The method is virtual, so that the deriving classes can extend its behavior, for example by modifying
     * the species before registering it.
     * @param speciesName name of the new species
     * @param species species object
     * @return ShapeData for the newly registered species
     * @throws PreconditionException if the species name contains whitespaces or quotes, as well as if @a speciesName
     * already exists in the registry
     */
    virtual ShapeData addSpecies(const std::string &speciesName, const ConcreteSpecies &species) {
        auto containsQuotes = [](const std::string &str) { return str.find('"') != std::string::npos; };
        Expects(!containsWhitespace(speciesName) && !containsQuotes(speciesName));
        Expects(!this->hasSpecies(speciesName));

        this->speciesStore.push_back(species);
        this->speciesNameIdxMap.emplace(speciesName, this->speciesStore.size() - 1);
        return ShapeData(Data{this->speciesStore.size() - 1});
    }

    /**
     * @brief Returns the default species (corresponding to ShapeDataManager::getDefaultShapeData).
     */
    [[nodiscard]] const ConcreteSpecies &getDefaultSpecies() const {
        const auto &defaultShapeData = this->getDefaultShapeData();
        ExpectsMsg(!defaultShapeData.empty(), "Default species is not defined");
        return this->getSpecies(defaultShapeData.at("species"));
    }

    /**
     * @brief Sets the default species (corresponding to ShapeDataManager::getDefaultShapeData).
     */
    void setDefaultSpecies(const std::string &speciesName) {
        Expects(this->hasSpecies(speciesName));
        this->setDefaultShapeData({{"species", speciesName}});
    }

    /**
     * @brief Returns @a true if the species named @a speciesName exists in the registry.
     */
    [[nodiscard]] bool hasSpecies(const std::string &speciesName) const {
        return this->speciesNameIdxMap.find(speciesName) != this->speciesNameIdxMap.end();
    }

    /**
     * @brief Returns the species named @a speciesName.
     * @throws PreconditionException if the species named @a speciesName does not exist
     */
    [[nodiscard]] const ConcreteSpecies &getSpecies(const std::string &shapeName) const {
        return this->speciesStore[this->getSpeciesIdx(shapeName)];
    }

    /**
     * @brief Returns the species with index @a speciesIdx.
     * @throws PreconditionException if @a speciesIdx is out of bounds
     */
    [[nodiscard]] const ConcreteSpecies &getSpecies(std::size_t speciesIdx) const {
        Expects(speciesIdx < this->speciesStore.size());
        return this->speciesStore[speciesIdx];
    }

    /**
     * @brief Returns the ShapeData for the species named @a speciesName.
     * @throws PreconditionException if the species named @a speciesName does not exist
     */
    [[nodiscard]] ShapeData shapeDataForSpecies(const std::string &speciesName) const {
        return ShapeData(Data{this->getSpeciesIdx(speciesName)});
    }

    /**
     * @brief Returns the ShapeData for the default species set by setDefaultSpecies().
     * @throws PreconditionException if the default species was not set
     */
    [[nodiscard]] ShapeData shapeDataForDefaultSpecies() const {
        const auto &defaultShapeData = this->getDefaultShapeData();
        ExpectsMsg(!defaultShapeData.empty(), "Default species is not defined");
        std::size_t speciesIdx = this->getSpeciesIdx(defaultShapeData.at("species"));
        return ShapeData(Data{speciesIdx});
    }

    /**
     * @brief Returns the name of the species with index @a speciesIdx.
     * @throws PreconditionException if @a speciesIdx is out of bounds
     */
    [[nodiscard]] const std::string &getSpeciesName(std::size_t speciesIdx) const {
        Expects(speciesIdx < this->speciesStore.size());
        for (const auto &[name, idx] : this->speciesNameIdxMap)
            if (idx == speciesIdx)
                return name;
        AssertThrow("Unreachable");
    }

    /**
     * @brief Returns the index of the species named @a speciesName.
     * @throws PreconditionException if the species named @a speciesName does not exist
     */
    [[nodiscard]] std::size_t getSpeciesIdx(const std::string &speciesName) const {
        auto it = this->speciesNameIdxMap.find(speciesName);
        ExpectsMsg(it != this->speciesNameIdxMap.end(), "Species " + speciesName + " not found");
        return it->second;
    }

    /**
     * @brief Returns @a std::vector of all species in the registry.
     */
    [[nodiscard]] const std::vector<ConcreteSpecies> &getAllSpecies() const {
        return this->speciesStore;
    }

    /**
     * @brief Returns the number of registered species.
     */
    [[nodiscard]] std::size_t getNumOfSpecies() const {
        return this->speciesStore.size();
    }
};


#endif //RAMPACK_GENERICSHAPEREGISTRY_H
