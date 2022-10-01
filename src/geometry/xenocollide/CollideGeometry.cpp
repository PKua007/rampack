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

#include "CollideGeometry.h"
#include "../Vector.h"


//////////////////////////////////////////////////////////////////////////////
// CollideGeometry

Vector<3> CollideGeometry::GetCenter()
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

Vector<3> CollidePoint::GetSupportPoint([[maybe_unused]] const Vector<3>& n)
{
	return mPoint;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollidePoint::GetCenter()
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

Vector<3> CollideSegment::GetSupportPoint(const Vector<3>& n)
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
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideRectangle::GetSupportPoint(const Vector<3>& n)
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

Vector<3> CollideSphere::GetSupportPoint(const Vector<3>& n)
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
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideEllipse::GetSupportPoint(const Vector<3>& n)
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
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideEllipsoid::GetSupportPoint(const Vector<3>& n)
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

Vector<3> CollideDisc::GetSupportPoint(const Vector<3>& n)
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
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideBox::GetSupportPoint(const Vector<3>& n)
{
	Vector<3> result = mRadius;
	if (n[0] < 0) result[0] = -result[0];
	if (n[1] < 0) result[1] = -result[1];
	if (n[2] < 0) result[2] = -result[2];
	return result;
}

//////////////////////////////////////////////////////////////////////////////
// CollideFootball

CollideFootball::CollideFootball(double length, double radius)
{
	mRadius = radius;
	mLength = length;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideFootball::GetSupportPoint(const Vector<3>& n)
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

CollideBullet::CollideBullet(double lengthTip, double lengthTail, double radius)
{
	mRadius = radius;
	mLengthTip = lengthTip;
	mLengthTail = lengthTail;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideBullet::GetSupportPoint(const Vector<3>& n)
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

Vector<3> CollideBullet::GetCenter()
{
	return Vector<3>({ 0.5f * (mLengthTail - mLengthTip), 0, 0 });
}

//////////////////////////////////////////////////////////////////////////////
// CollideSaucer

CollideSaucer::CollideSaucer(double radius, double halfThickness)
{
	mHalfThickness = halfThickness;
	mRadius = radius;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideSaucer::GetSupportPoint(const Vector<3>& n)
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
	mVert[mVertCount++] = p;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollidePolytope::GetSupportPoint(const Vector<3>& n)
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

CollideSum::CollideSum(std::shared_ptr<CollideGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1, std::shared_ptr<CollideGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = m1;
	this->t1 = t1;
	this->m2 = m2;
	this->t2 = t2;
}

//////////////////////////////////////////////////////////////////////////////

CollideSum::CollideSum(std::shared_ptr<CollideGeometry> g1, const Vector<3>& t1, std::shared_ptr<CollideGeometry> g2, const Vector<3>& t2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = Matrix<3,3>::identity();
	this->t1 = t1;
	this->m2 = Matrix<3,3>::identity(); // Quat(0, 0, 0, 1);
	this->t2 = t2;
}

//////////////////////////////////////////////////////////////////////////////

CollideSum::CollideSum(std::shared_ptr<CollideGeometry> g1, std::shared_ptr<CollideGeometry> g2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = Matrix<3,3>::identity();
	this->t1 = Vector<3>({0, 0, 0});
	this->m2 = Matrix<3,3>::identity();
	this->t2 = Vector<3>({0, 0, 0});
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideSum::GetSupportPoint(const Vector<3>& n)
{
	return m1*(mGeometry1->GetSupportPoint( (m1.transpose()*n))) + t1 + m2*(mGeometry2->GetSupportPoint((m2.transpose()*n))) + t2;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideSum::GetCenter()
{
	return m1*(mGeometry1->GetCenter()) + t1 + m2*(mGeometry2->GetCenter()) + t2;
}

//////////////////////////////////////////////////////////////////////////////
// CollideDiff

CollideDiff::CollideDiff(std::shared_ptr<CollideGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1, std::shared_ptr<CollideGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = m1;
	this->t1 = t1;
	this->m2 = m2;
	this->t2 = t2;
}

//////////////////////////////////////////////////////////////////////////////

CollideDiff::CollideDiff(std::shared_ptr<CollideGeometry> g1, const Vector<3>& t1, std::shared_ptr<CollideGeometry> g2, const Vector<3>& t2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = Matrix<3,3>::identity();
	this->t1 = t1;
	this->m2 = Matrix<3,3>::identity();
	this->t2 = t2;
}

//////////////////////////////////////////////////////////////////////////////

CollideDiff::CollideDiff(std::shared_ptr<CollideGeometry> g1, std::shared_ptr<CollideGeometry> g2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = Matrix<3,3>::identity();
	this->t1 = Vector<3>({0, 0, 0});
	this->m2 = Matrix<3,3>::identity();
	this->t2 = Vector<3>({0, 0, 0});
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideDiff::GetSupportPoint(const Vector<3>& n)
{
	return m1*(mGeometry1->GetSupportPoint( m1.transpose()*n)) + t1 - m2*(mGeometry2->GetSupportPoint((m2.transpose())*(-n))) - t2;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideDiff::GetCenter()
{
	return m1*(mGeometry1->GetCenter()) + t1 - m2*(mGeometry2->GetCenter()) - t2;
}

//////////////////////////////////////////////////////////////////////////////
// CollideNeg

CollideNeg::CollideNeg(std::shared_ptr<CollideGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1)
{
	mGeometry1 = g1;
	this->m1 = m1;
	this->t1 = t1;
}

//////////////////////////////////////////////////////////////////////////////

CollideNeg::CollideNeg(std::shared_ptr<CollideGeometry> g1, const Vector<3>& t1)
{
	mGeometry1 = g1;
	this->m1 = Matrix<3,3>::identity();
	this->t1 = t1;
}

//////////////////////////////////////////////////////////////////////////////

CollideNeg::CollideNeg(std::shared_ptr<CollideGeometry> g1)
{
	mGeometry1 = g1;
	this->m1 = Matrix<3,3>::identity();
	this->t1 = Vector<3>({0, 0, 0});
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideNeg::GetSupportPoint(const Vector<3>& n)
{
	return -(m1*(mGeometry1->GetSupportPoint( (m1.transpose())*(-n))) + t1);
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideNeg::GetCenter()
{
	return -(m1*(mGeometry1->GetCenter()) + t1);
}

//////////////////////////////////////////////////////////////////////////////
// CollideMax

CollideMax::CollideMax(std::shared_ptr<CollideGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1, std::shared_ptr<CollideGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = m1;
	this->t1 = t1;
	this->m2 = m2;
	this->t2 = t2;
}

//////////////////////////////////////////////////////////////////////////////

CollideMax::CollideMax(std::shared_ptr<CollideGeometry> g1, const Vector<3>& t1, std::shared_ptr<CollideGeometry> g2, const Vector<3>& t2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = Matrix<3,3>::identity();
	this->t1 = t1;
	this->m2 = Matrix<3,3>::identity();
	this->t2 = t2;
}

//////////////////////////////////////////////////////////////////////////////

CollideMax::CollideMax(std::shared_ptr<CollideGeometry> g1, std::shared_ptr<CollideGeometry> g2)
{
	mGeometry1 = g1;
	mGeometry2 = g2;
	this->m1 = Matrix<3,3>::identity();
	this->t1 = Vector<3>({0, 0, 0});
	this->m2 = Matrix<3,3>::identity();
	this->t2 = Vector<3>({0, 0, 0});
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideMax::GetSupportPoint(const Vector<3>& n)
{
	Vector<3> v1 = m1*(mGeometry1->GetSupportPoint((m1.transpose())*(n))) + t1;
	Vector<3> v2 = m2*(mGeometry2->GetSupportPoint((m2.transpose())*(n))) + t2;

	if ( (v2-v1) * n > 0 )
	{
		return v2;
	}

	return v1;
}

//////////////////////////////////////////////////////////////////////////////

Vector<3> CollideMax::GetCenter()
{
	// Return the average of the two centers
	return 0.5 * (m1*(mGeometry1->GetCenter()) + t1 + m2*(mGeometry2->GetCenter()) + t2);
}

//////////////////////////////////////////////////////////////////////////////

