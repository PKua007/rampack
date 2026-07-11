//
// Created by Codex on 12/07/2026.
//

#include "GravityField.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>

#include "utils/Exceptions.h"


namespace {
    bool is_in_unit_interval(const Vector<3> &vector) {
        return std::all_of(vector.begin(), vector.end(), [](const double coord) {
            return coord >= 0 && coord <= 1;
        });
    }

    std::string make_missing_point_message(const std::string &pointName) {
        std::ostringstream message;
        message << "GravityField point '" << pointName << "' is not present in ShapeGeometry";
        return message.str();
    }
}


GravityField::GravityField(const double g, const Vector<3> &directionHkl, std::optional<std::string> pointName,
                           const Vector<3> &boxAnchorRelative)
        : g{g}, directionHkl{directionHkl}, pointName{std::move(pointName)},
          boxAnchorRelative{boxAnchorRelative}
{
    Expects(this->g > 0);
    Expects(this->directionHkl.norm2() > EPSILON * EPSILON);
    Expects(is_in_unit_interval(this->boxAnchorRelative));
}

void GravityField::setupForShapeGeometry(const ShapeGeometry &geometry) {
    if (this->pointName.has_value()) {
        const auto &pointName_ = *this->pointName;
        ExpectsMsg(geometry.hasNamedPoint(pointName_), make_missing_point_message(pointName_));
        this->localPoint = geometry.getNamedPoint(pointName_);
    } else if (geometry.hasNamedPoint("cm")) {
        this->localPoint = geometry.getNamedPoint("cm");
    } else {
        this->localPoint = geometry.getGeometricOrigin({});
    }
}

void GravityField::setupForBox(const TriclinicBox &box) {
    const auto boxNormalTransform = box.getDimensions().inverse().transpose();
    this->directionAbs = (boxNormalTransform * this->directionHkl).normalized();
    this->boxAnchorAbs = box.relativeToAbsolute(this->boxAnchorRelative);
}

double GravityField::calculateEnergy(const Vector<3> &shapePos, const Matrix<3, 3> &shapeRot) const {
    const Vector<3> pointAbs = shapePos + shapeRot * this->localPoint;
    return -this->g * (this->directionAbs * (pointAbs - this->boxAnchorAbs));
}

std::array<bool, 3> GravityField::getContinuityAlongBoxAxes() const {
    return {std::abs(this->directionHkl[0]) <= EPSILON,
            std::abs(this->directionHkl[1]) <= EPSILON,
            std::abs(this->directionHkl[2]) <= EPSILON};
}
