//
// Created by pkua on 06.11.22.
//

#include <vector>
#include <algorithm>

#include "ShapeGeometry.h"
#include "utils/Exceptions.h"


Vector<3> NamedPoint::evaluateFor([[maybe_unused]] NamedPoint::StaticTag staticTag) const {
    Expects(this->type == Type::STATIC);
    return this->pointFunctor.target<StaticPointAdapter>()->point;
}

bool NamedPoint::isValidFor(const ShapeData &data) const {
    if (this->getType() != Type::TRANSIENT)
        return true;

    try {
        static_cast<void>(this->evaluateFor(data));
        return true;
    } catch (const NoSuchNamedPointForShapeException &) {
        return false;
    }
}

Vector<3> NamedPoint::evaluateFor(const Shape &shape) const {
    return shape.getOrientation() * this->pointFunctor(shape.getData()) + shape.getPosition();
}


ShapeGeometry::ShapeGeometry() {
    this->resetOriginPoint();
}

ShapeGeometry::ShapeGeometry(const ShapeGeometry &other)
        : nonTransientNamedPoints{other.nonTransientNamedPoints}, transientNamedPoint{other.transientNamedPoint}
{
    this->resetOriginPoint();
}

ShapeGeometry::ShapeGeometry(ShapeGeometry &&other) noexcept
        : nonTransientNamedPoints{std::move(other.nonTransientNamedPoints)},
          transientNamedPoint{std::move(other.transientNamedPoint)}
{
    this->resetOriginPoint();
}

ShapeGeometry &ShapeGeometry::operator=(const ShapeGeometry &other) {
    this->nonTransientNamedPoints = other.nonTransientNamedPoints;
    this->transientNamedPoint = other.transientNamedPoint;
    this->resetOriginPoint();
    return *this;
}

ShapeGeometry &ShapeGeometry::operator=(ShapeGeometry &&other) noexcept {
    this->nonTransientNamedPoints = std::move(other.nonTransientNamedPoints);
    this->transientNamedPoint = std::move(other.transientNamedPoint);
    this->resetOriginPoint();
    return *this;
}

void ShapeGeometry::resetOriginPoint() {
    this->nonTransientNamedPoints["o"] = NamedPoint("o", [this](const ShapeData &data) -> Vector<3> {
        Shape trialShape{};
        trialShape.setData(data.unmanagedCopy());       // Prevent copying data
        return this->getGeometricOrigin(trialShape);
    });
}

void ShapeGeometry::registerStaticNamedPoint(const std::string &pointName, const Vector<3> &point) {
    Expects(!this->hasNonTransientNamedPoint(pointName));
    this->nonTransientNamedPoints[pointName] = NamedPoint(pointName, point);
}

void ShapeGeometry::registerDynamicNamedPoint(const std::string &pointName, NamedPoint::DynamicEvaluator point) {
    Expects(!this->hasNonTransientNamedPoint(pointName));
    this->nonTransientNamedPoints[pointName] = NamedPoint(pointName, std::move(point));
}

void ShapeGeometry::registerTransientNamedPoint(NamedPoint::TransientEvaluator evaluator,
                                                NamedPoint::TransientLister lister)
{
    Expects(!this->transientNamedPoint.has_value());
    this->transientNamedPoint = TransientPointData{std::move(evaluator), std::move(lister)};
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
    switch (axis) {
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

NamedPoint ShapeGeometry::getNamedPoint(const std::string &pointName) const {
    auto point = this->nonTransientNamedPoints.find(pointName);
    if (point != this->nonTransientNamedPoints.end())
        return point->second;

    if (!this->transientNamedPoint.has_value())
        ExpectsThrow("ShapeGeometry::getNamedPoint : unknown non-transient point name '" + pointName + "'");

    return NamedPoint(pointName, this->transientNamedPoint->evaluator);
}

std::vector<NamedPoint> ShapeGeometry::getNamedPoints(const ShapeData &data) const {
    std::map<std::string, NamedPoint> allPointsMap = this->nonTransientNamedPoints;

    if (this->transientNamedPoint.has_value()) {
        auto transientPointNames = this->transientNamedPoint->lister(data);
        for (const auto &transientPointName : transientPointNames)
            allPointsMap[transientPointName] = NamedPoint(transientPointName, this->transientNamedPoint->evaluator);
    }

    std::vector<NamedPoint> namedPointsVec;
    namedPointsVec.reserve(allPointsMap.size());
    for (const auto &[name, point] : allPointsMap)
        namedPointsVec.push_back(point);

    return namedPointsVec;
}

std::map<std::string, Vector<3>> ShapeGeometry::evaluateNamedPoints(const ShapeData &shapeData) const {
    std::map<std::string, Vector<3>> namedPoints;

    for (const auto &[name, point] : this->nonTransientNamedPoints)
        namedPoints[name] = point.evaluateFor(shapeData);

    if (this->transientNamedPoint.has_value()) {
        auto transientPointNames = this->transientNamedPoint->lister(shapeData);
        for (const auto &transientPointName : transientPointNames)
            namedPoints[transientPointName] = this->transientNamedPoint->evaluator(transientPointName, shapeData);
    }

    return namedPoints;
}

std::map<std::string, Vector<3>> ShapeGeometry::evaluateNamedPoints(const Shape &shape) const {
    auto points = this->evaluateNamedPoints(shape.getData());
    for (auto &[name, point] : points)
        point = shape.getOrientation() * point + shape.getPosition();
    return points;
}

bool ShapeGeometry::hasNonTransientNamedPoint(const std::string &pointName) const {
    if (pointName == "o")
        return true;
    else
        return this->nonTransientNamedPoints.find(pointName) != this->nonTransientNamedPoints.end();
}

bool ShapeGeometry::hasNamedPoint(const std::string &pointName, const ShapeData &shapeData) const {
    if (this->hasNonTransientNamedPoint(pointName))
        return true;

    if (!this->transientNamedPoint.has_value())
        return false;

    auto transientPoints = this->transientNamedPoint->lister(shapeData);
    return transientPoints.find(pointName) != transientPoints.end();
}
