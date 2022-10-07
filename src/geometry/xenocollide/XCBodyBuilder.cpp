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

#include <list>
#include <sstream>
#include <string>

#include "XCBodyBuilder.h"
#include "utils/Utils.h"
#include "utils/Assertions.h"


std::shared_ptr<AbstractXCGeometry> XCBodyBuilder::getCollideGeometry(){
    if (mShapeStack.empty())
        throw ValidationException("BodyBuilder: shape stack empty");
    XCShape& s = *mShapeStack.back();
    return s.geom;
}

void XCBodyBuilder::cuboid(double sideX, double sideY, double sideZ){
    Validate(sideX > 0 && sideY > 0 && sideZ > 0);
    auto geom = std::make_shared<CollideBox>(Vector<3>({sideX / 2, sideY / 2, sideZ / 2}));
    mShapeStack.push_back(std::make_shared<XCShape>(geom));
}

void XCBodyBuilder::clear(){
    this->mShapeStack.clear();
}

void XCBodyBuilder::diff(){
    ValidateMsg(this->mShapeStack.size() >= 2, "Shape stack contains < 2 shapes; cannot compute Minkowski difference");

    auto shape2 = mShapeStack.back();
    mShapeStack.pop_back();

    auto shape1 = mShapeStack.back();
    mShapeStack.pop_back();

    auto newShape = std::make_shared<XCShape>();
    newShape->geom = std::make_shared<CollideDiff>(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
    mShapeStack.push_back(newShape);
}

void XCBodyBuilder::disk(double radius){
    Validate(radius > 0);
    auto geom = std::make_shared<CollideDisc>(radius);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::ellipse(double semiAxisX, double semiAxisY){
    Validate(semiAxisX > 0 && semiAxisY > 0);
    auto geom = std::make_shared<CollideEllipse>(semiAxisX, semiAxisY);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::dup(std::size_t numShapes){
    Validate(numShapes > 0);
    if (mShapeStack.size() < numShapes)
        return;
    auto it = mShapeStack.end();
    for (size_t i=0; i < numShapes; i++){
        it--;
    }
    for (size_t i=0; i < numShapes; i++){
        auto s = std::make_shared<XCShape>();
        *s = **it;
        mShapeStack.push_back( s );
        it++;
    }
}

void XCBodyBuilder::football(double length, double radius){
    Validate(length > 0 && radius > 0);
    Validate(length >= 2*radius);
    auto geom = std::make_shared<CollideFootball>(length/2, radius);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::move(double x, double y, double z){
    ValidateMsg(!this->mShapeStack.empty(), "Shape stack is empty; cannot move");
    mShapeStack.back()->x += Vector<3>({x, y, z});
}

void XCBodyBuilder::point(double x, double y, double z){
    auto geom = std::make_shared<CollidePoint>(Vector<3>({x, y, z}));
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::pop(){
    ValidateMsg(!this->mShapeStack.empty(), "Shape stack is empty; cannot pop last element");
    mShapeStack.pop_back();
}

void::XCBodyBuilder::rectangle(double sideX, double sideY){
    Validate(sideX > 0 && sideY > 0);
    auto geom = std::make_shared<CollideRectangle>(sideX / 2, sideY / 2);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::rot(double x, double y, double z){
    ValidateMsg(!this->mShapeStack.empty(), "Shape stack is empty; cannot rotate");
    mShapeStack.back()->m = Matrix<3,3>::rotation(x*M_PI/180.0, y*M_PI/180.0, z*M_PI/180.0) * mShapeStack.back()->m;
}

void XCBodyBuilder::saucer(double radius, double thickness){
    Validate(radius > 0 && thickness > 0);
    Validate(thickness <= 2*radius);
    auto geom = std::make_shared<CollideSaucer>(radius, thickness/2);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::segment(double length){
    Validate(length > 0);
    auto geom = std::make_shared<CollideSegment>(length / 2);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::sphere(double radius){
    Validate(radius > 0);
    auto geom = std::make_shared<CollideSphere>(radius);
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::ellipsoid(double semiAxisX, double semiAxisY, double semiAxisZ){
    Validate(semiAxisX > 0 && semiAxisY > 0 && semiAxisZ > 0);
    auto geom = std::make_shared<CollideEllipsoid>(Vector<3>({semiAxisX, semiAxisY, semiAxisZ}));
    mShapeStack.push_back( std::make_shared<XCShape>(geom) );
}

void XCBodyBuilder::swap(){
    ValidateMsg(this->mShapeStack.size() >= 2, "Shape stack contains < 2 shapes; cannot swap them");

    auto s1 = mShapeStack.back();
    mShapeStack.pop_back();

    auto s2 = mShapeStack.back();
    mShapeStack.pop_back();

    mShapeStack.push_back(s1);
    mShapeStack.push_back(s2);
}

void XCBodyBuilder::sum(){
    ValidateMsg(this->mShapeStack.size() >= 2, "Shape stack contains < 2 shapes; cannot compute Minkowski sum");

    auto shape1 = mShapeStack.back();
    mShapeStack.pop_back();

    auto shape2 = mShapeStack.back();
    mShapeStack.pop_back();

    auto newShape = std::make_shared<XCShape>();
    newShape->geom = std::make_shared<CollideSum>(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
    mShapeStack.push_back(newShape);
}

void XCBodyBuilder::wrap(){
    ValidateMsg(this->mShapeStack.size() >= 2, "Shape stack contains < 2 shapes; cannot compute convex hull");

    auto shape1 = mShapeStack.back();
    mShapeStack.pop_back();

    auto shape2 = mShapeStack.back();
    mShapeStack.pop_back();

    auto newShape = std::make_shared<XCShape>();
    newShape->geom = std::make_shared<CollideMax>(shape1->geom, shape1->m, shape1->x, shape2->geom, shape2->m, shape2->x);
    mShapeStack.push_back(newShape);
}

void XCBodyBuilder::processCommand(std::string cmd){
    cmd = trim(cmd);
    std::stringstream ss(cmd);
    std::string command;
    ss >> command;

    if (command == "cuboid"){
        double sideX, sideY, sideZ;
        ss >> sideX >> sideY >> sideZ;
        ValidateMsg(ss, "Malformed command arguments. Usage: cuboid [side x] [side y] [side z]");
        this->cuboid(sideX, sideY, sideZ);
    }
    else if (command == "diff"){
        this->diff();
    }
    else if (command == "disk"){
        double radius;
        ss >> radius;
        ValidateMsg(ss, "Malformed command arguments. Usage: disk [radius]");
        this->disk(radius);
    }
    else if (command == "dup"){
        std::size_t numShapes;
        ss >> numShapes;
        ValidateMsg(ss, "Malformed command arguments. Usage: dup [number of shapes]");
        this->dup(numShapes);
    }
    else if (command == "ellipse"){
        double semiAxisX, semiAxisY;
        ss >> semiAxisX >> semiAxisY;
        ValidateMsg(ss, "Malformed command arguments. Usage: ellipse [semi-axis x] [semi-axis y]");
        this->ellipse(semiAxisX, semiAxisY);
    }
    else if(command == "ellipsoid"){
        double semiAxisX, semiAxisY, semiAxisZ;
        ss >> semiAxisX >> semiAxisY >> semiAxisZ;
        ValidateMsg(ss, "Malformed command arguments. Usage: ellipsoid [semi-axis x] [semi-axis y] [semi-axis z]");
        this->ellipsoid(semiAxisX, semiAxisY, semiAxisZ);
    }
    else if (command == "football"){
        double length, radius;
        ss >> length >> radius;
        ValidateMsg(ss, "Malformed command arguments. Usage: football [length] [radius]");
        this->football(length, radius);
    }
    else if (command == "move"){
        double x, y, z;
        ss >> x >> y >> z;
        ValidateMsg(ss, "Malformed command arguments. Usage: move [delta x] [delta y] [delta z]");
        this->move(x, y, z);
    }
    else if (command == "point"){
        double x, y, z;
        ss >> x  >> y >> z;
        ValidateMsg(ss, "Malformed command arguments. Usage: point [x] [y] [z]");
        this->point(x, y, z);
    }
    else if (command == "pop"){
        this->pop();
    }
    else if (command == "rectangle"){
        double sideX, sideY;
        ss >> sideX  >> sideY;
        ValidateMsg(ss, "Malformed command arguments. Usage: rectangle [side x] [side y]");
        this->rectangle(sideX, sideY);
    }
    else if (command == "rot"){
        double angleX, angleY, angleZ;
        ss >> angleX >> angleY >> angleZ;
        ValidateMsg(ss, "Malformed command arguments. Usage: rot [angle x] [angle y] [angle z]");
        this->rot(angleX, angleY, angleZ);
    }
    else if (command == "saucer"){
        double radius, thickness;
        ss >> radius >> thickness;
        ValidateMsg(ss, "Malformed command arguments. Usage: saucer [radius] [thickness]");
        this->saucer(radius, thickness);
    }
    else if (command == "segment"){
        double length;
        ss >> length;
        ValidateMsg(ss, "Malformed command arguments. Usage: segment [length]");
        this->segment(length);
    }
    else if(command=="sphere"){
        double radius;
        ss >> radius;
        ValidateMsg(ss, "Malformed command arguments. Usage: sphere [radius]");
        this->sphere(radius);
    }
    else if (command == "sum"){
        this->sum();
    }
    else if (command == "swap"){
        this->swap();
    }
    else if (command == "wrap"){
        this->wrap();
    } else {
        throw ValidationException("Unknown XCBodyBuilder command: " + command);
    }
}
