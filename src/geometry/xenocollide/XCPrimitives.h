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


class CollidePoint : public AbstractXCGeometry {
private:
    Vector<3> pos;

public:
    explicit CollidePoint(const Vector<3> &pos) : pos{pos} { }

    [[nodiscard]] Vector<3> getSupportPoint([[maybe_unused]] const Vector<3> &n) const override { return this->pos; }
    [[nodiscard]] Vector<3> getCenter() const override { return this->pos; }
    [[nodiscard]] double getCircumsphereRadius() const override { return 0; }
};


class CollideSegment : public AbstractXCGeometry {
private:
    double halfLength{};

public:
    explicit CollideSegment(double halfLength);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->halfLength; }
};


class CollideRectangle : public AbstractXCGeometry {
private:
    Vector<3> halfSides;
    double halfDiagonal{};

public:
    CollideRectangle(double halfSideX, double halfSideY);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->halfDiagonal; }
};


class CollideBox : public AbstractXCGeometry {
private:
    Vector<3> halfSides;
    double halfDiagonal{};

public:
    explicit CollideBox(const Vector<3> &halfSides);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->halfDiagonal; }
};


class CollideDisc : public AbstractXCGeometry {
private:
    double radius{};

public:
    explicit CollideDisc(double radius);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->radius; }
};


class CollideSphere : public AbstractXCGeometry {
private:
    double radius{};

public:
    explicit CollideSphere(double radius);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override { return this->radius * n.normalized(); }
    [[nodiscard]] double getCircumsphereRadius() const override { return this->radius; }
};


class CollideEllipse : public AbstractXCGeometry {
private:
    Vector<3> semiAxes;
    double circumsphereRadius{};

public:
    CollideEllipse(double semiAxisX, double semiAxisY);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};


class CollideEllipsoid : public AbstractXCGeometry {
private:
    Vector<3> semiAxes;
    double circumsphereRadius{};

public:
    explicit CollideEllipsoid(const Vector<3> &semiAxes);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};


class CollideFootball : public AbstractXCGeometry {
private:
    double length{};
    double radius{};

public:
    CollideFootball(double length, double radius);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->length; }
};


class CollideBullet : public AbstractXCGeometry {
private:
    double lengthTip{};
    double lengthTail{};
    double radius{};
    double circumsphereRadius{};

public:
    CollideBullet(double lengthTip, double lengthTail, double radius);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};


class CollideSaucer : public AbstractXCGeometry {
private:
    double halfThickness{};
    double radius{};

public:
    CollideSaucer(double radius, double halfThickness);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->radius; }
};

#endif //RAMPACK_XCPRIMITIVES_H
