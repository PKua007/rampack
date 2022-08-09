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

#pragma once


#include <algorithm>

#include "MapPtr.h"
#include "../Vector.h"
#include "Quat.h"


bool is_vector_zero(const Vector<3> &v) {
    constexpr double ZERO = 9.094947e-13;
    return std::all_of(v.begin(), v.end(), [](double d) { return std::abs(d) < ZERO; });
}


//////////////////////////////////////////////////////////////////////////////
// This is the base class for XenoCollide shapes.  To create a new primitive,
// derive from CollideGeometry and implement the GetSupportPoint()
// method.  By default, GetCenter() will return (0, 0, 0).  If this isn't
// a deep interior point for your shape, override this method and return a
// different point.

class CollideGeometry{

//	friend class MapPtr<CollideGeometry>;
public:
	virtual ~CollideGeometry() {}
	virtual Vector<3> GetSupportPoint(const Vector<3>& n) = 0;
	virtual Vector<3> GetCenter();
};

//////////////////////////////////////////////////////////////////////////////

class CollidePoint : public CollideGeometry
{
	Vector<3> mPoint;

public:

	CollidePoint(const Vector<3>& p);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
	virtual Vector<3> GetCenter();
};

//////////////////////////////////////////////////////////////////////////////

class CollideSegment : public CollideGeometry
{
	double mRadius;

public:

	CollideSegment(double r);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideRectangle : public CollideGeometry
{
	Vector<3> mRadius;

public:

	CollideRectangle(double rx, double ry);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideBox : public CollideGeometry
{
	Vector<3> mRadius;

public:

	CollideBox(const Vector<3>& r);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideDisc : public CollideGeometry
{
	double mRadius;

public:

	CollideDisc(double r);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideSphere : public CollideGeometry
{
	double mRadius;

public:

	CollideSphere(double r);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideEllipse : public CollideGeometry
{
	Vector<3> mRadius;

public:

	CollideEllipse(double rx, double ry);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideEllipsoid : public CollideGeometry
{
	Vector<3> mRadius;

public:

	CollideEllipsoid(const Vector<3>& r);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideFootball : public CollideGeometry
{

	double mLength;
	double mRadius;

public:

	CollideFootball(double length, double radius);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideBullet : public CollideGeometry
{

	double mLengthTip;
	double mLengthTail;
	double mRadius;

public:

	CollideBullet(double lengthTip, double lengthTail, double radius);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
	virtual Vector<3> GetCenter();
};

//////////////////////////////////////////////////////////////////////////////

class CollideSaucer : public CollideGeometry
{

	double mHalfThickness;
	double mRadius;

public:

	CollideSaucer(double radius, double halfThickness);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollidePolytope : public CollideGeometry
{

	Vector<3>* mVert;
	int mVertMax;
	int mVertCount;

public:

	CollidePolytope(int n);

	void AddVert(const Vector<3>& p);
	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
};

//////////////////////////////////////////////////////////////////////////////

class CollideSum : public CollideGeometry
{

public:

	Quat	q1;
	Quat	q2;
	Vector<3>	t1;
	Vector<3>	t2;

	MapPtr<CollideGeometry>	mGeometry1;
	MapPtr<CollideGeometry>	mGeometry2;

public:

	CollideSum(CollideGeometry* g1, const Quat& q1, const Vector<3>& t1, CollideGeometry* g2, const Quat& q2, const Vector<3>& t2);
	CollideSum(CollideGeometry* g1, const Vector<3>& t1, CollideGeometry* g2, const Vector<3>& t2);
	CollideSum(CollideGeometry* g1, CollideGeometry* g2);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
	virtual Vector<3> GetCenter();
};

//////////////////////////////////////////////////////////////////////////////

class CollideDiff : public CollideGeometry
{

	Quat	q1;
	Quat	q2;
	Vector<3>	t1;
	Vector<3>	t2;

	MapPtr<CollideGeometry>	mGeometry1;
	MapPtr<CollideGeometry>	mGeometry2;

public:

	CollideDiff(CollideGeometry* g1, const Quat& q1, const Vector<3>& t1, CollideGeometry* g2, const Quat& q2, const Vector<3>& t2);
	CollideDiff(CollideGeometry* g1, const Vector<3>& t1, CollideGeometry* g2, const Vector<3>& t2);
	CollideDiff(CollideGeometry* g1, CollideGeometry* g2);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
	virtual Vector<3> GetCenter();
};

//////////////////////////////////////////////////////////////////////////////

class CollideNeg : public CollideGeometry
{

	Quat	q1;
	Vector<3>	t1;

	MapPtr<CollideGeometry>	mGeometry1;

public:

	CollideNeg(CollideGeometry* g1, const Quat& q1, const Vector<3>& t1);
	CollideNeg(CollideGeometry* g1, const Vector<3>& t1);
	CollideNeg(CollideGeometry* g1);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
	virtual Vector<3> GetCenter();
};

//////////////////////////////////////////////////////////////////////////////

class CollideMax : public CollideGeometry
{
	Quat	q1;
	Quat	q2;
	Vector<3>	t1;
	Vector<3>	t2;

	MapPtr<CollideGeometry>	mGeometry1;
	MapPtr<CollideGeometry>	mGeometry2;

public:

	CollideMax(CollideGeometry* g1, const Quat& q1, const Vector<3>& t1, CollideGeometry* g2, const Quat& q2, const Vector<3>& t2);
	CollideMax(CollideGeometry* g1, const Vector<3>& t1, CollideGeometry* g2, const Vector<3>& t2);
	CollideMax(CollideGeometry* g1, CollideGeometry* g2);

	virtual Vector<3> GetSupportPoint(const Vector<3>& n);
	virtual Vector<3> GetCenter();

};

//////////////////////////////////////////////////////////////////////////////
inline Vector<3> CompMul(const Vector<3>& a, const Vector<3>& b)
{
	Vector<3> v;
	v[0] = a[0]*b[0];
	v[1] = a[1]*b[1];
	v[2] = a[2]*b[2];
	return v;
}

