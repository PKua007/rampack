/*
XenoCollide Collision Detection and Physics Library
Copyright (c) 2007-2014 Gary Snethen http://xenocollide.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising
from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must
not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

/*
 * Heavily adapted by Michal Ciesla and Piotr Kubala
 */

#ifndef RAMPACK_XCPRIMITIVES_H
#define RAMPACK_XCPRIMITIVES_H

#include "AbstractXCGeometry.h"


/**
 * @brief AbstractXCGeometry describing a single point.
 */
class XCPoint : public AbstractXCGeometry {
private:
    Vector<3> pos;

public:
    /**
     * @brief Creates the point at position @a pos.
     */
    explicit XCPoint(const Vector<3> &pos) : pos{pos} { }

    [[nodiscard]] Vector<3> getSupportPoint([[maybe_unused]] const Vector<3> &n) const override { return this->pos; }
    [[nodiscard]] Vector<3> getCenter() const override { return this->pos; }
    [[nodiscard]] double getCircumsphereRadius() const override { return 0; }
    [[nodiscard]] double getInsphereRadius() const override { return 0; }
};


/**
 * @brief AbstractXCGeometry describing a segment.
 */
class XCSegment : public AbstractXCGeometry {
private:
    double halfLength{};

public:
    /**
     * @brief Creates the segment centered between the points: `{0, 0, -halfLength}`, `{0, 0, halfLength}`.
     */
    explicit XCSegment(double halfLength);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->halfLength; }
    [[nodiscard]] double getInsphereRadius() const override { return 0; }
};


/**
 * @brief AbstractXCGeometry describing a rectangle.
 */
class XCRectangle : public AbstractXCGeometry {
private:
    Vector<3> halfSides;
    double circumsphereRadius{};

public:
    /**
     * @brief Creates an axis aligned rectangle with two opposite vertices: {-@a halfSideX, -@a halfSideY, 0} and
     * {@a halfSideX, @a halfSideY, 0}.
     */
    XCRectangle(double halfSideX, double halfSideY);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
    [[nodiscard]] double getInsphereRadius() const override { return 0; }
};


/**
 * @brief AbstractXCGeometry describing a cuboid.
 */
class XCCuboid : public AbstractXCGeometry {
private:
    Vector<3> halfSides;
    double circumsphereRadius{};
    double insphereRadius{};

public:
    /**
     * @brief Creates an axis aligned cuboid with two opposite vertices:
     * {-@a halfSides[0], -@a halfSides[1], -@a halfSides[2]} and {@a halfSides[0], @a halfSides[1], @a halfSides[2]}.
     */
    explicit XCCuboid(const Vector<3> &halfSides);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
    [[nodiscard]] double getInsphereRadius() const override { return this->insphereRadius; }
};


/**
 * @brief AbstractXCGeometry describing a disk.
 */
class XCDisk : public AbstractXCGeometry {
private:
    double radius{};

public:
    /**
     * @brief Creates a disk with its center in {0, 0, 0}, lying in XY plane with radius @a radius.
     */
    explicit XCDisk(double radius);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->radius; }
    [[nodiscard]] double getInsphereRadius() const override { return 0; }
};


/**
 * @brief AbstractXCGeometry describing a sphere.
 */
class XCSphere : public AbstractXCGeometry {
private:
    double radius{};

public:
    /**
     * @brief Creates a sphere with its center in {0, 0, 0} and radius @a radius.
     */
    explicit XCSphere(double radius);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override { return this->radius * n.normalized(); }
    [[nodiscard]] double getCircumsphereRadius() const override { return this->radius; }
    [[nodiscard]] double getInsphereRadius() const override { return this->radius; }
};


/**
 * @brief AbstractXCGeometry describing an ellipse.
 */
class XCEllipse : public AbstractXCGeometry {
private:
    Vector<3> semiAxes;
    double circumsphereRadius{};

public:
    /**
     * @brief Creates a disk with its center in {0, 0, 0}, lying in XY plane with X semi-axis @a semiAxisX and
     * Y semi-axis @a semiAxisY.
     */
    XCEllipse(double semiAxisX, double semiAxisY);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
    [[nodiscard]] double getInsphereRadius() const override { return 0; }
};


/**
 * @brief AbstractXCGeometry describing an ellipsoid.
 */
class XCEllipsoid : public AbstractXCGeometry {
private:
    Vector<3> semiAxes;
    double circumsphereRadius{};
    double insphereRadius{};

public:
    /**
     * @brief Creates an ellipsoid with its center in {0, 0, 0} and axis-oriented semi-axes @a semiAxes (subsequent
     * indices correspond to, respectively, X, Y and Z semi-axis).
     */
    explicit XCEllipsoid(const Vector<3> &semiAxes);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
    [[nodiscard]] double getInsphereRadius() const override { return this->insphereRadius; }
};


/**
 * @brief AbstractXCGeometry describing a rugby ball.
 */
class XCFootball : public AbstractXCGeometry {
private:
    double halfLength{};
    double radius{};

public:
    /**
     * @brief Creates a rugby ball by drawing a circle segment between the points `{0, 0, -halfLength}` and
     * `{0, 0, halfLength}` and then revolving it around Z axis. The radius is chosen in such a way that the XY section
     * in the middle of the rugby ball is a disk with radius @a radius.
     */
    XCFootball(double halfLength, double radius);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->halfLength; }
    [[nodiscard]] double getInsphereRadius() const override { return this->radius; }
};


/**
 * @brief AbstractXCGeometry describing a saucer.
 */
class XCSaucer : public AbstractXCGeometry {
private:
    double halfThickness{};
    double radius{};

public:
    /**
     * @brief Creates saucer by drawing a circle segment between the points `{-radius, 0, 0}` and `{radius, 0, 0}` and
     * then revolving it around Z axis. The radius of the circle segment is chosen in such a way that the half-height of
     * the saucer is @a halfThickness.
     */
    XCSaucer(double radius, double halfThickness);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->radius; }
    [[nodiscard]] double getInsphereRadius() const override { return this->halfThickness; }
};

#endif //RAMPACK_XCPRIMITIVES_H
