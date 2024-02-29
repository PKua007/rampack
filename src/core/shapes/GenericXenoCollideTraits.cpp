//
// Created by Piotr Kubala on 28/02/2024.
//

#include <utility>

#include "GenericXenoCollideTraits.h"


GenericXenoCollideShape::GenericXenoCollideShape(std::shared_ptr<AbstractXCGeometry> geometry, double volume,
                                                 OptionalAxis primaryAxis, OptionalAxis secondaryAxis,
                                                 const Vector<3> &geometricOrigin,
                                                 const std::map<std::string, Vector<3>> &customNamedPoints)
        : geometries{std::move(geometry)}, interactionCentres{}, primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis},
          geometricOrigin{geometricOrigin}, volume{volume}, customNamedPoints{customNamedPoints}
{
    Expects(this->geometries.front());
    Expects(volume > 0);
    if (!this->primaryAxis.has_value())
        Expects(!this->secondaryAxis.has_value());
    if (this->primaryAxis.has_value())
        this->primaryAxis = this->primaryAxis->normalized();
    if (this->secondaryAxis.has_value())
        this->secondaryAxis = this->secondaryAxis->normalized();
}

GenericXenoCollideShape::GenericXenoCollideShape(const std::vector<GeometryData> &geometries, double volume,
                                                 OptionalAxis primaryAxis, OptionalAxis secondaryAxis,
                                                 const Vector<3> &geometricOrigin,
                                                 const std::map<std::string, Vector<3>> &customNamedPoints)
        : primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis}, geometricOrigin{geometricOrigin},
          customNamedPoints{customNamedPoints}
{
    Expects(!geometries.empty());
    ExpectsMsg(geometries.size() != 1, "For a single interaction center, use the other constructor");

    this->geometries.reserve(geometries.size());
    this->interactionCentres.reserve(geometries.size());
    for (const auto &[geometry, center] : geometries) {
        Expects(geometry);
        this->geometries.push_back(geometry);
        this->interactionCentres.push_back(center);
    }

    Expects(volume > 0);
    if (!this->primaryAxis.has_value())
        Expects(!this->secondaryAxis.has_value());
    if (this->primaryAxis.has_value())
        this->primaryAxis = this->primaryAxis->normalized();
    if (this->secondaryAxis.has_value())
        this->secondaryAxis = this->secondaryAxis->normalized();
}

Vector<3> GenericXenoCollideShape::getPrimaryAxis() const {
    if (!this->primaryAxis.has_value())
        throw std::runtime_error("GenericXenoCollideShape::getPrimaryAxis: primary axis not defined");
    return this->primaryAxis.value();
}

Vector<3> GenericXenoCollideShape::getSecondaryAxis() const {
    if (!this->secondaryAxis.has_value())
        throw std::runtime_error("GenericXenoCollideShape::getSecondaryAxis: secondary axis not defined");
    return this->secondaryAxis.value();
}

GenericXenoCollideTraits::GenericXenoCollideTraits(std::shared_ptr<AbstractXCGeometry> geometry,
                                                   OptionalAxis primaryAxis, OptionalAxis secondaryAxis,
                                                   const Vector<3> &geometricOrigin, double volume,
                                                   const std::map<std::string, Vector<3>> &customNamedPoints)
{
    GenericXenoCollideShape shape(std::move(geometry), volume, primaryAxis, secondaryAxis, geometricOrigin,
                                  customNamedPoints);
    this->addShape("A", shape);
    this->setDefaultShapeData({{"type", "A"}});
}

GenericXenoCollideTraits::GenericXenoCollideTraits(const std::vector<GeometryData> &geometries,
                                                   OptionalAxis primaryAxis, OptionalAxis secondaryAxis,
                                                   const Vector<3> &geometricOrigin, double volume,
                                                   const std::map<std::string, Vector<3>> &customNamedPoints)
{
    GenericXenoCollideShape shape(geometries, volume, primaryAxis, secondaryAxis, geometricOrigin, customNamedPoints);
    this->addShape("A", shape);
    this->setDefaultShapeData({{"type", "A"}});
}

std::vector<Vector<3>> GenericXenoCollideTraits::getInteractionCentres(const std::byte *data) const {
    return this->shapeFor(data).getInteractionCentres();
}
