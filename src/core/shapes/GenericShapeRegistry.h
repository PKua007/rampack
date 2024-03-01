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


template <typename SpeciesClass>
class GenericShapeRegistry : public ShapeGeometry, public ShapeDataManager {
private:
    std::map<std::string, std::size_t> speciesNameIdxMap;
    std::vector<SpeciesClass> speciesStore;

protected:
    void registerCustomNamedPoint(const std::string &pointName) {
        if (this->hasNamedPoint(pointName))
            return;

        this->registerDynamicNamedPoint(pointName, [this, pointName](const ShapeData &data) -> Vector<3> {
            std::size_t speciesIdx = data.as<Data>().speciesIdx;
            const auto &namedPoints = this->getSpecies(speciesIdx).getCustomNamedPoints();

            auto it = namedPoints.find(pointName);
            if (it == namedPoints.end())
                this->throwUnavailableNamedPoint(speciesIdx, pointName);

            return it->second;
        });
    }

    void throwUnavailableNamedPoint(std::size_t speciesIdx, const std::string &pointName) const {
        const std::string &speciesName = this->getSpeciesName(speciesIdx);
        std::ostringstream msg;
        msg << "Named point " << pointName << " is not available for species " << speciesName;
        throw NoSuchNamedPointForShapeException(msg.str());
    }

    [[nodiscard]] const SpeciesClass &speciesFor(const Shape &shape) const {
        return this->speciesFor(shape.getData());
    }

    [[nodiscard]] const SpeciesClass &speciesFor(const ShapeData &data) const {
        std::size_t speciesIdx = data.as<Data>().speciesIdx;
        Expects(speciesIdx < this->speciesStore.size());
        return this->speciesStore[speciesIdx];
    }

    [[nodiscard]] const SpeciesClass &speciesFor(const std::byte *data) const {
        std::size_t speciesIdx = ShapeData::as<Data>(data).speciesIdx;
        return this->speciesStore[speciesIdx];
    }

    [[nodiscard]] SpeciesClass &modifySpecies(std::size_t speciesIdx) {
        Expects(speciesIdx < this->speciesStore.size());
        return this->speciesStore[speciesIdx];
    }

public:
    struct Data {
        std::size_t speciesIdx{};

        friend bool operator==(Data lhs, Data rhs) { return lhs.speciesIdx == rhs.speciesIdx; }
    };

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const final {
        const auto &species = this->speciesFor(shape);
        return shape.getOrientation() * species.getPrimaryAxis();
    }

    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const final {
        const auto &species = this->speciesFor(shape);
        return shape.getOrientation() * species.getSecondaryAxis();
    }

    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const final {
        const auto &species = this->speciesFor(shape);
        return shape.getOrientation() * species.getGeometricOrigin();
    }

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

    [[nodiscard]] TextualShapeData serialize(const ShapeData &data) const final {
        std::size_t speciesIdx = data.as<Data>().speciesIdx;
        const std::string &speciesName = this->getSpeciesName(speciesIdx);

        ShapeDataSerializer serializer;
        serializer["species"] = speciesName;
        return serializer.toTextualShapeData();
    }

    [[nodiscard]] ShapeData deserialize(const TextualShapeData &data) const final {
        ShapeDataDeserializer deserializer(data);
        auto speciesName = deserializer.as<std::string>("species");
        deserializer.throwIfNotAccessed();

        if (!this->hasSpecies(speciesName))
            throw ShapeDataSerializationException("Unknown shape species: " + speciesName);
        return ShapeData(Data{this->getSpeciesIdx(speciesName)});
    }

    virtual ShapeData addSpecies(const std::string &speciesName, const SpeciesClass &species) {
        auto containsQuotes = [](const std::string &str) { return str.find('"') != std::string::npos; };
        Expects(!containsWhitespace(speciesName) && !containsQuotes(speciesName));
        Expects(!this->hasSpecies(speciesName));

        this->speciesStore.push_back(species);
        this->speciesNameIdxMap.emplace(speciesName, this->speciesStore.size() - 1);

        for (const auto &namedPoint : species.getCustomNamedPoints()) {
            const auto &pointName = namedPoint.first;
            this->registerCustomNamedPoint(pointName);
        }

        return ShapeData(Data{this->speciesStore.size() - 1});
    }

    [[nodiscard]] const SpeciesClass &getDefaultSpecies() const {
        const auto &defaultShapeData = this->getDefaultShapeData();
        ExpectsMsg(!defaultShapeData.empty(), "Default species is not defined");
        return this->getSpecies(defaultShapeData.at("species"));
    }

    void setDefaultShape(const std::string &speciesName) {
        Expects(this->hasSpecies(speciesName));
        this->setDefaultShapeData({{"species", speciesName}});
    }

    [[nodiscard]] bool hasSpecies(const std::string &speciesName) const {
        return this->speciesNameIdxMap.find(speciesName) != this->speciesNameIdxMap.end();
    }

    [[nodiscard]] const SpeciesClass &getSpecies(const std::string &shapeName) const {
        return this->speciesStore[this->getSpeciesIdx(shapeName)];
    }

    [[nodiscard]] const SpeciesClass &getSpecies(std::size_t speciesIdx) const {
        Expects(speciesIdx < this->speciesStore.size());
        return this->speciesStore[speciesIdx];
    }

    [[nodiscard]] ShapeData shapeDataForSpecies(const std::string &speciesName) const {
        return ShapeData(Data{this->getSpeciesIdx(speciesName)});
    }

    [[nodiscard]] ShapeData shapeDataForDefaultSpecies() const {
        const auto &defaultShapeData = this->getDefaultShapeData();
        ExpectsMsg(!defaultShapeData.empty(), "Default species is not defined");
        std::size_t speciesIdx = this->getSpeciesIdx(defaultShapeData.at("species"));
        return ShapeData(Data{speciesIdx});
    }

    [[nodiscard]] const std::string &getSpeciesName(std::size_t speciesIdx) const {
        Expects(speciesIdx < this->speciesStore.size());
        for (const auto &[name, idx] : this->speciesNameIdxMap)
            if (idx == speciesIdx)
                return name;
        AssertThrow("Unreachable");
    }

    [[nodiscard]] std::size_t getSpeciesIdx(const std::string &speciesName) const {
        auto it = this->speciesNameIdxMap.find(speciesName);
        ExpectsMsg(it != this->speciesNameIdxMap.end(), "Species " + speciesName + " not found");
        return it->second;
    }

    [[nodiscard]] const std::vector<SpeciesClass> &getAllSpecies() const {
        return this->speciesStore;
    }

    [[nodiscard]] std::size_t getNumOfSpecies() const {
        return this->speciesStore.size();
    }
};


#endif //RAMPACK_GENERICSHAPEREGISTRY_H
