//
// Created by Piotr Kubala on 20/06/2023.
//

#include "RandomAxisRotationTransformer.h"


RandomAxisRotationTransformer::RandomAxisRotationTransformer(const Vector<3> &axis, unsigned long seed)
        : mt(seed)
{
    // Absolute values of components at least approx 1e-10
    Expects(axis.norm2() > 1e-20);
    this->axis = axis.normalized();
}

void RandomAxisRotationTransformer::transform(Lattice &lattice, [[maybe_unused]] const ShapeTraits &shapeTraits) const {
    auto dim = lattice.getDimensions();
    for (std::size_t i{}; i < dim[0]; i++)
        for (std::size_t j{}; j < dim[1]; j++)
            for (std::size_t k{}; k < dim[2]; k++)
                for (auto &shape : lattice.modifySpecificCellMolecules(i, j, k))
                    this->rotateRandomly(shape);
}

void RandomAxisRotationTransformer::rotateRandomly(Shape &shape) const {
    std::uniform_real_distribution<double> zero2pi(0, 2*M_PI);
    shape.rotate(Matrix<3, 3>::rotation(this->axis, zero2pi(this->mt)));
}
