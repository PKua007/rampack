//
// Created by pkua on 23.05.22.
//

#ifndef RAMPACK_SHAPEGEOMETRY_H
#define RAMPACK_SHAPEGEOMETRY_H

#include "geometry/Vector.h"
#include "Shape.h"


/**
 * @brief An interface describing geometric properties of the shape.
 */
class ShapeGeometry {
public:
    /**
     * @brief Returns the volume of the shape.
     */
    [[nodiscard]] virtual double getVolume() const = 0;

    /**
     * @brief Returns the primary (long) molecular axis for a given @a shape.
     */
    [[nodiscard]] virtual Vector<3> getPrimaryAxis([[maybe_unused]] const Shape &shape) const {
        throw std::runtime_error("ShapeGeometry::getPrimaryAxis : unsupported");
    }

    /**
     * @brief Returns the secondary (polarization) molecular axis for a given @a shape.
     */
    [[nodiscard]] virtual Vector<3> getSecondaryAxis([[maybe_unused]] const Shape &shape) const {
        throw std::runtime_error("ShapeGeometry::getSecondaryAxis : unsupported");
    }

    /**
     * @brief Returns the geometric origin a given @a shape which is usually the center of its bounding box.
     * @details Geometric origin may be different from mass center (which is always {0, 0, 0}). It is used for example
     * for flip moves.
     */
    [[nodiscard]] virtual Vector<3> getGeometricOrigin([[maybe_unused]] const Shape &shape) const {
        return {0, 0, 0};
    }

    /**
     * @brief Returns a special, named point lying somewhere on a @a shape.
     * @details Default points are mass' center ("c") and geometric origin ("o"). Deriving classes can supplement their
     * own special points. They should always preserve all points from the inheritance hierarchy.
     * @param pointName name of the point
     * @param shape shape for which the point should be calculated
     * @return a special, named point lying somewhere on a @a shape
     */
    [[nodiscard]] virtual Vector<3> getNamedPoint(const std::string &pointName, const Shape &shape) const {
        if (pointName == "cm")
            return shape.getPosition();
        else if (pointName == "o")
            return shape.getPosition() + this->getGeometricOrigin(shape);
        else
            throw std::runtime_error("ShapeGeometry::getNamedPoint : unknown point name '" + pointName + "'");
    }
};


#endif //RAMPACK_SHAPEGEOMETRY_H
