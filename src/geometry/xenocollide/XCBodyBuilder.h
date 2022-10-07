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

#ifndef RAMPACK_XCBODYBUILDER_H
#define RAMPACK_XCBODYBUILDER_H

#include <list>
#include <memory>
#include <utility>
#include "AbstractXCGeometry.h"
#include "../Vector.h"


class XCBodyBuilder
{

public:
    // shapes
    void cuboid(double sideX, double sideY, double sideZ);
    void disk(double radius);
    void ellipse(double semiAxisX, double semiAxisY);
    void football(double length, double radius);
    void point(double x, double y, double z);
    void rectangle(double sideX, double sideY);
    void saucer(double radius, double thickness);
    void segment(double length);
    void sphere(double radius);
    void ellipsoid(double semiAxisX, double semiAxisY, double semiAxisZ);

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
    std::shared_ptr<AbstractXCGeometry> getCollideGeometry();

private:
    struct XCShape{
        XCShape() : geom(nullptr) { m = Matrix<3,3>::identity(); x = Vector<3>({0, 0, 0}); }
        XCShape(std::shared_ptr<AbstractXCGeometry> _geom, const Matrix<3,3>& _m, const Vector<3>& _x) : geom(std::move(_geom)), m(_m), x(_x) {}
        explicit XCShape(std::shared_ptr<AbstractXCGeometry>  _geom) : geom(std::move(_geom)) { m = Matrix<3,3>::identity(); x = Vector<3>({0, 0, 0}); }
        std::shared_ptr<AbstractXCGeometry>    geom;
        Matrix<3,3>                m;
        Vector<3>                x;
    };
    std::list< std::shared_ptr<XCShape> > mShapeStack;
};


#endif //RAMPACK_XCBODYBUILDER_H
