//
// Created by pkua on 06.11.22.
//

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
}

std::map<std::string, Vector<3>> ShapeGeometry::getNamedPoints() const {
    std::map<std::string, Vector<3>> namedPoints_ = this->namedPoints;
    namedPoints_["cm"] = {};
    namedPoints_["o"] = this->getGeometricOrigin({});
    return namedPoints_;
}

void ShapeGeometry::registerNamedPoints(const std::map<std::string, Vector<3>> &namedPoints) {
    for (const auto &[name, point] : namedPoints)
        this->registerNamedPoint(name, point);
}

void ShapeGeometry::moveNamedPoints(const Vector<3> &translation) {
    for (auto &[name, point] : this->namedPoints)
        point += translation;
}
