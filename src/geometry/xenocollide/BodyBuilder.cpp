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
 * Adapted by Michal Ciesla
 */




#include "BodyBuilder.h"
#include "MapPtr.h"
#include "../../utils/Utils.h"
#include "../../utils/Assertions.h"
#include <list>
#include <sstream>
#include <string>

//////////////////////////////////////////////////////////////

BodyBuilder::BodyBuilder()
{

}

//////////////////////////////////////////////////////////////

double BodyBuilder::getMaxRadius()
{
	XCShape& s = *mShapeStack.back();
	CollideGeometry *collideModel = s.geom;
	double radiusNegX = std::abs(collideModel->GetSupportPoint( Vector<3>({-1, 0, 0}) )[0]);
	double radiusPosX = std::abs(collideModel->GetSupportPoint( Vector<3>({1, 0, 0}) )[0]);
	double radiusNegY = std::abs(collideModel->GetSupportPoint( Vector<3>({0, -1, 0}) )[1]);
	double radiusPosY = std::abs(collideModel->GetSupportPoint( Vector<3>({0,  1, 0}) )[1]);
	double radiusNegZ = std::abs(collideModel->GetSupportPoint( Vector<3>({0, 0, -1}) )[2]);
	double radiusPosZ = std::abs(collideModel->GetSupportPoint( Vector<3>({0, 0,  1}) )[2]);

	Vector<3> maxRadiusVector
	({
		std::max(radiusNegX, radiusPosX),
		std::max(radiusNegY, radiusPosY),
		std::max(radiusNegZ, radiusPosZ)
	});

	return maxRadiusVector.norm();
}

MapPtr<CollideGeometry> BodyBuilder::getCollideGeometry(){
	if (mShapeStack.size() < 1)
		throw new ValidationException("BodyBuilder: shape stack empty");
	XCShape& s = *mShapeStack.back();
	return s.geom;
}

size_t BodyBuilder::getModelStackSize(){
	return this->mShapeStack.size();
}



BodyBuilder::~BodyBuilder()
{
	mShapeStack.clear();
}

void BodyBuilder::axis(double x){
	MapPtr<CollideGeometry> geom = new CollideSegment(x);
	mShapeStack.push_back( new XCShape(geom) );
    Matrix<3, 3>::rotation(Vector<3>{0,0,1}, M_PI/2);
	mShapeStack.push_back( new XCShape(geom, Matrix<3, 3>::rotation(Vector<3>{0,0,1}, M_PI/2), Vector<3>({0, 0, 0}) ) );
	mShapeStack.push_back( new XCShape(geom, Matrix<3, 3>::rotation(Vector<3>{0,1,0}, M_PI/2), Vector<3>({0, 0, 0}) ) );
}

void BodyBuilder::box(double x, double y, double z){
	MapPtr<CollideGeometry> geom = new CollideBox(Vector<3>({x, y, z}));
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::clear(){
	this->mShapeStack.clear();
}

void BodyBuilder::diff(){
	if (mShapeStack.size() < 2)
		return;
	MapPtr<XCShape> shape2 = mShapeStack.back();
	mShapeStack.pop_back();

	MapPtr<XCShape> shape1 = mShapeStack.back();
	mShapeStack.pop_back();

	MapPtr<XCShape> newShape = new XCShape();
	newShape->geom = new CollideDiff(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
	mShapeStack.push_back(newShape);
}

void BodyBuilder::disc(double x){
	MapPtr<CollideGeometry> geom = new CollideDisc(x);
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::disc(double x, double y){
	MapPtr<CollideGeometry> geom = new CollideEllipse(x, y);
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::dup(size_t n){
	if (mShapeStack.size() < n)
		return;
	std::list< MapPtr<XCShape> >::iterator it = mShapeStack.end();
	for (size_t i=0; i < n; i++){
			it--;
	}
	for (size_t i=0; i < n; i++){
		MapPtr<XCShape> s = new XCShape();
		*s = **it;
		mShapeStack.push_back( s );
		it++;
	}
}

void BodyBuilder::football(double l, double w){
	MapPtr<CollideGeometry> geom = new CollideFootball(l,w);
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::move(double x, double y, double z){
	if (mShapeStack.size() < 1)
		return;
	mShapeStack.back()->x += Vector<3>({x, y, z});
}

void BodyBuilder::point(double x, double y, double z){
	MapPtr<CollideGeometry> geom = new CollidePoint(Vector<3>({x, y, z}));
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::pop(){
	if (mShapeStack.size() < 1)
		return;
	mShapeStack.pop_back();
}

void::BodyBuilder::rect(double x, double y){
	MapPtr<CollideGeometry> geom = new CollideRectangle(x, y);
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::rot(double x, double y, double z){
	if (mShapeStack.size() < 1)
		return;
	mShapeStack.back()->m = Matrix<3,3>::rotation(x*M_PI/180.0, y*M_PI/180.0, z*M_PI/180.0) * mShapeStack.back()->m;
}

void BodyBuilder::saucer(double r, double t){
	MapPtr<CollideGeometry> geom = new CollideSaucer(r,t);
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::segment(double l){
	MapPtr<CollideGeometry> geom = new CollideSegment(l);
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::sphere(double r){
	MapPtr<CollideGeometry> geom = new CollideSphere(r);
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::sphere(double rx, double ry, double rz){
	MapPtr<CollideGeometry> geom = new CollideEllipsoid(Vector<3>({rx, ry, rz}));
	mShapeStack.push_back( new XCShape(geom) );
}

void BodyBuilder::swap(){
	if (mShapeStack.size() < 2)
		return;
	MapPtr<XCShape> s1 = mShapeStack.back();
	mShapeStack.pop_back();

	MapPtr<XCShape> s2 = mShapeStack.back();
	mShapeStack.pop_back();

	mShapeStack.push_back(s1);
	mShapeStack.push_back(s2);
}

void BodyBuilder::sweep(){
	if (mShapeStack.size() < 2)
		return;
	MapPtr<XCShape> shape1 = mShapeStack.back();
	mShapeStack.pop_back();

	MapPtr<XCShape> shape2 = mShapeStack.back();
	mShapeStack.pop_back();

	MapPtr<XCShape> newShape = new XCShape();
	newShape->geom = new CollideSum(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
	mShapeStack.push_back(newShape);
}

void BodyBuilder::wrap(){
	if (mShapeStack.size() < 2)
		return;
	MapPtr<XCShape> shape1 = mShapeStack.back();
	mShapeStack.pop_back();

	MapPtr<XCShape> shape2 = mShapeStack.back();
	mShapeStack.pop_back();

	MapPtr<XCShape> newShape = new XCShape();
	newShape->geom = new CollideMax(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
	mShapeStack.push_back(newShape);
}

//////////////////////////////////////////////////////////////


void BodyBuilder::ProcessCommand(std::string& commandLine){
	commandLine = trim(commandLine);
	std::stringstream ss(commandLine);
	std::string command;
	ss >> command;

	if (command == "axis"){
		double x;
		ss >> x;
		this->axis(x);
	}
	else if (command == "box"){
		double x, y, z;
		ss >> x;
		ss >> y;
		ss >> z;
		this->box(x, y, z);
	}
	else if (command == "diff"){
		this->diff();
	}
	else if (command == "disc"){
		double x;
		ss >> x;
		if(!ss.eof()){
			double y;
			ss >> y;
			this->disc(x, y);
		}else{
			this->disc(x);
		}
	}
	else if (command == "dup"){
		size_t n;
		ss >> n;
		this->dup(n);
	}
	else if (command == "script"){
		std::string commands;
		std::getline(ss, commands, '\0');
		std::stringstream commandsStream(commands);
		std::string cmdLine;
		while (std::getline(commandsStream, cmdLine, '&'))
			this->ProcessCommand(cmdLine);
	}
	else if (command == "football"){
		double l, w;
		ss >> l;
		ss >> w;
		this->football(l, w);
	}
	else if (command == "move"){
		double x, y, z;
		ss >> x;
		ss >> y;
		ss >> z;
		this->move(x, y, z);
	}
	else if (command == "point"){
		double x, y, z;
		ss >> x;
		ss >> y;
		ss >> z;
		this->point(x, y, z);
	}
	else if (command == "pop"){
		this->pop();
	}
	else if (command == "rect"){
		double x, y;
		ss >> x;
		ss >> y;
		this->rect(x, y);
	}
	else if (command == "rot"){
		double x, y, z;
		ss >> x;
		ss >> y;
		ss >> z;
		this->rot(x, y, z);
	}
	else if (command == "saucer"){
		double r, t;
		ss >> r;
		ss >> t;
		this->saucer(r, t);
	}
	else if (command == "segment"){
		double x;
		ss >> x;
		this->segment(x);
	}
	else if(command=="sphere"){
		double rx, ry, rz;
		ss >> rx;
		if(!ss.eof()){
			ss >> ry;
			ss >> rz;
			this->sphere(rx, ry, rz);
		}else{
			this->sphere(rx);
		}
	}
	else if (command == "swap"){
		this->swap();
	}
	else if (command == "sweep"){
		this->sweep();
	}
	else if (command == "wrap"){
		this->wrap();
	}
}

//////////////////////////////////////////////////////////////
