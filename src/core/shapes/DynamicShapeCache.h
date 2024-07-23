//
// Created by Piotr Kubala on 01/03/2024.
//

#ifndef RAMPACK_DYNAMICSHAPECACHE_H
#define RAMPACK_DYNAMICSHAPECACHE_H

#include <vector>

#include "core/ShapeDataManager.h"


/**
 * @brief ShapeDataManager, where ShapeData are indices dynamically associated to @a ConcreteSpecies instances.
 * @details <p> The class acts as a partial workaround for ShapeTraits, where the ShapeData should be created on the fly
 * from given arguments (in contrast to GenericShapeRegistry, where the shapes are predefined), but @a ConcreteSpecies
 * cannot act as valid ShapeData, because it is too large or not trivially copyable.
 *
 * <p> @a ConcreteSpecies cache is managed by the shapeDataForSpeciesImpl() method. If its arguments construct the @a
 * ConcreteSpecies which was not seen previously, it is inserted into the cache. Otherwise, the cached object is
 * referenced.
 *
 * <p> Contrary to GenericShapeRegistry, which serializes ShapeData by the species names, this class is supposed to
 * serialize them by the arguments of the @a ConcreteSpecies constructor. Thus, this class must be extended by a
 * ShapeDataManager class for the given @a ConcreteSpecies in order to implement serialize() and deserialize() methods.
 * To avoid devising an additional class, DynamicShapeCache can be extended directly by the concrete ShapeTraits
 * implementation.
 * @tparam ConcreteSpecies Class representing concrete species. It does not have to be trivially copyable (in contrast
 * to valid ShapeData types). It must have a constructor accepting all its parameters and the method `equal` with the
 * same signature, returning `true` if the @a ConcreteSpecies object constructed from method's arguments would be equal
 * to the current instance. Example class conforming to @a ConcreteSpecies requirements:
 * @code
 * // Three-dimensional text shape
 * class Text3D {
 * private:
 *     std::string text;    // std::string is not trivially copyable, thus Text3D would not be a valid ShapeData
 *     double fontSize;
 *
 * public:
 *     Text3D(std::string text, double fontSize) : text{text}, fontSize{fontSize} { }
 *
 *     bool equal(std::string text_, double fontSize_) { return text_ == this->text && fontSize_ == this->fontSize; }
 * };
 * @endcode
 * @sa GenericShapeRegistry
 */
template<typename ConcreteSpecies>
class DynamicShapeCache :  public ShapeGeometry, public ShapeDataManager {
private:
    struct Data {
        std::size_t speciesIdx{};

        friend bool operator==(Data lhs, Data rhs) {
            return lhs.speciesIdx == rhs.speciesIdx;
        }
    };

    mutable std::vector<ConcreteSpecies> speciesCache;

protected:
    /**
     * @brief Returns @a ConcreteSpecies reference for raw ShapeData @a data. As raw ShapeData are used in
     * performance-critical code path, the cache bounds are not checked.
     */
    [[nodiscard]] const ConcreteSpecies &speciesFor(const std::byte *data) const {
        std::size_t shapeIdx = ShapeData::as<Data>(data).speciesIdx;
        return this->speciesCache[shapeIdx];
    }

    /**
     * @brief Returns @a ConcreteSpecies reference for ShapeData @a data. The cache bounds are validated.
     */
    [[nodiscard]] const ConcreteSpecies &speciesFor(const ShapeData &data) const {
        std::size_t speciesIdx = data.as<Data>().speciesIdx;
        Expects(speciesIdx < this->speciesCache.size());
        return this->speciesCache[speciesIdx];
    }

    /**
     * @brief Returns @a ConcreteSpecies reference for ShapeData of the Shape @a shape. The cache bounds are validated.
     */
    [[nodiscard]] const ConcreteSpecies &speciesFor(const Shape &shape) const {
        return this->speciesFor(shape.getData());
    }

    /**
     * @brief Method used for retrieving ShapeData for already existing or newly registered @a ConcreteSpecies instance
     * with the constructor arguments @a args.
     * @details <p> Internally, the methods first attempts to find if the shape is already being cached, using
     * `ConcreteSpecies::equal` method. If not, a new instance is created and registered in the cache.
     *
     * <p> Implementing classes should expose this method by creating a thin wrapper around it (for example called
     * @a shapeDataForSpecies without @a "Impl") with a non-variadic list of arguments, coinciding with the signature of
     * the constructor of @a ConcreteSpecies. For example, for @a Text3D shape from the class description, a correct
     * implementation could be
     * @code
     * class Text3DTraits : public DynamicShapeCache<Text3D> {
     * public:
     *     ShapeData shapeDataForSpecies(std::string text, double fontSize) {
     *         return this->shapeDataForSpeciesImpl(text, fontSize);
     *     }
     *
     *     // other methods
     * };
     * @endcode
     * @return ShapeData referencing the `ConcreteSpecies(args...)` object in the shape cache.
     */
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
        const auto &wedgeData = data.as<Data>();
        ShapeDataValidateMsg(wedgeData.speciesIdx < this->speciesCache.size(), "Species index out of range");
    }

    [[nodiscard]] ShapeData::Comparator getComparator() const final {
        return ShapeData::Comparator::forType<Data>();
    }

    /**
     * @brief ShapeDataManager::serialize method, which must be implemented by the extending class.
     */
    [[nodiscard]] TextualShapeData serialize([[maybe_unused]] const ShapeData &data) const override {
        AssertThrow("Must be implemented");
    }

    /**
     * @brief ShapeDataManager::deserialize method, which must be implemented by the extending class.
     */
    [[nodiscard]] ShapeData deserialize([[maybe_unused]] const TextualShapeData &data) const override {
        AssertThrow("Must be implemented");
    }
};


#endif //RAMPACK_DYNAMICSHAPECACHE_H
