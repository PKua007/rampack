//
// Created by Piotr Kubala on 28/02/2024.
//

#include "GenericXenoCollideTraits.h"


GenericXenoCollideShape::GenericXenoCollideShape(std::shared_ptr<AbstractXCGeometry> geometry, double volume,
                                                 OptionalAxis primaryAxis, OptionalAxis secondaryAxis,
                                                 const Vector<3> &geometricOrigin,
                                                 const std::map<std::string, Vector<3>> &customNamedPoints)
        : geometry{std::move(geometry)}, primaryAxis{primaryAxis}, secondaryAxis{secondaryAxis},
          geometricOrigin{geometricOrigin}, customNamedPoints{customNamedPoints}
{
    Expects(this->geometry);
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
