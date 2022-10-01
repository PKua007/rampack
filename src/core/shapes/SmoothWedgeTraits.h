//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_SMOOTHWEDGETRAITS_H
#define RAMPACK_SMOOTHWEDGETRAITS_H

#include "XenoCollideTraits.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"


class SmoothWedgeTraits : public XenoCollideTraits<SmoothWedgeTraits>, public ShapePrinter {
private:
    double R{};
    double r{};
    double l{};
    std::shared_ptr<AbstractXCGeometry> shapeModel;

    static std::shared_ptr<AbstractXCGeometry> createShapeModel(double R, double r, double l);
    static double getVolume(double R, double r, double l);

public:
    SmoothWedgeTraits(double R, double r, double l);

    const AbstractXCGeometry &getCollideGeometry() const { return *shapeModel; }

    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;
};


#endif //RAMPACK_SMOOTHWEDGETRAITS_H
