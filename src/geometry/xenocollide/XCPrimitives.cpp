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
#include "utils/Exceptions.h"
#include "XCUtils.h"


XCSegment::XCSegment(double halfLength) : halfLength{halfLength} {
    Expects(halfLength > 0);
}

Vector<3> XCSegment::getSupportPoint(const Vector<3> &n) const {
    Vector<3> v;
    if (n[2] < 0)
        v[2] = -this->halfLength;
    else
        v[2] = this->halfLength;
    return v;
}

XCRectangle::XCRectangle(double halfSideX, double halfSideY)
        : halfSides{halfSideX, halfSideY, 0}, circumsphereRadius{halfSides.norm()}
{
    Expects(halfSideX > 0 && halfSideY > 0);
}

Vector<3> XCRectangle::getSupportPoint(const Vector<3> &n) const {
    Vector<3> result = this->halfSides;
    if (n[0] < 0) result[0] = -result[0];
    if (n[1] < 0) result[1] = -result[1];
    return result;
}

XCCuboid::XCCuboid(const Vector<3> &halfSides)
        : halfSides{halfSides}, circumsphereRadius{halfSides.norm()},
          insphereRadius{*std::min_element(halfSides.begin(), halfSides.end())}
{
    Expects(std::all_of(halfSides.begin(), halfSides.end(), [](double d) { return d > 0; }));
}

Vector<3> XCCuboid::getSupportPoint(const Vector<3> &n) const {
    Vector<3> result = this->halfSides;
    if (n[0] < 0) result[0] = -result[0];
    if (n[1] < 0) result[1] = -result[1];
    if (n[2] < 0) result[2] = -result[2];
    return result;
}

XCDisk::XCDisk(double radius) : radius{radius} {
    Expects(radius > 0);
}

Vector<3> XCDisk::getSupportPoint(const Vector<3> &n) const {
    Vector<3> n2 = n;
    n2[2] = 0;
    if (is_vector_zero(n2))
        return {};
    return radius * n2.normalized();
}

XCSphere::XCSphere(double radius) : radius{radius} {
    Expects(radius > 0);
}

XCEllipse::XCEllipse(double semiAxisX, double semiAxisY)
        : semiAxes{semiAxisX, semiAxisY, 0},
          circumsphereRadius{*std::max_element(semiAxes.begin(), semiAxes.end())}
{
    Expects(semiAxisX > 0 && semiAxisY > 0);
}

Vector<3> XCEllipse::getSupportPoint(const Vector<3> &n) const {
    Vector<3> n2 = vector_comp_mul(this->semiAxes, n);
    if (is_vector_zero(n2))
        return {};

    return vector_comp_mul(n2.normalized(), this->semiAxes);
}

XCEllipsoid::XCEllipsoid(const Vector<3> &semiAxes)
        : semiAxes{semiAxes},
          circumsphereRadius{*std::max_element(semiAxes.begin(), semiAxes.end())},
          insphereRadius{*std::min_element(semiAxes.begin(), semiAxes.end())}
{
    Expects(std::all_of(semiAxes.begin(), semiAxes.end(), [](double d) { return d > 0; }));
}

Vector<3> XCEllipsoid::getSupportPoint(const Vector<3> &n) const {
    Vector<3> n2 = vector_comp_mul(n, this->semiAxes).normalized();
    return vector_comp_mul(n2, this->semiAxes);
}

XCFootball::XCFootball(double length, double radius) : length{length}, radius{radius} {
    Expects(radius > 0);
    Expects(length >= radius);
}

Vector<3> XCFootball::getSupportPoint(const Vector<3> &n) const {
    double r1 = this->radius;
    double h = this->length;

    // Radius of curvature
    double r2 = 0.5f * (h*h/r1 + r1);

    Vector<3> n3 = n.normalized();
    if (n3[2] * r2 < -h)
        return {0, 0, -h};
    if (n3[2] * r2 > h)
        return {0, 0, h};

    Vector<3> n2 = Vector<3>{n[0], n[1], 0}.normalized();
    return -n2*(r2-r1) + n3*r2;
}

XCSaucer::XCSaucer(double radius, double halfThickness) : halfThickness{halfThickness}, radius{radius} {
    Expects(radius > 0);
    Expects(halfThickness <= radius);
}

Vector<3> XCSaucer::getSupportPoint(const Vector<3> &n) const {
    double t = this->halfThickness;
    double h = this->radius;

    // Radius of curvature
    double r2 = 0.5f * (h*h/t + t);

    Vector<3> n3 = n.normalized();
    Vector<3> n4{n[0], n[1], 0};
    if (!is_vector_zero(n4)) {
        n4 = n4.normalized();
        double threshold = n3[0]*n3[0] + n3[1]*n3[1];
        if (threshold * r2 * r2 > h * h)
            return h * n4;
    }

    Vector<3> n2{0, 0, 1};
    if (n[2] < 0)
        n2 = -n2;

    return -n2 * (r2 - t) + n3 * r2;
}
