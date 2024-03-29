//
// Created by pkua on 06.11.22.
//

#include <vector>
#include <algorithm>

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
        ExpectsThrow("ShapeGeometry::getNamedPoint : unknown point name '" + pointName + "'");
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

bool ShapeGeometry::hasPrimaryAxis() const {
    try {
        static_cast<void>(this->getPrimaryAxis({}));
        return true;
    } catch (std::runtime_error&) {
        return false;
    }
}

bool ShapeGeometry::hasSecondaryAxis() const {
    try {
        static_cast<void>(this->getSecondaryAxis({}));
        return true;
    } catch (std::runtime_error&) {
        return false;
    }
}

bool ShapeGeometry::hasAuxiliaryAxis() const {
    try {
        static_cast<void>(this->getAuxiliaryAxis({}));
        return true;
    } catch (std::runtime_error&) {
        return false;
    }
}

Vector<3> ShapeGeometry::findFlipAxis(const Shape &shape) const {
    Expects(this->hasPrimaryAxis());

    if (this->hasSecondaryAxis())
        return this->getSecondaryAxis(shape);

    Vector<3> primaryAxis = this->getPrimaryAxis({});

    auto minIt = std::min_element(primaryAxis.begin(), primaryAxis.end(), [](double c1, double c2) {
        return std::abs(c1) < std::abs(c2);
    });
    std::size_t minIdx = minIt - primaryAxis.begin();
    Vector<3> referenceDirection;
    referenceDirection[minIdx] = 1;

    Vector<3> flipAxis = primaryAxis ^ referenceDirection;
    return (shape.getOrientation() * flipAxis).normalized();
}

Vector<3> ShapeGeometry::getAxis(const Shape &shape, ShapeGeometry::Axis axis) const {
    switch(axis) {
        case Axis::PRIMARY:
            return this->getPrimaryAxis(shape);
        case Axis::SECONDARY:
            return this->getSecondaryAxis(shape);
        case Axis::AUXILIARY:
            return this->getAuxiliaryAxis(shape);
        default:
            AssertThrow("axis");
    }
}
