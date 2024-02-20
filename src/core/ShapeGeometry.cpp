//
// Created by pkua on 06.11.22.
//

#include <vector>
#include <algorithm>

#include "ShapeGeometry.h"
#include "utils/Exceptions.h"


ShapeGeometry::ShapeGeometry() {
    this->resetOriginPoint();
}

ShapeGeometry::ShapeGeometry(const ShapeGeometry &other) : namedPoints{other.namedPoints} {
    this->resetOriginPoint();
}

ShapeGeometry::ShapeGeometry(ShapeGeometry &&other) noexcept : namedPoints{std::move(other.namedPoints)} {
    this->resetOriginPoint();
}

ShapeGeometry &ShapeGeometry::operator=(const ShapeGeometry &other) {
    this->namedPoints = other.namedPoints;
    this->resetOriginPoint();
    return *this;
}

ShapeGeometry &ShapeGeometry::operator=(ShapeGeometry &&other) noexcept {
    this->namedPoints = std::move(other.namedPoints);
    this->resetOriginPoint();
    return *this;
}

const ShapeGeometry::NamedPoint &ShapeGeometry::getNamedPoint(const std::string &pointName) const {
    auto point = this->namedPoints.find(pointName);
    if (point == this->namedPoints.end())
        ExpectsThrow("ShapeGeometry::getNamedPoint : unknown point name '" + pointName + "'");
    return point->second;
}

void ShapeGeometry::registerStaticNamedPoint(const std::string &pointName, const Vector<3> &point) {
    Expects(!this->hasNamedPoint(pointName));
    this->namedPoints[pointName] = NamedPoint(pointName, point);
}

std::vector<ShapeGeometry::NamedPoint> ShapeGeometry::getNamedPoints() const {
    std::vector<NamedPoint> namedPointsVec;
    namedPointsVec.reserve(this->namedPoints.size());
    for (const auto &[name, point] : this->namedPoints)
        namedPointsVec.push_back(point);
    return namedPointsVec;
}

void ShapeGeometry::registerDynamicNamedPoint(const std::string &pointName,
                                              const std::function<Vector<3>(const ShapeData &)> &point)
{
    Expects(!this->hasNamedPoint(pointName));
    this->namedPoints[pointName] = NamedPoint(pointName, point);
}

void ShapeGeometry::registerNamedPoint(ShapeGeometry::NamedPoint namedPoint) {
    Expects(!this->hasNamedPoint(namedPoint.getName()));
    this->namedPoints[namedPoint.getName()] = std::move(namedPoint);
}

void ShapeGeometry::registerNamedPoints(const std::vector<NamedPoint> &namedPoints_) {
    for (const auto &namedPoint : namedPoints_)
        this->registerNamedPoint(namedPoint);
}

void ShapeGeometry::moveStaticNamedPoints(const Vector<3> &translation) {
    for (auto &[name, point] : this->namedPoints) {
        if (!point.isStatic())
            continue;

        point = NamedPoint(point.getName(), point.forShape({}) + translation);
    }
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

void ShapeGeometry::resetOriginPoint() {
    this->namedPoints["o"] = NamedPoint("o", [this](const ShapeData &data) -> Vector<3> {
        Shape trialShape{};
        trialShape.setData(data.unmanagedCopy());       // Prevent copying data
        return this->getGeometricOrigin(trialShape);
    });
}

Vector<3> ShapeGeometry::NamedPoint::forShape(const Shape &shape) const {
    return shape.getPosition() + shape.getOrientation() * this->forShapeData(shape.getData());
}

Vector<3> ShapeGeometry::NamedPoint::forStatic() const {
    Expects(this->isStatic());
    return this->pointFunctor.target<StaticPoint>()->point;
}
