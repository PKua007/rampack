//
// Created by pkua on 23.05.22.
//

#ifndef RAMPACK_SHAPEGEOMETRY_H
#define RAMPACK_SHAPEGEOMETRY_H

#include <map>
#include <string>
#include <functional>
#include <optional>
#include <set>

#include "geometry/Vector.h"
#include "Shape.h"


/**
 * @brief Exception thrown if the NamedPoint is valid only for some ShapeData and the unsupported one were passed.
 */
struct NoSuchNamedPointForShapeException : public RuntimeException {
    using RuntimeException::RuntimeException;
};


/**
 * @brief Class representing particular static, dynamic, or transient named point of ShapeGeometry, with a fixed point
 * name. See ShapeGeometry for the details on named points.
 */
class NamedPoint {
public:
    /**
     * @brief Type of the named point.
     */
    enum class Type {
        /** @brief Static point with constant coordinates. */
        STATIC,

        /** @brief Dynamic point with coordinates depending on ShapeData. */
        DYNAMIC,

        /** @brief Transient point, whose both the coordinates and the very existence depend on ShapeData. If the point
         * is evaluated on unsupported ShapeData, NoSuchNamedPointForShapeException is thrown. */
        TRANSIENT
    };

    /**
     * @brief Alias for an evaluator of a dynamic point - a function returning point coordinates for the given
     * ShapeData.
     */
    using DynamicEvaluator = std::function<Vector<3>(const ShapeData &)>;

    /**
     * @brief Alias for an evaluator of a transient point - a function returning point coordinates for the given point
     * name and ShapeData. If the point with this name does not exists for the given ShapeData,
     * NoSuchNamedPointForShapeException should be thrown.
     */
    using TransientEvaluator = std::function<Vector<3>(const std::string &, const ShapeData &)>;

    /**
     * @brief Alias for the lister of all transient points' names - a function returning the set of all names of the
     * supported transient points for given ShapeData.
     */
    using TransientLister = std::function<std::set<std::string>(const ShapeData &)>;

private:
    struct StaticPointAdapter {
        Vector<3> point;

        explicit StaticPointAdapter(const Vector<3> &point = {}) : point{point} { }
        Vector<3> operator()(const ShapeData &) const { return this->point; }
    };

    struct TransientPointAdapter {
        std::string name;
        std::function<Vector<3>(const std::string &, const ShapeData &)> evaluator;

        TransientPointAdapter(std::string name, TransientEvaluator evaluator)
                : name{std::move(name)}, evaluator{std::move(evaluator)}
        { }

        Vector<3> operator()(const ShapeData &data) const { return this->evaluator(this->name, data); }
    };

    Type type = Type::STATIC;
    std::string name;
    DynamicEvaluator pointFunctor;

public:
    /**
     * @brief Singleton type of STATIC_TAG used for the overloads of methods for static points.
     */
    struct StaticTag {
    private:
        StaticTag() = default;

        friend NamedPoint;
    };

    /**
     * @brief Tag used for the overloads of methods for static points.
     */
    constexpr static StaticTag STATIC_TAG{};

    /**
     * @brief Creates a {0, 0, 0} static point with an empty name.
     */
    NamedPoint() : type{Type::STATIC}, name{}, pointFunctor{StaticPointAdapter{}} { }

    /**
     * @brief Creates a static point named @a name placed in @a staticPoint coordinates.
     */
    NamedPoint(std::string name, const Vector<3> &staticPoint)
            : type{Type::STATIC}, name{std::move(name)}, pointFunctor{StaticPointAdapter(staticPoint)}
    { }

    /**
     * @brief Creates a dynamic point with the name @a name and DynamicEvaluator @a dynamicEvaluator
     */
    NamedPoint(std::string name, DynamicEvaluator dynamicEvaluator)
            : type{Type::DYNAMIC}, name{std::move(name)}, pointFunctor{std::move(dynamicEvaluator)}
    { }

    /**
     * @brief Creates a transient point with the name @a name and TransientEvaluator @a transientEvaluator. The validity
     * of the name may be checked only upon evaluation - then, in case of a failure, NoSuchNamedPointForShapeException
     * will be thrown.
     */
    NamedPoint(const std::string &name, TransientEvaluator transientEvaluator)
            : type{Type::TRANSIENT}, name{name}, pointFunctor{TransientPointAdapter(name, std::move(transientEvaluator))}
    { }

    /**
     * @brief Returns the name of the point.
     */
    [[nodiscard]] const std::string &getName() const { return this->name; }

    /**
     * @brief Returns the type of the point.
     */
    [[nodiscard]] Type getType() const { return this->type; }

    /**
     * @brief Returns the position of the named point in shape coordinates (for the default positioned and oriented
     * shape), assuming the point is static.
     * @param staticTag NamedPoint::STATIC_TAG should be passed to choose this overload
     * @throws PreconditionException if the point is not static
     */
    [[nodiscard]] Vector<3> evaluateFor(StaticTag staticTag) const;

    /**
     * @brief Returns the position of the named point in shape coordinates (for the default positioned and oriented
     * shape).
     * @throws NoSuchNamedPointForShapeException if the point is transient and does not support the specific ShapeData
     * @throws AssertionException if the point is dynamic and DynamicEvaluator incorrectly throws
     * NoSuchNamedPointForShapeException
     */
    [[nodiscard]] Vector<3> evaluateFor(const ShapeData &data) const;

    /**
     * @brief Returns the absolute position of the named point for a given shape, taking into account its position and
     * orientation.
     * @throws NoSuchNamedPointForShapeException if the point is transient and does not support the specific ShapeData
     * @throws AssertionException if the point is dynamic and DynamicEvaluator incorrectly throws
     * NoSuchNamedPointForShapeException
     */
    [[nodiscard]] Vector<3> evaluateFor(const Shape &shape) const;

    /**
     * @brief Returns @a true if the point is valid for the given @a data ShapeData, @a false otherwise. For static and
     * dynamic points it is always @a true.
     */
    [[nodiscard]] bool isValidFor(const ShapeData &data) const;
};


/**
 * @brief An interface describing geometric properties of the shape.
 * @details Geometric properties include shape origin, shape axes, shape volume, and named points. While shape volume is
 * self-explanatory, other properties are detailed below.
 *
 * **SHAPE ORIGIN** <br />
 * Shape's central point, around which it should by flipped (see for example FlipSampler or FlipRandomizingTransformer).
 * Usually it is {0, 0, 0} in shapes coordinates (which is the default), but it may be redefined.
 *
 * **SHAPE AXES** <br />
 * Cardinal axes of the shape, reflecting its symmetry. There are three possibilities:
 * - **Symmetric shape**: no axes are defined - all three getPrimaryAxis(), getSecondaryAxis(), and getAuxiliaryAxis()
 *   methods throw.
 * - **Axially symmetric shape**: only primary axis is defined: getSecondaryAxis() and getAuxiliaryAxis() methods throw.
 * - **Non-axially symmetric shape**: all three getPrimaryAxis(), getSecondaryAxis(), and getAuxiliaryAxis() methods are
 *   defined and return an orthonormal triad of shape axes.
 *
 * **NAMED POINT** <br />
 * The shape may define a set of special points within its coordinate frame, called *named points*. There are three
 * types of named points:
 * - **Static point**: point's coordinates are constant (independent of ShapeData).
 * - **Dynamic point**: point's coordinates depend on ShapeData. They are provided by NamedPoint::DynamicEvaluator.
 * - **Transient point**: different ShapeData may have independent sets of named points. Coordinates for a particular
 *   transient named point are provided by NamedPoint::TransientEvaluator based on BOTH point's name and ShapeData. If
 *   an unsupported combination of name and ShapeData is passed, NoSuchNamedPointForShapeException is thrown. A list of
 *   the names of all available transient named points for the given ShapeData is provided by
 *   NamedPoint::TransientLister.
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

private:
    struct TransientPointData {
        NamedPoint::TransientEvaluator evaluator;
        NamedPoint::TransientLister lister;
    };

    std::map<std::string, NamedPoint> nonTransientNamedPoints;
    std::optional<TransientPointData> transientNamedPoint;

    void resetOriginPoint();

protected:
    /**
     * @brief Registers a new static named point.
     * @param pointName name of the point
     * @param staticCoordinates static coordinates of the point
     * @throws PreconditionException if the point with the name @a pointName already exists
     */
    void registerStaticNamedPoint(const std::string &pointName, const Vector<3> &staticCoordinates);

    /**
     * @brief Registers a new dynamic named point.
     * @param pointName name of the point
     * @param evaluator functor evaluating point coordinates for given ShapeData
     * @throws PreconditionException if the point with the name @a pointName already exists
     */
    void registerDynamicNamedPoint(const std::string &pointName, NamedPoint::DynamicEvaluator evaluator);

    /**
     * @brief Registers a new transient named point.
     * @param evaluator functor evaluating point coordinates for given point's name and ShapeData
     * @param lister functor returning the set of all available transient named point's names for given ShapeData
     * @throws PreconditionException if the transient point is already defined
     */
    void registerTransientNamedPoint(NamedPoint::TransientEvaluator evaluator, NamedPoint::TransientLister lister);

public:
    ShapeGeometry();
    ShapeGeometry(const ShapeGeometry &other) = delete;
    ShapeGeometry(ShapeGeometry &&other) noexcept = delete;
    ShapeGeometry &operator=(const ShapeGeometry &other) = delete;
    ShapeGeometry &operator=(ShapeGeometry &&other) noexcept = delete;

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
     * @details Geometric origin may be different from the mass center. It is used for example for flip moves.
     */
    [[nodiscard]] virtual Vector<3> getGeometricOrigin([[maybe_unused]] const Shape &shape) const {
        return {0, 0, 0};
    }

    /**
     * @brief Returns a special, named point lying somewhere on a shape.
     * @details Geometric origin ("o") is a default point and is always present. The deriving classes can supplement
     * their own special points using registerStaticNamedPoint(), registerDynamicNamedPoint(), and
     * registerTransientNamedPoint() methods.
     * @param pointName name of the point to fetch
     * @return named point with the name @a pointName
     * @throws PreconditionException if the point does not exist. If transient point are present, the exception is not
     * thrown - instead, NoSuchNamedPointForShapeException will be thrown upon its evaluation.
     */
    [[nodiscard]] NamedPoint getNamedPoint(const std::string &pointName) const;

    /**
     * @brief Returns a list of all named points for the given @a shapeData.
     */
    [[nodiscard]] std::vector<NamedPoint> getNamedPoints(const ShapeData &shapeData) const;

    /**
     * @brief Returns a named point with name @a pointName in shape coordinates (for the default positioned and oriented
     * shape), assuming the point is static.
     * @throws PreconditionException if the point does not exist or is not static
     */
    [[nodiscard]] Vector<3> evaluateNamedPoint(const std::string &pointName, NamedPoint::StaticTag staticTag) const {
        return this->getNamedPoint(pointName).evaluateFor(staticTag);
    }

    /**
     * @brief Returns a named point with name @a pointName in shape coordinates (for the default positioned and oriented
     * shape), for the given @a shapeData ShapeData.
     * @throws PreconditionException if the point does not exist
     * @throws NoSuchNamedPointForShapeException if the point is transient and does not support the specific
     * @a shapeData
     */
    [[nodiscard]] Vector<3> evaluateNamedPoint(const std::string &pointName, const ShapeData &shapeData) const {
        return this->getNamedPoint(pointName).evaluateFor(shapeData);
    }

    /**
     * @brief Returns a named point with name @a pointName on a specifically positioned and oriented @a shape.
     * @throws PreconditionException if the point does not exist
     * @throws NoSuchNamedPointForShapeException if the point is transient and does not support the specific
     * @a shapeData
     */
    [[nodiscard]] Vector<3> evaluateNamedPoint(const std::string &pointName, const Shape &shape) const {
        return this->getNamedPoint(pointName).evaluateFor(shape);
    }

    /**
     * @brief Returns a map of all named points in shape coordinates (for the default positioned and oriented shape),
     * for the given @a shapeData ShapeData.
     */
    [[nodiscard]] std::map<std::string, Vector<3>> evaluateNamedPoints(const ShapeData &shapeData) const;

    /**
     * @brief Returns a map of all named points on a specifically positioned and oriented @a shape.
     */
    [[nodiscard]] std::map<std::string, Vector<3>> evaluateNamedPoints(const Shape &shape) const;

    /**
     * @brief Returns @a true if the non-transient named point with the name @a pointName exists.
     */
    [[nodiscard]] bool hasNonTransientNamedPoint(const std::string &pointName) const;

    /**
     * @brief Returns @a true if the named point with the name @a namedPoint exists for @a shapeData.
     */
    [[nodiscard]] bool hasNamedPoint(const std::string &pointName, const ShapeData &shapeData) const;

    /**
     * @brief Returns @a true if the named point with the name @a pointName exists for the specific ShapeData of
     * @a shape.
     */
    [[nodiscard]] bool hasNamedPoint(const std::string &pointName, const Shape &shape) const {
        return this->hasNamedPoint(pointName, shape.getData());
    }

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
     * the primary axis is returned.
     * @throws PreconditionException if the primary axis does not exists
     */
    [[nodiscard]] Vector<3> findFlipAxis(const Shape &shape) const;
};


#endif //RAMPACK_SHAPEGEOMETRY_H
