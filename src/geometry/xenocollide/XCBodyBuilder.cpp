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

#include "XCBodyBuilder.h"
#include "../../utils/Utils.h"
#include "../../utils/Assertions.h"
#include <list>
#include <sstream>
#include <string>


std::shared_ptr<AbstractXCGeometry> XCBodyBuilder::getCollideGeometry(){
    if (mShapeStack.empty())
        throw ValidationException("BodyBuilder: shape stack empty");
    XCShape& s = *mShapeStack.back();
    return s.geom;
}

void XCBodyBuilder::cuboid(double x, double y, double z){
    auto geom = std::make_shared<CollideBox>(Vector<3>({x, y, z}));
    mShapeStack.push_back(std::make_shared<XCShape>(geom));
}

void XCBodyBuilder::clear(){
    this->mShapeStack.clear();
}

void XCBodyBuilder::diff(){
    if (mShapeStack.size() < 2)
        return;
    auto shape2 = mShapeStack.back();
    mShapeStack.pop_back();

    auto shape1 = mShapeStack.back();
    mShapeStack.pop_back();

    auto newShape = std::make_shared<XCShape>();
    newShape->geom = std::make_shared<CollideDiff>(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
    mShapeStack.push_back(newShape);
}

void XCBodyBuilder::disc(double x){
    auto geom = std::make_shared<CollideDisc>(x);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::ellipse(double x, double y){
    auto geom = std::make_shared<CollideEllipse>(x, y);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::dup(size_t n){
    if (mShapeStack.size() < n)
        return;
    auto it = mShapeStack.end();
    for (size_t i=0; i < n; i++){
        it--;
    }
    for (size_t i=0; i < n; i++){
        auto s = std::make_shared<XCShape>();
        *s = **it;
        mShapeStack.push_back( s );
        it++;
    }
}

void XCBodyBuilder::football(double l, double w){
    auto geom = std::make_shared<CollideFootball>(l,w);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::move(double x, double y, double z){
    if (mShapeStack.size() < 1)
        return;
    mShapeStack.back()->x += Vector<3>({x, y, z});
}

void XCBodyBuilder::point(double x, double y, double z){
    auto geom = std::make_shared<CollidePoint>(Vector<3>({x, y, z}));
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::pop(){
    if (mShapeStack.size() < 1)
        return;
    mShapeStack.pop_back();
}

void::XCBodyBuilder::rect(double x, double y){
    auto geom = std::make_shared<CollideRectangle>(x, y);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::rot(double x, double y, double z){
    if (mShapeStack.size() < 1)
        return;
    mShapeStack.back()->m = Matrix<3,3>::rotation(x*M_PI/180.0, y*M_PI/180.0, z*M_PI/180.0) * mShapeStack.back()->m;
}

void XCBodyBuilder::saucer(double r, double t){
    auto geom = std::make_shared<CollideSaucer>(r,t);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::segment(double l){
    auto geom = std::make_shared<CollideSegment>(l);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::sphere(double r){
    auto geom = std::make_shared<CollideSphere>(r);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::ellipsoid(double rx, double ry, double rz){
    auto geom = std::make_shared<CollideEllipsoid>(Vector<3>({rx, ry, rz}));
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::swap(){
    if (mShapeStack.size() < 2)
        return;
    auto s1 = mShapeStack.back();
    mShapeStack.pop_back();

    auto s2 = mShapeStack.back();
    mShapeStack.pop_back();

    mShapeStack.push_back(s1);
    mShapeStack.push_back(s2);
}

void XCBodyBuilder::sweep(){
    if (mShapeStack.size() < 2)
        return;
    auto shape1 = mShapeStack.back();
    mShapeStack.pop_back();

    auto shape2 = mShapeStack.back();
    mShapeStack.pop_back();

    auto newShape = std::make_shared<XCShape>();
    newShape->geom = std::make_shared<CollideSum>(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
    mShapeStack.push_back(newShape);
}

void XCBodyBuilder::wrap(){
    if (mShapeStack.size() < 2)
        return;
    auto shape1 = mShapeStack.back();
    mShapeStack.pop_back();

    auto shape2 = mShapeStack.back();
    mShapeStack.pop_back();

    auto newShape = std::make_shared<XCShape>();
    newShape->geom = std::make_shared<CollideMax>(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
    mShapeStack.push_back(newShape);
}

//////////////////////////////////////////////////////////////


void XCBodyBuilder::ProcessCommand(std::string commandLine){
    commandLine = trim(commandLine);
    std::stringstream ss(commandLine);
    std::string command;
    ss >> command;

    if (command == "cube"){
        double x, y, z;
        ss >> x;
        ss >> y;
        ss >> z;
        this->cuboid(x, y, z);
    }
    else if (command == "diff"){
        this->diff();
    }
    else if (command == "ellipse"){
        double x;
        ss >> x;
        this->disc(x);
    }
    else if (command == "ellipse"){
        double x, y;
        ss >> x >> y;
        this->ellipse(x, y);
    }
    else if (command == "dup"){
        size_t n;
        ss >> n;
        this->dup(n);
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
        double r;
        ss >> r;
        this->sphere(r);
    }
    else if(command=="ellipsoid"){
        double rx, ry, rz;
        ss >> rx >> ry >> rz;
        this->ellipsoid(rx, ry, rz);
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
