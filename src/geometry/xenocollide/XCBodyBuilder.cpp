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
 * Heavily adapted by Michal Ciesla and Piotr Kubala
 */

#include <list>
#include <sstream>
#include <string>

#include "XCBodyBuilder.h"
#include "utils/Utils.h"
#include "utils/Exceptions.h"
#include "XCPrimitives.h"
#include "XCOperations.h"
#include "utils/ParseUtils.h"


std::shared_ptr<AbstractXCGeometry> XCBodyBuilder::releaseCollideGeometry() {
    if (this->shapeStack.empty()) {
        throw ValidationException("XCBodyBuilder: shape stack is empty");
    } else if (this->shapeStack.size() > 1) {
        throw ValidationException("XCBodyBuilder: shape stack contains more than 1 shape; did you forget to use sum, "
                                  "diff or wrap?");
    }

    auto geom = this->shapeStack.back().geometry;
    this->clear();
    return geom;
}

void XCBodyBuilder::cuboid(double sideX, double sideY, double sideZ) {
    ValidateMsg(sideX > 0 && sideY > 0 && sideZ > 0, "All cuboid side lengths should be positive");
    auto geom = std::make_shared<XCCuboid>(Vector<3>{sideX / 2, sideY / 2, sideZ / 2});
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::clear() {
    this->shapeStack.clear();
}

void XCBodyBuilder::diff() {
    ValidateMsg(this->shapeStack.size() >= 2, "Shape stack contains < 2 shapes; cannot compute Minkowski difference");

    auto shape2 = this->shapeStack.back();
    this->shapeStack.pop_back();

    auto shape1 = this->shapeStack.back();
    this->shapeStack.pop_back();

    auto geom = std::make_shared<XCDiff>(shape1.geometry, shape1.orientation, shape1.pos,
                                         shape2.geometry, shape2.orientation, shape2.pos);
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::disk(double radius) {
    ValidateMsg(radius > 0, "Disk radius should be positive");
    auto geom = std::make_shared<XCDisk>(radius);
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::ellipse(double semiAxisX, double semiAxisY) {
    ValidateMsg(semiAxisX > 0 && semiAxisY > 0, "Ellipse semiaxes' lengths should be positive");
    auto geom = std::make_shared<XCEllipse>(semiAxisX, semiAxisY);
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::ellipsoid(double semiAxisX, double semiAxisY, double semiAxisZ) {
    ValidateMsg(semiAxisX > 0 && semiAxisY > 0 && semiAxisZ > 0, "Ellipsoid semiaxes' lengths should be positive");
    auto geom = std::make_shared<XCEllipsoid>(Vector<3>{semiAxisX, semiAxisY, semiAxisZ});
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::dup(std::size_t numShapes) {
    ValidateMsg(numShapes > 0, "Number of shapes to duplicate should be positive");
    ValidateMsg(numShapes <= this->shapeStack.size(),
                "Number of shapes to duplicate is greater than total number of shapes");

    auto it = this->shapeStack.end();
    std::advance(it, -static_cast<decltype(it)::difference_type>(numShapes));
    for (std::size_t i{}; i < numShapes; i++) {
        this->shapeStack.push_back(*it);
        it++;
    }
}

void XCBodyBuilder::football(double length, double radius) {
    ValidateMsg(length > 0 && radius > 0, "Length and radius of a football should be positive");
    ValidateMsg(length >= 2*radius, "Length of football should not be not smaller than its diameter");
    auto geom = std::make_shared<XCFootball>(length / 2, radius);
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::move(double x, double y, double z) {
    ValidateMsg(!this->shapeStack.empty(), "Shape stack is empty; cannot move");
    this->shapeStack.back().pos += Vector<3>({x, y, z});
}

void XCBodyBuilder::point(double x, double y, double z) {
    auto geom = std::make_shared<XCPoint>(Vector<3>{x, y, z});
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::pop() {
    ValidateMsg(!this->shapeStack.empty(), "Shape stack is empty; cannot pop last element");
    this->shapeStack.pop_back();
}

void XCBodyBuilder::rectangle(double sideX, double sideY) {
    ValidateMsg(sideX > 0 && sideY > 0, "Rectangle side lengths should be positive");
    auto geom = std::make_shared<XCRectangle>(sideX / 2, sideY / 2);
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::rot(double x, double y, double z) {
    ValidateMsg(!this->shapeStack.empty(), "Shape stack is empty; cannot rotate");
    auto &orientation = this->shapeStack.back().orientation;
    orientation = Matrix<3,3>::rotation(x*M_PI/180, y*M_PI/180, z*M_PI/180) * orientation;
}

void XCBodyBuilder::saucer(double radius, double thickness) {
    ValidateMsg(radius > 0 && thickness > 0, "Saucer radius and thickness should be positive");
    ValidateMsg(thickness <= 2*radius, "Saucer thickness cannot be larger than its diameter");
    auto geom = std::make_shared<XCSaucer>(radius, thickness / 2);
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::segment(double length) {
    ValidateMsg(length > 0, "Segment length should be positive");
    auto geom = std::make_shared<XCSegment>(length / 2);
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::sphere(double radius) {
    ValidateMsg(radius > 0, "Sphere radius should be positive");
    auto geom = std::make_shared<XCSphere>(radius);
    this->shapeStack.emplace_back(geom);
}

void XCBodyBuilder::sum(std::size_t count) {
    ValidateMsg(count >= 2, "Minkowski sum requires at least 2 components");
    ValidateMsg(this->shapeStack.size() >= count, "Shape stack contains too few shapes; cannot compute Minkowski sum");

    auto sum = std::make_shared<XCSum>();
    for (std::size_t i{}; i < count; i++) {
        auto shape = this->shapeStack.back();
        this->shapeStack.pop_back();
        sum->add(shape.geometry, shape.pos, shape.orientation);
    }

    this->shapeStack.emplace_back(sum);
}

void XCBodyBuilder::swap() {
    ValidateMsg(this->shapeStack.size() >= 2, "Shape stack contains < 2 shapes; cannot swap them");

    auto it1 = std::prev(this->shapeStack.end());
    auto it2 = std::prev(it1);
    std::swap(*it1, *it2);
}

void XCBodyBuilder::wrap(std::size_t count) {
    ValidateMsg(count >= 2, "Convex hull sum requires at least 2 components");
    ValidateMsg(this->shapeStack.size() >= count, "Shape stack contains too few shapes; cannot compute convex hull");

    auto wrap = std::make_shared<XCMax>();
    for (std::size_t i{}; i < count; i++) {
        auto shape = this->shapeStack.back();
        this->shapeStack.pop_back();
        wrap->add(shape.geometry, shape.pos, shape.orientation);
    }

    this->shapeStack.emplace_back(wrap);
}

void XCBodyBuilder::processCommand(std::string cmd) {
    cmd = trim(cmd);
    std::stringstream cmdStream(cmd);
    std::string commandName;
    cmdStream >> commandName;

    if (commandName == "cuboid") {
        double sideX, sideY, sideZ;
        cmdStream >> sideX >> sideY >> sideZ;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: cuboid [side x] [side y] [side z]");
        this->cuboid(sideX, sideY, sideZ);
    } else if (commandName == "diff") {
        this->diff();
    } else if (commandName == "disk") {
        double radius;
        cmdStream >> radius;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: disk [radius]");
        this->disk(radius);
    } else if (commandName == "dup") {
        std::size_t numShapes;
        cmdStream >> numShapes;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: dup [number of shapes]");
        this->dup(numShapes);
    } else if (commandName == "ellipse") {
        double semiAxisX, semiAxisY;
        cmdStream >> semiAxisX >> semiAxisY;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: ellipse [semi-axis x] [semi-axis y]");
        this->ellipse(semiAxisX, semiAxisY);
    } else if(commandName == "ellipsoid") {
        double semiAxisX, semiAxisY, semiAxisZ;
        cmdStream >> semiAxisX >> semiAxisY >> semiAxisZ;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: ellipsoid [semi-axis x] [semi-axis y] [semi-axis z]");
        this->ellipsoid(semiAxisX, semiAxisY, semiAxisZ);
    } else if (commandName == "football") {
        double length, radius;
        cmdStream >> length >> radius;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: football [length] [radius]");
        this->football(length, radius);
    } else if (commandName == "move") {
        double x, y, z;
        cmdStream >> x >> y >> z;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: move [delta x] [delta y] [delta z]");
        this->move(x, y, z);
    } else if (commandName == "point") {
        double x, y, z;
        cmdStream >> x >> y >> z;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: point [x] [y] [z]");
        this->point(x, y, z);
    } else if (commandName == "pop") {
        this->pop();
    } else if (commandName == "rectangle") {
        double sideX, sideY;
        cmdStream >> sideX >> sideY;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: rectangle [side x] [side y]");
        this->rectangle(sideX, sideY);
    } else if (commandName == "rot") {
        double angleX, angleY, angleZ;
        cmdStream >> angleX >> angleY >> angleZ;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: rot [angle x] [angle y] [angle z]");
        this->rot(angleX, angleY, angleZ);
    } else if (commandName == "saucer") {
        double radius, thickness;
        cmdStream >> radius >> thickness;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: saucer [radius] [thickness]");
        this->saucer(radius, thickness);
    } else if (commandName == "segment") {
        double length;
        cmdStream >> length;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: segment [length]");
        this->segment(length);
    } else if(commandName == "sphere") {
        double radius;
        cmdStream >> radius;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: sphere [radius]");
        this->sphere(radius);
    } else if (commandName == "sum") {
        if (!ParseUtils::isAnythingLeft(cmdStream)) {
            this->sum();
            return;
        }

        std::size_t numShapes;
        cmdStream >> numShapes;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: sum ([number of shapes])");
        this->sum(numShapes);
    } else if (commandName == "swap") {
        this->swap();
    } else if (commandName == "wrap") {
        if (!ParseUtils::isAnythingLeft(cmdStream)) {
            this->wrap();
            return;
        }

        std::size_t numShapes;
        cmdStream >> numShapes;
        ValidateMsg(cmdStream, "Malformed command arguments. Usage: wrap ([number of shapes])");
        this->wrap(numShapes);
    } else {
        throw ValidationException("Unknown XCBodyBuilder command: " + commandName);
    }
}
