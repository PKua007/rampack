//
// Created by Piotr Kubala on 01/03/2024.
//

#ifndef RAMPACK_DYNAMICSHAPECACHE_H
#define RAMPACK_DYNAMICSHAPECACHE_H

#include <vector>

#include "core/ShapeDataManager.h"


template<typename ConcreteSpecies>
class DynamicShapeCache : public ShapeDataManager {
private:
    struct Data {
        std::size_t speciesIdx{};

        friend bool operator==(Data lhs, Data rhs) {
            return lhs.speciesIdx == rhs.speciesIdx;
        }
    };

    mutable std::vector<ConcreteSpecies> speciesCache;

protected:
    [[nodiscard]] const ConcreteSpecies &speciesFor(const std::byte *data) const {
        std::size_t shapeIdx = ShapeData::as<Data>(data).speciesIdx;
        return this->speciesCache[shapeIdx];
    }

    [[nodiscard]] const ConcreteSpecies &speciesFor(const ShapeData &data) const {
        std::size_t speciesIdx = data.as<Data>().speciesIdx;
        Expects(speciesIdx < this->speciesCache.size());
        return this->speciesCache[speciesIdx];
    }

    [[nodiscard]] const ConcreteSpecies &speciesFor(const Shape &shape) const {
        return this->speciesFor(shape.getData());
    }

    template<typename ...Args>
    ShapeData shapeDataForSpeciesImpl(Args &&...args) const {
        for (std::size_t speciesIdx{}; speciesIdx < this->speciesCache.size(); speciesIdx++) {
            const auto &species = this->speciesCache[speciesIdx];
            if (species.equal(std::forward<Args>(args)...))
                return ShapeData(Data{speciesIdx});
        }

        this->speciesCache.emplace_back(std::forward<Args>(args)...);
        return ShapeData(Data{this->speciesCache.size() - 1});
    }

public:
    [[nodiscard]] std::size_t getShapeDataSize() const final {
        return sizeof(Data);
    }

    void validateShapeData(const ShapeData &data) const final {
        const auto &wedgeData = data.as<Data>();
        ShapeDataValidateMsg(wedgeData.speciesIdx < this->speciesCache.size(), "Species index out of range");
    }

    [[nodiscard]] ShapeData::Comparator getComparator() const final {
        return ShapeData::Comparator::forType<Data>();
    }

    [[nodiscard]] TextualShapeData serialize([[maybe_unused]] const ShapeData &data) const override {
        AssertThrow("Must be implemented");
    }

    [[nodiscard]] ShapeData deserialize([[maybe_unused]] const TextualShapeData &data) const override {
        AssertThrow("Must be implemented");
    }
};


#endif //RAMPACK_DYNAMICSHAPECACHE_H
