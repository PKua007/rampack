//
// Created by pkua on 07.10.22.
//

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
