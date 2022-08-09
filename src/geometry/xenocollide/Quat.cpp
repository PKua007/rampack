/*
 * Quat.cpp
 *
 *  Created on: Jul 26, 2022
 *      Author: XenoCollide
 */

#include "Quat.h"
#include "../Vector.h"


bool Quat::isZero() const
{
	double ZERO = 9.094947e-13;
    return (
    		(this->x < ZERO && this->x > -ZERO) &&
    		(this->y < ZERO && this->y > -ZERO) &&
    		(this->z < ZERO && this->z > -ZERO) &&
    		(this->w < ZERO && this->w > -ZERO)
    		);
}

Vector<3> Quat::Rotate(const Vector<3>& v) const
{
	Quat qv(v[0], v[1], v[2], 0);
	qv = (*this * qv * ~*this);
	Vector<3> w({qv.x, qv.y, qv.z});
	return w;
}

Quat Quat::QuatExp(const Quat& a)
{
	double theta = a.Len();

	if (theta == 0.0f)
	{
		return Quat(0, 0, 0, 1);
	}

	double s = sinf(0.5f * theta);
	double c = cosf(0.5f * theta);

	Quat q = a * (s / theta);
	q.W() = c;

	return q;
}

void Quat::Normalize()
{
	double d = this->Len();
	this->x /= d;
	this->y /= d;
	this->z /= d;
	this->w /= d;
}

void Quat::Build(const Vector<3>& v1, const Vector<3>& v2)
{
	if (v1.norm2() > 0 && v2.norm2() > 0)
	{
		Vector<3> u1 = v1.normalized(), u2 = v2.normalized();

		Vector<3> mid = 0.5 * (u1 + u2);
		Vector<3> axis = u1 ^ mid;

		*this = Quat(axis[0], axis[1], axis[2], u1*mid);

		if (isZero())
		{
			if (std::abs(u1[2]) > 0.5f)
			{
				*this = Quat(u1[2], 0.f, -u1[0], 0.f);
			}
			else
			{
				*this = Quat(u1[1], -u1[0], 0.f, 0.f);
			}
		}

		this->Normalize();
	}
	else
	{
		*this = Quat(0, 0, 0, 1);
	}
}

void Quat::Build(const Matrix<3,3>& R)
{
	double trace = R(0,0) + R(1,1) + R(2,2);

	if (trace > 0)
	{
		this->x = R(2,1) - R(1,2);
		this->y = R(0,2) - R(2,0);
		this->z = R(1,0) - R(0,1);
		this->w = 1 + trace;
	}
	else
	{
		double mx = R(0,0);
		uint ix = 0;

		if (R(1,1) > mx)
		{
			mx = R(1,1);
			ix = 1;
		}

		if (R(2,2) > mx)
		{
			mx = R(2,2);
			ix = 2;
		}

		switch ( ix )
		{

			case 0:

				this->x = 1 + R(0,0) - R(1,1) - R(2,2);
				this->y = R(1,0) + R(0,1);
				this->z = R(2,0) + R(0,2);
				this->w = R(2,1) - R(1,2);
				break;

			case 1:

				this->x = R(0,1) + R(1,0);
				this->y = 1 - R(0,0) + R(1,1) - R(2,2);
				this->z = R(2,1) + R(1,2);
				this->w = R(0,2) - R(2,0);
				break;

			case 2:

				this->x = R(0,2) + R(2,0);
				this->y = R(1,2) + R(2,1);
				this->z = 1 - R(0,0) - R(1,1) + R(2,2);
				this->w = R(1,0) - R(0,1);
				break;
		}
	}

	this->Normalize();
}

void Quat::ConvertToAxisAngle(Vector<3>* axis, double* angle) const
{
	double halfAngle = acosf( W() );
	double s = sinf( halfAngle );

	*angle = 2 * halfAngle;

	if (s == 0)
	{
		*axis = Vector<3>({0, 0, 0});
		return;
	}
	(*axis)[0] = this->x / s;
	(*axis)[0] = this->y / s;
	(*axis)[0] = this->z / s;
}


