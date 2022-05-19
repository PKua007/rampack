//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_ORTHORHOMBICARRANGINGMODEL_H
#define RAMPACK_ORTHORHOMBICARRANGINGMODEL_H

#include <vector>
#include <array>

#include "core/Shape.h"
#include "core/BoundaryConditions.h"
#include "LatticeTraits.h"

/**
 * @brief A class arranging shapes into orthorhombic lattice with a single molecule within a unit cell.
 */
class OrthorhombicArrangingModel {
public:
    using Axis = LatticeTraits::Axis;
    using Polarization = LatticeTraits::Polarization;
    using Clinicity = LatticeTraits::Clinicity;

private:
    Polarization polarization = Polarization::IMPLICIT;
    std::size_t polarAxisNum = 0;
    Clinicity clinicity = Clinicity::IMPLICIT;
    std::size_t tiltAxisNum = 0;
    double tiltAngle = 0;

public:
    /**
     * @brief Translates Axis @a axis to integer axis index (x = 0, y = 1, z = 2).
     */
    [[nodiscard]] static std::size_t getAxisNumber(Axis axis);

    /**
     * @brief Prepare the arranging model.
     * @param polarization polarization of layers
     * @param polarAxis layer wavevector (for any @a polarization apart from ANTIPOLAR it does not do anything)
     * @param clinicity layer clinicity
     * @param tiltAxis axis around which the shapes should be tilted
     * @param tiltAngle angle of layer tilt
     */
    explicit OrthorhombicArrangingModel(Polarization polarization = Polarization::IMPLICIT, Axis polarAxis = Axis::X,
                                        Clinicity clinicity = Clinicity::SYNCLINIC, Axis tiltAxis = Axis::X,
                                        double tiltAngle = 0)
            : polarization{polarization}, polarAxisNum{getAxisNumber(polarAxis)}, clinicity{clinicity},
              tiltAxisNum{getAxisNumber(tiltAxis)}, tiltAngle{tiltAngle}
    { }

    /**
     * @brief Arrange the shapes in @a ceil(cbrt(numOfParticle))^3 lattice in box of dimensions @a dimensions.
     */
    [[nodiscard]] std::vector<Shape> arrange(std::size_t numOfParticles, const std::array<double, 3> &dimensions) const;

    /**
     * @brief Arrange lattice with number of cell in each direction given by @a particlesInLine, unit cell dimensions
     * @a cellDimensions and box dimensions @a boxDimensions.
     * @details If box dimensions are bigger than the lattice size, the lattice will be placed in the middle of the box.
     */
    [[nodiscard]] std::vector<Shape> arrange(std::size_t numOfParticles,
                                             const std::array<std::size_t, 3> &particlesInLine,
                                             const std::array<double, 3> &cellDimensions,
                                             const std::array<double, 3> &boxDimensions) const;
};


#endif //RAMPACK_ORTHORHOMBICARRANGINGMODEL_H
