//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_ROUNDEDCONETRAITS_H
#define RAMPACK_ROUNDEDCONETRAITS_H

#include "XenoCollideTraits.h"
#include "geometry/xenocollide/AbstractCollideGeometry.h"


class RoundedConeTraits : public XenoCollideTraits<RoundedConeTraits>, public ShapePrinter {
private:
    double R{};
    double r{};
    double l{};
    std::shared_ptr<AbstractCollideGeometry> shapeModel;

    static std::shared_ptr<AbstractCollideGeometry> createShapeModel(double R, double r, double l);
    static double getVolume(double R, double r, double l);

public:
    RoundedConeTraits(double R, double r, double l);

    const AbstractCollideGeometry &getCollideGeometry() const { return *shapeModel; }

    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;
};


#endif //RAMPACK_ROUNDEDCONETRAITS_H
