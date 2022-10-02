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
 * Adapted by Michal Ciesla and Piotr Kubala
 */

#include "AbstractXCGeometry.h"
#include "../Vector.h"

#include "utils/Assertions.h"


//////////////////////////////////////////////////////////////////////////////
// CollideGeometry

Vector<3> AbstractXCGeometry::getCenter() const
{
	Vector<3> v({0, 0, 0});
	return v;
}

//////////////////////////////////////////////////////////////////////////////
// CollidePoint

CollidePoint::CollidePoint(const Vector<3>& p)
{
	mPoint = p;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollidePoint::getSupportPoint([[maybe_unused]] const Vector<3>& n) const
{
	return mPoint;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollidePoint::getCenter() const
{
	return mPoint;
}

//////////////////////////////////////////////////////////////////////////////
// CollideSegment

CollideSegment::CollideSegment(double r)
{
	mRadius = r;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideSegment::getSupportPoint(const Vector<3>& n) const
{
	Vector<3> v({0, 0, 0});
	if (n[0] < 0)
		v[0] = -mRadius;
	else
		v[0] = mRadius;
	return v;
}

//////////////////////////////////////////////////////////////////////////////
// CollideRectangle

CollideRectangle::CollideRectangle(double rx, double ry)
{
	mRadius[0] = rx;
	mRadius[1] = ry;
	mRadius[2] = 0;
    this->halfDiagonal = this->mRadius.norm();
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideRectangle::getSupportPoint(const Vector<3>& n) const
{
	Vector<3> result = mRadius;
	if (n[0] < 0) result[0] = -result[0];
	if (n[1] < 0) result[1] = -result[1];
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// CollideSphere

CollideSphere::CollideSphere(double r)
{
	mRadius = r;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideSphere::getSupportPoint(const Vector<3>& n) const
{
	Vector<3> n2 = n;
	n2 = n2.normalized();
	return mRadius * n2;
}

//////////////////////////////////////////////////////////////////////////////
// CollideEllipse

CollideEllipse::CollideEllipse(double rx, double ry)
{
	mRadius[0] = rx;
	mRadius[1] = ry;
	mRadius[2] = 0;
    this->circumsphereRadius = *std::max_element(this->mRadius.begin(), this->mRadius.end());
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideEllipse::getSupportPoint(const Vector<3>& n) const
{
	Vector<3> n2 = CompMul(mRadius, n);
	if (is_vector_zero(n2)){
		Vector<3> v({0, 0, 0});
		return v;
	}
	n2 = n2.normalized();
	return CompMul(n2, mRadius);
}

//////////////////////////////////////////////////////////////////////////////
// CollideEllipsoid

CollideEllipsoid::CollideEllipsoid(const Vector<3>& r)
{
	mRadius = r;
    this->circumsphereRadius = *std::max_element(this->mRadius.begin(), this->mRadius.end());
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideEllipsoid::getSupportPoint(const Vector<3>& n) const
{
	Vector<3> n2 = CompMul(n, mRadius);
	n2 = n2.normalized();
	return CompMul(n2, mRadius);
}

//////////////////////////////////////////////////////////////////////////////
// CollideDisc

CollideDisc::CollideDisc(double r)
{
	mRadius = r;
}

Vector<3> CollideDisc::getSupportPoint(const Vector<3>& n) const
{
	Vector<3> n2 = n;
	n2[3] = 0;
	if (is_vector_zero(n2))
	{
		Vector<3> v({0, 0, 0});
		return v;
	}
	n2 = n2.normalized();
	return mRadius * n2;
}

//////////////////////////////////////////////////////////////////////////////
// CollideBox

CollideBox::CollideBox(const Vector<3>& r)
{
	mRadius = r;
    this->halfDiagonal = this->mRadius.norm();
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideBox::getSupportPoint(const Vector<3>& n) const
{
	Vector<3> result = mRadius;
	if (n[0] < 0) result[0] = -result[0];
	if (n[1] < 0) result[1] = -result[1];
	if (n[2] < 0) result[2] = -result[2];
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// CollideFootball

CollideFootball::CollideFootball(double length, double radius) {
    Expects(length >= radius);

	mRadius = radius;
	mLength = length;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideFootball::getSupportPoint(const Vector<3>& n) const
{
	// Radius
	double r1 = mRadius;

	// Half-length
	double h = mLength;

	// Radius of curvature
	double r2 = 0.5f * (h*h/r1 + r1);

	Vector<3> n3 = n.normalized();

	if (n3[0] * r2 < -h) return Vector<3>({-h, 0, 0});
	if (n3[0] * r2 > h) return Vector<3>({h, 0, 0});

	Vector<3> n2 = Vector<3>({0, n[1], n[2]}).normalized();

	Vector<3> p = -n2*(r2-r1) + n3*r2;
	return p;
}

//////////////////////////////////////////////////////////////////////////////
// CollideBullet

CollideBullet::CollideBullet(double lengthTip, double lengthTail, double radius) {
    Expects(lengthTip >= radius);

	mRadius = radius;
	mLengthTip = lengthTip;
	mLengthTail = lengthTail;
    this->circumsphereRadius = std::max(lengthTip, std::sqrt(lengthTail*lengthTip + radius*radius));
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideBullet::getSupportPoint(const Vector<3>& n) const
{
	if (n[0] < 0)
	{
		// Radius
		double r1 = mRadius;
		// Half-length
		double h = mLengthTip;

		// Radius of curvature
		double r2 = 0.5f * (h*h/r1 + r1);

		Vector<3> n3 = n.normalized();

		if (n3[0] * r2 < -h) return Vector<3>({-h, 0, 0});
		if (n3[0] * r2 > h) return Vector<3>({h, 0, 0});

		Vector<3> n2 = Vector<3>({0, n[1], n[2]}).normalized();

		Vector<3> p = -n2*(r2-r1) + n3*r2;
		return p;
	}
	else
	{
		Vector<3> n2 = n;
		n2[0] = 0;
		if (is_vector_zero(n2))
		{
			return Vector<3>({mLengthTail, 0, 0});
		}
		n2 = n2.normalized();
		return mRadius * n2 + Vector<3>({mLengthTail, 0, 0});
	}
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideBullet::getCenter() const
{
	return Vector<3>({ 0.5f * (mLengthTail - mLengthTip), 0, 0 });
}

//////////////////////////////////////////////////////////////////////////////
// CollideSaucer

CollideSaucer::CollideSaucer(double radius, double halfThickness) {
    Expects(halfThickness <= radius);

	mHalfThickness = halfThickness;
	mRadius = radius;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideSaucer::getSupportPoint(const Vector<3>& n) const
{
	// Half-thickness
	double t = mHalfThickness;

	// Half-length
	double h = mRadius;

	// Radius of curvature
	double r2 = 0.5f * (h*h/t + t);

	Vector<3> n3 = n.normalized();

	Vector<3> n4({0, n[1], n[2]});
	if (!is_vector_zero(n4))
	{
		n4 = n4.normalized();
		double threshold = n3[1]*n3[1] + n3[2]*n3[2];
		if (threshold * r2 * r2 > h * h) return h * n4;
	}

	Vector<3> n2({1, 0, 0});
	if (n[0] < 0)
	{
		n2 = -n2;
	}

	Vector<3> p = -n2*(r2-t) + n3*r2;
	return p;
}

//////////////////////////////////////////////////////////////////////////////
// CollidePolytope

CollidePolytope::CollidePolytope(int maxVerts)
{
	mVertMax = maxVerts;
	mVert = new Vector<3>[maxVerts];
	mVertCount = 0;
}

//////////////////////////////////////////////////////////////////////////////

void CollidePolytope::AddVert(const Vector<3>& p)
{
	if (mVertCount >= mVertMax) return;
    double distance = p.norm();
    if (distance > this->circumsphereRadius)
        this->circumsphereRadius = distance;
    mVert[mVertCount++] = p;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollidePolytope::getSupportPoint(const Vector<3>& n) const
{
	int i = mVertCount-1;
	Vector<3> r = mVert[i--];
	while (i>=0)
	{
		if ( (mVert[i] - r) * n > 0 )
		{
			r = mVert[i];
		}
		i--;
	}
	return r;
}

//////////////////////////////////////////////////////////////////////////////
// CollideSum

CollideSum::CollideSum(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1,
                       std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2)
        : m1{m1}, m2{m2}, t1{t1}, t2{t2}, mGeometry1{std::move(g1)}, mGeometry2{std::move(g2)}
{
    double circumsphere1 = this->mGeometry1->getCircumsphereRadius();
    double circumsphere2 = this->mGeometry2->getCircumsphereRadius();
    this->circumsphereRadius = (this->t1 + this->t2).norm() + circumsphere1 + circumsphere2;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideSum::getSupportPoint(const Vector<3>& n) const
{
	return m1*(mGeometry1->getSupportPoint((m1.transpose() * n))) + t1 + m2 * (mGeometry2->getSupportPoint(
            (m2.transpose() * n))) + t2;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideSum::getCenter() const
{
	return m1*(mGeometry1->getCenter()) + t1 + m2 * (mGeometry2->getCenter()) + t2;
}

//////////////////////////////////////////////////////////////////////////////
// CollideDiff

CollideDiff::CollideDiff(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1,
                         std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2)
        : m1{m1}, m2{m2}, t1{t1}, t2{t2}, mGeometry1{std::move(g1)}, mGeometry2{std::move(g2)}
{
    double circumsphere1 = this->mGeometry1->getCircumsphereRadius();
    double circumsphere2 = this->mGeometry2->getCircumsphereRadius();
    this->circumsphereRadius = (this->t1 - this->t2).norm() + circumsphere1 + circumsphere2;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideDiff::getSupportPoint(const Vector<3>& n) const
{
	return m1*(mGeometry1->getSupportPoint(m1.transpose() * n)) + t1 - m2 * (mGeometry2->getSupportPoint(
            (m2.transpose()) * (-n))) - t2;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideDiff::getCenter() const
{
	return m1*(mGeometry1->getCenter()) + t1 - m2 * (mGeometry2->getCenter()) - t2;
}

//////////////////////////////////////////////////////////////////////////////
// CollideNeg

CollideNeg::CollideNeg(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1)
        : m1{m1}, t1{t1}, mGeometry1{std::move(g1)}
{
	this->circumsphereRadius = this->t1.norm() + this->mGeometry1->getCircumsphereRadius();
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideNeg::getSupportPoint(const Vector<3>& n) const
{
	return -(m1*(mGeometry1->getSupportPoint((m1.transpose()) * (-n))) + t1);
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideNeg::getCenter() const
{
	return -(m1*(mGeometry1->getCenter()) + t1);
}

//////////////////////////////////////////////////////////////////////////////
// CollideMax

CollideMax::CollideMax(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1,
                       std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2)
        : m1{m1}, m2{m2}, t1{t1}, t2{t2}, mGeometry1{std::move(g1)}, mGeometry2{std::move(g2)}
{
    double circumsphere1 = this->t1.norm() + this->mGeometry1->getCircumsphereRadius();
    double circumsphere2 = this->t2.norm() + this->mGeometry2->getCircumsphereRadius();
    this->circumsphereRadius = std::max(circumsphere1, circumsphere2);
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideMax::getSupportPoint(const Vector<3>& n) const
{
	Vector<3> v1 = m1*(mGeometry1->getSupportPoint((m1.transpose()) * (n))) + t1;
	Vector<3> v2 = m2*(mGeometry2->getSupportPoint((m2.transpose()) * (n))) + t2;

	if ( (v2-v1) * n > 0 )
	{
		return v2;
	}

	return v1;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideMax::getCenter() const
{
	// Return the average of the two centers
	return 0.5 * (m1*(mGeometry1->getCenter()) + t1 + m2 * (mGeometry2->getCenter()) + t2);
}

//////////////////////////////////////////////////////////////////////////////

