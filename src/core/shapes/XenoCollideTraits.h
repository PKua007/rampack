//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_XENOCOLLIDETRAITS_H
#define RAMPACK_XENOCOLLIDETRAITS_H

#include "core/ShapeTraits.h"
#include "geometry/xenocollide/CollideGeometry.h"
#include "geometry/xenocollide/MapPtr.h"


class XenoCollideTraits : public ShapeTraits, public Interaction, public ShapeGeometry {
private:
    MapPtr<CollideGeometry> shapeModel;
    Vector<3> primaryAxis;
    Vector<3> secondaryAxis;
    Vector<3> geometricOrigin;
    double volume{};
    double rangeRadius{};

public:
    XenoCollideTraits(Vector<3> pa, Vector<3> sa, Vector<3> cm, double v, const std::string &attr);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override {
        return shape.getOrientation() * this->primaryAxis;
    }

    [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const override {
        return shape.getOrientation() * this->secondaryAxis;
    }

    [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const override {
        return shape.getOrientation() * this->geometricOrigin;
    }

    [[nodiscard]] double getVolume() const override { return this->volume; }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasWallPart() const override { return false; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;

    [[nodiscard]] double getRangeRadius() const override { return this->rangeRadius; }
};


#endif //RAMPACK_XENOCOLLIDETRAITS_H
