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

#ifndef RAMPACK_XCOPERATIONS_H
#define RAMPACK_XCOPERATIONS_H

#include "AbstractXCGeometry.h"


class CollideSum : public AbstractXCGeometry {
private:
    Matrix<3,3> rot1;
    Matrix<3,3> rot2;
    Vector<3> pos1;
    Vector<3> pos2;
    double circumsphereRadius{};

    std::shared_ptr<AbstractXCGeometry> geom1;
    std::shared_ptr<AbstractXCGeometry> geom2;

public:
    CollideSum(std::shared_ptr<AbstractXCGeometry> geom1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
               std::shared_ptr<AbstractXCGeometry> geom2, const Matrix<3, 3> &rot2, const Vector<3> &pos2);

    CollideSum(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1,
               std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2)
            : CollideSum(std::move(geom1), Matrix<3, 3>::identity(), pos1,
                         std::move(geom2), Matrix<3, 3>::identity(), pos2)
    { }

    CollideSum(std::shared_ptr<AbstractXCGeometry> geom1, std::shared_ptr<AbstractXCGeometry> geom2)
            : CollideSum(std::move(geom1), {}, std::move(geom2), {})
    { }

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};


class CollideDiff : public AbstractXCGeometry {
private:
    Matrix<3,3> rot1;
    Matrix<3,3> rot2;
    Vector<3> pos1;
    Vector<3> pos2;
    double circumsphereRadius{};

    std::shared_ptr<AbstractXCGeometry> geom1;
    std::shared_ptr<AbstractXCGeometry> geom2;

public:
    CollideDiff(std::shared_ptr<AbstractXCGeometry> geom1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
                std::shared_ptr<AbstractXCGeometry> geom2, const Matrix<3, 3> &rot2, const Vector<3> &pos2);

    CollideDiff(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1,
                std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2)
            : CollideDiff(std::move(geom1), Matrix<3, 3>::identity(), pos1,
                          std::move(geom2), Matrix<3, 3>::identity(), pos2)
    { }

    CollideDiff(std::shared_ptr<AbstractXCGeometry> geom1, std::shared_ptr<AbstractXCGeometry> geom2)
            : CollideDiff(std::move(geom1), {}, std::move(geom2), {})
    { }

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};


class CollideMax : public AbstractXCGeometry {
private:
    Matrix<3,3> rot1;
    Matrix<3,3> rot2;
    Vector<3> pos1;
    Vector<3> pos2;
    double circumsphereRadius{};

    std::shared_ptr<AbstractXCGeometry> geom1;
    std::shared_ptr<AbstractXCGeometry> geom2;

public:
    CollideMax(std::shared_ptr<AbstractXCGeometry> geom1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
               std::shared_ptr<AbstractXCGeometry> geom2, const Matrix<3, 3> &rot2, const Vector<3> &pos2);

    CollideMax(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1,
               std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2)
            : CollideMax(std::move(geom1), Matrix<3, 3>::identity(), pos1,
                         std::move(geom2), Matrix<3, 3>::identity(), pos2)
    { }

    CollideMax(std::shared_ptr<AbstractXCGeometry> geom1, std::shared_ptr<AbstractXCGeometry> geom2)
            : CollideMax(std::move(geom1), {}, std::move(geom2), {})
    { }

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
};


#endif //RAMPACK_XCOPERATIONS_H
