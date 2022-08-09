//
// Created by ciesla on 8/9/22.
//

#ifndef RAMPACK_ROUNDEDCONETRAITS_H
#define RAMPACK_ROUNDEDCONETRAITS_H

#include "XenoCollideTraits.h"


class RoundedConeTraits : public XenoCollideTraits, public ShapePrinter {
private:
    double R{};
    double r{};
    double l{};

    static MapPtr<CollideGeometry> createShapeModel(double r, double l);
    static double getVolume(double R, double r, double l);

public:
    RoundedConeTraits(double r, double l);

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;
};


#endif //RAMPACK_ROUNDEDCONETRAITS_H
