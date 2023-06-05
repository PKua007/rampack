//
// Created by pkua on 20.05.22.
//

#include "FlipRandomizingTransformer.h"
#include "core/FreeBoundaryConditions.h"

void FlipRandomizingTransformer::transform(Lattice &lattice, const ShapeTraits &shapeTraits) const {
    const auto &geometry = shapeTraits.getGeometry();

    bool wasNormalized = lattice.isNormalized();

    Vector<3> flipAxis = geometry.findFlipAxis({});

    const auto &dim = lattice.getDimensions();
    FreeBoundaryConditions fbc;
    for (std::size_t i{}; i < dim[0]; i++) {
        for (std::size_t j{}; j < dim[1]; j++) {
            for (std::size_t k{}; k < dim[2]; k++) {
                for (auto &shape : lattice.modifySpecificCellMolecules(i, j, k)) {
                    std::uniform_int_distribution flipOrNot(0, 1);
                    if (flipOrNot(this->rng) == 1) {
                        Vector<3> axis = shape.getOrientation() * flipAxis;
                        Matrix<3, 3> rotation = Matrix<3, 3>::rotation(axis.normalized(), M_PI);
                        Vector<3> geometricOrigin = geometry.getGeometricOrigin(shape);
                        Vector<3> translation = -rotation * geometricOrigin + geometricOrigin;
                        shape.rotate(rotation);
                        shape.translate(lattice.getCellBox().absoluteToRelative(translation), fbc);
                    }
                }
            }
        }
    }

    if (wasNormalized)
        lattice.normalize();
}
