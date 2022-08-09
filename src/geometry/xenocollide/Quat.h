#ifndef GEOMETRY_QUAT_H_
#define GEOMETRY_QUAT_H_

//////////////////////////////////////////////////////////////////////////////////////////////
// Quaternion Class
//////////////////////////////////////////////////////////////////////////////////////////////
#include "../Vector.h"
#include "../Matrix.h"
#include "../../utils/Assertions.h"


class Quat
{
private:
	double x, y, z, w;

public:
						Quat() {x=0; y=0; z=0; w=0;};
	explicit			Quat(const Matrix<3,3>& m);
						Quat(double x, double y, double z, double w);
						Quat(const Vector<3>& axis, double angle);
						Quat(const Vector<3>& a, const Vector<3>& b);
	static Quat			QuatExp(const Quat& a);


	double&				X();
	double&				Y();
	double&				Z();
	double&				W();
	double&				operator() (int i);

	double				X() const;
	double				Y() const;
	double				Z() const;
	double				W() const;
	double				operator() (int i) const;

	double 				Len() const;
	bool				isZero() const;
	void				Normalize();

	void				Build(const Matrix<3,3>& m);
	void				Build(const Vector<3>& axis, double angle);
	void				Build(const Vector<3>& a, const Vector<3>& b);

	Quat&				operator*=(const Quat& q);

	Vector<3>			Rotate(const Vector<3>& v) const;
	void				ConvertToAxisAngle(Vector<3>* axis, double* angle) const;

};


inline Quat::Quat(double x, double y, double z, double w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

inline Quat::Quat(const Matrix<3, 3>& m)
{
	this->Build(m);
}

inline Quat::Quat(const Vector<3>& axis, double angle)
{
	this->Build(axis, angle);
}

inline Quat::Quat(const Vector<3>& a, const Vector<3>& b)
{
	this->Build(a, b);
}

inline double& Quat::X()
{
	return this->x;
}

inline double& Quat::Y()
{
	return this->y;
}

inline double& Quat::Z()
{
	return this->z;
}

inline double& Quat::W()
{
	return this->w;
}

inline double Quat::X() const
{
	return this->x;
}

inline double Quat::Y() const
{
	return this->y;
}

inline double Quat::Z() const
{
	return this->z;
}

inline double Quat::W() const
{
	return this->w;
}

inline double Quat::Len() const
{
	return std::sqrt(this->x*this->x + this->y*this->y + this->z*this->z + this->w*this->w);
}

inline bool operator ==(const Quat& a, const Quat& b)
{
	return (a.X() == b.X() && a.Y() == b.Y() && a.Z() == b.Z() && a.W() == b.W());
}

inline bool operator !=(const Quat& a, const Quat& b)
{
	return !(a == b);
}

inline Quat operator +(const Quat& a, const Quat& b)
{
	return Quat(a.X()+b.X(), a.Y()+b.Y(), a.Z()+b.Z(), a.W()+b.W());
}

inline Quat operator *(const Quat& a, const Quat& b)
{
	Quat c;
	const double aW = a.W(), aX = a.X(), aY = a.Y(), aZ = a.Z();
	const double bW = b.W(), bX = b.X(), bY = b.Y(), bZ = b.Z();
	c.X() = (aW * bX) + (bW * aX) + (aY * bZ) - (aZ * bY);
	c.Y() = (aW * bY) + (bW * aY) + (aZ * bX) - (aX * bZ);
	c.Z() = (aW * bZ) + (bW * aZ) + (aX * bY) - (aY * bX);
	c.W() = (aW * bW) - ((aX * bX) + (aY * bY) + (aZ * bZ));
	return c;
}

inline Quat& Quat::operator *=(const Quat& q)
{
	*this = (*this) * q;
	return *this;
}

inline Quat operator *(const Quat& a, double b)
{
	return Quat(a.X()*b, a.Y()*b, a.Z()*b, a.W()*b);
}

inline Quat operator *(double b, const Quat& a)
{
	return Quat(a.X()*b, a.Y()*b, a.Z()*b, a.W()*b);
}

inline Quat operator /(const Quat& a, double b)
{
    ValidateMsg(b!=0, "divide by 0");
	return Quat(a.X()/b, a.Y()/b, a.Z()/b, a.W()/b);
}

inline Quat operator ~(const Quat& a)
{
	Quat q = -1.0f * a;
	q.W() = a.W();
	return q;
}

#endif /* GEOMETRY_QUAT_H_ */
