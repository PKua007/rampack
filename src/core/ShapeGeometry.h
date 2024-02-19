//
// Created by pkua on 23.05.22.
//

#ifndef RAMPACK_SHAPEGEOMETRY_H
#define RAMPACK_SHAPEGEOMETRY_H

#include <map>
#include <string>
#include <variant>
#include <functional>

#include "geometry/Vector.h"
#include "Shape.h"


/**
 * @brief An interface describing geometric properties of the shape.
 */
class ShapeGeometry {
public:
    /**
     * @brief Molecular axis enumeration.
     */
    enum class Axis {
        /** @brief Primary (long) molecular axis. */
        PRIMARY,
        /** @brief Secondary molecular axis. */
        SECONDARY,
        /** @brief Auxiliary (third) molecular axis. */
        AUXILIARY
    };

    class NamedPoint {
    private:
        struct StaticPoint {
            Vector<3> point;

            Vector<3> operator()(const ShapeData &) const { return this->point; }
        };

        std::string name;
        std::function<Vector<3>(const ShapeData &)> pointFunctor;

    public:
        NamedPoint() : name{}, pointFunctor{StaticPoint{}} { }

        NamedPoint(std::string name, const Vector<3> &staticPoint)
                : name{std::move(name)}, pointFunctor{StaticPoint{staticPoint}}
        { }

        NamedPoint(std::string name, std::function<Vector<3>(const ShapeData &)> dynamicPoint)
                : name{std::move(name)}, pointFunctor{std::move(dynamicPoint)}
        { }

        [[nodiscard]] bool isStatic() const { return this->pointFunctor.target_type() == typeid(StaticPoint); }
        [[nodiscard]] bool isDynamic() const { return !this->isStatic(); }
        [[nodiscard]] Vector<3> forShape(const Shape &shape) const;
        [[nodiscard]] Vector<3> forShapeData(const ShapeData &data) const { return this->pointFunctor(data); }
        [[nodiscard]] Vector<3> forStatic() const;
        [[nodiscard]] const std::string &getName() const { return this->name; }
    };

private:
    std::map<std::string, NamedPoint> namedPoints;

    void resetOriginPoint();

protected:
    /**
     * @brief Registers a new named point @a point with name @a pointName (see getNamedPoint()).
     * @details The order of points registered using this method or registerStaticNamedPoints() is remembered.
     */
    void registerStaticNamedPoint(const std::string &pointName, const Vector<3> &point);

    /**
     * @brief Registers all named points from @a namedPoints_.
     * @details The order of points registered using this method or registerStaticNamedPoint() is remembered.
     */
    void registerStaticNamedPoints(const std::vector<std::pair<std::string, Vector<3>>> &namedPoints_);

    void registerNamedPoint(NamedPoint namedPoint);

    void registerNamedPoints(const std::vector<NamedPoint> &namedPoints_);

    /**
     * @brief Translates all named points, except for "o" by a given @a translation.
     */
    void moveStaticNamedPoints(const Vector<3> &translation);

public:
    using StaticNamedPoints = std::vector<std::pair<std::string, Vector<3>>>;

    ShapeGeometry();
    ShapeGeometry(const ShapeGeometry &other);
    ShapeGeometry(ShapeGeometry &&other) noexcept;
    ShapeGeometry &operator=(const ShapeGeometry &other);
    ShapeGeometry &operator=(ShapeGeometry &&other) noexcept;
    // Surprisingly, the default destructor is okay (the rule of five does not apply here)

    /**
     * @brief Returns the volume of the shape.
     */
    [[nodiscard]] virtual double getVolume(const Shape &shape) const = 0;

    /**
     * @brief Returns the primary (long) molecular axis for a given @a shape.
     */
    [[nodiscard]] virtual Vector<3> getPrimaryAxis([[maybe_unused]] const Shape &shape) const {
        throw std::runtime_error("ShapeGeometry::getPrimaryAxis : unsupported");
    }

    /**
     * @brief Returns the secondary molecular axis for a given @a shape.
     */
    [[nodiscard]] virtual Vector<3> getSecondaryAxis([[maybe_unused]] const Shape &shape) const {
        throw std::runtime_error("ShapeGeometry::getSecondaryAxis : unsupported");
    }

    /**
     * @brief Returns the auxiliary (third) molecular axis for a given @a shape, orthogonal to the other two.
     */
    [[nodiscard]] Vector<3> getAuxiliaryAxis(const Shape &shape) const {
        return (this->getPrimaryAxis(shape) ^ this->getSecondaryAxis(shape)).normalized();
    }

    /**
     * @brief Returns shape axis of type @a axis for shape @a shape.
     */
     [[nodiscard]] Vector<3> getAxis(const Shape &shape, Axis axis) const;

    /**
     * @brief Returns the geometric origin a given @a shape (with respect to its centre) which is usually the center of
     * its bounding box.
     * @details Geometric origin may be different from mass center. It is used for example for flip moves.
     */
    [[nodiscard]] virtual Vector<3> getGeometricOrigin([[maybe_unused]] const Shape &shape) const {
        return {0, 0, 0};
    }

    /**
     * @brief Returns a special, named point lying somewhere on a shape with default position and orientation.
     * @details Geometric origin ("o") is a default point. Deriving classes can supplement their own special points
     * using registerStaticNamedPoint() and registerStaticNamedPoints() methods.
     * @param pointName name of the point
     * @return a special, named point
     */
    [[nodiscard]] const NamedPoint &getNamedPoint(const std::string &pointName) const;

    /**
     * @brief Returns a named point with name @a pointName (see getNamedPoint()) on a specifically positioned and
     * oriented @a shape.
     */
    [[nodiscard]] Vector<3> getNamedPointForShape(const std::string &pointName, const Shape &shape) const {
        return this->getNamedPoint(pointName).forShape(shape);
    }

    [[nodiscard]] Vector<3> getNamedPointForData(const std::string &pointName, const ShapeData &shapeData) const {
        return this->getNamedPoint(pointName).forShapeData(shapeData);
    }

    [[nodiscard]] Vector<3> getNamedPointForStatic(const std::string &pointName) const {
        return this->getNamedPoint(pointName).forStatic();
    }

    /**
     * @brief Returns a list of all named points (see getNamedPoint()).
     */
    [[nodiscard]] std::vector<NamedPoint> getNamedPoints() const;

    /**
     * @brief Returns @a true if named point @a namedPoint exists.
     */
    [[nodiscard]] bool hasNamedPoint(const std::string &pointName) const;

    /**
     * @brief Returns @a true if the primary axis exists.
     */
    [[nodiscard]] bool hasPrimaryAxis() const;

    /**
     * @brief Returns @a true if the secondary axis exists.
     */
    [[nodiscard]] bool hasSecondaryAxis() const;

    /**
     * @brief Returns @a true if the auxiliary axis exists.
     */
    [[nodiscard]] bool hasAuxiliaryAxis() const;

    /**
     * @brief Finds a flip axis for a given @a shape - an axis which flips the sign of the primary axis when one
     * performs a 180-degree rotation around it.
     * @details If the secondary axis exists, it is returned as the flip axis. Otherwise, arbitrary axis orthogonal to
     * the primary axis is return.
     * @throws PreconditionException if the primary axis does not exists.
     */
    [[nodiscard]] Vector<3> findFlipAxis(const Shape &shape) const;

};


#endif //RAMPACK_SHAPEGEOMETRY_H
