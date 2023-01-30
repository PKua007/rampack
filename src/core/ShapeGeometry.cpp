//
// Created by pkua on 06.11.22.
//

#include <vector>
#include "ShapeGeometry.h"
#include "utils/Exceptions.h"


Vector<3> ShapeGeometry::getNamedPointForShape(const std::string &pointName, const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->getNamedPoint(pointName);
}

Vector<3> ShapeGeometry::getNamedPoint(const std::string &pointName) const {
    if (pointName == "o")
        return this->getGeometricOrigin({});

    auto point = this->namedPoints.find(pointName);
    if (point == this->namedPoints.end())
        throw PreconditionException("ShapeGeometry::getNamedPoint : unknown point name '" + pointName + "'");
    return point->second;
}

void ShapeGeometry::registerNamedPoint(const std::string &pointName, const Vector<3> &point) {
    Expects(this->namedPoints.find(pointName) == this->namedPoints.end());
    this->namedPoints[pointName] = point;
    this->namedPointsOrdered.emplace_back(pointName, point);
}

ShapeGeometry::NamedPoints ShapeGeometry::getNamedPoints() const {
    NamedPoints namedPoints_;
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

bool ShapeGeometry::hasNamedPoint(const std::string &pointName) const {
    if (pointName == "o")
        return true;
    else
        return this->namedPoints.find(pointName) != this->namedPoints.end();
}
