/*
 * BodyBuilder.h
 *
 *  Created on: Aug 5, 2022
 *      Author: Michal Ciesla
 */

#ifndef GEOMETRY_XENOCOLLIDE_BODYBUILDER_H_
#define GEOMETRY_XENOCOLLIDE_BODYBUILDER_H_

#include <list>
#include <memory>
#include <utility>
#include "CollideGeometry.h"
#include "../Vector.h"

class BodyBuilder
{

public:

	BodyBuilder();
	~BodyBuilder();

	// shapes
	void axis(double x);
	void box(double x, double y, double z);
	void disc(double x);
	void disc(double x, double y);
	void football(double l, double w);
	void point(double x, double y, double z);
	void rect(double x, double y);
	void saucer(double r, double t);
	void segment(double l);
	void sphere(double x);
	void sphere(double x, double y, double z);

	// shapes transformations
	void move(double x, double y, double z);
	void rot(double x, double y, double z);

	// shape combination
	void diff();
	void sweep();
	void wrap();

	// stack operations
	void dup(size_t n);
	void pop();
	void swap();
	void clear();

	void ProcessCommand(std::string& cmd);
	std::shared_ptr<CollideGeometry> getCollideGeometry();
	double getMaxRadius();
	size_t getModelStackSize();

private:
	struct XCShape{
		XCShape() : geom(nullptr) { m = Matrix<3,3>::identity(); x = Vector<3>({0, 0, 0}); }
		XCShape(std::shared_ptr<CollideGeometry> _geom, const Matrix<3,3>& _m, const Vector<3>& _x) : geom(std::move(_geom)), m(_m), x(_x) {}
		explicit XCShape(std::shared_ptr<CollideGeometry>  _geom) : geom(std::move(_geom)) { m = Matrix<3,3>::identity(); x = Vector<3>({0, 0, 0}); }
		std::shared_ptr<CollideGeometry>	geom;
		Matrix<3,3>				m;
		Vector<3>			    x;
	};
	std::list< std::shared_ptr<XCShape> > mShapeStack;
};


#endif /* GEOMETRY_XENOCOLLIDE_BODYBUILDER_H_ */
