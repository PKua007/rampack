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

#include "XCPrimitives.h"
#include "utils/Assertions.h"
#include "XCUtils.h"


CollideSegment::CollideSegment(double halfLength) : halfLength{halfLength} {
    Expects(halfLength > 0);
}

Vector<3> CollideSegment::getSupportPoint(const Vector<3> &n) const {
    Vector<3> v;
    if (n[0] < 0)
        v[0] = -this->halfLength;
    else
        v[0] = this->halfLength;
    return v;
}

CollideRectangle::CollideRectangle(double halfSideX, double halfSideY)
        : halfSides{halfSideX, halfSideX, 0}, halfDiagonal{halfSides.norm()}
{
    Expects(halfSideX > 0 && halfSideY > 0);
}

Vector<3> CollideRectangle::getSupportPoint(const Vector<3> &n) const {
    Vector<3> result = this->halfSides;
    if (n[0] < 0) result[0] = -result[0];
    if (n[1] < 0) result[1] = -result[1];
    return result;
}

CollideBox::CollideBox(const Vector<3> &halfSides) : halfSides{halfSides}, halfDiagonal{halfSides.norm()} {
    Expects(std::all_of(halfSides.begin(), halfSides.end(), [](double d) { return d > 0; }));
}

Vector<3> CollideBox::getSupportPoint(const Vector<3> &n) const {
    Vector<3> result = this->halfSides;
    if (n[0] < 0) result[0] = -result[0];
    if (n[1] < 0) result[1] = -result[1];
    if (n[2] < 0) result[2] = -result[2];
    return result;
}

CollideDisc::CollideDisc(double radius) : radius{radius} {
    Expects(radius > 0);
}

Vector<3> CollideDisc::getSupportPoint(const Vector<3> &n) const {
    Vector<3> n2 = n;
    n2[2] = 0;
    if (is_vector_zero(n2))
        return {};
    return radius * n2.normalized();
}

CollideSphere::CollideSphere(double radius) : radius{radius} {
    Expects(radius > 0);
}

CollideEllipse::CollideEllipse(double semiAxisX, double semiAxisY) : semiAxes{semiAxisX, semiAxisY, 0} {
    Expects(semiAxisX > 0 && semiAxisY > 0);
    this->circumsphereRadius = *std::max_element(this->semiAxes.begin(), this->semiAxes.end());
}

Vector<3> CollideEllipse::getSupportPoint(const Vector<3> &n) const {
    Vector<3> n2 = vector_comp_mul(this->semiAxes, n);
    if (is_vector_zero(n2))
        return {};

    return vector_comp_mul(n2.normalized(), this->semiAxes);
}

CollideEllipsoid::CollideEllipsoid(const Vector<3> &semiAxes) : semiAxes{semiAxes} {
    Expects(std::all_of(semiAxes.begin(), semiAxes.end(), [](double d) { return d > 0; }));
    this->circumsphereRadius = *std::max_element(this->semiAxes.begin(), this->semiAxes.end());
}

Vector<3> CollideEllipsoid::getSupportPoint(const Vector<3> &n) const {
    Vector<3> n2 = vector_comp_mul(n, this->semiAxes).normalized();
    return vector_comp_mul(n2, this->semiAxes);
}

CollideFootball::CollideFootball(double length, double radius) : length{length}, radius{radius} {
    Expects(radius > 0);
    Expects(length >= radius);
}

Vector<3> CollideFootball::getSupportPoint(const Vector<3> &n) const {
    double r1 = this->radius;
    double h = this->length;

    // Radius of curvature
    double r2 = 0.5f * (h*h/r1 + r1);

    Vector<3> n3 = n.normalized();
    if (n3[0] * r2 < -h)
        return {-h, 0, 0};
    if (n3[0] * r2 > h)
        return {h, 0, 0};

    Vector<3> n2 = Vector<3>{0, n[1], n[2]}.normalized();
    return -n2*(r2-r1) + n3*r2;
}

CollideBullet::CollideBullet(double lengthTip, double lengthTail, double radius)
        : lengthTip{lengthTip}, lengthTail{lengthTail}, radius{radius}
{
    Expects(radius > 0);
    Expects(lengthTail > 0);
    Expects(lengthTip >= radius);

    this->circumsphereRadius = std::max(lengthTip, std::sqrt(lengthTail*lengthTip + radius*radius));
}

Vector<3> CollideBullet::getSupportPoint(const Vector<3> &n) const {
    if (n[0] < 0) {
        double r1 = this->radius;
        double h = this->lengthTip;

        // Radius of curvature
        double r2 = 0.5f * (h*h/r1 + r1);

        Vector<3> n3 = n.normalized();
        if (n3[0] * r2 < -h)
            return Vector<3>{-h, 0, 0};
        if (n3[0] * r2 > h)
            return Vector<3>{h, 0, 0};

        Vector<3> n2 = Vector<3>{0, n[1], n[2]}.normalized();
        return -n2 * (r2 - r1) + n3 * r2;
    } else {
        Vector<3> n2 = n;
        n2[0] = 0;
        if (is_vector_zero(n2))
            return {this->lengthTail, 0, 0};
        n2 = n2.normalized();
        return this->radius * n2 + Vector<3>({this->lengthTail, 0, 0});
    }
}

Vector<3> CollideBullet::getCenter() const {
    return {0.5*(this->lengthTail - this->lengthTip), 0, 0};
}

CollideSaucer::CollideSaucer(double radius, double halfThickness) : halfThickness{halfThickness}, radius{radius} {
    Expects(radius > 0);
    Expects(halfThickness <= radius);
}

Vector<3> CollideSaucer::getSupportPoint(const Vector<3> &n) const {
    double t = this->halfThickness;
    double h = this->radius;

    // Radius of curvature
    double r2 = 0.5f * (h*h/t + t);

    Vector<3> n3 = n.normalized();
    Vector<3> n4{0, n[1], n[2]};
    if (!is_vector_zero(n4)) {
        n4 = n4.normalized();
        double threshold = n3[1]*n3[1] + n3[2]*n3[2];
        if (threshold * r2 * r2 > h * h)
            return h * n4;
    }

    Vector<3> n2{1, 0, 0};
    if (n[0] < 0)
        n2 = -n2;

    return -n2 * (r2 - t) + n3 * r2;
}
