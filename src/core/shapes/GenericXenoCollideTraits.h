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


class GenericXenoCollideShape {
private:
    std::vector<std::shared_ptr<const AbstractXCGeometry>> geometries;
    std::vector<Vector<3>> interactionCentres;
    std::optional<Vector<3>> primaryAxis;
    std::optional<Vector<3>> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};
    std::map<std::string, Vector<3>> customNamedPoints;

public:
    struct GeometryData {
        std::shared_ptr<AbstractXCGeometry> geometry;
        Vector<3> center;
    };

    GenericXenoCollideShape(std::shared_ptr<AbstractXCGeometry> geometry, double volume,
                            OptionalAxis primaryAxis = std::nullopt, OptionalAxis secondaryAxis = std::nullopt,
                            const Vector<3> &geometricOrigin = {0, 0, 0},
                            const std::map<std::string, Vector<3>> &customNamedPoints = {});

    GenericXenoCollideShape(const std::vector<GeometryData> &geometries, double volume,
                            OptionalAxis primaryAxis = std::nullopt, OptionalAxis secondaryAxis = std::nullopt,
                            const Vector<3> &geometricOrigin = {0, 0, 0},
                            const std::map<std::string, Vector<3>> &customNamedPoints = {});

    [[nodiscard]] Vector<3> getPrimaryAxis() const;
    [[nodiscard]] Vector<3> getSecondaryAxis() const;
    [[nodiscard]] Vector<3> getGeometricOrigin() const {
        return this->geometricOrigin;
    }
    [[nodiscard]] double getVolume([[maybe_unused]] const Shape &shape) const { return this->volume; }
    [[nodiscard]] const std::vector<std::shared_ptr<const AbstractXCGeometry>> &getGeometries() const {
        return this->geometries;
    }
    [[nodiscard]] const std::vector<Vector<3>> &getInteractionCentres() const { return this->interactionCentres; }
    [[nodiscard]] const std::map<std::string, Vector<3>> &getCustomNamedPoints() const {
        return this->customNamedPoints;
    }
};

/**
 * @brief XenoCollideTraits using AbstractXCGeometry as @a CollideGeometry.
 * @details It is an adapter class, which enables one to used some implementation of AbstractXCGeometry, for example
 * coming from XCBodyBuilder.
 */
class GenericXenoCollideTraits
    : public XenoCollideTraits<GenericXenoCollideTraits>, public GenericShapeRegistry<GenericXenoCollideShape>
{
public:
    using GeometryData = GenericXenoCollideShape::GeometryData;

    GenericXenoCollideTraits() = default;

    GenericXenoCollideTraits(std::shared_ptr<AbstractXCGeometry> geometry, OptionalAxis primaryAxis,
                             OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin, double volume,
                             const std::map<std::string, Vector<3>> &customNamedPoints = {});

    GenericXenoCollideTraits(const std::vector<GeometryData> &geometries, OptionalAxis primaryAxis,
                             OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin, double volume,
                             const std::map<std::string, Vector<3>> &customNamedPoints = {});

    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    [[nodiscard]] const AbstractXCGeometry &
    getCollideGeometry(const std::byte *data, std::size_t i = 0) const /* override */ {
        const auto &shape = this->speciesFor(data);
        const auto &geometries = shape.getGeometries();
        return *geometries[i];
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const override;
};


#endif //RAMPACK_GENERICXENOCOLLIDETRAITS_H
