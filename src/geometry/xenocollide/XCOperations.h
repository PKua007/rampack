//
// Created by pkua on 07.10.22.
//

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
