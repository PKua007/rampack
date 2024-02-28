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
    std::shared_ptr<AbstractXCGeometry> geometry;
    std::optional<Vector<3>> primaryAxis;
    std::optional<Vector<3>> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};
    std::map<std::string, Vector<3>> customNamedPoints;

public:
    GenericXenoCollideShape(std::shared_ptr<AbstractXCGeometry> geometry, double volume,
                            OptionalAxis primaryAxis = std::nullopt, OptionalAxis secondaryAxis = std::nullopt,
                            const Vector<3> &geometricOrigin = {0, 0, 0},
                            const std::map<std::string, Vector<3>> &customNamedPoints = {});

    [[nodiscard]] Vector<3> getPrimaryAxis() const;
    [[nodiscard]] Vector<3> getSecondaryAxis() const;
    [[nodiscard]] Vector<3> getGeometricOrigin() const {
        return this->geometricOrigin;
    }
    [[nodiscard]] double getVolume([[maybe_unused]] const Shape &shape) const { return this->volume; }
    [[nodiscard]] const AbstractXCGeometry &getGeometry() const { return *this->geometry; }
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
    GenericXenoCollideTraits() = default;

    GenericXenoCollideTraits(std::shared_ptr<AbstractXCGeometry> geometry, OptionalAxis primaryAxis,
                             OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin, double volume,
                             const std::map<std::string, Vector<3>> &customNamedPoints);

    [[nodiscard]] const AbstractXCGeometry &
    getCollideGeometry(const std::byte *data, [[maybe_unused]] std::size_t i = 0) const /* override */ {
        return this->shapeFor(data).getGeometry();
    }

    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
};


#endif //RAMPACK_GENERICXENOCOLLIDETRAITS_H
