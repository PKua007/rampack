//
// Created by pkua on 06.11.22.
//

#include <vector>
#include "ShapeGeometry.h"
#include "utils/Assertions.h"


Vector<3> ShapeGeometry::getNamedPoint(const std::string &pointName, const Shape &shape) const {
    if (pointName == "cm")
        return shape.getPosition();
    else if (pointName == "o")
        return shape.getPosition() + this->getGeometricOrigin(shape);

    auto point = this->namedPoints.find(pointName);
    if (point == this->namedPoints.end())
        throw std::runtime_error("ShapeGeometry::getNamedPoint : unknown point name '" + pointName + "'");
    return shape.getPosition() + shape.getOrientation()*point->second;
}

void ShapeGeometry::registerNamedPoint(const std::string &pointName, const Vector<3> &point) {
    Expects(this->namedPoints.find(pointName) == this->namedPoints.end());
    this->namedPoints[pointName] = point;
    this->namedPointsOrdered.emplace_back(pointName, point);
}

ShapeGeometry::NamedPoints ShapeGeometry::getNamedPoints() const {
    NamedPoints namedPoints_;
    namedPoints_.emplace_back("cm", Vector<3>{});
    namedPoints_.emplace_back("o", this->getGeometricOrigin({}));
    namedPoints_.insert(namedPoints_.end(), this->namedPointsOrdered.begin(), this->namedPointsOrdered.end());
    return namedPoints_;
}

void ShapeGeometry::registerNamedPoints(const std::vector<std::pair<std::string, Vector<3>>> &namedPoints_) {
    for (const auto &[name, point] : namedPoints_)
        this->registerNamedPoint(name, point);
}

void ShapeGeometry::moveNamedPoints(const Vector<3> &translation) {
    for (auto &[name, point] : this->namedPoints)
        point += translation;
    for (auto &[name, point] : this->namedPointsOrdered)
        point += translation;
}
