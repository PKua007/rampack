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

#include <utility>
#include <vector>

#include "AbstractXCGeometry.h"


/**
 * @brief Creates the Minkowski sum of two AbstractXCGeometry -ies.
 */
class XCSum : public AbstractXCGeometry {
private:
    struct Entry {
        std::shared_ptr<AbstractXCGeometry> geometry;
        Vector<3> pos;
        Matrix<3, 3> rot;

        Entry(std::shared_ptr<AbstractXCGeometry> geometry, const Vector<3> &pos, const Matrix<3, 3> &rot)
                : geometry{std::move(geometry)}, pos{pos}, rot{rot}
        { }
    };

    double circumsphereRadius{};
    double insphereRadius{};
    std::vector<Entry> entries;

    void recalculateGeometry();

public:
    XCSum() = default;

    /**
     * @brief Creates the Minkowski sum for two AbstractXCGeometry -ies with fully specified positions and orientations
     * of them.
     */
    XCSum(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1, const Matrix<3, 3> &rot1,
          std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2, const Matrix<3, 3> &rot2);

    /**
     * @brief Creates the Minkowski sum for two AbstractXCGeometry -ies with fully specified their positions, but
     * default orientations.
     */
    XCSum(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1,
          std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2)
            : XCSum(std::move(geom1), pos1, Matrix<3, 3>::identity(),
                    std::move(geom2), pos2, Matrix<3, 3>::identity())
    { }

    /**
     * @brief Creates the Minkowski sum for two AbstractXCGeometry -ies with default positions and orientations.
     */
    XCSum(std::shared_ptr<AbstractXCGeometry> geom1, std::shared_ptr<AbstractXCGeometry> geom2)
            : XCSum(std::move(geom1), {}, std::move(geom2), {})
    { }

    void add(std::shared_ptr<AbstractXCGeometry> geom, const Vector<3> &pos = {},
             const Matrix<3, 3> &rot = Matrix<3, 3>::identity());

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
    [[nodiscard]] double getInsphereRadius() const override { return this->insphereRadius; }
};


/**
 * @brief Creates the Minkowski difference of two AbstractXCGeometry -ies.
 */
class XCDiff : public AbstractXCGeometry {
private:
    Matrix<3,3> rot1;
    Matrix<3,3> rot2;
    Vector<3> pos1;
    Vector<3> pos2;
    double circumsphereRadius{};
    double insphereRadius{};

    std::shared_ptr<AbstractXCGeometry> geom1;
    std::shared_ptr<AbstractXCGeometry> geom2;

public:
    /**
     * @brief Creates the Minkowski difference for two AbstractXCGeometry -ies with fully specified positions and
     * orientations of them.
     */
    XCDiff(std::shared_ptr<AbstractXCGeometry> geom1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
           std::shared_ptr<AbstractXCGeometry> geom2, const Matrix<3, 3> &rot2, const Vector<3> &pos2);

    /**
     * @brief Creates the Minkowski difference for two AbstractXCGeometry -ies with fully specified their positions, but
     * default orientations.
     */
    XCDiff(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1,
           std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2)
            : XCDiff(std::move(geom1), Matrix<3, 3>::identity(), pos1,
                     std::move(geom2), Matrix<3, 3>::identity(), pos2)
    { }

    /**
     * @brief Creates the Minkowski difference for two AbstractXCGeometry -ies with default positions and orientations.
     */
    XCDiff(std::shared_ptr<AbstractXCGeometry> geom1, std::shared_ptr<AbstractXCGeometry> geom2)
            : XCDiff(std::move(geom1), {}, std::move(geom2), {})
    { }

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
    [[nodiscard]] double getInsphereRadius() const override { return this->insphereRadius; }
};


/**
 * @brief Creates the convex hull of two AbstractXCGeometry -ies.
 */
class XCMax : public AbstractXCGeometry {
private:
    Matrix<3,3> rot1;
    Matrix<3,3> rot2;
    Vector<3> pos1;
    Vector<3> pos2;
    double circumsphereRadius{};
    double insphereRadius{};

    std::shared_ptr<AbstractXCGeometry> geom1;
    std::shared_ptr<AbstractXCGeometry> geom2;

    [[nodiscard]] double calculateInsphereRadius() const;

public:
    /**
     * @brief Creates the convex hull for two AbstractXCGeometry -ies with fully specified positions and orientations of
     * them.
     */
    XCMax(std::shared_ptr<AbstractXCGeometry> geom1, const Matrix<3, 3> &rot1, const Vector<3> &pos1,
          std::shared_ptr<AbstractXCGeometry> geom2, const Matrix<3, 3> &rot2, const Vector<3> &pos2);

    /**
     * @brief Creates the convex hull for two AbstractXCGeometry -ies with fully specified their positions, but default
     * orientations.
     */
    XCMax(std::shared_ptr<AbstractXCGeometry> geom1, const Vector<3> &pos1,
          std::shared_ptr<AbstractXCGeometry> geom2, const Vector<3> &pos2)
            : XCMax(std::move(geom1), Matrix<3, 3>::identity(), pos1,
                    std::move(geom2), Matrix<3, 3>::identity(), pos2)
    { }

    /**
     * @brief Creates the convex hull for two AbstractXCGeometry -ies with default positions and orientations.
     */
    XCMax(std::shared_ptr<AbstractXCGeometry> geom1, std::shared_ptr<AbstractXCGeometry> geom2)
            : XCMax(std::move(geom1), {}, std::move(geom2), {})
    { }

    [[nodiscard]] Vector<3> getSupportPoint(const Vector<3>& n) const override;
    [[nodiscard]] Vector<3> getCenter() const override;
    [[nodiscard]] double getCircumsphereRadius() const override { return this->circumsphereRadius; }
    [[nodiscard]] double getInsphereRadius() const override { return this->insphereRadius; }
};


#endif //RAMPACK_XCOPERATIONS_H
