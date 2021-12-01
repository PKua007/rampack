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
    enum class Axis {
        X,
        Y,
        Z
    };

    enum class Clinicity {
        IMPLICIT,
        SYNCLINIC,
        ANTICLINIC
    };

    enum class Polarization {
        IMPLICIT,
        FERRO,
        ANTIFERRO
    };

private:
    Polarization polarization = Polarization::IMPLICIT;
    std::size_t polarAxisNum = 0;
    Clinicity clinicity = Clinicity::IMPLICIT;
    std::size_t tiltAxisNum = 0;
    double tiltAngle = 0;

public:
    [[nodiscard]] static std::size_t getAxisNumber(Axis axis);

    explicit OrthorombicArrangingModel(Polarization polarization = Polarization::IMPLICIT, Axis polarAxis = Axis::X,
                                       Clinicity clinicity = Clinicity::SYNCLINIC, Axis tiltAxis = Axis::X,
                                       double tiltAngle = 0)
            : polarization{polarization}, polarAxisNum{getAxisNumber(polarAxis)}, clinicity{clinicity},
              tiltAxisNum{getAxisNumber(tiltAxis)}, tiltAngle{tiltAngle}
    { }

    [[nodiscard]] std::vector<Shape> arrange(std::size_t numOfParticles, const std::array<double, 3> &dimensions) const;
    [[nodiscard]] std::vector<Shape> arrange(std::size_t numOfParticles,
                                             const std::array<std::size_t, 3> &particlesInLine,
                                             const std::array<double, 3> &cellDimensions,
                                             const std::array<double, 3> &boxDimensions) const;
};


#endif //RAMPACK_ORTHOROMBICARRANGINGMODEL_H
