//
// Created by pkua on 20.05.22.
//

#include "FlipRandomizingTransformer.h"

void FlipRandomizingTransformer::transform(Lattice &lattice) const {
    const auto &dim = lattice.getDimensions();
    for (std::size_t i{}; i < dim[0]; i++) {
        for (std::size_t j{}; j < dim[1]; j++) {
            for (std::size_t k{}; k < dim[2]; k++) {
                for (auto &shape : lattice.modifyCell(i, j, k)) {
                    std::uniform_int_distribution flipOrNot(0, 1);
                    if (flipOrNot(this->rng) == 1) {
                        Vector<3> axis = shape.getOrientation() * this->secondaryAxis;
                        shape.rotate(Matrix<3, 3>::rotation(axis, M_PI));
                    }
                }
            }
        }
    }
}

FlipRandomizingTransformer::FlipRandomizingTransformer(const Vector<3> &secondaryAxis, unsigned long seed)
        : secondaryAxis{secondaryAxis.normalized()}, rng(seed)
{ }
