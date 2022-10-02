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

#ifndef RAMPACK_ABSTRACTXCGEOMETRY_H
#define RAMPACK_ABSTRACTXCGEOMETRY_H


#include <algorithm>
#include <memory>

#include "../Vector.h"


inline bool is_vector_zero(const Vector<3> &v) {
    constexpr double ZERO = 9.094947e-13;
    return std::all_of(v.begin(), v.end(), [](double d) { return std::abs(d) < ZERO; });
}


//////////////////////////////////////////////////////////////////////////////
// This is the base class for XenoCollide shapes.  To create a new primitive,
// derive from CollideGeometry and implement the GetSupportPoint()
// method.  By default, GetCenter() will return (0, 0, 0).  If this isn't
// a deep interior point for your shape, override this method and return a
// different point.

class AbstractXCGeometry {
public:
	virtual ~AbstractXCGeometry() = default;

	[[nodiscard]] virtual Vector<3> getSupportPoint(const Vector<3>& n) const = 0;
	[[nodiscard]] virtual Vector<3> getCenter() const;
    [[nodiscard]] virtual double getCircumsphereRadius() const { return 0; };
};

//////////////////////////////////////////////////////////////////////////////

class CollidePoint : public AbstractXCGeometry {
private:
	Vector<3> mPoint;

public:
	explicit CollidePoint(const Vector<3>& p);

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
	[[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return 0; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideSegment : public AbstractXCGeometry {
private:
	double mRadius;

public:
	explicit CollideSegment(double r);

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->mRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideRectangle : public AbstractXCGeometry {
private:
	Vector<3> mRadius;
    double halfDiagonal{};

public:
	CollideRectangle(double rx, double ry);

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->halfDiagonal; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideBox : public AbstractXCGeometry {
private:
	Vector<3> mRadius;
    double halfDiagonal{};

public:
	explicit CollideBox(const Vector<3>& r);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->halfDiagonal; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideDisc : public AbstractXCGeometry {
private:
	double mRadius;

public:
	explicit CollideDisc(double r);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->mRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideSphere : public AbstractXCGeometry {
private:
	double mRadius;

public:
	explicit CollideSphere(double r);

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->mRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideEllipse : public AbstractXCGeometry {
private:
	Vector<3> mRadius;
    double circumsphereRadius{};

public:
	CollideEllipse(double rx, double ry);

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideEllipsoid : public AbstractXCGeometry {
private:
	Vector<3> mRadius;
    double circumsphereRadius{};

public:
	explicit CollideEllipsoid(const Vector<3>& r);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideFootball : public AbstractXCGeometry {
private:
	double mLength;
	double mRadius;

public:
	CollideFootball(double length, double radius);

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->mLength; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideBullet : public AbstractXCGeometry {
private:
	double mLengthTip;
	double mLengthTail;
	double mRadius;
    double circumsphereRadius;

public:
	CollideBullet(double lengthTip, double lengthTail, double radius);

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
	[[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideSaucer : public AbstractXCGeometry {
private:
	double mHalfThickness;
	double mRadius;

public:
	CollideSaucer(double radius, double halfThickness);

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->mRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollidePolytope : public AbstractXCGeometry {
private:
	Vector<3>* mVert;
	int mVertMax;
	int mVertCount;
    double circumsphereRadius{};

public:
	explicit CollidePolytope(int n);

	void AddVert(const Vector<3>& p);
	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideSum : public AbstractXCGeometry {
private:
	Matrix<3,3>	m1;
	Matrix<3,3> m2;
	Vector<3>	t1;
	Vector<3>	t2;
    double circumsphereRadius{};

	std::shared_ptr<AbstractXCGeometry>	mGeometry1;
	std::shared_ptr<AbstractXCGeometry>	mGeometry2;

public:
	CollideSum(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1,
               std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2);

    CollideSum(std::shared_ptr<AbstractXCGeometry> g1, const Vector<3>& t1,
               std::shared_ptr<AbstractXCGeometry> g2, const Vector<3>& t2)
            : CollideSum(std::move(g1), Matrix<3, 3>::identity(), t1, std::move(g2), Matrix<3, 3>::identity(), t2)
    { }

    CollideSum(std::shared_ptr<AbstractXCGeometry> g1, std::shared_ptr<AbstractXCGeometry> g2)
            : CollideSum(std::move(g1), {}, std::move(g2), {})
    { }

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
	[[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideDiff : public AbstractXCGeometry {
private:
	Matrix<3,3>	m1;
	Matrix<3,3>	m2;
	Vector<3>	t1;
	Vector<3>	t2;
    double circumsphereRadius{};

	std::shared_ptr<AbstractXCGeometry>	mGeometry1;
	std::shared_ptr<AbstractXCGeometry>	mGeometry2;

public:
    CollideDiff(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1,
                std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2);

    CollideDiff(std::shared_ptr<AbstractXCGeometry> g1, const Vector<3>& t1,
                std::shared_ptr<AbstractXCGeometry> g2, const Vector<3>& t2)
            : CollideDiff(std::move(g1), Matrix<3, 3>::identity(), t1, std::move(g2), Matrix<3, 3>::identity(), t2)
    { }

    CollideDiff(std::shared_ptr<AbstractXCGeometry> g1, std::shared_ptr<AbstractXCGeometry> g2)
            : CollideDiff(std::move(g1), {}, std::move(g2), {})
    { }

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
	[[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideNeg : public AbstractXCGeometry {
private:
	Matrix<3,3>	m1;
	Vector<3>	t1;
    double circumsphereRadius{};

	std::shared_ptr<AbstractXCGeometry>	mGeometry1;

public:
	CollideNeg(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1);

	CollideNeg(std::shared_ptr<AbstractXCGeometry> g1, const Vector<3>& t1)
            : CollideNeg(std::move(g1), Matrix<3, 3>::identity(), t1)
    { }

	explicit CollideNeg(std::shared_ptr<AbstractXCGeometry> g1) : CollideNeg(std::move(g1), {})
    { }

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
	[[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};

//////////////////////////////////////////////////////////////////////////////

class CollideMax : public AbstractXCGeometry {
private:
	Matrix<3,3>	m1;
	Matrix<3,3>	m2;
	Vector<3>	t1;
	Vector<3>	t2;
    double circumsphereRadius{};

	std::shared_ptr<AbstractXCGeometry>	mGeometry1;
	std::shared_ptr<AbstractXCGeometry>	mGeometry2;

public:
    CollideMax(std::shared_ptr<AbstractXCGeometry> g1, const Matrix<3,3>& m1, const Vector<3>& t1,
               std::shared_ptr<AbstractXCGeometry> g2, const Matrix<3,3>& m2, const Vector<3>& t2);

    CollideMax(std::shared_ptr<AbstractXCGeometry> g1, const Vector<3>& t1,
               std::shared_ptr<AbstractXCGeometry> g2, const Vector<3>& t2)
            : CollideMax(std::move(g1), Matrix<3, 3>::identity(), t1, std::move(g2), Matrix<3, 3>::identity(), t2)
    { }

    CollideMax(std::shared_ptr<AbstractXCGeometry> g1, std::shared_ptr<AbstractXCGeometry> g2)
            : CollideMax(std::move(g1), {}, std::move(g2), {})
    { }

	[[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
	[[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
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

#endif //RAMPACK_ABSTRACTXCGEOMETRY_H