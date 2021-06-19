//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_ORTHOROMBICARRANGINGMODEL_H
#define RAMPACK_ORTHOROMBICARRANGINGMODEL_H

#include <vector>
#include <array>

#include "core/Shape.h"
#include "core/BoundaryConditions.h"

class OrthorombicArrangingModel {
public:
    enum class PolarAxis {
        X,
        Y,
        Z
    };

private:
    bool polar = false;
    std::size_t axisNum = 0;

public:
    [[nodiscard]] static std::size_t getAxisNumber(PolarAxis axis);

    explicit OrthorombicArrangingModel(bool polar = false, PolarAxis axis = PolarAxis::X)
            : polar{polar}, axisNum{getAxisNumber(axis)}
    { }

    [[nodiscard]] std::vector<Shape> arrange(std::size_t numOfParticles, const std::array<double, 3> &dimensions) const;
    [[nodiscard]] std::vector<Shape> arrange(std::size_t numOfParticles,
                                             const std::array<std::size_t, 3> &particlesInLine,
                                             const std::array<double, 3> &cellDimensions,
                                             const std::array<double, 3> &boxDimensions) const;
};


#endif //RAMPACK_ORTHOROMBICARRANGINGMODEL_H
