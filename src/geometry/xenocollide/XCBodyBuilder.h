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

#ifndef RAMPACK_XCBODYBUILDER_H
#define RAMPACK_XCBODYBUILDER_H

#include <list>
#include <memory>
#include <utility>

#include "AbstractXCGeometry.h"
#include "geometry/Vector.h"


class XCBodyBuilder {
private:
    struct XCShape {
        std::shared_ptr<AbstractXCGeometry> geometry;
        Matrix<3 ,3> orientation;
        Vector<3> pos;

        XCShape(std::shared_ptr<AbstractXCGeometry> geometry, const Matrix<3, 3> &orientation, const Vector<3> &pos)
                : geometry{std::move(geometry)}, orientation{orientation}, pos{pos}
        { }

        explicit XCShape(std::shared_ptr<AbstractXCGeometry> geometry)
                : geometry{std::move(geometry)}, orientation{Matrix<3, 3>::identity()}, pos{}
        { }
    };

    std::list<XCShape> shapeStack;

public:
    // shapes
    void bullet(double lengthTip, double lengthTail, double radius);
    void cuboid(double sideX, double sideY, double sideZ);
    void disk(double radius);
    void ellipse(double semiAxisX, double semiAxisY);
    void ellipsoid(double semiAxisX, double semiAxisY, double semiAxisZ);
    void football(double length, double radius);
    void point(double x, double y, double z);
    void rectangle(double sideX, double sideY);
    void saucer(double radius, double thickness);
    void segment(double length);
    void sphere(double radius);

    // shapes transformations
    void move(double x, double y, double z);
    void rot(double x, double y, double z);

    // shape combination
    void diff();
    void sum();
    void wrap();

    // stack operations
    void dup(std::size_t numShapes);
    void pop();
    void swap();
    void clear();

    void processCommand(std::string cmd);
    std::shared_ptr<AbstractXCGeometry> releaseCollideGeometry();
};


#endif //RAMPACK_XCBODYBUILDER_H
