//
// Created by pkua on 05.10.22.
//

#ifndef RAMPACK_GENERICXENOCOLLIDETRAITS_H
#define RAMPACK_GENERICXENOCOLLIDETRAITS_H

#include <optional>
#include <map>

#include "XenoCollideTraits.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "GenericShapeRegistry.h"
#include "XCGeometryCenter.h"


class GenericXenoCollideTraits;

/**
 * @brief Generic convex shape with XenoCollide intersection test (GenericShapeRegistry species for
 * GenericXenoCollideTraits).
 */
class GenericXenoCollideShape /* : public GenericShapeRegistry::ConcreteSpecies */ {
private:
    std::vector<std::shared_ptr<const AbstractXCGeometry>> geometries;
    std::vector<Vector<3>> interactionCentres;
    std::optional<Vector<3>> primaryAxis;
    std::optional<Vector<3>> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};
    std::map<std::string, Vector<3>> namedPoints;
    bool convex{};

    friend GenericXenoCollideTraits;

public:
    /**
     * @brief Creates the shapes with a single interaction center. The shape is by definition convex.
     * @param geometry XenoCollide geometry of the shape
     * @param volume volume of the shape
     * @param primaryAxis primary axis of the shape (may be left undefined)
     * @param secondaryAxis secondary axis of the shape, orthogonal to the primary axis (may be left undefined)
     * @param geometricOrigin geometric origin of the shape
     * @param customNamedPoints optional map of name points, where the key is point's name and the value is its position
     */
    GenericXenoCollideShape(std::shared_ptr<AbstractXCGeometry> geometry, double volume,
                            OptionalAxis primaryAxis = std::nullopt, OptionalAxis secondaryAxis = std::nullopt,
                            const Vector<3> &geometricOrigin = {0, 0, 0},
                            const std::map<std::string, Vector<3>> &customNamedPoints = {});

    /**
     * @brief Creates the shape with multiple interaction centers. Multi-part shape is by default treated as concave.
     * @param geometries vector of geometries of all interaction centers
     * @param volume volume of the shape
     * @param primaryAxis primary axis of the shape (may be left undefined)
     * @param secondaryAxis secondary axis of the shape, orthogonal to the primary axis (may be left undefined)
     * @param geometricOrigin geometric origin of the shape
     * @param customNamedPoints optional map of name points, where the key is point's name and the value is its position
     * @param forceConvex should be set @a true if the shape is convex despite being multi-part
     */
    GenericXenoCollideShape(const std::vector<XCGeometryCenter> &geometries, double volume,
                            OptionalAxis primaryAxis = std::nullopt, OptionalAxis secondaryAxis = std::nullopt,
                            const Vector<3> &geometricOrigin = {0, 0, 0},
                            const std::map<std::string, Vector<3>> &customNamedPoints = {},
                            bool forceConvex = false);

    [[nodiscard]] Vector<3> getPrimaryAxis() const;
    [[nodiscard]] Vector<3> getSecondaryAxis() const;
    [[nodiscard]] Vector<3> getGeometricOrigin() const {
        return this->geometricOrigin;
    }
    [[nodiscard]] double getVolume() const { return this->volume; }
    [[nodiscard]] const std::vector<std::shared_ptr<const AbstractXCGeometry>> &getGeometries() const {
        return this->geometries;
    }
    [[nodiscard]] const std::vector<Vector<3>> &getInteractionCentres() const { return this->interactionCentres; }
    [[nodiscard]] const std::map<std::string, Vector<3>> &getNamedPoints() const {
        return this->namedPoints;
    }
    [[nodiscard]] bool isConvex() const { return this->convex; }
};


/**
 * @brief XenoCollideTraits using AbstractXCGeometry as @a CollideGeometry.
 * @details It is an adapter class for XenoCollideTraits, which enables one to used some implementation of
 * AbstractXCGeometry, for example coming from XCBodyBuilder. The class is a registry of predefined named species
 * (see GenericShapeRegistry).
 */
class GenericXenoCollideTraits
    : public XenoCollideTraits<GenericXenoCollideTraits>, public GenericShapeRegistry<GenericXenoCollideShape>
{
private:
    static void imbueFakeInteractionCenter(GenericXenoCollideShape &shape);

    bool isMulticentre_{};

public:
    /**
     * @brief Creates the class with no initially registered species.
     */
    GenericXenoCollideTraits() = default;

    /**
     * @brief Creates the class with one species @a shape named `A`, which is set as a default species
     * (setDefaultSpecies()).
     */
    explicit GenericXenoCollideTraits(const GenericXenoCollideShape &shape);

    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    /**
     * @brief Returns interaction centers for given raw ShapeData @a data.
     * @details If all registered species have empty interaction center list, the function will also return empty
     * interaction center list. However, if at least one has non-empty interaction center list, the function will return
     * a one-element list with [0, 0, 0] center for all species with empty interaction center list.
     */
    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const override;

    /**
     * @brief Returs @a true is all registered species are convex, @a false otherwise.
     * @return
     */
    [[nodiscard]] bool isConvex() const override;

    [[nodiscard]] const AbstractXCGeometry &
    getCollideGeometry(const std::byte *data, std::size_t i = 0) const /* override */ {
        const auto &shape = this->speciesFor(data);
        const auto &geometries = shape.getGeometries();
        return *geometries[i];
    }

    ShapeData addSpecies(const std::string &speciesName, const GenericXenoCollideShape &species) final;

    /**
     * @brief Returns @a true if at least one of registered species has a non-empty interaction center list.
     */
    [[nodiscard]] bool isMulticentre() const { return this->isMulticentre_; }
};


#endif //RAMPACK_GENERICXENOCOLLIDETRAITS_H
