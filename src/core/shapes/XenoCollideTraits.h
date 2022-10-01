//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_XENOCOLLIDETRAITS_H
#define RAMPACK_XENOCOLLIDETRAITS_H

#include <map>

#include "core/ShapeTraits.h"
#include "geometry/xenocollide/AbstractCollideGeometry.h"


class XenoCollideTraits : public ShapeTraits, public Interaction, public ShapeGeometry {
private:
    Vector<3> primaryAxis;
    Vector<3> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};
    double rangeRadius{};
    std::map<std::string, Vector<3>> customNamedPoints{};

protected:

public:
    std::shared_ptr<AbstractCollideGeometry> shapeModel;

    XenoCollideTraits(Vector<3> pa, Vector<3> sa, Vector<3> cm, double v, const std::string &attr,
                      std::map<std::string, Vector<3>> customNamedPoints = {});
    XenoCollideTraits(Vector<3> pa, Vector<3> sa, Vector<3> cm, double v, std::shared_ptr<AbstractCollideGeometry> shapeModel,
                      double rangeRadius, std::map<std::string, Vector<3>> customNamedPoints = {});

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const final {
        return shape.getOrientation() * this->primaryAxis;
    }

    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const final {
        return shape.getOrientation() * this->secondaryAxis;
    }

    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const final {
        return shape.getOrientation() * this->geometricOrigin;
    }

    [[nodiscard]] double getVolume() const final { return this->volume; }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasWallPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;

    [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, std::size_t idx,
                                      const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override;


    [[nodiscard]] double getRangeRadius() const override { return this->rangeRadius; }

    [[nodiscard]] Vector<3> getNamedPoint(const std::string &pointName, const Shape &shape) const override;
};


#endif //RAMPACK_XENOCOLLIDETRAITS_H
