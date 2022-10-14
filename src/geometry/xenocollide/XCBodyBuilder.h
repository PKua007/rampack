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


/**
 * @brief Class constructing complex shapes with XenoCollide primitives and operations.
 * @details It contains a stack on which the shapes are added to be later combined using Minkowski sum, difference,
 * convex hull, etc. Each added shape has default position and orientation. It can be rotated and moved using move() and
 * rot() methods. Please note that position and orientation are applied only when combining two shapes. Position and
 * orientation of a single shape are ignored when exporting the geometry by releaseCollideGeometry()
 */
class XCBodyBuilder {
private:
    struct XCShape {
        std::shared_ptr<AbstractXCGeometry> geometry;
        Matrix<3, 3> orientation;
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
    /** @brief Creates a XCCuboid shape */
    void cuboid(double sideX, double sideY, double sideZ);
    /** @brief Creates a XCDisk shape */
    void disk(double radius);
    /** @brief Creates a XCEllipse shape */
    void ellipse(double semiAxisX, double semiAxisY);
    /** @brief Creates a XCEllipsoid shape */
    void ellipsoid(double semiAxisX, double semiAxisY, double semiAxisZ);
    /** @brief Creates a XCFootball shape */
    void football(double length, double radius);
    /** @brief Creates a XCPoint shape */
    void point(double x, double y, double z);
    /** @brief Creates a XCRectangle shape */
    void rectangle(double sideX, double sideY);
    /** @brief Creates a XCSaucer shape */
    void saucer(double radius, double thickness);
    /** @brief Creates a XCSegment shape */
    void segment(double length);
    /** @brief Creates a XCSphere shape */
    void sphere(double radius);

    /** @brief Moves last shape in the stack by vector {@a x, @a y, @a z} */
    void move(double x, double y, double z);
    /** @brief Rotates last shape in the stack by angles @a ax, @a ay, @a az (in degrees) around axes X, Y, Z in exactly
     * that order */
    void rot(double x, double y, double z);

    /** @brief Computes Minkowski difference of last two shapes in the stack. The sum replaces those two in the
     * stack. */
    void diff();
    /** @brief Computes Minkowski sum of last two shapes in the stack. The sum replaces those two in the stack. */
    void sum();
    /** @brief Computes convex hull of last two shapes in the stack. The sum replaces those two in the stack. */
    void wrap();

    /** @brief Duplicates @a numShape last entries in the stack. */
    void dup(std::size_t numShapes);
    /** @brief Removes last entry from the stack. */
    void pop();
    /** @brief Swaps last two entries in the stack. */
    void swap();
    /** @brief Clears the stack. */
    void clear();

    /**
     * @brief Processed the command @a cmd.
     * @details The commands names are identical as all methods used to manipulate the shape stack (excluding clear())
     * with identical, space separated arguments. For example, to invoke XCBodyBuilder::saucer (2, 1) the command
     * "saucer 2 1" should be used.
     */
    void processCommand(std::string cmd);

    /**
     * @brief Releases the AbstractXCGeometry built on a stack and clears it.
     * @details The stack has to contain exactly one shape, otherwise, the error is reported (combining operations should
     * be fully performed). Position and orientation of that last shape changed using move() and rot() are ignored.
     */
    std::shared_ptr<AbstractXCGeometry> releaseCollideGeometry();
};


#endif //RAMPACK_XCBODYBUILDER_H
