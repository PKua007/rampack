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

#ifndef RAMPACK_ABSTRACTXCGEOMETRY_H
#define RAMPACK_ABSTRACTXCGEOMETRY_H


#include <algorithm>
#include <memory>

#include "geometry/Vector.h"


class AbstractXCGeometry {
public:
    virtual ~AbstractXCGeometry() = default;

    [[nodiscard]] virtual Vector<3> getSupportPoint(const Vector<3>& n) const = 0;
    [[nodiscard]] virtual Vector<3> getCenter() const;
    [[nodiscard]] virtual double getCircumsphereRadius() const { return 0; };
};


template<typename CollideGeometry>
class PolymorphicCollideAdapter : public AbstractXCGeometry {
private:
    CollideGeometry geometry;

public:
    explicit PolymorphicCollideAdapter(CollideGeometry geometry) : geometry{std::move(geometry)} { }

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3> &n) const override {
        return this->geometry.getSupportPoint(n);
    }
    [[nodiscard]] Vector<3> getCenter() const override { return this->geometry.getCenter(); }
    [[nodiscard]] double getCircumsphereRadius() const override { return this->geometry.getCircumsphereRadius(); }
};


#endif //RAMPACK_ABSTRACTXCGEOMETRY_H