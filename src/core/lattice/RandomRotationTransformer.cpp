//
// Created by Piotr Kubala on 20/06/2023.
//

#include "RandomRotationTransformer.h"


void RandomRotationTransformer::transform(Lattice &lattice, [[maybe_unused]] const ShapeTraits &shapeTraits) const {
    auto dim = lattice.getDimensions();
    for (std::size_t i{}; i < dim[0]; i++)
        for (std::size_t j{}; j < dim[1]; j++)
            for (std::size_t k{}; k < dim[2]; k++)
                for (auto &shape : lattice.modifySpecificCellMolecules(i, j, k))
                    this->rotateRandomly(shape);
}

void RandomRotationTransformer::rotateRandomly(Shape &shape) const {
    std::uniform_real_distribution<double> zero2pi(0, 2*M_PI);
    std::uniform_real_distribution<double> plusMinusOne(-1, 1);

    // For Tait-Bryan angles, this gives a uniform probability of rotations
    double angleX = zero2pi(this->mt);
    double angleY = std::asin(plusMinusOne(this->mt));
    double angleZ = zero2pi(this->mt);

    shape.rotate(Matrix<3, 3>::rotation(angleX, angleY, angleZ));
}
